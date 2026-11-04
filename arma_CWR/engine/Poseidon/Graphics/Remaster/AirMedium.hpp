#pragma once

#include <functional>

// The air of the remastered lighting: pure math, no GL, no engine state.
//
// Three parts of one medium, each behind its own feature switch:
//
//   - aerial haze (airHaze): thin air whose density falls off with height
//     (scale height kHazeScaleHeight), so low ground is hazier than ridges.
//     Beyond kHazeNearDistance a surface fades toward the sky behind it,
//     C' = C T + S, with a slightly stronger extinction of blue than of red
//     (kHazeChroma). The haze has no sun lobe of its own: the sky it fades
//     toward already holds the glow toward the sun. Air that a ridge or a
//     cloud shades (the march's sun visibility) keeps only a dimmer
//     zenith-coloured sky light, kHazeShadowAmbient of the lit air's, so the
//     ridge's shadow in the haze stays grey. The classic fog then keeps only
//     the view-distance edge: its start moves out to kHazeEdgeFraction of its
//     end in clear weather (FogEdgeScale);
//   - valley mist (valleyMist): a flat-topped layer kValleyMistTop metres
//     above the lowest ground within about 850 m (LocalLowGround), so it
//     pools on valley floors and water and leaves ridges clear. It forms with
//     an eastern sun from just before sunrise, is full up to
//     kMorningFadeSunDeg and gone by kMorningGoneSunDeg (MorningWeight),
//     thins under an overcast and also lies in weather fog;
//   - beams (airBeams): single scattering of the sun (and of headlights) in a
//     ground mist, marched at half resolution with the sun's visibility from
//     the shadow map, the terrain horizon and the cloud field, so crowns,
//     roofs and ridges cut light shafts out of it. The mist is thin in clear
//     weather and thick in setFog and rain (fans of god rays). The shafts'
//     contrast works on the share of the scattering that sees the sun
//     (BeamShaft), so a ray that only runs longer through the mist, as over
//     open ground toward the horizon, is not boosted into a band.
//
// The shaders (EngineGL33_PostAirShaders.cpp) mirror the optical-depth math
// below; the weather and time rules live only here and reach them as numbers
// (ComputeAirMedium). AirOpticalDepth gives the same optical depth for a line
// of sight.

