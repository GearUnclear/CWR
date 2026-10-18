#pragma once

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

// Long tree shadows at any distance (INTEGRATION-PLAN.md step B9, research
// track hfrt): the canopy heightfield, its sun-visibility map and the rules
// the GL backend and the shaders share. Pure, no GL, no engine state.
//
// What the player sees: at a low sun every tree, bush, forest block and
// building on the island casts its long shadow, also beyond the reach of the
// sun shadow maps (a few hundred metres), so whole hillsides of trees stand on
// their shadows at golden hour instead of floating.
//
// How:
//   - The canopy (CanopyField): every landscape object that can shade the
//     ground at a distance is drawn top-down into a texture at about 6.25 m a
//     texel (k texels per terrain cell): r = the absolute height of the top of
//     the tallest object over the texel (its height above the ground at its
//     centre, set on the texel's ground; the ground where nothing covers it),
//     so one fetch per step holds the terrain too; g = its occluder density,
//     1 - the product of each object's (1 - opacity * coverage). Crowns are
//     ellipses at kCanopyCrownShrink of their box, forest blocks and buildings
//     the convex hull of their geometry level. A mip texel keeps the mean
//     ground plus the tallest covered child's height above its own ground, and
//     the mean density, so a far occluder blurs into a wider, lighter penumbra
//     and a slope stays a slope at every level. Built on the
//     terrain horizon map's worker thread at island load (CanopyRaster); a
//     destroyed tree or building takes its footprint out and re-rasterises
//     only the texels it covered (CanopyRaster::Remove).
//   - The sun-visibility map: a world-space map around the camera (2048
//     texels a side, 1.5 to 3.1 m a texel by the view distance) that a GPU
//     pass fills by cone-tracing the canopy toward the sun from every texel
//     (CanopySunAt is its C++ mirror). It is rebuilt only when the sun has
//     moved kCanopySunRebuildDeg, the camera has left the middle of the map,
//     or the canopy changed, so it is world-locked: no dither, nothing to
//     accumulate, no crawl in motion. Each texel stores the visibility of the
//     ground, the visibility of a receiver kCanopyRaisedLift above it that
//     skips the first kCanopyRaisedSkip metres (its own crown), and the height
//     above the ground below which a raised receiver (a crown, a wall, a roof)
//     is in a neighbour's shadow, with that edge's softness.
//   - The world shaders read the map once per fragment beyond the shadow maps'
//     reach (cross-faded over their far fade) and multiply the sun visibility
//     by it (CanopyReceiverVisibility is the mirror of that read).
//
// The plan asked for a half-resolution screen buffer built from last frame's
// depth and accumulated over frames. A world-space map gives the same trace at
// a fixed cost that does not grow with the screen, is paid only when the sun or
// the camera moved enough, needs no reprojection or history, and cannot crawl.
//
// Feature switch canopyShadows; gate: the sun between 2 and 25 deg and the sky
// more than half clear (FeatureGates.cpp); the shadows fade in and out over the
// edges of that band (CanopyStrength) so a sunset time-lapse sees no switch.

