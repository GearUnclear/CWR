#pragma once

#include <cstdint>

// Light rig for the remastered lighting: pure math, no GL, no engine state.
//
// Classic lighting sums a yellow sun (0.85, 0.75, 0.4) and a blue ambient
// (0.55, 0.6, 0.8), both scaled by the 1.6 brightness, then clamps the sum to 1
// before texturing. At noon both the sunlit side and the shaded side of most
// surfaces clamp to white, so the scene shows bare texture colour with almost
// no shading. The remastered path lights in linear HDR instead and tonemaps
// per fragment; this rig supplies the light it integrates:
//
//   - a sun whose colour follows its elevation (near-neutral by day; below 16
//     degrees it takes the classic sunset colour, so evenings are as warm on
//     screen as the original's) and whose strength follows the classic
//     overcast and night ramps,
//   - a sky hemisphere (zenith, ground bounce and a horizon band split into a
//     sunward half and an anti-sun half). The sunward horizon takes the engine
//     fog colour, so a mod's sky tints the light and a low sun gives warm light
//     on the sun side and cool shade on the other,
//   - an exposure meter, so sunlit ground keeps its classic on-screen
//     brightness from noon down to a low sun (the frame does not just get
//     darker),
//   - a moon key light at night (cool, pale, dimmed by overcast) with the
//     moonlit sky as its fill; a moonless night keeps the classic ambient.
//
// All colours here are linear and exclude the material terms; the shader
// multiplies by the material diffuse / ambient (which carry the user
// brightness) exactly as the classic path does.

namespace Poseidon::remaster
{

struct Rgb
{
    float r = 0.0f, g = 0.0f, b = 0.0f;

