#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <cstdint>

// Photoreal light and camera (look lane "light", feature switch photoLight).
// Pure math, no GL, no engine state; the GL33 backend runs it
// (EngineGL33_Camera.cpp) and the shaders mirror the constants below.
//
// The remastered world used to tonemap every fragment (Khronos Neutral without
// its toe, identity below 0.8) after an exposure meter that held sunlit ground
// at the classic brightness: the frame stayed as flat as the original. With
// the switch on:
//
//   - the light rig is physical (LightRig.cpp ComputePhysicalRig): the sun and
//     the sky fill come from the sky table's atmosphere (Atmosphere.hpp) in
//     their real ratio and colours (ComputePhysicalSky integrates the clear
//     sky over the hemisphere for an up-facing and two vertical surfaces), so a
//     noon sun out-lights the blue sky fill about six to one, a low sun is
//     golden and much dimmer, and the clear sky dome and the fog take the
//     physical sky's brightness against the sunlit ground;
//   - the world's passes write scene light, not a tonemapped colour: linear
//     light up to kEncodeKnee as it is, brighter light folded into the codes
//     above it (Encode), so the post composite can unfold it (Decode);
//   - a camera in the post composite: a metered exposure from a log-luminance
//     histogram of the frame (Meter: the darkest kMeterLowCut and the
//     brightest kMeterHighCut of the weighted samples ignored, the centre
//     weighted, the sky counted less), eye adaptation over about a second
//     (AdaptExposure), a white balance that adapts to the light the way a
//     camera does (neutral at noon, golden hour kept warm, blue hour blue;
//     WhiteBalanceStrength), a scene saturation, and a filmic tone curve with
//     a real shoulder that keeps hue and saturation up to the highlights and
//     then goes to white (Lottes 2016; ToneCurve), so the sun, the sky near it
//     and glints read as bright light and the shade keeps its colour.

