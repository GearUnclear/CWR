#include <Poseidon/Graphics/Remaster/NightLights.hpp>

#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>

#include <algorithm>

namespace Poseidon::remaster
{

namespace
{

float Smooth01(float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

float MoonKeyShare(const FeatureGates& gates)
{
    if (!gates[Feature::MoonShadows].on)
        return 0.0f;
    const GateInputs& in = gates.inputs;
    const float sun = Smooth01((kGateMoonKeyMaxSunDeg - in.sunElevationDeg) / kMoonKeyFadeDeg);
    const float moon = Smooth01((in.moonElevationDeg - kGateMoonKeyMinMoonDeg) / kMoonKeyFadeDeg);
    const float phase = Smooth01((in.moonPhase - kGateMoonKeyMinPhase) / kMoonKeyFadePhase);
    return sun * moon * phase;
}

NightConstants ComputeNightConstants(const LightingTuning& tuning, const FeatureGates& gates, bool tiledLights,
                                     float moonShare)
{
    NightConstants c;
    c.night[3] = std::clamp(moonShare, 0.0f, 1.0f);
    if (!gates[Feature::NightLights].on || !tiledLights)
        return c;
    c.night[0] = 1.0f;
    c.night[1] = lighting::kLampIntensityPerStart * std::max(tuning.nightLamp, 0.0f);
    c.night[2] = lighting::kLowBeamPeak * std::max(tuning.nightSpot, 0.0f);
    return c;
}

StarSplat MakeStarSplat(float x, float y, float alpha, std::uint32_t index)
{
    StarSplat s;
    const float r = kStarSplatRadius;
    const float dx[4] = {-r, r, r, -r};
    const float dy[4] = {-r, -r, r, r};
    s.x[0] = x;
    s.y[0] = y;
    s.alpha[0] = std::clamp(alpha * kStarCentreGain, 0.0f, 255.0f);
    for (int c = 0; c < 4; ++c)
    {
        s.x[c + 1] = x + dx[c];
        s.y[c + 1] = y + dy[c];
        s.alpha[c + 1] = 0.0f;
    }
    // A faint colour temperature per star (Knuth's multiplicative hash of the
    // index), from a cool white to a warm one, each at unit luminance.
    const float h = static_cast<float>((index * 2654435761u) & 0xffffu) / 65535.0f;
    const float cool[3] = {0.80f, 0.87f, 1.00f};
    const float warm[3] = {1.00f, 0.90f, 0.74f};
    float t[3];
    for (int i = 0; i < 3; ++i)
        t[i] = cool[i] + (warm[i] - cool[i]) * h;
    const float lum = 0.2126f * t[0] + 0.7152f * t[1] + 0.0722f * t[2];
    for (int i = 0; i < 3; ++i)
        s.tint[i] = t[i] / lum;
    return s;
}

} // namespace Poseidon::remaster