    Rgb() = default;
    Rgb(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}
    Rgb operator*(float s) const { return {r * s, g * s, b * s}; }
    Rgb operator+(const Rgb& o) const { return {r + o.r, g + o.g, b + o.b}; }
    Rgb operator*(const Rgb& o) const { return {r * o.r, g * o.g, b * o.b}; }
};

float Luminance(const Rgb& c);
Rgb Mix(const Rgb& a, const Rgb& b, float t);
// sRGB-ish display <-> linear, the 2.2 power the shaders use.
float ToLinear(float display);
Rgb ToLinear(const Rgb& display);
float ToDisplay(float linear);

// Khronos PBR Neutral tone mapping without its toe, the curve the shaders
// apply. Identity below 0.8 (keeps authored texture colour), soft shoulder
// above, hue preserved, highlights desaturate toward white. Input and output
// linear.
Rgb TonemapNeutral(const Rgb& c);

// The feature switches of the remastered lighting, one float per switch: 0 off,
// 1 on (the shipped default). A switch counts as on above 0. Each member is a
// triSetLighting key of the same name; the registry that names, labels and
// groups them, and the gate that decides each frame whether an enabled feature
// runs, are in FeatureSwitches.hpp and FeatureGates.hpp. The comment after each
// member names the integration step that makes it do something (plan section
// 4 of research/INTEGRATION-PLAN.md); until then nothing reads it.
struct FeatureSet
{
    float drawGrouping = 1.0f;    // A4 perf: instanced runs at any light, unchanged constants not re-sent
    float hillCull = 1.0f;        // A4 perf: objects behind hills are not drawn
    float taa = 1.0f;             // A5 temporal anti-aliasing
    float lightTiles = 1.0f;      // A6 one tiled light list for every local light
    float atmosphere = 1.0f;      // A7 sky table for the dome and the fog colour
    float treeShadows = 1.0f;     // B1 trees and buildings in the sun shadow map, leaves without self-shadow stripes
    float softShadows = 1.0f;     // B1 contact-hardening (PCSS) sun shadows
    float lowSunRig = 1.0f;       // B2 low-sun hue and readable shaded sides
    float crownCanopy = 1.0f;     // B2 tree crowns lit as a canopy
    float nightLights = 1.0f;     // B3 physical lamps and low-beam headlights
    float moonShadows = 1.0f;     // B3 the moon as a shadow-casting key light
    float stars = 1.0f;           // B3 stars in a dark sky
    float combatLights = 1.0f;    // B4 fires, flashes and flares light the scene
    float combatShadows = 1.0f;   // B4 combat lights cast shadows
    float seaWaves = 1.0f;        // B5 wave relief and a broken sun path on the sea
    float ssr = 1.0f;             // B6 screen-space reflections on water and wet ground
    float wetWeather = 1.0f;      // B7 rain wets surfaces and fills puddles
    float lampStreaks = 1.0f;     // B7 lamp light streaks on wet ground
    float airBeams = 1.0f;        // B8 sun beams and headlight cones in the air
    float airHaze = 1.0f;         // B8 aerial haze with distance
    float valleyMist = 1.0f;      // B8 morning mist pooled in the valleys
    float canopyShadows = 1.0f;   // B9 long tree shadows beyond the shadow map
    float litClouds = 1.0f;       // B10 sky clouds lit from their sun side
    float skyCloudShadows = 1.0f; // B10 cloud shadows that match the sky
    float terrainDetail = 1.0f;   // B11 grass and rock grain on terrain
    float sharpen = 1.0f;         // B11 luma sharpening when TAA is off
};

// Art-direction knobs, live-tunable from the dev overlay Lighting tab. The
// defaults are the shipped look.
struct LightingTuning
{
    bool remastered = true;        // false = classic shaders, bit-for-bit legacy look
    bool postEffects = true;       // ambient occlusion + bloom at the world/HUD boundary
    float exposure = 1.0f;         // overall gain before the tonemap
    float saturation = 1.12f;      // display-space saturation after the tonemap (1 = none)
    float sun = 1.0f;              // direct sun strength
    float sky = 1.0f;              // sky hemisphere (shade fill) strength
    float bounce = 1.0f;           // ground bounce light
    float warmth = 1.0f;           // 0 = neutral sun at every elevation, 1 = physical, >1 stronger
    float terrainShadows = 1.0f;   // hills shadowing the sun (horizon map)
    float terrainOcclusion = 0.6f; // valleys losing sky light (horizon map)
    float cloudShadows = 1.0f;     // moving cloud shadows in broken overcast
    float specular = 1.0f;         // sun highlights and sky reflection on glossy materials
    float localLights = 1.0f;      // headlights, lamps, fires
    float moonlight = 1.0f;        // moon key light (and its sky fill) at night
    float ambientOcclusion = 0.7f; // screen-space AO strength (post effects)
    float bloom = 0.35f;           // glow around the sun, sky and highlights (post effects)
    float sunShafts = 1.0f;        // light shafts toward a low sun (post effects)
    float shadowSoftness = 1.0f;   // shadow-map filter radius multiplier
    // Temporal anti-aliasing (features.taa, TemporalAA.hpp): the sharpen after
    // the resolve undoes the softening of the accumulation, the texture LOD
    // bias of the world's passes wins back the texture detail it averages, and
    // alpha is this frame's share in the blend with the history.
    float taaSharpen = 0.5f;
    float taaMipBias = -0.5f;
    float taaAlpha = 0.1f;
    // Sky table (FeatureSet::atmosphere, Atmosphere.hpp) sub-knobs: the hue of
    // the clear sky (0 the original sky's hue relit by the atmosphere, 1 the
    // physical hue), the display-space saturation of the dome and the fog
    // colour, and the haze (Mie density multiplier; a change rebuilds the
    // atmosphere tables on a worker thread).
    float atmoHue = 0.5f;
    float atmoSat = 1.15f;
    float atmoHaze = 1.0f;
    // Terrain detail (features.terrainDetail, TerrainDetail.hpp): the strength
    // of the grain the land takes from its own texture. The luma sharpen
    // (features.sharpen, with TAA off): its contrast-adaptive strength, 0..1.
    float detailStrength = 1.5f;
    float sharpenStrength = 0.3f;
    // Sun shadows (features.softShadows and features.treeShadows,
    // SunShadows.hpp): the sun's apparent diameter in degrees for the soft
    // shadows (0.53 is the real sun; 1 keeps contact sharp and long shadow
    // ends soft), how far along the sun ray a blocker still softens its shadow
    // more (metres; the penumbra stops growing there), and how many
    // shadow-map texels a leaf's own shadow lookup moves toward the sun.
    float sunSize = 1.0f;
    float shadowReach = 40.0f;
    float leafBias = 6.0f;
    // Sea (FeatureSet::seaWaves, SeaWaves.hpp) sub-knobs, each a multiplier of
    // the shipped value: the wave relief, the mean sky reflection and the
    // sheen toward the sun in the original sun's hue.
    float seaRelief = 1.0f;
    float seaSky = 1.0f;
    float seaSheen = 1.0f;
    // Frame gates (FeatureGates.hpp): on runs an enabled feature only when it
    // can show; off (gates=0) runs every enabled feature on every frame, for
    // debugging.
    bool gates = true;
    FeatureSet features;
    // Diagnostic view: 0 off, 1 sun visibility, 2 terrain sky visibility,
    // 3 normals, 4 ambient occlusion, 5 the post pass's half-resolution
    // linear depth (PostStages.hpp kDebugViewLinearDepth), 6 the lights per
    // tile of the tiled light list (FrameLights.hpp kDebugViewLightTiles).
    int debugView = 0;
};

struct LightRigInputs
{
    float towardSun[3] = {0.0f, 1.0f, 0.0f};   // unit, y up
    float towardMoon[3] = {0.0f, -1.0f, 0.0f}; // unit, y up
    float moonPhaseLight = 0.0f;               // 0 new moon .. 1 full moon
    Rgb classicSunDiffuse;                     // LightSun::Diffuse(), overcast already applied
    Rgb classicAmbient;                        // LightSun::Ambient()
    Rgb fogColor;                              // engine fog colour (display space)
    float skyThrough = 1.0f;                   // 1 clear .. 0 overcast
    float nightEffect = 0.0f;                  // 0 day .. 1 night
    bool cloudShadowsActive = false;           // the sun dimming by overcast moves into the cloud field
};

struct LightRig
{
    Rgb sun;           // direct sun radiance (linear)
    Rgb skyZenith;     // irradiance onto an up-facing surface
    Rgb skyHorizon;    // irradiance onto a vertical surface facing away from the sun
    Rgb skyHorizonSun; // irradiance onto a vertical surface facing the sun
    Rgb skyGround;     // irradiance onto a down-facing surface (ground bounce)
    Rgb moon;          // moon radiance (linear)
    float sunElevationDeg = 90.0f;
    // Gain before the tonemap: tuning.exposure times a meter that keeps sunlit
    // ground at its classic on-screen brightness as the sun gets lower.
    float exposure = 1.0f;
    // Broken-cloud field for the shader: fraction of the ground under cloud
    // shadow (0 under a clear sky, the sky cover once it is clearly cloudy) and
    // the fraction of sunlight that survives under a cloud.
    float cloudCover = 0.0f;
    float cloudTransmit = 1.0f;
};

LightRig ComputeLightRig(const LightRigInputs& in, const LightingTuning& tuning);

// Sun chroma at an elevation (degrees), normalised to unit luminance.
Rgb SunChroma(float elevationDeg, float warmth);

// The sun chroma the rig uses: SunChroma by day, and in the evening (below
// 16 degrees, nearly fully by 7) the classic sunset colour `classicSun` (display
// space, LightSun's colorFull or Diffuse) taken to linear, so evenings keep the
// original's warm hue on screen. `warmth` 0 = neutral.
Rgb EveningSunChroma(float elevationDeg, const Rgb& classicSun, float warmth);

// Ellipsoid that stands in for a tree crown when lighting foliage, from the
// model's bounding box (min, max; y up). Trunks make the box taller than the
// crown, so the centre sits at 60% of the height. Writes the centre and the
// inverse half extents; the shader lights a leaf with the ellipsoid's normal.
// Always finite: an axis with NaN or infinite bounds gets a 5 cm half extent
// centred on the origin.
void CrownEllipsoid(const float boxMin[3], const float boxMax[3], float center[3], float invHalf[3]);

// Sun shafts are sunlight scattered toward the eye by the air in front of a
// surface, so they grow with the amount of air on the view ray. The post
// composite keeps ShaftInscatter(distance) of the shaft light it gathers from
// the visible sky: all of it over the sky, most of it on distant ground, next
// to none on a vehicle a few metres away. The scattering length is
// kShaftScatterLength in clear air; fog or rain that brings the fog end nearer
// than kShaftFogLengths of it shortens it, so thick haze carries its shafts
// closer.
constexpr float kShaftScatterLength = 300.0f; // metres
constexpr float kShaftFogLengths = 3.0f;      // scattering lengths that fit in the fog range

// Scattering length in metres for a scene whose fog ends at `fogEnd` metres.
float ShaftScatterLength(float fogEnd);
// Share of the gathered shaft light kept on a surface `distance` metres from
// the eye: 1 - exp(-distance / scatterLength), the composite shader's formula.
float ShaftInscatter(float distance, float scatterLength);

} // namespace Poseidon::remaster
