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
//     the sky fill come from the sky table's atmosphere (Atmosphere.hpp; under
//     the camera with a summer haze, camHaze) in their real ratio and colours
//     (ComputePhysicalSky integrates the clear sky over the hemisphere for an
//     up-facing and two vertical surfaces), so a noon sun out-lights the blue
//     sky fill several times, a low sun is golden and much dimmer, and the
//     clear sky dome, the fog and the sky clouds are the table's own sky at the
//     rig's scale (a deep zenith over a pale bright horizon);
//   - the world's passes write scene light, not a tonemapped colour, at the
//     camera's current exposure (pre-exposure, so the stored frame is near
//     display level and the passes that blend into it or read it keep their
//     space): linear light up to kEncodeKnee as it is, brighter light folded
//     into the codes above it on a near-logarithmic scale (Encode), so the
//     post composite can unfold it (Decode). Pre-lit draws (particles, fire,
//     the far horizon) carry the original game's display colour and are
//     stored as they are, not exposed again (a fire keeps its hue); the sun's
//     sprite is replaced by a small disc in the sunlight's colour;
//   - a camera in the post composite: a metered exposure from a log-luminance
//     histogram of the frame (Meter: the darkest kMeterLowCut and the
//     brightest kMeterHighCut of the weighted samples ignored, the centre
//     weighted, the sky counted less) with guards on the highlights, the sky's
//     bright end and the sky by a low sun in frame, eye adaptation over about
//     a second (AdaptExposure), a white balance that adapts to the light the
//     way a camera does (neutral at noon, golden hour kept warm, blue hour
//     blue; WhiteBalanceShare), a scene saturation, the night's rod shift,
//     and a filmic tone curve with a real shoulder that keeps hue and
//     saturation up to the highlights and then goes to white (Lottes 2016;
//     ToneCurve), so the sun, the sky near it and glints read as bright light
//     and the shade keeps its colour.

namespace Poseidon::remaster
{

// ---- The world pass's encoding (shader camEncode / camDecode) --------------
// Pre-exposed linear light (scene light times the camera's exposure of the
// frame it was drawn in, so middle grey is near 0.18) up to the knee is stored
// as it is (display gamma 2.2). Above it the peak channel's code follows
// cK + (1 - cK) (1 - (knee / m)^p) / (1 - (knee / max)^p), cK the knee's
// code: the code steps grow smoothly with the light (about 16 codes from 1 to
// 2, 10 from 2 to 4, 10 from 4 to the top), where the old fold t / (1 + t)
// left the top five codes for 4.9 to 64 and an edge or a blend across them
// jumped by 1.5 to 3.4 x (a ragged white sun, white fire). Hue kept (the
// other channels keep their linear ratio to the peak). Exactly invertible;
// light above kDecodeMax stores as the top code.
constexpr float kEncodeKnee = 0.6f;
constexpr float kDecodeMax = 16.0f;
constexpr float kEncodePower = 0.7f;
// The peak channel's stored code (display, 0..1) and back.
float EncodePeakCode(float linearPeak);
float DecodePeakCode(float code);
// Linear light to the stored display value and back (per pixel, hue kept).
Rgb CameraEncode(const Rgb& linear);
Rgb CameraDecode(const Rgb& display);

// ---- Metering ---------------------------------------------------------------
// The meter pass writes kMeterWidth x kMeterHeight texels: the weighted mean
// log2 luminance of its block of the frame (scene light, decoded) and the
// block's weight (centre weighted; sky pixels count kMeterSkyWeight).
constexpr int kMeterWidth = 64;
constexpr int kMeterHeight = 36;
constexpr float kMeterSkyWeight = 0.4f;
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
    // The brightest tap of the sky by a low sun in frame (SunRingPeak; log2),
    // and how much the guard on it counts (0: no sun in frame, or a high one).
    float sunRing = kMeterFloorLog2;
    float sunRingWeight = 0.0f;
    // The bright end of the sky itself (SkyTopPeak; log2): the sky seen
    // through a canopy, a hazy horizon, the glow by a sun just out of frame.
    float skyTop = kMeterFloorLog2;
};
// `logLum` and `weight` hold n samples (texels of the meter pass); `maxLog`
// (or nullptr: logLum) each sample's brightest tap, which the highlight
// percentile reads, so a small bright patch (the sky at the sun) is seen. The
// highlight percentile counts every block that has any weight alike: the sky
// seen through a canopy and the sky by a low sun at the frame's edge count as
// much as the centre (they are what blows out).
MeterResult Meter(const float* logLum, const float* weight, const float* maxLog, int n);
// The sky by the sun: the brightest tap (maxLog) of the blocks of the
// kMeterWidth x kMeterHeight grid (row 0 at the bottom) whose centres lie
// between kSunRingInner and kSunRingOuter frame heights from the sun's place
// (uv, v up; `aspect` width over height), outside the disc and the blocks it
// touches. kMeterFloorLog2 when none.
constexpr float kSunRingInner = 0.07f;
constexpr float kSunRingOuter = 0.16f;
float SunRingPeak(const float* maxLog, const float* weight, float sunU, float sunV, float aspect);
// The sky's bright end: the kSkyTopPercentile of the blocks' brightest sky tap
// (skyMaxLog; a block with no sky counts as none), leaving out the blocks
// within kSunRingInner of a sun in front of the camera (sunU, sunV; `sunOn`
// false: no sun to leave out), whose glare may clip.
constexpr float kSkyTopPercentile = 0.995f;
float SkyTopPeak(const float* skyMaxLog, bool sunOn, float sunU, float sunV, float aspect);
// The guard's weight for a sun at this elevation (degrees): full under
// kSunRingFullDeg, none above kSunRingOffDeg.
constexpr float kSunRingFullDeg = 15.0f;
constexpr float kSunRingOffDeg = 30.0f;
float SunRingWeight(float sunElevationDeg);

