#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

// Terrain horizon map for the remastered lighting: pure CPU math, no GL, no
// engine state.
//
// For every landscape grid vertex the builder marches outward in kDirections
// azimuths and records the elevation angle of the highest terrain seen. The lit
// shaders compare that horizon against the sun elevation, so hills cast soft
// sun shadows across valleys at any distance (the cascaded shadow maps only
// cover objects near the camera, and the terrain never casts into them). The
// same angles give a sky-visibility term that darkens the ambient light at the
// bottom of valleys and gullies.
//
// Conventions (match the engine world frame and the GLSL that samples this):
//   - Heights are row-major, heights[z * size + x], vertex (x, z) at world
//     position (x * cellSize, z * cellSize). Y is up.
//   - Direction i points along azimuth i * 360 / kDirections degrees, measured
//     from +Z toward +X: dir = (sin a, cos a) in (x, z).
//   - Horizon angles are smoothed with a 3x3 binomial filter and encoded to
//     16-bit unorm over [kMinAngleDeg, kMaxAngleDeg]. Near sunset the sun
//     visibility swings ~7% per 0.2 degree of horizon, so 8-bit storage drew
//     the texel grid as contour lines on the ground; 16 bits are smooth.
//   - Layout of `horizon`: kDirections / 4 layers, each size*size RGBA texels;
//     direction i lives in layer i / 4, component i % 4. It uploads 1:1 to a
//     GL_TEXTURE_2D_ARRAY of GL_RGBA16.

namespace Poseidon::remaster
{

struct TerrainHorizonMap
{
    static constexpr int kDirections = 16;
    static constexpr int kLayers = kDirections / 4;
    // Encoded range. Sun shadows only need the low horizon; a steeper horizon
    // still counts in the sky visibility, which is computed before encoding.
    static constexpr float kMinAngleDeg = -5.0f;
    static constexpr float kMaxAngleDeg = 45.0f;

    int size = 0;                            // texels per side
    float cellSize = 0.0f;                   // metres between texels
    std::vector<std::uint16_t> horizon;      // kLayers * size * size * 4
    std::vector<std::uint8_t> skyVisibility; // size * size, 255 = open sky
    std::vector<float> height;               // size * size, metres (the sampled heights)
    // Colour of the light the ground bounces, as a luminance-neutral tint:
    // size * size pairs (r, b) with g = (1 - 0.2126 r - 0.0722 b) / 0.7152, so
    // the tint never changes how bright the bounce is, only its colour. Empty
    // when no ground colours were given (the shader then uses a neutral tint).
    std::vector<float> groundTint;
    // The ground's own colour (display space 0..1): size * size rgb triples,
    // the landscape textures' average colours blended over the squares that
    // share each vertex (box-downsampled like the heights). The wet weather
    // tells grass from bare ground and how porous the land is from this
    // continuous field (SampleGroundColor, the shaders' groundFieldAt), not
    // from each square's own texture: every square has its own texture, and a
    // class read per pixel jumped at a square whose texture differed a little
    // from its neighbour's (a straight seam in the wet sheen and the puddles).
    // Empty when no ground colours were given.
    std::vector<float> groundColor;

    bool Valid() const { return size > 1 && cellSize > 0.0f; }
    // Decoded horizon angle (degrees) of direction `dir` at texel (x, z).
    float HorizonDeg(int x, int z, int dir) const;
    float SkyVisibility(int x, int z) const;
};

struct TerrainHorizonOptions
{
    // Texels per side are capped: a larger landscape is box-downsampled first.
    // The horizon is low frequency; 1024 at 50 m covers a 51 km island.
    int maxSize = 1024;
    // Farthest terrain that can occlude the sun or sky, metres.
    float maxDistance = 6000.0f;
    // Worker threads; 0 = half the hardware threads (the game keeps running
    // while a map builds).
    int threads = 0;
    // Set to abandon a build early (the landscape changed, the game quits);
    // the builder then returns false. Checked once per row.
    const std::atomic<bool>* cancel = nullptr;
};

std::uint16_t EncodeHorizonDeg(float deg);
float DecodeHorizonDeg(std::uint16_t value);

// Builds the map from a heights grid. Returns false (and leaves `out` invalid)
// for an empty grid, a non-positive cell size or a cancelled build. `groundRgb` (optional,
// size * size * 3, display-space 0..1, same layout as the heights) is the
// colour of the ground at each vertex: the landscape texture's average colour.
bool BuildTerrainHorizonMap(const float* heights, int size, float cellSize, const TerrainHorizonOptions& options,
                            TerrainHorizonMap& out, const float* groundRgb = nullptr);

// The luminance-neutral bounce tint (r, b) for a display-space ground colour:
// the colour's linear chroma at unit luminance, softened toward white by
// `strength` (0 = white, 1 = the full chroma) and kept within [0.5, 1.6] per
// channel so a saturated texture cannot stain the shade.
void GroundTint(float r, float g, float b, float strength, float& tintR, float& tintB);

// The ground colour at world (x, z) as the shaders read it (groundFieldAt in
// EngineGL33_LightingShadersSurface.cpp): between the four texels around the
// point with smoothstep weights, so the field and its slope are continuous
// across every grid line (no edge and no crease), clamped at the map's edges.
// False (rgb untouched) when the map has no ground colours.
bool SampleGroundColor(const TerrainHorizonMap& map, float x, float z, float rgb[3]);

// Reference visibility the shader computes: horizon interpolated between the
// two directions around the sun azimuth, compared with the sun elevation over a
// penumbra of `softnessDeg`. `towardSun` need not be normalised (y up).
float SunVisibility(const TerrainHorizonMap& map, int x, int z, float towardSunX, float towardSunY, float towardSunZ,
                    float softnessDeg);

// FNV-1a over the grid dimensions and every height. The engine rebuilds the
// map only when this changes.
std::uint64_t HashHeights(const float* heights, int size, float cellSize);

} // namespace Poseidon::remaster