namespace Poseidon::remaster
{

// Haze.
constexpr float kHazeSigma = 1.1e-4f;         // extinction at sea level, green (1/m): 35 km visibility
constexpr float kHazeChromaR = 0.85f;         // per-channel extinction relative to green
constexpr float kHazeChromaB = 1.2f;          // (red thins out last, blue first)
constexpr float kHazeScaleHeight = 350.0f;    // m
constexpr float kHazeNearDistance = 150.0f;   // m: the near field is untouched
constexpr float kHazeShadowAmbient = 0.55f;   // sky light of shaded air relative to lit air's (the apply)
constexpr float kHazeFogSigmaGain = 3.0f;     // setFog 1 multiplies the haze by 1 + this
constexpr float kHazeRainSigmaGain = 1.0f;    // rain density 1 multiplies the haze by 1 + this
constexpr float kHazeEdgeFraction = 0.55f;    // classic fog start, as a share of its end, in clear weather
constexpr float kHazeClearFogEndMin = 900.0f; // fog ends nearer than this: the classic fog unchanged
constexpr float kHazeClearFogEndMax = 2000.0f;
constexpr int kHazeVisSteps = 12; // sun visibility samples along the haze's part of the ray

// Valley mist.
constexpr float kValleyMistSigma = 2.5e-3f; // 1/m at full strength (1.2 km visibility inside it)
constexpr float kValleyMistTop = 40.0f;     // m above the local low ground (varied by noise banks)
constexpr float kValleyMistFall = 25.0f;    // m: density e-fold above the top
constexpr float kValleyNearRadius = 400.0f; // LocalLowGround's two rings
constexpr float kValleyFarRadius = 850.0f;
constexpr float kValleyDepthFrom = 20.0f;  // m the rings' mean ground stands above the floor: no mist below
constexpr float kValleyDepthFull = 60.0f;  // full mist from here (a plain or the open sea pools none)
constexpr float kMorningMinSunDeg = -4.0f; // the mist forms from here
constexpr float kMorningFullSunDeg = 1.0f;
constexpr float kMorningFadeSunDeg = 6.0f; // and burns off between these (about an hour after sunrise)
constexpr float kMorningGoneSunDeg = 10.0f;
constexpr float kMistFogFrom = 0.05f; // setFog that lays mist in the valleys at any hour
constexpr float kMistFogFull = 0.3f;

// Beams.
constexpr float kBeamMistSigma = 0.004f;    // ground mist extinction per unit of mist scale (1/m)
constexpr float kBeamMistHeight = 18.0f;    // m: ground mist scale height
constexpr float kBeamMistClear = 0.12f;     // mist scale in clear air
constexpr float kBeamMistMisty = 1.5f;      // in fog, rain, at sunrise and sunset
constexpr float kBeamMistHeavyFog = 3.0f;   // more per unit of setFog above kBeamFogFull
constexpr float kBeamGainClear = 4.0f;      // shaft contrast gain in clear air
constexpr float kBeamGainMisty = 2.5f;      // and in mist
constexpr float kBeamHighpassClear = 0.85f; // share of the smooth glow the contrast takes out, clear air
constexpr float kBeamHighpassMisty = 0.6f;  // and in mist
constexpr float kBeamDistance = 250.0f;     // m: the march length (about the shadow map's reach)
constexpr int kBeamSteps = 24;              // per half-resolution pixel (TAA averages the jitter)
constexpr float kBeamG = 0.75f;             // forward lobe asymmetry; 0.15 of a back lobe at -0.3
constexpr float kBeamFogFrom = 0.02f;       // setFog that starts to thicken the beam mist
constexpr float kBeamFogFull = 0.1f;        // and makes it misty
constexpr float kBeamRainFrom = 0.1f;       // rain density likewise
constexpr float kBeamRainFull = 0.5f;
constexpr float kBeamLowSunDeg = 6.0f;   // sun elevation below which the mist thickens (sunrise, sunset)
constexpr int kAirMaxSpots = 4;          // headlight cones per frame
constexpr float kAirSpotRange = 150.0f;  // m from the camera
constexpr float kAirSpotStrength = 3.0f; // cone scattering gain (2 soft .. 4 strong)
constexpr int kAirSpotSteps = 24;        // march steps through a cone

float SmoothStep(float edge0, float edge1, float x);

// What decides the medium; the backend fills it once per frame.
struct AirInputs
{
    float sunElevationDeg = 90.0f;
    float sunEast = 0.0f;    // x (east) of the unit direction toward the sun
    float fog = 0.0f;        // setFog, 0..1
    float rain = 0.0f;       // rain density, 0..1
    float skyThrough = 1.0f; // 1 clear .. 0 overcast
    float fogStart = 0.0f;   // the classic display fog (m)
    float fogEnd = 0.0f;
};

// Which parts run this frame: each part's switch and frame gate.
struct AirParts
{
    bool haze = false;
    bool valleyMist = false;
    bool beams = false;
};

// The medium of one frame. Zero densities mean "not this frame".
struct AirMedium
{
    float hazeSigma = 0.0f;      // 1/m at sea level, green
    float valleyMist = 0.0f;     // 1/m at full strength
    float beamMist = 0.0f;       // 1/m at the ground
    float beamAir = 0.0f;        // the beams' air, 1/m at sea level (the haze's density, whatever its switch)
    float beamGain = 1.0f;       // shaft contrast gain (1 = none)
    float beamHighpass = 0.0f;   // share of the smooth glow the contrast takes out
    float fogEdgeScale = 1.0f;   // classic fog factor f becomes min(1, f * this)
    float fogStart = 0.0f;       // the classic fog's start as the world draws apply it (moved out with the haze)
    float fogEnd = 0.0f;         // and its end (m)
    float beamMistWeight = 0.0f; // 0 clear .. 1 misty (for logs)
};

// Morning weight of the valley mist: an eastern sun from just below the
// horizon until it burns off.
float MorningWeight(float sunElevationDeg, float sunEast);
// 0 in clear air .. 1 misty: setFog, rain and a sun at the horizon.
float BeamMistWeight(const AirInputs& in);
// The classic fog factor's scale when its start moves out to
// kHazeEdgeFraction of its end (clear weather only; 1 below kHazeClearFogEndMin).
// With f = clamp(1 - (d - start) / (end - start)), min(1, f * scale) is exactly
// the factor of the moved start for every distance d.
float FogEdgeScale(float fogStart, float fogEnd);
// The moved start itself (fogStart when nothing moves it).
float FogEdgeStart(float fogStart, float fogEnd);
// `gainKnob` below 0 takes the gain from the weather (kBeamGainClear in clear
// air, kBeamGainMisty in mist); 0 or more is used as given.
AirMedium ComputeAirMedium(const AirInputs& in, const AirParts& parts, float gainKnob);

// The sun beams' contrast at one pixel (the apply's beam step, which runs the
// same steps on a vec2 with the headlight cones). `lit` is the ray's
// scattering that sees the sun, `open` the same ray's scattering as if all of
// it did, `meanLit` and `meanOpen` their depth-matched local means, `conf` the
// upsample's depth match (0..1). The contrast works on the share that sees the
// sun, lit / open, against the mean share: what stands above it is boosted by
// gain - 1, the rest loses up to `highpass` of itself but never more, and the
// result goes back to scattering (times open). A field that sees the sun all
// over keeps (1 - highpass) of its scattering, however that varies with the
// ray's length through the mist.
float BeamShaft(float lit, float open, float meanLit, float meanOpen, float conf, float highpass, float gain);

// Optical length (m, at sea-level density) of a straight segment from height
// yA to yB, `len` metres long, through air of density exp(-y / H): closed form.
float ExpOpticalLength(float yA, float yB, float len, float scaleHeight);
// Optical length (m, at full density) of the segment through a layer of
// density min(1, exp(-(y - top) / fall)): flat-topped mist with a soft upper edge.
float LayerOpticalLength(float yA, float yB, float len, float top, float fall);

// Valley floor under a point: the lowest ground of the point and two rings of
// taps (8 at kValleyNearRadius, 6 at kValleyFarRadius), never below sea level.
float LocalLowGround(const std::function<float(float x, float z)>& heightAt, float x, float z);
// The same floor and how deep the valley is there: the mean of the 14 ring taps
// above the floor. The mist's density takes ValleyDepthWeight of it.
struct LocalValley
{
    float low = 0.0f;
    float depth = 0.0f;
};
LocalValley LocalValleyAt(const std::function<float(float x, float z)>& heightAt, float x, float z);
float ValleyDepthWeight(float depth);
// The mist's top and density factor at a point from the low ground and the
// two noise banks n1 (1700 m) and n2 (520 m) in 0..1.
float ValleyMistTop(float lowGround, float n1, float n2);
float ValleyMistDensity(float n1, float n2);

// Green optical depth of the haze and the valley mist along the line of
// sight from `eye` to `target` (x, y, z world, y up), from `fromDistance`
// metres on (the visual haze starts at kHazeNearDistance; a line of sight for
// AI starts at 0). `mistTop` is the valley mist's top at the target.
float AirOpticalDepth(const AirMedium& m, const float eye[3], const float target[3], float fromDistance, float mistTop,
                      float mistDensity = 1.0f);

} // namespace Poseidon::remaster
