#include <Poseidon/Graphics/Remaster/SeaWaves.hpp>

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

float Smoothstep(float a, float b, float x)
{
    const float t = Clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}
} // namespace

SeaFrameBlend SeaAnimationBlend(float phase, int frames, int bound)
{
    SeaFrameBlend b;
    if (frames <= 1)
        return b;
    b.bound = std::max(0, std::min(frames - 1, bound));
    // Frame k shows alone at phase k / (n - 1); in between, the two around
    // the phase share it linearly.
    const float s = Clamp01(phase) * static_cast<float>(frames - 1);
    const int lo = std::min(static_cast<int>(std::floor(s)), frames - 2);
    const float f = Clamp01(s - static_cast<float>(lo)); // share of lo + 1
    if (b.bound == lo)
    {
        b.other = lo + 1;
        b.otherWeight = f;
    }
    else if (b.bound == lo + 1)
    {
        b.other = lo;
        b.otherWeight = 1.0f - f;
    }
    else
    {
        // Neither (AnimateTexture wraps phase 1 exactly to frame 0, and its
        // rounding floor may bind the frame below at an exact frame edge):
        // the nearer of the two, through the other unit alone.
        b.other = f >= 0.5f ? lo + 1 : lo;
        b.otherWeight = 1.0f;
    }
    return b;
}

float SeaFrameWeight(const SeaFrameBlend& blend, int frame)
{
    float w = 0.0f;
    if (frame == blend.bound)
        w += 1.0f - blend.otherWeight;
    if (frame == blend.other)
        w += blend.otherWeight;
    return w;
}

float SeaCrossFadeGain(float w)
{
    w = Clamp01(w);
    const float a = 1.0f - w;
    const float spread = a * a + w * w + 2.0f * w * a * kSeaFrameCorrelation;
    return 1.0f / std::sqrt(std::max(spread, 1e-6f));
}

float SeaHighSun(float sunElevationDeg)
{
    return Smoothstep(kSeaHighSunStartDeg, kSeaHighSunFullDeg, sunElevationDeg);
}

SeaWaveConstants ComputeSeaWaveConstants(float sunElevationDeg, const LightingTuning& tuning,
                                         const SeaFrameBlend& blend)
{
    SeaWaveConstants c;
    const float high = SeaHighSun(sunElevationDeg);
    c.relief = std::max(0.0f, tuning.seaRelief);
    c.crossFade = Clamp01(blend.otherWeight);
    c.skyMean = kSeaSkyMean * (1.0f + (kSeaSkyMeanNoon - 1.0f) * high) * std::max(0.0f, tuning.seaSky);
    c.skyTint = kSeaSkyTintNoon * high;
    c.sheen = (kSeaSheen + (kSeaSheenNoon - kSeaSheen) * high) * std::max(0.0f, tuning.seaSheen);
    c.sheenFlat = high;
    return c;
}

Rgb SeaSheenLight(const Rgb& rigSun, const Rgb& classicSunDiffuse, float sheen)
{
    const float lum = Luminance(rigSun);
    const float classicLum = Luminance(classicSunDiffuse);
    const Rgb hue = classicLum > 1e-6f ? classicSunDiffuse * (1.0f / classicLum) : Rgb(1.0f, 1.0f, 1.0f);
    return hue * (lum * std::max(0.0f, sheen));
}

} // namespace Poseidon::remaster
