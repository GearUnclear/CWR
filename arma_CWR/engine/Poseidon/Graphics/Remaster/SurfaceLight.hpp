#pragma once

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <functional>

// Surfaces and ambient light (feature switch `surfaces`). Pure, no GL, no
// engine state.
//
// What the player sees: surfaces sit in the world as they do in photographs.
//   - Contact darkening: a horizon-based ambient occlusion (GTAO) at half
//     resolution in the post pass, with Jimenez's multi-bounce correction from
//     each pixel's albedo, applied to the sky and bounce light only (never the
//     direct sun). Tyres, tracks, boots, doorways, the foot of a wall and the
//     ground under a hull darken as they do in reality. The world shaders write
//     what the pass may darken into a second colour target of the frame
//     (SurfaceTarget below): unlike the frame alpha the stock occlusion reads,
//     it also covers alpha-tested and blended draws (vehicles, soldiers, tree
//     crowns), which the stock pass never darkened.
//   - Large-scale occlusion: the sky a receiver loses under the canopy
//     heightfield (Canopy.hpp: tree crowns, forest blocks, buildings), read at
//     three scales in the world shaders, so the inside of a wood and the
//     ground under a tree are as dark as they are, at any distance.
//   - Directional sky light: the clear sky of the sky table (Atmosphere.hpp)
//     projected onto spherical harmonics up to band 2 and convolved with the
//     clamped cosine (Ramamoorthi and Hanrahan 2001), in the sun's frame, so a
//     face toward the bright horizon under a low sun takes more light than one
//     toward the blue zenith opposite it; the ground below bounces its own
//     tinted light up. Matched to the rig's zenith brightness, so exposure is
//     unchanged, and blended toward the rig's hemisphere under overcast and at
//     night.
//   - Specular on every surface: GGX sun highlights and a split-sum sky
//     reflection with Fresnel, roughness and F0 per surface class
//     (SurfaceMaterials.hpp SurfaceClass: paint, metal, glass, rubber, cloth,
//     skin, road, masonry, wood, leather, weapon, lens, roof; terrain and
//     foliage in the shader), so edges catch the light and a painted hull
//     reads as painted metal.

