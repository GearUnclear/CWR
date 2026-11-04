#pragma once

#include <Poseidon/Graphics/Rendering/RenderPassDescriptor.hpp>

#include <cstdint>

// Screen-space reflections (feature switch `ssr`): the parts that are plain
// math, shared by the backend
// (EngineGL33_State.cpp, EngineGL33_Queue.cpp, EngineGL33_PostSsr.cpp), the
// shaders (EngineGL33_PostSsrShaders.cpp) and the tests. No GL, no engine state.
//
// What the player sees: calm water mirrors the boats, rocks and hills on screen,
// broken up by ripples, and in the rain streets and bare ground turn dark and
// wet, with puddles that mirror the houses, lamp posts and the sky. Grass stays
// dry. It can only reflect what is on screen.
//
// How it works:
//   1. Surface tags in the stencil. Every world draw already writes the stencil
//      (REPLACE 0) for Classic's projected-shadow exclusion, which counts in the
//      low bits (EQUAL 0, then INCR). While the switch is on, the world draws
//      write a tag in the top bits instead of 0 (SsrStencilForDraw): what
//      surface a pixel shows, and whether a blended draw that writes depth (a
//      bush card, glass) covers it. The shadow test then compares the low bits
//      only. A later draw over a pixel replaces the tag, so the tag always
//      belongs to the surface the pixel shows.
//   2. The post pass reads the tags out of the resolved stencil (the TagReadout
//      stage), measures the share of the screen that is water or ground the
//      rain wets (read back a few frames later for the frame gate), and traces
//      the reflections at half resolution over the half-resolution linear
//      depth (the Ssr stage), softens the outlines of rough reflections with
//      a horizontal gather as wide as each ray's cone, then lays them on the
//      full-resolution frame with a depth- and surface-aware upsample. Under
//      TAA the march start turns every frame and the history averages the
//      result.
//   3. Water: the sea's own program (seaWaves) already reflects the sky with
//      its Fresnel weight. Where the traced ray hits something on screen, the
//      reflection replaces that sky share (SsrWaterBlend): the hill and the
//      boat take the sky's place in the sea, at the sea's own weight. Rays that
//      end on the sky or leave the frame leave the sea as it is, so the sky in
//      the sea stays the sea program's (no pale sunset wash).
//   4. Wet ground (the rain, or `ssrWet`): roads and bare ground darken and
//      take a rough, streaked reflection; level hollows hold puddles, which are
//      dark mirrors with rain rings; grass only darkens a little. With the wet
//      weather (Wetness.hpp) on, only its puddles reflect: the ground around them
//      darkens in the forward pass and reflects nothing here.

