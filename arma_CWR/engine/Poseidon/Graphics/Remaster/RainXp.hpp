#pragma once

#include <cstdint>

// Rain you can see (perf-squad lane EX4-RAIN, plan item R-9): world-space rain
// by default since 2026-11-13. Pure, no GL, no engine state.
//
// Today the rain is a screen overlay (Scene::DrawRain: two tiled layers of the
// rain texture, their phase from the clock) drawn into the world pass after
// the translucent objects. Under TAA the resolve averages its streaks, which
// move every frame, into a faint vertical haze. POSEIDON_XP_RAIN picks:
//   "0", "off"         the overlay as before;
//   "1", "late"        (a) the same overlay drawn after the TAA resolve and its
//                      sharpen, before the HUD (Scene::DrawRainLate), so its
//                      streaks stay crisp;
//   unset, "2", "world" (b) camera-anchored world-space streaks in place of the
//                      overlay, drawn after the resolve and lit by the sky, the
//                      sun and the frame's lamps and headlights, hidden behind
//                      what stands in front of them and faded by the fog at
//                      their depth (EngineGL33_XpRain.cpp).
// Both fall back to the overlay as today in a frame with a cockpit (pass 3),
// which draws over the world after the rain.
namespace Poseidon::remaster
{

enum class RainXpMode : std::uint8_t
{
    Off,
    Late,
    World,
};

// The mode a POSEIDON_XP_RAIN value names (nullptr or empty: Off; an unknown
// value: Off).
RainXpMode RainXpModeFromString(const char* value);
// The process's mode (the environment, read once): RainXpModeFromEnvValue.
RainXpMode RainXpModeFromEnv();
// The mode for a POSEIDON_XP_RAIN value: unset or empty is World (the shipped
// look); otherwise RainXpModeFromString, so "0" is Off (today's overlay).
RainXpMode RainXpModeFromEnvValue(const char* value);

// The drop volume (m, world axes: east, up, north) the backend draws, and the
// drops' layers: each layer falls at its own speed (big drops faster, small
// ones slower and further with the wind), so the rain is not one rigid sheet.
constexpr float kRainBoxSize[3] = {40.0f, 24.0f, 40.0f};
constexpr int kRainLayers = 4;
constexpr float kRainLayerFall[kRainLayers] = {0.84f, 0.95f, 1.05f, 1.16f};
constexpr float kRainLayerWind[kRainLayers] = {1.12f, 1.04f, 0.97f, 0.9f};

// What the scene hands the backend for the world-space rain each frame.
struct RainXpFrame
{
    std::uint32_t frame = 0xffffffffu; // the engine frame it is for
    float density = 0.0f;              // the landscape's rain density (0..1)
    double camPos[3] = {};             // the camera's world position (m)
    // Per layer: how far its drops have travelled since the rain began (m,
    // world axes, RainDriftStep), and their velocity relative to the camera
    // (m/s), which draws the streak.
    double travel[kRainLayers][3] = {};
    float relVel[kRainLayers][3] = {};
};

// The drops fill a box of `size` (m, world axes) whose minimum corner sits at
// `boxMin` from the camera. A drop's position is its own fixed point in the
// lattice of boxes, moved by `travel` (m): seen from the camera at camPos it
// lies at boxMin + ((base + shift) mod size), with
//   shift = (travel - camPos - boxMin) mod size,
// which this computes in double precision (the world position is too large
// for a float sum on the GPU), each axis in [0, size).
void RainWrapShift(const double camPos[3], const double travel[3], const float boxMin[3], const float size[3],
                   float out[3]);

// The drops' travel, kept from frame to frame (round 2, the fix for rain that
// stood still or rose). Round 1 placed a drop at velocity times the clock. The
// velocity is not constant: the rain density wanders all the time
// (Weather::MoveClouds, up to 0.011 a second), the fall speed follows it, and
// the wind steps every 5 s and with each gust. A change dv of the velocity
// then moved every drop by dv times the whole clock at once: with the density
// falling by 0.01 a second the drops gained 0.025 m/s of rise for every second
// the mission had run, so after about 340 s the rain stood still and then
// rose, and each wind step threw every drop tens of metres aside. Here the
// travel is summed: each frame adds velocity times that frame's step, so a
// change of velocity only changes how fast the drops move from then on.
//
// It also keeps the rain falling on screen whatever the camera does: the
// drops' velocity relative to the camera is held to at least kRainKeepFall of
// the fall speed downward (a camera that dives with the rain) and to at most
// kRainMaxSlant of it sideways (a fast vehicle, a gale); what the hold takes
// off, the drops travel with the camera.
constexpr float kRainKeepFall = 0.6f;
constexpr float kRainMaxSlant = 0.9f;
// The wind and the camera's velocity as the rain takes them: eased over these
// times (s), so a gust leans the streaks over instead of snapping them.
constexpr float kRainWindEase = 1.5f;
constexpr float kRainCamEase = 0.12f;
// The wind the rain takes at most (m/s, a gale).
constexpr float kRainWindMax = 15.0f;
// A camera step faster than this (m/s) is a cut, not motion.
constexpr float kRainCutSpeed = 120.0f;
// The travel is folded by this length (m, a multiple of every box edge), so
// it stays small however long the rain lasts.
constexpr double kRainTravelFold = 3840.0;

struct RainXpDrift
{
    bool valid = false;
    double time = 0.0;     // the clock of the last step (s)
    double camPos[3] = {}; // the camera of the last step
    float camVel[3] = {};  // the camera's velocity, eased (m/s)
    float wind[3] = {};    // the wind, eased (m/s)
    float fall = 0.0f;     // the fall speed, eased (m/s)
    double travel[kRainLayers][3] = {};
    // Round 1's drop placement (velocity times the clock), kept as a probe and
    // for POSEIDON_XP_RAIN_R1: where it would put the drops, the velocity it
    // used, and the velocity (m/s) at which that place moved over the last
    // step.
    double r1Place[3] = {};
    float r1Vel[3] = {};
    float r1Speed[3] = {};
};

// One frame's step: the clock `t` (s), the rain density, the wind (m/s) and
// the camera's position now. Fills out.density, camPos, travel and relVel
// (out.frame is the caller's). The first step, a step back in time, or one
// longer than half a second (a pause, a re-pinned capture clock) moves nothing
// and takes the wind and the camera as they are.
void RainDriftStep(RainXpDrift& drift, double t, float density, const float wind[3], const double camPos[3],
                   RainXpFrame& out);
// Round 1's placement, for POSEIDON_XP_RAIN_R1: every layer's travel becomes
// velocity times the clock and the streak the plain relative velocity, as
// round 1 drew it.
void RainDriftAsRound1(const RainXpDrift& drift, RainXpFrame& out);

// POSEIDON_XP_RAIN_R1 (read once): draw the round 1 placement (the bug), for
// the proof clip only. POSEIDON_XP_RAIN_PROBE (read once): log every rain
// frame's clock, density, wind and both placements' speeds.
bool RainXpRound1FromEnv();
bool RainXpProbeFromEnv();

// The fall speed of a raindrop (m/s) in light and in heavy rain: big drops
// fall faster (Gunn and Kinzer 1949: 1 mm 4 m/s, 2 mm 6.5 m/s, 4 mm 9 m/s).
constexpr float kRainFallSlow = 6.0f;
constexpr float kRainFallFast = 8.5f;
float RainFallSpeed(float density);

// The smoke in front of the rain (round 2): a coarse screen grid the backend
// fills from the frame's smoke and fire puffs (each a soft disc of its
// opacity at its depth), so drops behind a puff fade by what the puff covers.
constexpr int kRainCoverW = 48;
constexpr int kRainCoverH = 27;
struct RainCoverGrid
{
    float clear[kRainCoverW * kRainCoverH];  // what the puffs let through (1: no smoke)
    float depth[kRainCoverW * kRainCoverH];  // the puffs' depth (m), weighted by their opacity
    float weight[kRainCoverW * kRainCoverH]; // the sum of those weights
    int puffs = 0;
    void Clear();
    // A puff at (x, y) of a screen w by h (any unit, y either way), its half
    // size (sx, sy) in the same unit, its opacity (0..1) and depth (m).
    void Add(float x, float y, float sx, float sy, float w, float h, float opacity, float depthM);
    // The cell's cover (0..1) and depth (m; 0 where nothing covers).
    void Cell(int i, float& cover, float& depthM) const;
};

} // namespace Poseidon::remaster