// ---- Exposure ---------------------------------------------------------------
// The camera maps metered scene light to the curve's input: the frame's
// metered mean goes to middle grey (kMiddleGrey) times 2^compensation. The
// compensation: the knob camEv; a scene darker than kDaylightMeanLog2 (a
// sunlit scene of average tone; a bright plain or desert meters about 0.9
// above it) keeps kDarkKeep of how much darker it is (evening, an overcast day
// and interiors stay darker than noon, as the eye sees them, rather than all
// being pulled to one grey), a brighter one kBrightKeep of how much brighter;
// at night camNight more. Guards pull the exposure down: the kMeterHighlight
// percentile of the blocks' brightest taps toward kHighlightLimit, the sky's
// bright end toward kSkyTopLimit and the sky by a low sun in frame to
// kSunRingLimit (a sunset sky is not blown out to rescue the dark ground).
// The result is clamped to kExposureMinLog2 .. kExposureMaxLog2.
constexpr float kMiddleGrey = 0.18f;
constexpr float kDaylightMeanLog2 = -3.0f;
constexpr float kDarkKeep = 0.4f;
// Around sunset (CameraDuskKeep) a dark scene keeps up to kDuskDarkKeep of
// how much darker it is (look director round 2: a firefight 2 degrees after
// sunset read as a pale salt flat at kDarkKeep, its ground brighter than the
// sky at the horizon; the ground under a dusk sky is far darker than the
// sky, and the camera exposes for the sky).
constexpr float kDuskDarkKeep = 0.62f;
// A scene brighter than that keeps this much of how much brighter it is:
// negative, it is exposed under the average scene's grey (look director
// round 2: the pale Abel fields and the Sinai sand at noon stayed near-white
// paper at +0.15; a photographer exposes a bright plain for its highlights,
// so the ground gets its tonal range and texture, about 0.3 to 0.4 EV under).
// Look fix "noon" (round 4): at -0.45 a bright plain went 0.38 EV under on
// top of the high sun's kCameraHighSunEv, 0.63 EV in all: the Sinai sand sat
// 35 levels under the original (149 against 186) and read dull rather than
// sun-baked, and the clear sky over it went slate with it. With the high
// sun's step a bright plain is still about 0.3 EV under the average scene.
constexpr float kBrightKeep = -0.05f;
// The guard: the kMeterHighlight percentile of the blocks' brightest taps
// stays at this scene light after exposure (the curve shows 1.3 at about 0.8
// of white: a bright sky, the glow by a low sun, the sky between crowns keep
// their colour), pulled that far.
constexpr float kHighlightLimit = 1.3f;
constexpr float kHighlightGuard = 0.85f;
// The sky by a low sun in frame (MeterResult::sunRing) stays at this scene
// light after exposure (the curve shows it a little under white, golden, not
// blown); the foreground goes darker, as a photograph into the sun does.
constexpr float kSunRingLimit = 2.0f;
// The sky's bright end (MeterResult::skyTop) stays at this scene light after
// exposure (under the curve's white, so a gap in a canopy or a hazy horizon
// is pale sky, not a white hole), pulled this far.
constexpr float kSkyTopLimit = 2.2f;
constexpr float kSkyTopGuard = 0.7f;
constexpr float kExposureMinLog2 = -3.0f;
// Up to 5 EV: a civil twilight or a dark wood is far darker than noon (the
// highlight guards hold whatever is bright in it).
constexpr float kExposureMaxLog2 = 5.0f;
// At night the exposure may brighten the frame at most this far: a moonlit
// street stays a night (ground near luma 30, not an even grey day for night),
// and the lamps and fires (camLocal) stand out of it.
constexpr float kExposureNightMaxLog2 = -1.0f;
// `dusk` (CameraDusk, 0..1) takes kCameraDuskEv off: a dusk or a dawn before
// the sun is up stays a dim picture (the meter alone lifted a firefight at
// sunset to a grey noon).
// `duskKeep` (CameraDuskKeep, 0..1) raises the dark keep toward kDuskDarkKeep.
// `timeEv` (CameraTimeOfDayEv) is added (EV).
float TargetExposureLog2(const MeterResult& m, float nightEffect, const LightingTuning& t, float dusk = 0.0f,
                         float duskKeep = 0.0f, float timeEv = 0.0f);