namespace Poseidon::remaster
{

// ---- The world pass's encoding (shader camEncode / camDecode) --------------
// Linear light up to the knee is stored as it is (display gamma 2.2); the
// peak channel above it is folded as knee + (1 - knee) t / (1 + t), t = (m -
// knee) / (1 - knee), hue kept. Exactly invertible; Decode caps the unfolded
// peak at kDecodeMax (an 8-bit top code).
constexpr float kEncodeKnee = 0.6f;
constexpr float kDecodeMax = 64.0f;
float EncodePeak(float linearPeak);
float DecodePeak(float encodedPeak);
// Linear light to the stored display value and back (per pixel, hue kept).
Rgb CameraEncode(const Rgb& linear);
Rgb CameraDecode(const Rgb& display);

// ---- Metering ---------------------------------------------------------------
// The meter pass writes kMeterWidth x kMeterHeight texels: the weighted mean
// log2 luminance of its block of the frame (scene light, decoded) and the
// block's weight (centre weighted; sky pixels count kMeterSkyWeight).
constexpr int kMeterWidth = 64;
constexpr int kMeterHeight = 36;
constexpr float kMeterSkyWeight = 0.6f;
constexpr float kMeterLowCut = 0.05f;    // darkest share of the weight ignored
constexpr float kMeterHighCut = 0.03f;   // brightest share ignored
constexpr float kMeterHighlight = 0.97f; // the percentile of the blocks' brightest taps the guard reads
constexpr float kMeterFloorLog2 = -16.0f;

struct MeterResult
{
    bool valid = false;
    float mean = 0.0f;      // weighted mean log2 luminance between the cuts
    float low = 0.0f;       // log2 luminance at kMeterLowCut
    float high = 0.0f;      // at 1 - kMeterHighCut
    float highlight = 0.0f; // at kMeterHighlight
    float weight = 0.0f;    // total weight
};
// `logLum` and `weight` hold n samples (texels of the meter pass); `maxLog`
// (or nullptr: logLum) each sample's brightest tap, which the highlight
// percentile reads, so a small bright patch (the sky at the sun) is seen.
MeterResult Meter(const float* logLum, const float* weight, const float* maxLog, int n);

// ---- Exposure ---------------------------------------------------------------
// The camera maps metered scene light to the curve's input: the frame's
// metered mean goes to middle grey (kMiddleGrey) times 2^compensation. The
// compensation: the knob camEv; a scene darker than a sunny day
// (kDaylightMeanLog2) keeps kDarkKeep of how much darker it is (evening, an
// overcast day and interiors stay darker than noon, as the eye sees them,
// rather than all being pulled to one grey); at night camNight more. A
// highlight guard pulls the exposure down so the kMeterHighlight percentile
// stays below kHighlightLimit (a sunset sky is not blown out to rescue the dark
// ground). The result is clamped to kExposureMinLog2 .. kExposureMaxLog2.
constexpr float kMiddleGrey = 0.18f;
constexpr float kDaylightMeanLog2 = -2.3f;
constexpr float kDarkKeep = 0.4f;
// A scene brighter than that keeps this much of how much brighter it is (a
// sunlit plain is a bright picture, not a grey one).
constexpr float kBrightKeep = 0.4f;
constexpr float kHighlightLimit = 2.2f; // scene light after exposure, in the curve's input units
constexpr float kHighlightGuard = 0.6f; // how far the guard pulls toward its limit
constexpr float kExposureMinLog2 = -3.0f;
constexpr float kExposureMaxLog2 = 2.5f;
// At night the exposure may brighten the frame at most this far.
constexpr float kExposureNightMaxLog2 = 0.0f;
float TargetExposureLog2(const MeterResult& m, float nightEffect, const LightingTuning& t);
// How much of the night the camera's exposure and colour treat the frame as
// (the night compensation, the night's exposure limit, the white balance's
// fade and the cool night white): 0 down to kCameraNightStartDeg of sun, 1
// from kCameraNightFullDeg. Classic's own night effect is 1 as soon as the
// sun is under the horizon, which would hold the blue hour as dark as night.
constexpr float kCameraNightStartDeg = -2.0f;
constexpr float kCameraNightFullDeg = -10.0f;
float CameraNight(float sunElevationDeg);
// One step of the eye's adaptation (log2 exposure), `dt` seconds, toward
// `target`, with time constant `seconds` (opening up to the dark takes 1.5 x
// as long as closing down to the light).
float AdaptExposureLog2(float current, float target, float dt, float seconds);

// ---- Tone curve (Lottes 2016, "Advanced techniques and optimization of HDR
// color pipelines") --------------------------------------------------------
// y = x^a / (x^(a d) b + c), through (midIn, midOut) and (white, 1); a > 1 is
// the toe (deeper, richer shadows), d < 1 the shoulder. The hue-keeping form
// applies it to the peak channel and keeps the channel ratios, which go to
// white as the output nears 1 (kCrosstalk), so a bright sky stays blue and a
// low sun stays orange until it is truly bright.
constexpr float kCurveMidOut = 0.18f;
constexpr float kCrosstalk = 4.0f;
struct ToneCurve
{
    float a = 1.0f, d = 1.0f, b = 0.0f, c = 1.0f;
};
// `white` is in units of middle grey (scene light kMiddleGrey * white maps to 1).
ToneCurve MakeToneCurve(float contrast, float shoulder, float white);
float ApplyToneCurve(const ToneCurve& curve, float x);
// The hue-keeping form on a colour (linear in, linear display out).
Rgb ApplyToneCurveHueKeeping(const ToneCurve& curve, const Rgb& c);

// ---- White balance and colour ----------------------------------------------
struct Mat3
{
    float m[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1}; // row major
    Rgb Apply(const Rgb& c) const;
};
Mat3 Multiply(const Mat3& a, const Mat3& b);
// Bradford adaptation in linear sRGB: maps the illuminant's colour toward
// white by `strength` (0 none, 1 full), keeping luminance of white.
Mat3 WhiteBalance(const Rgb& illuminant, float strength);
// Scene saturation about luminance.
Mat3 Saturation(float s);
// The camera's white balance is set for daylight: the light of the clear sky
// with the sun at kPhysicalReferenceSunDeg (PhysicalSky::referenceIlluminant)
// is white. On top of that it follows the light it is in by this share
// (WhiteBalanceShare): half of it at a high sun (an overcast day greys rather
// than going blue), little at a low sun (golden hour stays golden, the blue
// hour blue). At night it fades out (the night eye handles the moon).
float WhiteBalanceShare(float sunElevationDeg);
// At night the camera is balanced for a light warmer than the night's (a
// tungsten-ish white), by this much at full night, so moonlight reads cool
// while lamps and fires keep some warmth.
inline const Rgb kNightCameraWhite{1.0f, 0.9f, 0.75f};
constexpr float kNightCameraCool = 0.45f;
// The light the balance makes white: the daylight reference and the light
// the camera is in, mixed by the share (geometric, per channel, at unit
// luminance).
Rgb WhiteBalanceTarget(const Rgb& reference, const Rgb& current, float share);

// ---- The physical sun and sky (from the sky table's atmosphere) ------------
struct AtmosphereTables;
// The clear sky's light at the viewer for a sun at this elevation (radians):
// the direct sun's transmittance and the sky irradiance on an up-facing and
// two vertical surfaces, integrated over the hemisphere on a coarse grid of
// the sky-view parametrisation (CPU, about a millisecond). No reference.
PhysicalSky ComputePhysicalSky(const AtmosphereTables& t, float sunElevationRad);
// The light on an open up-facing surface under that sky (sun plus sky).
Rgb PhysicalIlluminant(const PhysicalSky& p);
// Recompute when the sun moved this far.
constexpr float kPhysicalSkyRebuildDeg = 0.25f;

// ---- The sun disk -----------------------------------------------------------
// The sun disk's pre-lit sprite is display white at best; the camera takes it
// as light this many times brighter (it blooms and holds its colour).
constexpr float kSunDiskGain = 3.0f;
// The scene light (after exposure) where bloom starts, and its knee: light
// past the curve's white.
constexpr float kCameraBloomThreshold = 3.0f;
constexpr float kCameraBloomKnee = 1.0f;

} // namespace Poseidon::remaster