namespace Poseidon::remaster
{

// ---- Stencil tags ------------------------------------------------------------
//
// Bits 7 and 6: the surface. Bit 5: a blended draw that writes depth covers the
// pixel (its depth is the card's, not the surface's: no reflection there, and a
// ray that meets it goes on, the panel laid over what it finds as much as it
// hides: SsrCoverOpacity). Bits 4 to 0: Classic's projected-shadow counter
// (EQUAL 0, INCR), untouched by the tags.
namespace ssrtag
{
constexpr std::uint8_t kShadowBits = 0x1F;
constexpr std::uint8_t kCover = 0x20;
constexpr std::uint8_t kSurfaceBits = 0xC0;
constexpr std::uint8_t kNone = 0x00;
constexpr std::uint8_t kTerrain = 0x40; // the land (detail and grass layers)
constexpr std::uint8_t kWater = 0x80;   // the sea's water program
constexpr std::uint8_t kRoad = 0xC0;    // roads, tracks and other on-surface decals
} // namespace ssrtag

enum class SsrSurface : std::uint8_t
{
    None,
    Terrain,
    Water,
    Road,
};

// The surface and the cover flag of a stencil value.
SsrSurface SsrSurfaceOfStencil(std::uint8_t stencil);
bool SsrCoveredStencil(std::uint8_t stencil);

// The stencil state of one draw: the reference REPLACE writes, the write mask,
// and the compare mask of the projected-shadow test (DepthMode::Shadow).
struct SsrStencil
{
    std::uint8_t ref = 0;
    std::uint8_t writeMask = 0xFF;
    std::uint8_t shadowCompareMask = 0xFF;
    bool operator==(const SsrStencil& o) const
    {
        return ref == o.ref && writeMask == o.writeMask && shadowCompareMask == o.shadowCompareMask;
    }
    bool operator!=(const SsrStencil& o) const { return !(*this == o); }
};

// The stencil state of a draw. `landLayer`: a detail-family draw carries the
// land's detail texture (the sea's faces, when the water program does not draw
// them, carry the specular layer alone). `tagging`: the world pass writes tags
// (the switch on, the remastered world pass); without it every draw gets the
// classic state (REPLACE 0, full masks), so the frame is unchanged bit for bit.
//
// While tagging:
//   - the projected shadows compare the shadow bits only;
//   - a surface (the water program, the land's detail and grass layers, an
//     on-surface draw of the normal family) writes its tag, every bit;
//   - any other draw that writes depth: opaque, clears the tag (the pixel shows
//     it now); blended (a bush card, glass), sets the cover bit and keeps the
//     surface underneath;
//   - a draw that writes no depth (a particle, a flare, the sky) resets only
//     the shadow bits, as in the classic state, and keeps the tag.
SsrStencil SsrStencilForDraw(const render::RenderPassDescriptor& d, bool landLayer, bool tagging);

// GL's stencil update for a fragment that passes (REPLACE under the write
// mask), and the projected-shadow draw (EQUAL 0 under the compare mask, then
// INCR): what the stored value becomes. For the tests.
std::uint8_t SsrReplace(std::uint8_t stored, const SsrStencil& s);
std::uint8_t SsrShadowDraw(std::uint8_t stored, const SsrStencil& s, bool& passed);

// ---- The frame gate's screen shares ------------------------------------------

// The share pass: the tag texture sampled on a grid of kSsrShareW x kSsrShareH
// cells of kSsrShareTaps x kSsrShareTaps taps each, read back a few frames
// later (no stall) for the frame gate.
constexpr int kSsrShareW = 32;
constexpr int kSsrShareH = 18;
constexpr int kSsrShareTaps = 6;
// While the gate is off the readout runs only every kSsrShareProbeFrames-th
// frame, enough to keep the shares fresh; a share older than kSsrShareMaxAge
// frames is "not measured" (the tags stopped).
constexpr int kSsrShareProbeFrames = 8;
constexpr int kSsrShareMaxAge = 30;
static_assert(kSsrShareProbeFrames * 2 < kSsrShareMaxAge, "a probe lands well before the last share goes stale");

// ---- The march --------------------------------------------------------------
//
// One ray per half-resolution texel over the half-resolution linear depth,
// linear in screen space with a perspective-correct depth, an overlap test
// against a surface of kSsrThickBase + kSsrThickPerM * depth metres, and a
// binary refine. The shader (psPostSsrTrace) and SsrMarchRow share these.
constexpr int kSsrSteps = 48;          // steps at most
constexpr float kSsrStride = 1.5f;     // half-resolution texels per step at least
constexpr float kSsrStartSkip = 1.5f;  // half-resolution texels skipped at the start
constexpr int kSsrRefine = 5;          // binary refine steps on a hit
constexpr float kSsrThickBase = 0.4f;  // metres
constexpr float kSsrThickPerM = 0.04f; // metres per metre of depth
constexpr float kSsrBehindNear = 3.0f; // a ray behind a surface within this many metres sees its hidden side
constexpr float kSsrUnderside = 0.35f; // that hidden side, unlit, against the surface's lit front
constexpr float kSsrSkyDepth = 1.0e5f; // linear depth at or above this is sky (the linear depth stores 1e6)
constexpr float kSsrEdgeFade = 0.06f;  // reflections fade out over this share of the frame at its edges

enum class SsrHit : std::uint8_t
{
    LeftFrame, // the ray left the frame (or ran out) without meeting anything
    Surface,   // it met a surface on screen
    Sky,       // it ended on open sky inside the frame
    BehindNear // it slipped behind a surface right after its start (under a vehicle); t is where
};

struct SsrRowHit
{
    SsrHit kind = SsrHit::LeftFrame;
    float t = 0.0f;      // where along the ray (0 the start, 1 its end)
    float travel = 0.0f; // metres from the start to the hit (BehindNear: to where it went behind)
    int hidden = 0;      // steps the ray spent behind a surface (closer to the eye than the ray)
    // The first covered texel (a blended panel) the ray met, which did not stop
    // it: where (t, refined) and how far, and the first uncovered texel the
    // march reached after it (afterT, -1 when the march ended on the panel);
    // afterInFront: that texel stands in front of the ray (closer than the ray
    // by more than the thickness), so the panel is set in something solid.
    bool cover = false;
    float coverT = 0.0f;
    float coverTravel = 0.0f;
    float afterT = -1.0f;
    bool afterInFront = false;
};

// The ray's own surface. On the ground a ray starts on a plane and leaves it
// slowly: a grazing ray along a wet road is a few centimetres above the road
// for its first pixels. The half-resolution depth holds the nearest or the
// farthest sample of each 2 x 2 block, and a nearest sample can put the road in
// front of the ray there: the ray met its own road (dark dots on the wet road,
// in the interleaved pattern of the march start). A candidate whose surface
// lies within SsrSelfTolerance(depth) metres of the start's plane is that
// surface, not something the ray meets (the trace's plane test; water needs
// none, a ray never stops on water). The depth error of one 2 x 2 block is
// about a pixel's angle times the depth from that plane; the tolerance holds a
// few of them.
constexpr float kSsrSelfBase = 0.03f;  // metres
constexpr float kSsrSelfPerM = 0.004f; // metres per metre of depth
float SsrSelfTolerance(float depth);

// Covered panels. A blended draw that writes depth (a vehicle's window or
// canvas, glass, a sign, a bush card) writes its depth over the whole card,
// transparent parts too, so neither stopping on it (a box under a bush: the
// ground seen through the card) nor passing it (a pale box in a vehicle's
// reflection: the sky through its rear window) is right everywhere. The march
// passes it and keeps the first one it met; how much of what lies beyond it
// hides is its opacity. A panel set in something solid (the first uncovered
// texel the march reached after it stands in front of the ray: the canvas
// around a vehicle's window, the frame of a pane) is opaque: the ray would meet
// the inside of that object, which the screen does not hold. So is a panel the
// march never left. Otherwise SsrCoverOpacity, from how far the panel's colour
// is from that texel, the background next to the panel, which a transparent
// part of a card shows. The panel then lies over what the ray found beyond:
// R = w Rpanel + (1 - w) R, w = the opacity times its edge fade.
constexpr float kSsrCoverSeeFrom = 0.04f; // colour distance (display, largest channel): see-through ...
constexpr float kSsrCoverSeeTo = 0.12f;   // ... and opaque
constexpr float kSsrCoverLod = 1.0f;      // the mip level the two colours are compared at
float SsrCoverOpacity(const float panel[3], const float behind[3]);

// Sky that a ray on the ground found after a stretch behind a surface may be
// what that surface hid (a soldier in front of a tent): its weight falls from 1
// at kSsrHiddenFrom hidden steps to 0 at kSsrHiddenTo. Thin poles and wires
// (a step or so) keep it.
constexpr float kSsrHiddenFrom = 1.0f;
constexpr float kSsrHiddenTo = 4.0f;
float SsrHiddenTrust(int hiddenSteps);
// The hidden side a ray slipped behind (BehindNear) fades out between half of
// kSsrBehindNear and kSsrBehindNear metres, where the information ends.
float SsrUndersideFade(float travel);

// CPU mirror of the shader's march along one screen row: `sceneZ` holds the
// row's linear depth (metres, >= kSsrSkyDepth for the sky), `stencil` its tags
// (nullptr: untagged), `n` texels. The ray runs from texel 0 at view depth z0
// to texel n - 1 at view depth z1, rz is its view-space z per metre travelled
// (the ray's direction's z component). A candidate on water (water never
// reflects itself) does not count, nor one whose surface lies on the start's
// plane: `planeDist` holds each texel's distance (metres) from that plane
// (nullptr: no plane test, as on water). A candidate on a covered pixel (a
// blended panel) does not stop the ray; the first one is kept in `cover`.
SsrRowHit SsrMarchRow(const float* sceneZ, const std::uint8_t* stencil, int n, float z0, float z1, float rz,
                      float jitter, const float* planeDist = nullptr);

// ---- The streak ---------------------------------------------------------------
//
// A reflection in wet ground or wavy water stretches toward the viewer: taps
// along the screen vertical over pxV pixels each way (kSsrStreakMaxPx at most),
// read at the mip level of the horizontal spread pxH. The taps stand no farther
// apart than that level's footprint: more taps where the footprint is small (on
// water at a grazing angle the horizontal spread is a fraction of the vertical
// one), up to kSsrStreakTaps a side, then a coarser level; taps spaced wider
// than the footprint show as a ladder of copies on water. On the ground
// (pxV = 2 px, pxH = 0.7 px) the footprint already covers the spacing: there
// it stays 3 taps a side at the same level until the streak reaches
// kSsrStreakMaxPx.
constexpr float kSsrStreakMaxPx = 72.0f;
constexpr int kSsrStreakTaps = 12;
struct SsrStreak
{
    int taps = 1;        // on each side
    float stepPx = 0.0f; // between taps (pixels)
    float lod = 0.0f;    // the mip level read
};
SsrStreak SsrStreakTaps(float pxV, float pxH, float maxLod);

// The fade of a reflection at uv (0..1 over the 3D viewport) toward the frame's
// edges: 0 at the edge, 1 kSsrEdgeFade inside it.
float SsrEdgeFade(float u, float v);

// ---- The rough blur -----------------------------------------------------------
//
// One ray per texel decides hit or miss sharply, so a rough reflection (wet
// asphalt, bare ground) would draw the hard outline of what it mirrors: a box
// under a vehicle, a seam at a tent's edge. Its cone already blurs the colour
// it takes, not where the rays stop hitting. So the trace writes each texel's
// spread, the width of its cone at what it reflects in half-resolution texels
// (SsrBlurSpread of the horizontal spread in full-resolution pixels), and a
// horizontal gather (psPostSsrBlur, SsrBlurRow) over that spread, on the same
// surface at a similar depth, softens those outlines. It takes a tap per texel
// of the spread (SsrBlurTaps, kSsrBlurMaxTaps a side at most); fewer taps
// would step over texels and copy the trace's texel-scale pattern at their
// stride. A puddle's or calm water's mirror (a
// spread under kSsrBlurMinSpread) stays sharp. The upward stretch of a
// reflection already blurs it vertically.
constexpr int kSsrBlurMaxTaps = 24;        // taps on each side at most
constexpr float kSsrBlurMaxSpread = 24.0f; // half-resolution texels (48 pixels)
constexpr float kSsrBlurMinSpread = 0.75f; // below this a texel keeps its own value
static_assert(kSsrBlurMaxTaps >= static_cast<int>(kSsrBlurMaxSpread), "a tap per texel of the widest spread");
float SsrBlurSpread(float pixelsH);
// The taps on each side of a gather over `spread` texels: one per texel.
int SsrBlurTaps(float spread);
// The weight of tap k (1..taps) of a gather of `taps` a side, of a tap at depth
// tz for a texel at depth z (metres; the composite's upsample uses the same),
// and of a tap whose own spread is narrower than the texel's (squared ratio: a
// puddle's sharp mirror does not spill into the rough road around it).
float SsrBlurTapWeight(int k, int taps);
float SsrBlurDepthWeight(float tz, float z);
float SsrBlurSpreadWeight(float tapSpread, float spread);
// CPU mirror of the gather along one half-resolution row: `value` one channel
// of the trace, `surface` the surface it is for (0 none), `z` the linear depth,
// `spread` the trace's spread, `n` texels, `out` the result.
void SsrBlurRow(const float* value, const std::uint8_t* surface, const float* z, const float* spread, int n,
                float* out);

// ---- Water -------------------------------------------------------------------
//
// The sea program's colour is lin = body (1 - w) + S w (+ the sun), with S its
// sky in the reflected direction and w its Fresnel sky weight, which follows
// its own wave normal facet by facet. The post pass cannot rebuild that normal,
// so it estimates the share of the sea's light the reflected sky gives near the
// pixel, `skyShare` (the mean weight times the sky over the blurred sea), and
// where a ray meets a surface R it swaps that share of the pixel's light for the
// reflection: lin' = lin ((1 - x) + x R / S) per channel, x = skyShare *
// ssrWater (capped at kSsrWaterSwapMax), R / S capped at kSsrWaterRatioMax.
// For a pixel whose light is the sky's share alone this is exactly S w -> R w;
// on a facet that shows more sky the reflection shows more, as the waves
// break it up; it never goes negative or shifts a channel the sky lacks.
// `fade` (the edge fade, the fog, shallow water) blends from lin (0) to lin'
// (1): lin' = lin (keep + swap R / S).
struct SsrWaterBlend
{
    float keep = 1.0f; // the frame's colour times this
    float swap = 0.0f; // plus the frame's colour times R / S times this
};
constexpr float kSsrWaterSwapMax = 0.95f;
constexpr float kSsrWaterRatioMax = 4.0f;
// The ripples that move the mirror image on water (four octaves of drifting
// noise, 11 m to 0.65 m) at this share of their full slopes (a calm sea). The
// sea program's own waves already break the reflection up; full slopes smear a
// hill's reflection into a brown band. Finer octaves than a pixel blur instead.
constexpr float kSsrWaterRipple = 0.35f;
SsrWaterBlend SsrWaterComposite(float skyShare, float ssrWater, float fade);
// R / S of one channel, capped (a sky that lacks a channel at dusk would blow
// the ratio up).
float SsrWaterRatio(float r, float s);

// ---- Wet ground --------------------------------------------------------------
//
// Wet roads and bare ground darken (water fills the pores) and take a rough
// reflection at Fresnel weight; a puddle is a darker, smooth mirror. Grass
// darkens less and reflects little.
constexpr float kSsrWetRoadDark = 0.80f;   // a wet road's colour times this
constexpr float kSsrWetGroundDark = 0.80f; // bare ground
constexpr float kSsrWetGrassDark = 0.92f;  // grass
constexpr float kSsrPuddleDark = 0.55f;    // a puddle's colour under its reflection
constexpr float kSsrPuddleRoad = 0.61f;    // puddle noise threshold on roads
constexpr float kSsrPuddleGround = 0.67f;  // on bare ground (none on grass)
// Puddles fade out with distance, where they would shrink below a pixel: the far
// ground is one wet sheen (blotches on far hillsides otherwise).
constexpr float kSsrPuddleFadeStart = 150.0f;
constexpr float kSsrPuddleFadeEnd = 400.0f;

// LightingTuning::debugView values of the reflections' diagnostic views (the
// Ssr stage draws them): the tags over the frame, and the reflection alone.
// They follow the albedo (7) and the air (8) views.
constexpr int kDebugViewSsrTags = 9;
constexpr int kDebugViewSsrReflection = 10;

} // namespace Poseidon::remaster