// The golden hour (look director round 2: a 12 degree sun read as a neutral
// mid-afternoon): the camera takes kCameraGoldenEv off (the low sun is a
// dimmer light, the picture a darker, warmer one) and balances a little warm
// (kCameraGoldenWarm, a photographer's "shade" setting on a daylight film),
// full from 5 to 16 degrees of sun, none from 32 or under 1.
constexpr float kCameraGoldenEv = -0.7f;
inline const Rgb kCameraGoldenWarm{1.1f, 1.0f, 0.76f};
float CameraGolden(float sunElevationDeg);
// The warm balance's gain per channel at golden share `golden` (0..1):
// kCameraGoldenWarm at unit luminance, that far.
Rgb GoldenWarmGain(float golden);
// The golden hour's warmth is the low sun's light: the composite lays it on
// what that light shows and leaves the blue alone (look fix "fire": over the
// whole frame it turned the clear sky and the clouds teal-grey, (129, 148,
// 163) on forest_edge_golden, and the picture read as a dull afternoon).
// The share a colour takes: none at a warmth (r - b) / peak of
// kGoldenShareFrom (a blue sky), all from kGoldenShareTo (a sunlit field; a
// grey cloud takes most of it). A wide ramp: from -0.25 to 0.1 the sunrise
// sky drew an edge round the sun's glow where it met the blue.
constexpr float kGoldenShareFrom = -0.7f;
constexpr float kGoldenShareTo = 0.4f;
float GoldenWarmShare(const Rgb& c);
// A high sun under a clear sky (look director round 2: the noon ground stayed
// near-white paper): kCameraHighSunEv from 35 degrees, full from 55, gone
// under the weather's veil.
constexpr float kCameraHighSunEv = -0.25f;
// The time of day's EV: the golden hour's and the high sun's.
float CameraTimeOfDayEv(float sunElevationDeg, float veil);
// The weight of kDuskDarkKeep: full from the sun at the horizon to 3 degrees
// under it, none from 5 degrees up or 6 under (the blue hour keeps its own
// exposure).
float CameraDuskKeep(float sunElevationDeg);
// How much of the night the camera's exposure and colour treat the frame as
// (the night compensation, the night's exposure limit, the white balance's
// fade and the night's rod shift): 0 down to kCameraNightStartDeg of sun, 1
// from kCameraNightFullDeg. Classic's own night effect is 1 as soon as the
// sun is under the horizon, which would hold the blue hour as dark as night;
// civil twilight (to 6 degrees under) still lights the ground.
constexpr float kCameraNightStartDeg = -4.5f;
constexpr float kCameraNightFullDeg = -12.0f;
float CameraNight(float sunElevationDeg);
// The dusk's exposure (look director): EV taken off as the sun goes from
// kCameraDuskStartDeg down to kCameraDuskFullDeg, ahead of the night's own
// compensation (CameraNight), so the light visibly fails at sunset and the
// blue hour reads as evening, not as an overcast noon.
constexpr float kCameraDuskStartDeg = 4.0f;
constexpr float kCameraDuskFullDeg = -4.0f;
constexpr float kCameraDuskEv = -1.25f;
float CameraDusk(float sunElevationDeg);
// One step of the eye's adaptation (log2 exposure), `dt` seconds, toward
// `target`, with time constant `seconds` (opening up to the dark takes 1.5 x
// as long as closing down to the light).
float AdaptExposureLog2(float current, float target, float dt, float seconds);