namespace Poseidon::remaster
{

// ---- The canopy ---------------------------------------------------------------

// Target texel of the canopy (metres): k texels per terrain cell, k the nearest
// whole number to cell / this, at most kCanopyMaxSize texels a side.
constexpr float kCanopyTargetTexel = 6.25f;
constexpr int kCanopyMaxSize = 4096;
// Crowns are rounder and a little smaller than their boxes.
constexpr float kCanopyCrownShrink = 0.85f;
// Objects whose top is less than this above the ground at their centre do not
// shade anything; tops more than kCanopyMaxRelTop above it are clamped there.
constexpr float kCanopyMinRelTop = 0.5f;
constexpr float kCanopyMaxRelTop = 100.0f;
// Supersamples per texel side when a footprint is rasterised.
constexpr int kCanopySamples = 4;
// A mip texel keeps the tallest top among its children denser than this.
constexpr float kCanopyMipCovered = 0.004f;

// What a landscape object is to the canopy.
enum class CanopyKind : std::uint8_t
{
    None,      // does not shade the ground at a distance (fences, walls, crosses, ...)
    Tree,      // MapTree
    SmallTree, // MapSmallTree
    Bush,      // MapBush
    Forest,    // the three forest block types
    Solid,     // buildings, houses, churches, towers, bunkers, rocks, ...
};

// The kind of a landscape shape's map type (World/MapTypes.hpp MapType).
CanopyKind CanopyKindOfMapType(int mapType);
// Opacity of a texel the object covers whole: tree 0.8, small tree 0.65, bush
// 0.5, forest 0.95, solid 1; 0 for None.
float CanopyOpacity(CanopyKind kind);
// Crowns (tree, small tree, bush) are ellipses; the others boxes or hulls.
bool CanopyRound(CanopyKind kind);

// One footprint. World metres, x east, z north, heights above sea level.
struct CanopyItem
{
    int id = -1;                // the object's id (Object::ID), for Remove
    float cx = 0.0f, cz = 0.0f; // centre
    float ux = 1.0f, uz = 0.0f; // unit axis 1
    float vx = 0.0f, vz = 1.0f; // unit axis 2
    float a = 0.0f, b = 0.0f;   // half extents along the axes
    // Height of the object's top above the ground at its centre (m). The
    // canopy keeps it above the ground of every texel the object covers, so a
    // forest block on a slope follows the slope.
    float height = 0.0f;
    float opacity = 0.0f;
    bool round = false;      // the ellipse inscribed in the box (crowns)
    std::vector<float> hull; // convex hull (x, z pairs, counter-clockwise); empty = the box
    // Bounding box of the footprint (CanopyItemBounds fills it).
    float minX = 0.0f, maxX = 0.0f, minZ = 0.0f, maxZ = 0.0f;
};

// Fills an item's bounding box from its hull, or from its axes and extents.
void CanopyItemBounds(CanopyItem& item);
bool CanopyInside(const CanopyItem& item, float x, float z);
// Andrew's monotone chain over (x, z) pairs; counter-clockwise, no repeated
// points; empty for fewer than three distinct points or a degenerate set.
std::vector<float> CanopyConvexHull(std::vector<float> xz);

// The canopy texture as it is uploaded: every mip level of RG16 unorm pairs,
// r = (top - heightBase) / heightRange, g = density.
struct CanopyField
{
    int size = 0;       // level-0 texels a side; texel j covers [j, j + 1) * texel metres
    float texel = 0.0f; // metres per level-0 texel
    float heightBase = 0.0f;
    float heightRange = 1.0f;
    std::vector<std::vector<std::uint16_t>> levels; // level l: LevelSize(l)^2 RG pairs
    // Bumped by every change (a build, a removal), so the sun-visibility map
    // knows to rebuild.
    std::uint32_t version = 0;

    bool Valid() const { return size > 0 && texel > 0.0f && !levels.empty(); }
    int Levels() const { return static_cast<int>(levels.size()); }
    int LevelSize(int level) const;
    float Top(int level, int x, int z) const;     // decoded, metres
    float Density(int level, int x, int z) const; // decoded, 0..1
};

// A changed texel rectangle of one level, [x0, x1) x [z0, z1).
struct CanopyRect
{
    int level = 0;
    int x0 = 0, z0 = 0, x1 = 0, z1 = 0;
};

// Texels per terrain cell for a cell size and terrain range.
int CanopyTexelsPerCell(float cellSize, int range);

// Builds and keeps the canopy of one landscape. Build runs on a worker; after
// it the owner (the main thread) may Remove objects.
class CanopyRaster
{
  public:
    // `heights`: the terrain grid, range * range, row-major heights[z * range + x],
    // vertex (x, z) at (x * grid, z * grid). k texels per cell. False (and no
    // field) for an empty grid or a cancelled build.
    bool Build(std::vector<CanopyItem> items, std::vector<float> heights, int range, float grid, int k,
               const std::atomic<bool>* cancel = nullptr);
    // The field (shared with the backend, which uploads it); null before Build.
    std::shared_ptr<const CanopyField> Field() const { return _field; }
    // Takes object `id`'s footprint out and re-rasterises the texels it
    // covered, at every level; returns those rectangles (empty when the object
    // is not in the canopy or was removed already).
    std::vector<CanopyRect> Remove(int id);
    int ItemCount() const { return static_cast<int>(_items.size()); }

