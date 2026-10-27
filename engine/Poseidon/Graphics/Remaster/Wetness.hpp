#pragma once

// The world's wetness for the remastered wet weather (feature switch
// `wetWeather`). Pure, no GL, no engine state: the landscape's weather keeps a WetnessState and steps it
// with the simulation (Weather::MoveClouds), the renderer reads it once a
// frame (FrameWetness).
//
// What the player sees: rain wets the world over a minute or two and the
// puddles fill over ten; when it stops, surfaces dry over minutes, faster in
// sun and wind, and the puddles linger long after them.
//
//   - surface: how wet surfaces are, 0 dry .. 1 soaked. Under rain it rises
//     toward WetSurfaceTarget(rain) with a time constant of kWetUpSeconds at
//     full rain (longer in light rain); above that target it dries toward it
//     with kDryBaseSeconds over DryingFactor (sun, clear sky, wind).
//   - puddles: how full the hollows are, 0 .. 1, toward WetPuddleTarget(rain),
//     with the slower kPuddleUpSeconds and kPuddleDryBaseSeconds.
//   - A scripted weather change made at once (`0 setRain x`: a mission start,
//     a capture view) snaps the state to what that rain settles at
//     (SettledWetness), so a mission that starts in rain starts wet.

namespace Poseidon::remaster
{

constexpr float kWetRainFull = 0.35f;            // rain density that soaks surfaces fully
constexpr float kPuddleRainFrom = 0.1f;          // puddles form above this rain ...
constexpr float kPuddleRainFull = 0.8f;          // ... and are full at this one
constexpr float kWetUpSeconds = 90.0f;           // surfaces wet with this time constant in full rain
constexpr float kPuddleUpSeconds = 600.0f;       // puddles fill with this one
constexpr float kDryBaseSeconds = 1200.0f;       // surfaces dry with this one in still, overcast air
constexpr float kPuddleDryBaseSeconds = 3600.0f; // puddles drain with this one
constexpr float kWetMinRainRate = 0.1f;          // light rain wets at least this share of full rain's speed
constexpr float kDrySun = 2.0f;                  // DryingFactor: high clear sun adds this much
constexpr float kDryWindPerMs = 0.12f;           // ... and every m/s of wind this much
constexpr float kDryWindMax = 15.0f;             // wind beyond this (m/s) dries no faster
constexpr float kWetSnapMinChange = 0.01f;       // a rain change this large made at once snaps the state
constexpr float kWetSnapMaxSeconds = 1.0f;       // "at once": over less than this

struct WetnessInputs
{
    float rain = 0.0f;              // the landscape's rain density, 0..1
    float sunElevationDeg = -90.0f; // the sun's height
    float skyThrough = 1.0f;        // 1 clear .. 0 overcast
    float windSpeed = 0.0f;         // m/s
};

struct WetnessState
{
    float surface = 0.0f;
    float puddles = 0.0f;
};

float WetSurfaceTarget(float rain);
float WetPuddleTarget(float rain);
// 1 in still, overcast air or at night; up to 1 + kDrySun under a high clear
// sun, plus the wind's share.
float DryingFactor(const WetnessInputs& in);
// What a long spell of this rain (or of no rain) settles at.
WetnessState SettledWetness(float rain);
// One simulation step of `dt` seconds (0 or less changes nothing).
void StepWetness(WetnessState& state, const WetnessInputs& in, float dt);
// Whether a SetRain(density, time) call snaps the state: a change of at least
// kWetSnapMinChange from the current density over less than kWetSnapMaxSeconds.
bool WetnessSnaps(float currentRain, float newRain, float seconds);

// What the renderer uses this frame: the world's state, or the `wetness` knob
// (LightingTuning::wetness: 0..1 forces surfaces and puddles alike; below 0 =
// the world's), and the puddles times the `puddles` knob (LightingTuning::puddles).
struct WetnessFrame
{
    float surface = 0.0f;
    float puddles = 0.0f;
    bool forced = false;
};
WetnessFrame FrameWetness(const WetnessState& world, float wetnessKnob, float puddlesKnob);
// The larger of the two: what the frame gates read (puddles keep the wet path
// on after the surfaces have dried).
float FrameWetnessGate(const WetnessFrame& w);

} // namespace Poseidon::remaster