// ---- Tone curve (Lottes 2016, "Advanced techniques and optimization of HDR
// color pipelines") --------------------------------------------------------
// y = x^a / (x^(a d) b + c), through (midIn, midOut) and (white, 1); a > 1 is
// the toe (deeper, richer shadows), d < 1 the shoulder. The hue-keeping form
// applies it to the peak channel and keeps the channel ratios. They go to
// white late: as the output nears 1 (y^kCrosstalk) and with the light past
// the curve's white (kWhiteExcessStops stops past it is white), so a bright
// sky stays blue, a flame orange and a low sun orange-gold around a white
// core.
constexpr float kCurveMidOut = 0.18f;
constexpr float kCrosstalk = 8.0f;
constexpr float kWhiteExcessStops = 2.5f;
// Saturated light goes to white less (look director round 2: a fire's flame,
// its codes saturated by the additive stack, read as a white slab): the
// whitening above is scaled by 1 - kChromaKeep * chroma^2, chroma = 1 - the
// smallest channel over the peak (a white sun 0, a flame about 0.9), so a
// flame rolls off in hue to a yellow-orange core and the noon sun still clips
// white.
constexpr float kChromaKeep = 0.8f;
// Each channel goes to white by the whitening to the power 1 + kHueRoll * (1 -
// its share of the peak) (look fix "burst"): the channels the light holds
// less of follow later, as film's dyes do, so a flame's core and the stones in
// its light roll off to yellow-orange and orange instead of cream (one mix for
// all three turned (1, 0.55, 0.16) into (1, 0.68, 0.38)); a white light is
// unchanged.
constexpr float kHueRoll = 3.0f;
struct ToneCurve
{
    float a = 1.0f, d = 1.0f, b = 0.0f, c = 1.0f;
    float white = 1.0f; // the scene light that reaches 1
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
// (WhiteBalanceShare): about half of it at a high sun (an overcast day greys
// rather than going blue), almost none under 8 degrees of sun (a camera on
// daylight balance: the golden hour's sunlit ground and grass take the sun's
// amber, the shade the sky's blue, the blue hour stays blue). At night it
// fades out: the camera stays on daylight, so lamps and fires are warm and
// the moonlight is the rig's cool moon.
constexpr float kWhiteBalanceShareLowSun = 0.08f;
constexpr float kWhiteBalanceShareHighSun = 0.35f;
float WhiteBalanceShare(float sunElevationDeg);
// The scene saturation falls toward the low sun by this much (the golden
// light already saturates every warm surface; pines went mustard).
constexpr float kLowSunSaturationDrop = 0.1f;
// The night's rod vision (the composite, scaled by the camera's night): dim
// light below kScotopicLum (scene light after exposure) goes toward a cool
// grey of the same brightness, kScotopicShift at most, so moonlit ground and
// shade read blue-grey while lamp and fire pools, far above it, keep their
// colour.
// Warm light (lamps, fire, the smoke the fire lights) keeps its colour
// whatever its brightness: the shift fades out as a pixel's warmth
// ((r - b) / peak) goes from kScotopicWarmFrom to kScotopicWarmTo, so a plume
// is warm through, not orange particles among grey ones.
constexpr float kScotopicLum = 0.25f;
constexpr float kScotopicShift = 0.7f;
constexpr float kScotopicWarmFrom = 0.08f;
constexpr float kScotopicWarmTo = 0.35f;
inline const Rgb kScotopicTint{0.78f, 0.92f, 1.32f};
// A street lamp's classic ambient (a glow it adds to everything near it,
// facing it or not, which lit the pole evenly along its whole height) keeps
// this share under the camera.
constexpr float kCameraLampAmbient = 0.25f;
// A street lamp's light under the camera at night (on top of camLocal): the
// pool it lays on the road and the walls under it, warm and plain against the
// moonlit street. Its head shades the first metres round the bulb (the
// shader fades the light in from 1.5 to 4.5 m), so its own pole is not lit
// white from top to bottom.
constexpr float kCameraLampGain = 2.0f;
// The light flares (Scene::DrawFlare, drawn without the depth test) keep this
// share of their alpha under the camera at full night: a lamp's halo between
// the full starburst and a bare bulb. By day none: the sun's disc and the
// camera's glare carry the sun, and its flare's rays were the starburst round
// a low sun.
constexpr float kCameraFlareNight = 0.25f;
// The air pass under the camera (B8's apply; the look-fog lane's volume owns
// this once it lands): the haze's density times camAerial (aerial
// perspective); within 500 m the haze's light toward a low sun is capped at
// kCameraNearHazeCap times the horizon sky's light in the sun's side azimuth
// (rising to 8 x by 4 km: near silhouettes stay dark, far ridges glow); the
// sun's beams reach a surface's full strength at kCameraBeamFullDistance and
// keep kCameraSkyBeamsBySun of themselves on the sky by the sun.
constexpr float kCameraNearHazeCap = 1.5f;
constexpr float kCameraSkyBeamsBySun = 0.3f;
constexpr float kCameraBeamFullDistance = 1500.0f;
// At a low sun (full under 15 degrees, none from 45; not at night, not under
// the weather's veil) what the sky lights more than the sun (the world's sky
// light share in the frame's alpha, from 0.2 to full at 0.6) leans toward the
// sky's blue by kCameraShadeCool (the composite's camShade): the shade a
// daylight-balanced camera sees, against the sun's amber on what it lights.
// The pines' self-shaded crowns read green-dark instead of mustard.
constexpr float kCameraShadeCool = 0.04f;
inline const Rgb kCameraShadeTint{0.84f, 1.0f, 1.3f};
// By day the dark tones keep kCameraDarkSaturation of their colour (relative,
// on top of the scene saturation), rising to all of it from kCameraDarkFrom
// to kCameraDarkTo of scene light after exposure (middle grey is 0.18): a
// real camera's shadows carry less chroma than its mid tones, and a pine
// crown's olive shade read mustard (g/b 1.87) without it.
constexpr float kCameraDarkSaturation = 0.97f;
constexpr float kCameraDarkFrom = 0.02f;
constexpr float kCameraDarkTo = 0.2f;
// The weather's veil (CameraWeatherVeil) takes this much off the scene
// saturation at 1: a grey day has less colour.
constexpr float kVeilSaturationDrop = 0.15f;
// How far a low sun (under 5 degrees, none from 25) deepens the blue of the
// upper clear sky and of the sky the sea mirrors (the dome's chroma times 1 +
// this at 30 degrees up and above): the haze's grey otherwise washed the
// sunset's upper sky (133, 131, 133) and the sea with it.
constexpr float kCameraLowSunSkyBlue = 0.6f;
// A clear day's sky as a camera renders it (look fix "noon", round 4; the
// dome's camSkyLook and camSkyDeepen, EngineGL33_LightingShadersSky.cpp). The
// table's clear sky has the real sky's chromaticity (zenith near x 0.25,
// y 0.27) and the hue-keeping curve keeps it exactly, where a camera's colour
// rendition (its per-channel film curve) deepens that blue. Without it the
// noon sky over a bright plain read as slate, (89, 112, 146): the original's
// and landed's hue at a third of their light. The clear sky's chroma about
// its luminance (linear light: same luminance, same dominant hue) is scaled
// by 1 + kCameraDaySkyDeep times DaySkyDeepWeight: by day only (none under 8
// degrees of sun, all from 25), above the horizon band (from a view
// elevation's sine of 0.03 to 0.4, so the horizon stays pale), not in the
// sun's glow (none within about 10 degrees of the sun, all from 30), not
// under the weather's veil. No channel goes under kCameraDaySkyFloor of the
// luminance (no neon blue).
constexpr float kCameraDaySkyDeep = 0.65f;
constexpr float kCameraDaySkyFloor = 0.25f;
// The weight (0..1) at a sun elevation (degrees), the view's sine of
// elevation, its cosine to the sun and the weather's veil.
float DaySkyDeepWeight(float sunElevationDeg, float sinViewElevation, float cosToSun, float veil);
// The clear sky's colour (linear) deepened at that weight.
Rgb DaySkyDeepen(const Rgb& c, float weight);
// Additive pre-lit draws (the fire's flame) at full night: their codes times
// this on top of camAdditive (the flame reads at a glance against the night).
constexpr float kCameraAdditiveNight = 1.0f;
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

// ---- The sky's highlights ---------------------------------------------------
// Under the camera the sky's light at the frame's pre-exposure rolls off from
// kSkyRolloffKnee toward kSkyRolloffCap (curve input; the shader's
// camSkyRolloff), hue kept, so only the sun's disc clips: the aureole round a
// low sun, a silver lining and a hazy horizon become a smooth gradient under
// white. The guards' limits on the sky (kSunRingLimit, kSkyTopLimit) sit
// under the cap, so the meter still sees them.
constexpr float kSkyRolloffKnee = 1.2f;
constexpr float kSkyRolloffCap = 2.4f;
float SkyRolloff(float preExposedPeak);
// Toward the sun the cap rises to kSkySunCap (past the curve's white): a real
// camera burns the sky round the sun white and its glare falls off smoothly.
// With one cap everywhere the aureole round a low sun was a flat cream
// plateau (0.85 of white) out to about 2 degrees, a pale disc inside it. The
// rise is SkySunGlow, 1 / (1 + 2 (1 - cos) / kSkySunGlowRad^2), about
// 1 / (1 + (angle / kSkySunGlowRad)^2): half at kSkySunGlowRad from the sun.
constexpr float kSkySunCap = 10.0f;
constexpr float kSkySunGlowRad = 0.021f; // 1.2 degrees
float SkySunGlow(float cosToSun);
float SkyRolloffCapToward(float cosToSun);
// The roll-off of the sky along a direction with this cosine to the sun.
float SkyRolloffToward(float preExposedPeak, float cosToSun);

// ---- The grey deck ------------------------------------------------------------
// Under the camera a full grey deck (the overcast cloud floor under the
// weather's veil) shows its cloud base: the painted deck's light relative to
// its row's mean is raised to the power 1 + kDeckRelief (the lumps deepen,
// the thin parts brighten), and the deck is kDeckDark of its light, both
// faded in from kDeckFromSin to kDeckFullSin of elevation (the deck over the
// horizon, where the horizon ring measures the far air, keeps its light and
// stays smooth), by day (a night deck is unchanged, the sun from 6 degrees
// under the horizon to 2 above). (Look fix round 4 "mist": at its full light the deck sat in
// the curve's shoulder as a near-white sheet, (200, 203, 209) with a spread
// of 10 against 22 to 31 in the original; a rain day read as dry mist.)
constexpr float kDeckRelief = 2.4f;
constexpr float kDeckDark = 0.5f;
constexpr float kDeckFromSin = 0.12f;
constexpr float kDeckFullSin = 0.3f;

// ---- The sun disk -----------------------------------------------------------
// Under the camera the sun's sprite (its painted halo and star) is not drawn:
// both of its quads draw a disc of this angular radius around the sun's
// direction, anti-aliased over its rim, at kSunDiscLevel (pre-exposed light:
// past the curve's white at any exposure, so the sun is the frame's one
// clipped white point) in the colour of the sunlight through the air taken
// kSunDiscWhite of the way to white (a sensor clips every channel of the sun:
// in the sunlight's own colour the hue-keeping curve held a low sun's disc at
// a pale peach, 255, 238, 219), all of it from 4 degrees up, none at the
// horizon, where the reddened sun keeps its colour. The disc is round on the
// screen (an equal-area circle, not the lens's oval near the frame's corners).
// Its glow is the sky's roll-off toward the sun (kSkySunCap), the sky table's
// aureole and the camera's bloom.
constexpr float kSunDiscRadiusDeg = 0.45f;
constexpr float kSunDiscLevel = 12.0f;
constexpr float kSunDiscWhite = 0.85f;
// The scene light (after exposure) where bloom starts, and its knee: light
// past the curve's white.
constexpr float kCameraBloomThreshold = 3.0f;
constexpr float kCameraBloomKnee = 1.0f;

} // namespace Poseidon::remaster
