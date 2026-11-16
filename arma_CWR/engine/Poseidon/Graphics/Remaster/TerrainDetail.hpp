#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <array>

// Terrain detail for the remastered lighting: the shape constants and the
// math of the terrain
// detail main (s_psDetailRemasterMain in EngineGL33_LightingShaders.cpp), as a
// C++ mirror the tests check. Pure, no GL, no engine state.
//
// Why: every terrain square is one 128x128 texture over 50 m (39 cm per
// texel), so from about 10 m to 150 m one texel covers 5 to 15 pixels and the
// slope is a bilinear smear; the 128 px detail texture has averaged itself
// flat by 20 m. Filtering is not the cause (16x anisotropy, full residency and
// the stored mip chains do not help).
//
// What: three layers on the land's albedo, all luminance-only and relative to
// the local mean, so hue and average brightness stay as authored:
//   - self-detail: each fragment gets the luminance high-pass of its own
//     square's texture, zoomed kZoom times, read near the fragment (grass
//     from grass, rock from rock). Cells of kCellTexels base texels each show
//     the patch around their own centre, rotated by a per-cell random angle
//     (no parallel "mowing stripes") and offset by a per-cell jitter; the four
//     cells around a fragment are blended so the blend keeps the grain's
//     variance (sum w h / sqrt(sum w^2)), and the result goes through a soft
//     limiter at +-kLimit. The zoomed sample takes the true zoomed
//     derivatives, so it mip-maps away by itself once the base texture is no
//     longer magnified (150 to 300 m). Near the eye the zoomed patch is itself
//     magnified into soft blobs over the authored detail texture's pebbles
//     (blotches on a path at 2 to 8 m), so it fades
//     in as its own mip level rises from kNearFromLod to kNearToLod (8 to 16 m
//     on flat ground from a standing eye at 1080p);
//   - detail kept: the detail texture's deviation from its mean is scaled up
//     as the GPU picks coarser mips of it (2^(lod * kKeptRate / 2), capped at
//     kKeptCap: a 4x boost paints dark blotches on paths), faded back
//     to 1 by kMaxDistance;
//   - anti-tiling: a second detail sample on a rotated, rescaled grid is
//     mixed in (deviation rescaled by 1 / sqrt 2) where the detail texture is
//     minified and its grain is boosted, so the kept grain does not print as a
//     grid; the magnified detail near the eye stays as authored.
// Everything fades out between kFadeStart and kMaxDistance and nothing is
// evaluated beyond (the frame gate's "terrain within 300 m").
//
// Only the land gets it: the sea is drawn by the same pixel shader (the
// landscape's water faces carry SpecularTexture, the land DetailTexture), so
// the backend flags each draw (LitConstants litDetail.w).

namespace Poseidon::remaster
{

namespace terrain_detail
{

constexpr float kZoom = 6.0f;                       // the zoomed patch is this many times finer than the base texture
constexpr float kPatchTexels = 12.0f;               // span of one cell's patch in base texels (4.7 m)
constexpr float kCellTexels = kPatchTexels / kZoom; // cell size in base texels (2 texels, 78 cm)
constexpr float kJitterTexels = 6.0f;               // per-cell offset range of the patch (decorrelates neighbours)
constexpr float kLimit = 0.4f;                      // soft limit of the self-detail modulation
constexpr float kMinLocalMean = 0.04f;              // floor of the local mean the high-pass is relative to
constexpr float kFadeStart = 220.0f;                // metres: everything starts to fade out here
constexpr float kMaxDistance = 300.0f;              // metres: nothing beyond (the frame gate's range)
constexpr float kKeptRate = 2.0f;                   // detail kept: boost exponent per mip level
constexpr float kKeptCap = 2.0f;                    // detail kept: largest boost of the detail grain
constexpr float kKeptFadeStart = 150.0f;            // metres: the boost fades back to 1 from here to kMaxDistance
constexpr float kAntiTile = 1.0f;                   // weight of the rotated second detail sample
constexpr float kNearFromLod = -3.5f;               // self-detail fades in from this mip level of the zoomed patch
constexpr float kNearToLod = -2.5f;                 // ... to full at this one (a zoomed texel 11 to 6 pixels)
constexpr float kMaxAniso = 16.0f;                  // the samplers' anisotropy, for the GPU's mip level estimate
constexpr float kDefaultStrength = 1.5f;            // self-detail strength (LightingTuning::detailStrength)

// Dave Hoskins' hash22: a cell index to two values in
// [0, 1). Float arithmetic as in the shader.
std::array<float, 2> Hash2(float x, float y);

// Rotation of a cell's patch, radians in [0, 2 pi).
float CellAngle(float cellX, float cellY);

// The patch offset of a cell in base texels, each axis in
// [-kJitterTexels / 2, kJitterTexels / 2).
std::array<float, 2> CellJitter(float cellX, float cellY);

// Where cell (cellX, cellY) reads the base texture for a fragment at base
// texel position (px, py): its patch around its own centre, rotated and
// zoomed kZoom times, plus its jitter. Base texels.
std::array<float, 2> CellSample(float px, float py, float cellX, float cellY);

// The four cells around a fragment and their bilinear weights (sum 1): cell
// (i0 + (k & 1), j0 + (k >> 1)) has weight w[k].
struct CellBlend
{
    float i0 = 0.0f, j0 = 0.0f;
    std::array<float, 4> w{};
};
CellBlend CellsAround(float px, float py);

// The blend of four cell values h that keeps their variance:
// sum w h / sqrt(sum w^2).
float VariancePreservingBlend(const std::array<float, 4>& w, const std::array<float, 4>& h);

// m / sqrt(1 + (m / kLimit)^2): about m for small m, never beyond +-kLimit.
float SoftLimit(float m);

// 1 up to kFadeStart, 0 from kMaxDistance on (smoothstep in between).
float DistanceFade(float distance);

// The mip level the GPU picks with kMaxAniso anisotropy for a footprint whose
// two screen axes cover l1 and l2 texels per pixel: log2 of the major axis
// over min(kMaxAniso, major / minor).
float FootprintLod(float l1, float l2);

// Self-detail weight near the eye from the base texture's mip level `baseLod`
// (FootprintLod of the base texture): the zoomed patch's level is baseLod +
// log2(kZoom); 0 below kNearFromLod, 1 from kNearToLod on.
float NearFade(float baseLod);

// Weight of the anti-tiling sample at a mip level `lod` of the detail texture
// and a distance: 0 while the detail is magnified, kAntiTile from one level
// of minification on, faded out with DistanceFade.
float AntiTileWeight(float lod, float distance);

// Detail kept: the factor on the detail texture's deviation from its mean at
// a mip level `lod` of the detail texture and a distance: 1 when magnified,
// 2^(lod * kKeptRate / 2) beyond, at most kKeptCap, back to 1 by
// kMaxDistance.
float KeptBoost(float lod, float distance);

// The terrain detail's frame constants (LitConstants litDetail.xyz): the
// self-detail strength, the detail-kept rate and the anti-tiling weight. All
// zero when the switch or its frame gate is off: the terrain main is then the
// stock one exactly. litDetail.w (land or sea) is per draw.
std::array<float, 3> FrameConstants(const LightingTuning& tuning, bool gateOn);

} // namespace terrain_detail

} // namespace Poseidon::remaster
