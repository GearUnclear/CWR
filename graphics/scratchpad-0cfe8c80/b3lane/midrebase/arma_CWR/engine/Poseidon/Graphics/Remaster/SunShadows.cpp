#include <Poseidon/Graphics/Remaster/SunShadows.hpp>

#include <Poseidon/Graphics/Remaster/NightLights.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

float SunTanRadius(float sunSizeDeg)
{
    if (!(sunSizeDeg > 0.0f))
        return 0.0f;
    const double radius = 0.5 * static_cast<double>(sunSizeDeg) * 3.14159265358979323846 / 180.0;
    return static_cast<float>(std::tan(radius));
}

float PcssSearchTexels(float tanRadius, float texelMetres, float reachMetres)
{
    const float t = std::max(reachMetres, kPcssMinReachMetres) * tanRadius / std::max(texelMetres, 1e-6f);
    return std::clamp(t, kPcssMinSearchTexels, kPcssMaxSearchTexels);
}

float PcssBlockerMetres(float receiverDepth, float meanBlockerDepth, float depthPerMetre)
{
    return std::max(receiverDepth - meanBlockerDepth, 0.0f) / std::max(depthPerMetre, 1e-9f);
}

float PcssPenumbraTexels(float blockerMetres, float tanRadius, float texelMetres, float searchTexels)
{
    const float t = blockerMetres * tanRadius / std::max(texelMetres, 1e-6f);
    return std::clamp(t, kPcssMinFilterTexels, std::max(searchTexels, kPcssMinFilterTexels));
}

int PcssSearchTaps(int tier, int omniTiers, bool temporal)
{
    if (temporal)
        return kPcssTemporalTaps;
    return tier < omniTiers ? kPcssSearchTapsNear : kPcssSearchTapsFar;
}

int PcssFilterTaps(float penumbraTexels, bool temporal)
{
    if (temporal)
        return kPcssTemporalTaps;
    return penumbraTexels < kPcssTaps24FromTexels ? 12 : (penumbraTexels < kPcssTaps32FromTexels ? 24 : 32);
}

float PcssStrength(float sunElevationDeg)
{
    const float t = std::clamp(
        (sunElevationDeg - kGateSunShadowMinSunDeg) / (kPcssFullFromSunDeg - kGateSunShadowMinSunDeg), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float PcssFrameTurn(std::uint32_t sequence)
{
    return static_cast<float>(std::fmod(kPcssFrameTurnStep * static_cast<double>(sequence), 1.0));
}

SunShadowConstants ComputeSunShadowConstants(const LightingTuning& tuning, const FeatureGates& gates, bool taaFrame,
                                             std::uint32_t taaSequence)
{
    SunShadowConstants c;
    const Gate& soft = gates[Feature::SoftShadows];
    const float tanRadius = soft.on ? SunTanRadius(tuning.sunSize) : 0.0f;
    if (tanRadius > 0.0f)
    {
        c.taa[0] = taaFrame ? PcssFrameTurn(taaSequence) : 0.0f;
        c.taa[1] = taaFrame ? 1.0f : 0.0f;
        c.taa[2] = static_cast<float>(kPcssTemporalTaps);
        c.taa[3] = tanRadius;
        c.pcss[0] = std::max(tuning.shadowReach, kPcssMinReachMetres);
        c.pcss[1] = kPcssMinFilterTexels;
        // The gate opens at 4 deg; the soft filter fades in over the next 2
        // degrees. Forced on (gates=0) it runs whole. Under the moon key
        // (B3) the stock filter casts the moon's shadows: at a fiftieth of the
        // noon sun no one sees a contact-hardening penumbra, and the soft
        // filter's taps would cost the night's world pass for nothing.
        const float sun = gates.inputs.sunElevationDeg;
        c.pcss[2] = MoonKeyShare(gates) > 0.0f                        ? 0.0f
                    : (tuning.gates && sun > kGateSunShadowMinSunDeg) ? PcssStrength(sun)
                                                                      : 1.0f;
    }
    const Gate& trees = gates[Feature::TreeShadows];
    c.pcss[3] = trees.on ? std::max(tuning.leafBias, 0.0f) : 0.0f;
    return c;
}

bool CastsIntoSunShadowMap(bool castShadow, bool staticObject, bool treeShadowsOn)
{
    return castShadow || (staticObject && treeShadowsOn);
}

} // namespace Poseidon::remaster
