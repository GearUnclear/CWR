#include <Poseidon/Graphics/Remaster/Wetness.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{

float Clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

float Smoothstep(float e0, float e1, float x)
{
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

// Moves `v` toward `target` over `dt` with time constant `tau` (exact for a
// constant target, so a long frame never overshoots).
float Approach(float v, float target, float dt, float tau)
{
    return target + (v - target) * std::exp(-dt / std::max(tau, 1e-3f));
}

} // namespace

float WetSurfaceTarget(float rain)
{
    return Smoothstep(0.0f, kWetRainFull, rain);
}

float WetPuddleTarget(float rain)
{
    return Smoothstep(kPuddleRainFrom, kPuddleRainFull, rain);
}

float DryingFactor(const WetnessInputs& in)
{
    const float sunUp = Clamp01(std::sin(in.sunElevationDeg * 0.017453293f));
    const float wind = std::min(std::max(in.windSpeed, 0.0f), kDryWindMax);
    return 1.0f + kDrySun * sunUp * Clamp01(in.skyThrough) + kDryWindPerMs * wind;
}

WetnessState SettledWetness(float rain)
{
    return WetnessState{WetSurfaceTarget(rain), WetPuddleTarget(rain)};
}

void StepWetness(WetnessState& state, const WetnessInputs& in, float dt)
{
    if (!(dt > 0.0f))
        return;
    const float rain = Clamp01(in.rain);
    // Light rain wets slower than heavy rain; drying follows the sun and wind.
    const float rate = std::max(rain / kWetRainFull, kWetMinRainRate);
    const float dry = DryingFactor(in);
    const float surfaceTarget = WetSurfaceTarget(rain);
    const float puddleTarget = WetPuddleTarget(rain);
    const float surfaceTau = state.surface < surfaceTarget ? kWetUpSeconds / rate : kDryBaseSeconds / dry;
    const float puddleTau = state.puddles < puddleTarget ? kPuddleUpSeconds / rate : kPuddleDryBaseSeconds / dry;
    state.surface = Clamp01(Approach(state.surface, surfaceTarget, dt, surfaceTau));
    state.puddles = Clamp01(Approach(state.puddles, puddleTarget, dt, puddleTau));
}

bool WetnessSnaps(float currentRain, float newRain, float seconds)
{
    return seconds < kWetSnapMaxSeconds && std::fabs(newRain - currentRain) >= kWetSnapMinChange;
}

WetnessFrame FrameWetness(const WetnessState& world, float wetnessKnob, float puddlesKnob)
{
    WetnessFrame f;
    f.forced = wetnessKnob >= 0.0f;
    f.surface = f.forced ? Clamp01(wetnessKnob) : Clamp01(world.surface);
    const float puddles = f.forced ? Clamp01(wetnessKnob) : Clamp01(world.puddles);
    f.puddles = Clamp01(puddles * std::max(puddlesKnob, 0.0f));
    return f;
}

float FrameWetnessGate(const WetnessFrame& w)
{
    return std::max(w.surface, w.puddles);
}

} // namespace Poseidon::remaster
