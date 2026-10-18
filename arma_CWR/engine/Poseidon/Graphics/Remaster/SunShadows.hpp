#pragma once

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>

#include <cstdint>

// Sun shadows of the remastered lighting (INTEGRATION-PLAN.md step B1): the
// rules and the math the shadow pass, the GL backend and the shaders share.
// Pure, no GL, no engine state.
//
// Two feature switches, each with the sun shadow gate (the sun above 4 deg, or
// B3's moon key):
//
//   treeShadows  trees, bushes and buildings always cast into the remastered
//                sun shadow map, whatever the legacy per-profile object shadow
//                toggle says (the Graphics shadow tier still turns the maps
//                off), and a leaf's own shadow lookup moves a few shadow-map
//                texels toward the sun. Leaf cards are drawn two-sided into the
//                map, so without that each card shadows itself in stripes; the
//                move is in texels, so it grows with the tier and the crowns'
//                self-shadow thins with distance, where it only flickers.
//   softShadows  contact-hardening soft shadows (PCSS): the sun is a disk of
//                sunSize degrees, a blocker search in the raw depth map gives
//                how far above the receiver its blocker is, and the filter
//                widens with that distance. Sharp where a caster meets the
//                ground, soft at the far end of a long evening shadow. Without
//                TAA the taps are a fixed per-pixel rotation of a 16-tap search
//                (8 on the frustum tiers) and a 12, 24 or 32-tap filter; in a
//                TAA frame 4 search and 4 filter taps turn every frame and the
//                history averages them.
//
// The shader reads two LitConstants slots (EngineGL33_Lighting.hpp LitConst):
//   Taa  x per-frame turn of the tap pattern (0 outside a TAA frame), y 1 in a
//        TAA frame, z taps per frame there, w tan of the sun's angular radius
//        (0 = the stock 12-tap filter)
//   Pcss x blocker search reach (metres along the sun ray, shadowReach), y smallest filter
//        radius (texels), z soft filter strength (fades in above the gate
//        edge), w leaf lift toward the sun (texels; 0 = none)

namespace Poseidon::remaster
{

// Blockers up to this far along the sun ray are found by the search (its
// radius in the light plane is this times tan(sun angular radius)), and the
// penumbra stops growing there: the default of LightingTuning::shadowReach.
constexpr float kPcssSearchMetres = 40.0f;
// The reach the shader gets is at least this (a reach of 0 would search
// nothing).
constexpr float kPcssMinReachMetres = 1.0f;
// The search radius in shadow-map texels is clamped to this range.
constexpr float kPcssMinSearchTexels = 2.0f;
constexpr float kPcssMaxSearchTexels = 48.0f;
// Smallest filter radius (texels): the contact end of a shadow.
constexpr float kPcssMinFilterTexels = 0.6f;
// Search taps without TAA: the omni (near) tiers search up to 48 texels, the
// frustum tiers 2 to 4 (a texel there covers decimetres).
constexpr int kPcssSearchTapsNear = 16;
constexpr int kPcssSearchTapsFar = 8;
// Search and filter taps per frame in a TAA frame.
constexpr int kPcssTemporalTaps = 4;
// Filter taps without TAA by the penumbra radius (texels): 12 below 3, 24
// below 10, 32 above.
constexpr float kPcssTaps24FromTexels = 3.0f;
constexpr float kPcssTaps32FromTexels = 10.0f;
// The soft filter fades in from the gate's edge (kGateSunShadowMinSunDeg) to
// this sun height, so a sunset time-lapse sees no switch of filters.
constexpr float kPcssFullFromSunDeg = 6.0f;
// The golden ratio step of the per-frame turn (the post AO and shafts turn
// their patterns by the same step, EngineGL33_PostFX.cpp).
constexpr double kPcssFrameTurnStep = 0.6180339887;

// tan of the sun's angular radius for an apparent diameter in degrees; 0 for
// a size of 0 or less (the stock filter).
float SunTanRadius(float sunSizeDeg);

// The blocker search radius in texels of a tier whose texel covers
// `texelMetres` in the light plane, for blockers up to `reachMetres` away.
float PcssSearchTexels(float tanRadius, float texelMetres, float reachMetres = kPcssSearchMetres);

// Metres along the sun ray from the receiver to its mean blocker, from their
// shadow-map depths and the tier's depth per metre (the length of the
// light view-projection's z row, as the shader takes it).
float PcssBlockerMetres(float receiverDepth, float meanBlockerDepth, float depthPerMetre);

// The filter radius in texels: the penumbra of a blocker `blockerMetres` away,
// at least kPcssMinFilterTexels and at most the search radius.
float PcssPenumbraTexels(float blockerMetres, float tanRadius, float texelMetres, float searchTexels);

int PcssSearchTaps(int tier, int omniTiers, bool temporal);
int PcssFilterTaps(float penumbraTexels, bool temporal);

// The soft filter's share at a sun height: 0 at or below the gate's 4 deg, 1
// from kPcssFullFromSunDeg (smoothstep between).
float PcssStrength(float sunElevationDeg);

// The pattern's turn (in turns, [0, 1)) for TAA frame `sequence`.
float PcssFrameTurn(std::uint32_t sequence);

struct SunShadowConstants
{
    float taa[4] = {0.0f, 0.0f, 0.0f, 0.0f};  // LitConst::Taa
    float pcss[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // LitConst::Pcss
};

// The two slots for this frame: `taaFrame` when the frame's world passes are
// drawn jittered and resolved (EngineGL33::TaaWantedThisFrame), `taaSequence`
// the TAA frame index.
SunShadowConstants ComputeSunShadowConstants(const LightingTuning& tuning, const FeatureGates& gates, bool taaFrame,
                                             std::uint32_t taaSequence);

// Whether an object goes into the sun shadow map: its own say (the legacy
// object and vehicle shadow toggles, Object::CastShadow), or, with the
// treeShadows gate on, any static object (trees, bushes, buildings).
bool CastsIntoSunShadowMap(bool castShadow, bool staticObject, bool treeShadowsOn);

} // namespace Poseidon::remaster
