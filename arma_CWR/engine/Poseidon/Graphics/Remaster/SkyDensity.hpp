#pragma once

#include <cstdint>
#include <vector>

// The sky's own clouds for the remastered sky (feature switches litClouds and
// skyCloudShadows). Pure math, no GL, no engine state; the GL33 backend (EngineGL33_SkyClouds.cpp) builds the density
// map on a worker thread and the shaders mirror the functions below
// (EngineGL33_LightingShaders*.cpp; the tests check the constants match).
//
// The sky is the dome data3D\obloha.p3d under the weather sky texture (jasno
// clear, oblacno broken, zatazeno overcast, interpolated by the engine). Its
// clouds are painted into that texture, so the remastered sky reads them back
// from it, no new art:
//
//   - the density map: per texel of the weather sky, r = broken cloud (whiter
//     than the clearest sky of the same row: the skies are painted panoramas
//     whose clear blue pales toward the horizon, so a test against the row
//     finds the bluish streaks overhead and does not call the pale horizon
//     cloud), g = the row is grey all the way round (an overcast deck; the
//     shaders count it only as far as the weather is overcast), b = darkness
//     (thicker cloud). The palettised textures would show their steps as
//     contours under a threshold, so r and b are blurred (5-tap binomial, u
//     wraps, v clamps).
//   - lit clouds: a short march from a sky pixel toward the sun through the
//     dome's own texture mapping gives the cloud the sunlight crosses before it
//     reaches that pixel; the difference to the pixel's own density is
//     mean-free relief (sun-facing edges lit and warm, the far side shaded), a
//     forward-scattered silver lining on the rim, darker thick cloud against
//     the sun and deeper bellies under an overcast deck. All of it only at a
//     low sun (every term fades between kLitCloudFadeStartDeg and
//     kLitCloudFadeEndDeg, LitCloudTerms): at midday the relief would emboss
//     the painted sky, and the darkening against the sun alone would only
//     darken it.
//   - sky-matched cloud shadows: a lit point is projected along the sun to a
//     deck at kSkyShadowDeckAltitude and the direction of that deck point is
//     looked up in the density map: the cloud the player sees in the sky is
//     the cloud whose shadow lies on the ground. The projection is seen from an
//     anchor that follows the camera in steps (SkyShadowAnchor), so the
//     shadows stay where they are on the ground while the player moves.
//   - the dome turns about the zenith with the sky clouds' own drift
//     (Landscape::CloudsPosition, the sky clouds' wind integrator), and its
//     clouds, their light and their shadows turn together.

