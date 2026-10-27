#pragma once

#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <cstdint>
#include <string>

// Frame gates for the remastered lighting features: whether an enabled
// feature runs this frame, and at what resolution. Pure, no GL, no engine
// state; the GL33 backend fills GateInputs once per frame (at the world's
// first 3D pass, EngineGL33::UploadLitFrameConstants) and the features read
// the result.
//
// The budget (RTX 3060 class, 1080p, 60 fps) only holds if the heavy features
// run only where they can show: beams at a low sun in view, wet shading in the
// wet, canopy shadows at a low clear sun, and so on. Every gate is a pure
// function of the inputs below, so a
// gate's off state can be tested and logged (triFeatureState) and its cost
// checked at zero there.
//
// Order of the checks for every feature: classic lighting turns everything
// off; then the feature's switch; then night vision (the world draws through
// the classic programs under the intensifier, EngineGL33_Queue.cpp BeginPass),
// except for the frame-cost features; then `gates` off forces the feature on
// (debugging); then the feature's own condition.

namespace Poseidon::remaster
{

// Thresholds of the gates (degrees, shares, metres).
constexpr float kGateSkyTableMinSunDeg = -9.0f;    // sky table: the night fade ends there
constexpr float kGateSunShadowMinSunDeg = 4.0f;    // sun shadow map
constexpr float kGateNightMinEffect = 0.01f;       // night features (LightSun::NightEffect)
constexpr float kGateMoonKeyMaxSunDeg = -3.0f;     // moon key: the sun this far under the horizon
constexpr float kGateMoonKeyMinMoonDeg = 3.0f;     // moon key: the moon this high
constexpr float kGateMoonKeyMinPhase = 0.25f;      // moon key: this much of a full moon's light
constexpr float kGateCombatRange = 60.0f;          // combat lights count within this many metres
constexpr float kGateCombatMaskMinNight = 0.3f;    // combat shadow mask by day only above this night
constexpr float kGateSsrMinShare = 0.01f;          // water or wet tags on this share of the screen
constexpr float kGateWetMin = 0.02f;               // wetness that shows
constexpr float kGateBeamsMinSunDeg = 0.0f;        // sun beams: the sun above the horizon
constexpr float kGateBeamsMaxSunDeg = 30.0f;       // sun beams: a low sun
constexpr float kGateBeamsMaxViewAngleDeg = 60.0f; // sun beams: the sun this close to the view
constexpr float kGateMistMinFog = 0.05f;           // setFog that counts as mist
constexpr float kGateMorningMinSunDeg = -4.0f;     // morning mist: from just before sunrise
constexpr float kGateMorningMaxSunDeg = 10.0f;     // morning mist: gone by this sun height (AirMedium.hpp)
constexpr float kGateCanopyMinSunDeg = 2.0f;       // long canopy shadows: low sun band
constexpr float kGateCanopyMaxSunDeg = 25.0f;
constexpr float kGateCanopyMinSkyThrough = 0.5f; // long canopy shadows: mostly clear sky
constexpr float kGateLitCloudsMaxSkyThrough = 0.85f;
// Sky-matched cloud shadows: down to where the rig's sun is gone (its horizon
// fade starts at -1.5 deg, LightRig.cpp), so the hand-over to the noise field
// below it never shows.
constexpr float kGateSkyShadowsMinSunDeg = -1.5f;

// What a gate looks at, filled by the backend once per frame. An input the
// backend has no source for keeps its "unknown" default, noted per field.
struct GateInputs
{
    float sunElevationDeg = 90.0f;
    float sunAzimuthDeg = 180.0f; // 0 north, 90 east (clockwise)
    // Angle between the view direction and the direction to the sun.
    float sunViewAngleDeg = 180.0f;
    float moonElevationDeg = -90.0f;
    float moonPhase = 0.0f;   // 0 new .. 1 full (the light the rig takes from it)
    float nightEffect = 0.0f; // LightSun::NightEffect: 0 day .. 1 night
    float skyThrough = 1.0f;  // 1 clear .. 0 overcast
    float cloudCover = 0.0f;  // the rig's cloud-shadow cover (0 under a clear sky)
    float rainDensity = 0.0f;
    float fog = 0.0f; // setFog value, 0..1
    // World wetness 0..1 (Wetness.hpp). Negative = no wetness state yet:
    // the rain density stands in.
    float wetness = -1.0f;
    bool worldDrawn = true;
    bool interior = false;    // last frame cleared the world depth (vehicle interior)
    bool optics = false;      // last frame drew a screen overlay in the world pass
    bool nightVision = false; // the intensifier is on
    int localLights = 0;      // point and spot lights on, within reach of the camera
    int spotLights = 0;       // of those, spot lights (headlights)
    // Combat lights (CombatLights.hpp) lit this frame whose reach comes
    // within kGateCombatRange of the camera (a flare 100 m up counts).
    int combatLightsNear = 0;
    // A recent frame's screen share of water and of the ground the rain wets
    // (terrain and roads), from the reflections' tag readout (read back a
    // few frames later). The ground counts only while it is wet (the wetness
    // below, or the ssrWet knob). Negative = not measured (no tags written).
    float waterShare = -1.0f;
    float wetShare = -1.0f;
    // The sky table's transmittance and multiple-scattering tables are built
    // (a worker thread builds them when the haze changes; Atmosphere.hpp).
    bool skyTablesReady = true;
    // The cloud-density map of the current weather sky is built (a worker
    // thread builds it when the weather sky changes; SkyDensity.hpp).
    bool skyDensityReady = true;
};

struct Gate
{
    bool on = false;
    // Resolution of the feature's pass relative to the frame (0.5 = half).
    float scale = 1.0f;
    // Why it is on or off, for the overlay and triFeatureState. Static text.
    const char* reason = "not computed";
};

// The photoreal camera's state (photoLight, PhotoCamera.hpp) for the capture
// logs: the exposure in use and the one it adapts toward (log2), the last
// meter's mean and highlight (log2 scene luminance), the white balance's
// strength, the clear sky's gain, the rig's sun and sky luminance on an
// up-facing surface, and how many meter readbacks arrived.
struct CameraReadout
{
    bool valid = false;
    float exposureLog2 = 0.0f;
    float targetLog2 = 0.0f;
    float meterMean = 0.0f;
    float meterHighlight = 0.0f;
    float whiteBalance = 0.0f;
    float skyScale = 0.0f;
    float preExposureLog2 = 0.0f;
    float sunLum = 0.0f;
    float skyUpLum = 0.0f;
    std::uint32_t readbacks = 0;
    // The camera's own CPU time on the render thread (ms, smoothed): taking a
    // finished readback with the meter's histogram and guards, and queuing
    // this frame's meter; and the meters not queued because the ring's slot
    // was still in flight (the CPU never waits on the GPU).
    float cpuCollectMs = 0.0f;
    float cpuQueueMs = 0.0f;
    std::uint32_t skippedMeters = 0;
    // The guards' readings (log2 scene luminance; kMeterFloorLog2 when none):
    // the sky's bright end, the sky by a low sun in frame and that guard's
    // weight, and the exposure the meter's mean alone asks for (no guard), so
    // a log shows which guard holds the frame down.
    float meterSkyTop = -16.0f;
    float meterSunRing = -16.0f;
    float meterSunRingWeight = 0.0f;
    float unguardedLog2 = 0.0f;
};

struct FeatureGates
{
    Gate gate[kFeatureCount];
    GateInputs inputs;
    CameraReadout camera; // filled by the backend when it hands the gates out
    bool computed = false;
    std::uint32_t frame = 0; // backend frame counter when computed
    const Gate& operator[](Feature f) const { return gate[static_cast<int>(f)]; }
};

FeatureGates ComputeFeatureGates(const GateInputs& in, const LightingTuning& tuning);

// The wetness the gates use: the integrator's when there is one, else the
// rain density.
float GateWetness(const GateInputs& in);

// triFeatureState: the switches and their gates as one line, for capture logs.
//   "mode=remastered gates=1 computed=1 frame=<n> sun=<deg> night=<0..1> sky=<0..1> rain=<0..1> wet=<0..1>
//    fog=<0..1> lights=<n> landed=<n>/<n> | <key>=<value>:<on|off>[@<scale>]:<reason>[:inert] ...
//    [| cam ev=.. target=.. mean=.. high=.. wb=.. sky=.. sun=.. skyup=.. reads=..]"
// Fields are separated by spaces, never ';' (the UDCAP log lines split on it);
// reasons have their spaces replaced by '-'. ":inert" marks a switch that
// nothing reads yet (FeatureInfo::landed false), so its on and off look the
// same.
std::string FeatureStateString(const LightingTuning& tuning, const FeatureGates& gates);

} // namespace Poseidon::remaster