namespace Poseidon::remaster
{

struct AtmosphereTables;

// ---- The sky light as spherical harmonics --------------------------------------
//
// The sun frame of the sky table: x toward the sun's azimuth, y up, z across.
// The clear sky is mirror-symmetric in z, so of the nine band 0..2 terms only
// the six even in z are not zero. Basis (real, orthonormal):
//   0: 0.282095                1: 0.488603 y        2: 0.488603 x
//   3: 0.315392 (3 y^2 - 1)    4: 1.092548 x y      5: 0.546274 (x^2 - z^2)
// The stored coefficients are of the irradiance (already convolved with the
// clamped cosine: pi, 2 pi / 3 and pi / 4 per band).
constexpr int kSkyShCoeffs = 6;
float SkyShBasis(int i, float x, float y, float z);

struct SkyLightSH
{
    Rgb c[kSkyShCoeffs];
    // The direct sun's transmittance through the same air (ProjectSkyLight):
    // the sky's irradiance against the sun's on flat ground is the physical
    // sky level (LightingTuning::surfSkyLevel).
    Rgb sunTransmittance = Rgb(1.0f, 1.0f, 1.0f);
    float sunElevationRad = 0.0f;
    float haze = -1.0f;
    bool valid = false;
};

// Projects the radiance of the upper hemisphere (`radiance(elevation, azimuth
// from the sun)`, radians; the lower hemisphere is zero) onto the irradiance
// coefficients. Midpoint rule, uniform in sin(elevation) and azimuth over
// [0, pi] (the mirror half doubles it).
SkyLightSH ProjectSkyRadiance(const std::function<Rgb(float, float)>& radiance, int elevationSteps, int azimuthSteps);
// The clear sky of the sky table for a sun at this elevation (radians).
SkyLightSH ProjectSkyLight(const AtmosphereTables& t, float sunElevationRad, int elevationSteps = 12,
                           int azimuthSteps = 16);
// Irradiance onto a unit normal (x, y, z in the sun frame), not clamped.
Rgb EvalSkyLight(const SkyLightSH& sh, float x, float y, float z);

// ---- The frame constants -------------------------------------------------------

// LitConstants slots Surface .. SurfaceSh5 (EngineGL33_Lighting.hpp).
struct SurfaceFrameConstants
{
    // x 1 while the switch and its gate are on (0 = the world shaders without
    // it), y the specular strength, z the canopy occlusion strength, w the
    // weight of the spherical-harmonics sky light against the rig's hemisphere.
    float surface[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // rgb: the sky light's coefficients in the rig's units, the physical
    // sky's colour.
    float sh[kSkyShCoeffs][4] = {};
    // x the relief strength (surfRelief; 0 while off), y the surfaces' debug
    // view the lit shaders draw (1 sky light, 2 canopy sky visibility, 3 specular,
    // 0 none; LightRig.hpp kDebugViewSkyLight..), z the share of the physical
    // sky's colour in the sky light (surfSkyTint; the rest is the rig's own
    // colour toward the normal), w the share of the sky fill a non-leaf
    // surface keeps (SurfaceShadeFactor; 1 = the rig's fill).
    float surface2[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    // x the cavity strength (surfCavity; 0 while off): how much of the sun
    // and the sky a texture's pits lose; y 1 while a vehicle interior is
    // drawn (EngineGL33::SurfaceEnterInterior; 0 here), zw unused.
    float surface3[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

// litSurface2.y for a debug view (LightingTuning::debugView): 1 to 3 for the
// surfaces' views 12 to 14, 0 otherwise.
float SurfaceDebugView(int debugView);

// `rig` is the frame's light rig, `sh` the clear sky of the sky table (or not
// valid yet), `skyThrough` 1 clear .. 0 overcast.
SurfaceFrameConstants ComputeSurfaceFrameConstants(const LightingTuning& tuning, const FeatureGates& gates,
                                                   const LightRig& rig, const SkyLightSH& sh, float skyThrough);

// The share of the rig's sky fill a non-leaf surface keeps (surfShade): the
// knob under a clear sky with the sun above 18 degrees, back to 1 as the sun
// sinks to 3 degrees (a low sun's fill is a larger share of its light, and the
// night keeps the original's moonlit fill) and as the sky clouds over (an
// overcast sky is the only light, and it lights the shade).
float SurfaceShadeFactor(float knob, float sunElevationDeg, float skyThrough);

// The sky light's weight against the rig's hemisphere: the knob, faded out
// with the sky table at night and under overcast (the table is a clear sky).
float SkyLightWeight(float knob, float sunElevationDeg, float skyThrough);

// ---- The horizon occlusion in the post pass ------------------------------------

// Constants of psPostGtao, in metres and shares.
constexpr float kGtaoFadeStart = 150.0f;      // the occlusion fades out from here ...
constexpr float kGtaoFadeEnd = 300.0f;        // ... to here (metres of view depth)
constexpr float kGtaoMaxScreenRadius = 0.25f; // the search's reach at most this share of the frame height
constexpr int kGtaoSlices = 3;                // slice directions per pixel (turned per pixel and per TAA frame)
constexpr int kGtaoSteps = 8;                 // samples per side of a slice
// Near pixels whose search spans more than this many half-resolution pixels
// take one slice fewer: their samples are far apart and cost the most, and
// the TAA turn covers the other directions over the frames.
constexpr float kGtaoOneSliceRadiusPx = 60.0f;
// The weight of an occluder below the receiver (the ground around a hull's
// side or a wall's foot): the ground bounces the light it hides.
constexpr float kGtaoBelowWeight = 0.3f;
// Under TAA the occlusion is accumulated over frames (psPostGtaoAccum: the
// last frames' occlusion reprojected by depth, this frame's mixed in at this
// weight), and two slices a pixel suffice; without it every pixel takes
// kGtaoSlices.
constexpr int kGtaoTemporalSlices = 2;
constexpr float kGtaoHistoryWeight = 0.85f;
// A vehicle interior (the world's depth cleared, the crew compartment drawn
// with its own near plane): the occlusion searches this far (metres), so the
// corners, the gun's breech and the hatch rims darken.
constexpr float kGtaoInteriorRadius = 1.0f;
// Static architecture (houses, walls, roofs: the Static draw kind, masonry,
// roofs, wood and roads) takes the occlusion at this multiple of surfAo: its
// eaves, door openings and the foot of a wall are large, flat, light surfaces
// that a vehicle's power leaves pale. Written per pixel in the surface
// target's alpha (kSurfaceAoStaticCode).
constexpr float kSurfaceStaticAoBoost = 1.8f;
// ... and searches this multiple of surfAoRadius (the GTAO pass reads the code
// from the resolved target): a house's eaves and the wall-ground seam are
// metres across.
constexpr float kGtaoStaticRadiusScale = 2.0f;
// The texture relief and cavities under a sun of 4 degrees and lower keep
// this share of themselves, all of it from 14 degrees up.
constexpr float kSurfLowSunGrain = 0.3f;
// The share of a pre-lit interior fragment's light the occlusion may darken
// (the classic vertex light of a crew compartment is mostly fill).
constexpr float kSurfaceInteriorShare = 0.9f;
// ---- Surface classes -----------------------------------------------------------
//
// The per-draw class comes from SurfaceMaterials.hpp (the texture's name and
// the drawing object's kind); the shader's table
// (surfaceClassParams, surfaceClassRelief, surfaceClassCavity in the surface
// piece) indexes its SurfaceClass values: 1 paint, 2 metal, 3 glass, 4 rubber,
// 5 fabric, 6 skin, 7 road, 8 masonry, 9 wood, 10 leather, 11 weapon, 12 lens,
// 13 roof, 0 the default.

} // namespace Poseidon::remaster