namespace Poseidon::remaster
{

// ---- The sky dome's texture mapping ---------------------------------------
// obloha.p3d's own mapping, dumped from the mesh in game: u = azimuth / 180
// degrees (the texture wraps twice around the horizon, REPEAT), azimuth from
// north (+z) toward east (+x); v piecewise linear in elevation over the dome's
// rings.
constexpr int kSkyDomeRings = 5;
constexpr float kSkyDomeRingElevationDeg[kSkyDomeRings] = {-15.0f, 0.87f, 29.5f, 59.5f, 89.0f};
constexpr float kSkyDomeRingV[kSkyDomeRings] = {1.0f, 0.9869f, 0.5434f, 0.1736f, 0.036f};

struct SkyUv
{
    float u = 0.0f;
    float v = 0.0f;
};

// The dome's texture coordinate of the direction (x east, y up, z north; any
// length but zero). u in [-1, 1].
SkyUv SkyDomeUv(float x, float y, float z);
// v of an elevation in degrees (clamped to the rings).
float SkyDomeV(float elevationDeg);
// Elevation in degrees of a v (the inverse, clamped to the rings).
float SkyDomeElevationDeg(float v);

// ---- The cloud-density map --------------------------------------------------
struct SkyDensityMap
{
    int width = 0;
    int height = 0;
    // RGBA8, row-major, row 0 at v = 0 (the zenith end): r broken cloud, g
    // overcast-deck row, b darkness, a 255.
    std::vector<std::uint8_t> rgba;
    bool Valid() const;
    // Channels of texel (x, y) as 0..1.
    float R(int x, int y) const;
    float G(int x, int y) const;
    float B(int x, int y) const;
};

// The map of a weather sky: `rgb` holds width * height texels of 3 floats
// (display colour 0..1), row-major, row 0 at v = 0.
SkyDensityMap BuildSkyDensityMap(const float* rgb, int width, int height);
// Two weather skies mixed as the engine mixes them for the dome
// (TextureGL33::GetPixel: a * (1 - f) + b * f). Same size, or `a` is returned.
std::vector<float> MixSkyTexels(const std::vector<float>& a, const std::vector<float>& b, float f);

// How much of an overcast-deck row counts as cloud at this sky-through: none
// under a clear or broken sky (the pale horizon rows of a clear sky are grey
// all round too), all of it once the weather is overcast.
float SkyDeckWeight(float skyThrough);
// The cloud density of a map texel (channels 0..1) at a deck weight, as the
// shaders take it: broken cloud or deck, thicker where darker.
float SkyCloudDensity(float r, float g, float b, float deckWeight);

// ---- Sky-matched cloud shadows ---------------------------------------------
constexpr float kSkyShadowDeckAltitude = 1800.0f; // metres above sea level
constexpr float kSkyShadowThreshold = 0.55f;      // density at the middle of a shadow edge
constexpr float kSkyShadowEdge = 0.12f;           // half width of the edge (density)
// The sun disk behind a dome cloud keeps this much of its light.
constexpr float kSkySunBehindCloud = 0.12f;
// The coverage is the shadowed share of the ground within this radius of the
// anchor (the ground the player sees close up).
constexpr float kSkyShadowCoverageRadius = 3000.0f;

// Shadow of a density: 0 none .. 1 the full cloud shadow.
float SkyShadowCover(float density);
// The shadowed share of the ground within kSkyShadowCoverageRadius of the
// anchor, for a sun at this elevation (degrees), averaged over the dome's turn:
// each ground point looks the dome up in the direction of its deck point, as
// the shaders do (domeCloudCover), so a low sun takes its shadows from the
// rows near the horizon and a high sun from the rows overhead. The light rig
// keeps the classic overcast dimming on average over it (LightRigInputs::
// skyShadowCoverage). 0 for an invalid map.
float SkyShadowCoverage(const SkyDensityMap& map, float deckWeight, float sunElevationDeg);

// SkyShadowCoverage on a grid of deck weights and sun elevations, built with
// the map (the deck weight follows the sky-through every frame, the sun its
// height), and looked up bilinearly.
struct SkyShadowCoverageTable
{
    static constexpr int kDeckSteps = 17; // deck weight 0, 1/16, .., 1
    static constexpr int kSunSteps = 37;  // sun elevation 0, 2.5, .., 90 degrees
    float cover[kDeckSteps][kSunSteps] = {};
    float At(float deckWeight, float sunElevationDeg) const;
};
SkyShadowCoverageTable BuildSkyShadowCoverageTable(const SkyDensityMap& map);

// ---- Lit sky clouds ---------------------------------------------------------
constexpr float kLitCloudRelief = 1.8f;        // relief gain
constexpr float kLitCloudSilver = 1.0f;        // silver-lining gain
constexpr float kLitCloudStepDeg = 1.2f;       // first step of the march toward the sun
constexpr int kLitCloudSteps = 6;              // march steps (k-th at kLitCloudStepDeg * k * (k + 1) / 2)
constexpr float kLitCloudFadeStartDeg = 25.0f; // every lit-cloud term fades from this sun elevation
constexpr float kLitCloudFadeEndDeg = 45.0f;   // ... and is gone from this one (the shaders skip the march)
// Weight of the lit-cloud terms at a sun elevation (degrees).
float LitCloudLowSun(float sunElevationDeg);
// Angle of march step k (1..kLitCloudSteps) from the pixel toward the sun, degrees.
float LitCloudStepAngleDeg(int k);

// The sun's light on a sky cloud: the luminance gain, how far the colour goes
// toward the sun's (warm) and the silver lining (in the cloud's own luminance).
struct LitCloudShade
{
    float gain = 1.0f;
    float warm = 0.0f;
    float silver = 0.0f;
};
// The shaders' litCloudTerms (EngineGL33_LightingShadersSky.cpp): the relief
// (density here against the march's mean density toward the sun), the
// darkening of thick cloud seen against the sun, the bellies under a deck and
// the silver lining, all faded by LitCloudLowSun, so from kLitCloudFadeEndDeg
// up the result is identity (gain 1, warm 0, silver 0). `cosViewSun` is the
// cosine between the view ray and the sun, `density` the cloud density here,
// `towardSun` the march's mean density, `feature`, `dark` and `deck` the
// pixel's broken cloud, darkness and deck weight, `skyThrough` the weather's.
LitCloudShade LitCloudTerms(float sunElevationDeg, float cosViewSun, float density, float towardSun, float feature,
                            float dark, float deck, float skyThrough);

// ---- The sky's turn -------------------------------------------------------
// The dome turns this many degrees about the zenith per unit of the sky
// clouds' position (Landscape::CloudsPosition, which grows by the weather's
// cloud speed, 0.2 to 1.0 per second): about 6 degrees a minute at overcast
// 0.5. Pinning the position (triPinCloudDrift) pins the turn too.
constexpr float kSkyTurnDegPerCloudsPos = 0.3f;
// The dome's u offset at a clouds position, in [0, 1) (u wraps every 180
// degrees of azimuth).
float SkyDomeTurnU(float cloudsPosition);

// ---- The shadow anchor -----------------------------------------------------
// The dome is centred on the camera, so a shadow projected from the camera
// would slide over the ground with every step the player takes. The
// projection is seen from an anchor instead, which stays put until the camera
// is kSkyShadowAnchorRadius away; then it moves that far toward the camera and
// the shadows cross-fade from the old anchor's over kSkyShadowFadeSeconds. A
// camera move of kSkyShadowAnchorJump or more in one step is a cut (a
// teleport, a new view): the anchor snaps to the camera without a fade.
constexpr float kSkyShadowAnchorRadius = 1500.0f;
constexpr float kSkyShadowAnchorJump = 500.0f;
constexpr float kSkyShadowFadeSeconds = 4.0f;

struct SkyShadowAnchor
{
    bool valid = false;
    float x = 0.0f; // world metres (east, north)
    float z = 0.0f;
    // The previous anchor sits kSkyShadowAnchorRadius from this one at this
    // angle (radians, atan2(dz, dx)); its shadows weigh prevWeight (0 = no fade).
    float prevAngle = 0.0f;
    float prevWeight = 0.0f;
    float lastCamX = 0.0f;
    float lastCamZ = 0.0f;

    // The anchor at the camera, no fade.
    void Snap(float camX, float camZ);
    // One step of `dt` seconds with the camera at (camX, camZ). `hold` keeps
    // the anchor where it is (captures, while the clouds are pinned); a cut
    // still snaps.
    void Update(float camX, float camZ, float dt, bool hold);
    float PrevX() const;
    float PrevZ() const;
};

} // namespace Poseidon::remaster