  private:
    static constexpr int kBinTexels = 32;
    void RasterLevel0(int x0, int z0, int x1, int z1);
    void Downsample(int level, int x0, int z0, int x1, int z1); // level - 1 into level
    float Ground(float x, float z) const;
    std::uint16_t Encode(float height) const;
    float Decode(std::uint16_t value) const;

    std::shared_ptr<CanopyField> _field;
    // The mean ground of every texel of every level, encoded as r is.
    std::vector<std::vector<std::uint16_t>> _ground;
    std::vector<CanopyItem> _items;
    std::vector<bool> _removed;
    std::unordered_map<int, int> _byId;
    std::vector<std::vector<int>> _bins; // item indices per kBinTexels square, ascending
    int _bins1 = 0;                      // bins a side
    std::vector<float> _heights;
    int _range = 0;
    float _grid = 0.0f;
};

// ---- Sampling and the trace (C++ mirrors of the GPU pass) ----------------------

// Trilinear sample of the field (GL_LINEAR_MIPMAP_LINEAR, clamp to edge) at world
// (x, z) and level of detail `lod`: the decoded top (m) and density.
void CanopySample(const CanopyField& field, float x, float z, float lod, float& top, float& density);

// The trace's shape (shared with psCanopySun).
constexpr float kCanopyReach = 45.0f;       // nothing in the canopy stands taller above its ground
constexpr float kCanopyMaxTrace = 900.0f;   // metres toward the sun at most
constexpr float kCanopyGrowth = 0.15f;      // step = max(texel, growth * distance)
constexpr int kCanopyMaxSteps = 40;         // the loop bound of the shader (a 2 deg sun needs 29)
constexpr float kCanopySoftDeg = 0.8f;      // the occluder edge's angular softness
constexpr float kCanopyMinDensity = 0.003f; // a sample thinner than this is skipped
constexpr float kCanopyMinTransmit = 0.02f; // one sample takes out at most this much
constexpr float kCanopyGroundLift = 0.3f;   // a ground receiver sits this high
constexpr float kCanopyRaisedLift = 1.8f;   // the lowest raised receiver (1.5 m plus the lift)
constexpr float kCanopyRaisedSkip = 6.0f;   // a raised receiver never shades itself within this
constexpr float kCanopyRaisedFrom = 1.5f;   // receivers this high above the ground are raised
constexpr float kCanopyGroundTo = 0.5f;     // receivers this low are ground (blend between)
// A sample sets the raised receivers' edge in full from this crossing on, and
// not at all below kCanopyEdgeCrossFrom (its edge sinks below the ground).
constexpr float kCanopyEdgeCrossFrom = 0.3f;
constexpr float kCanopyEdgeCrossTo = 0.6f;
constexpr float kCanopyEdgeRange = 48.0f;   // stored edge height range (m)
constexpr float kCanopySoftRange = 16.0f;   // stored edge softness range (m)
constexpr float kCanopyMinSoft = 0.5f;      // softness of "no edge"
constexpr float kCanopyMinReadSoft = 0.25f; // the read never takes a narrower edge (8-bit storage)

// The sun as the trace takes it: horizontal unit direction toward the sun and
// the tangents of its elevation and of the edge softness.
struct CanopySunRay
{
    float dirX = 0.0f, dirZ = 1.0f;
    float tanSun = 0.0f;
    float tanSoft = 0.0f;
    float length = 0.0f; // trace length (m)
};
// From the direction toward the sun (need not be normalised, y up).
CanopySunRay CanopySunRayToward(float x, float y, float z);

// One texel of the sun-visibility map: ground visibility, the lowest raised
// receiver's visibility, and the raised receivers' shadow edge (height above
// the ground, softness), all as stored before the 8-bit quantisation.
struct CanopySunTexel
{
    float ground = 1.0f;
    float raisedLow = 1.0f;
    float edge = 0.0f;
    float soft = kCanopyMinSoft;
};
// The trace from the ground at (x, z), height groundY: psCanopySun's loop.
CanopySunTexel CanopySunAt(const CanopyField& field, float x, float z, float groundY, const CanopySunRay& ray);
// What a receiver `heightAboveGround` above the ground reads from a texel (the
// world shaders' canopySunVisibility).
float CanopyReceiverVisibility(const CanopySunTexel& texel, float heightAboveGround);

// ---- The sun-visibility map: placement, rebuilds, strength, constants ---------

constexpr int kCanopySunSize = 2048;                  // texels a side
constexpr float kCanopySunMinTexel = 1.5625f;         // metres
constexpr float kCanopySunMaxTexel = 3.125f;          // metres
constexpr float kCanopySunTexelPerVd = 1.0f / 900.0f; // texel = view distance / 900, clamped
constexpr float kCanopySunRebuildDeg = 0.05f;         // the sun moved this much: rebuild
constexpr float kCanopySunRecentre = 0.125f;          // the camera left the middle by this share of the map: rebuild
// The map fades out toward its edges (uv distance from the centre).
constexpr float kCanopySunEdgeFrom = 0.42f;
constexpr float kCanopySunEdgeTo = 0.48f;
// Without sun shadow maps the canopy starts this far out (fading in over the
// fade before it), so the near range keeps no tree shadows, as before.
constexpr float kCanopyNoMapReach = 150.0f;
constexpr float kCanopyNoMapFade = 50.0f;
// The strength fades over the gate's edges (degrees of sun, sky-through).
constexpr float kCanopyFadeInToSunDeg = 4.0f;
constexpr float kCanopyFadeOutFromSunDeg = 21.0f;
constexpr float kCanopyFadeInToSkyThrough = 0.6f;

// The map's texel for a view distance (metres), a multiple of 1/32 m.
float CanopySunTexelFor(float viewDistance);
// The map's origin (its corner, texel-aligned) for a camera at (camX, camZ).
void CanopySunOrigin(float camX, float camZ, float texel, float& originX, float& originZ);

// What the map was built for, and what the frame wants.
struct CanopySunKey
{
    bool valid = false;
    float sun[3] = {0.0f, 1.0f, 0.0f}; // toward the sun, normalised
    float originX = 0.0f, originZ = 0.0f;
    float texel = 0.0f;
    std::uint32_t canopyVersion = 0;
};
// True when `want` (for a camera at camX, camZ) needs a new map: none yet, a new
// canopy or texel, the sun moved kCanopySunRebuildDeg, or the camera is more than
// kCanopySunRecentre of the map from its middle.
bool CanopySunNeedsRebuild(const CanopySunKey& built, const CanopySunKey& want, float camX, float camZ);

// The shadows' strength at a sun height and sky-through: 0 outside the gate,
// fading in from 2 to 4 deg and out from 21 to 25 deg of sun, and in from a
// sky-through of 0.5 to 0.6.
float CanopyStrength(float sunElevationDeg, float skyThrough);

// LitConstants slot Canopy (76): x strength (0 = off: the switch, its gate, no
// canopy or no map), yz the map's origin (m), w 1 / its extent (1/m). With the
// gates off (gates=0, debugging) an enabled switch runs at full strength while
// the sun is up.
struct CanopyFrameConstants
{
    float canopy[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};
CanopyFrameConstants ComputeCanopyFrameConstants(const LightingTuning& tuning, const FeatureGates& gates, bool mapReady,
                                                 float sunElevationDeg, float skyThrough, float originX, float originZ,
                                                 float texel);

} // namespace Poseidon::remaster
