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
//   - an optional moon fill at night.
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
    float moonlight = 1.0f;        // directional moon fill at night
    float ambientOcclusion = 0.7f; // screen-space AO strength (post effects)
    float bloom = 0.35f;           // glow around the sun, sky and highlights (post effects)
    float sunShafts = 1.0f;        // light shafts toward a low sun (post effects)
    float shadowSoftness = 1.0f;   // shadow-map filter radius multiplier
    // Diagnostic view: 0 off, 1 sun visibility, 2 terrain sky visibility,
    // 3 normals, 4 ambient occlusion.
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
    // Broken-cloud field for the shader: fraction of the sky covered and the
    // fraction of sunlight that survives under a cloud.
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

} // namespace Poseidon::remaster
