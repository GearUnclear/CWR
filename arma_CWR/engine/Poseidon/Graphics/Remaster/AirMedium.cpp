#include <Poseidon/Graphics/Remaster/AirMedium.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{
constexpr float kSeaFloor = -30.0f; // heights below this count as this (the closed forms stay finite)
} // namespace

float SmoothStep(float edge0, float edge1, float x)
{
    if (edge1 == edge0)
        return x < edge0 ? 0.0f : 1.0f;
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t * t * (3.0f - 2.0f * t);
}

float MorningWeight(float sunElevationDeg, float sunEast)
{
    return SmoothStep(-0.15f, 0.25f, sunEast) *
           (1.0f - SmoothStep(kMorningFadeSunDeg, kMorningGoneSunDeg, sunElevationDeg)) *
           SmoothStep(kMorningMinSunDeg, kMorningFullSunDeg, sunElevationDeg);
}

float BeamMistWeight(const AirInputs& in)
{
    const float fog = SmoothStep(kBeamFogFrom, kBeamFogFull, in.fog);
    const float rain = SmoothStep(kBeamRainFrom, kBeamRainFull, in.rain);
    // The sun at the horizon (sunrise, sunset): the air near the ground is at
    // its most humid. A narrow band, so a clear morning an hour on is clear.
    const float e = in.sunElevationDeg;
    const float lowSun = SmoothStep(-4.0f, -1.0f, e) * (1.0f - SmoothStep(1.0f, kBeamLowSunDeg - 2.0f, e));
    return std::max(fog, std::max(rain, lowSun));
}

float FogEdgeStart(float fogStart, float fogEnd)
{
    if (!(fogEnd > fogStart) || !(fogEnd > 0.0f))
        return fogStart;
    const float clearW = SmoothStep(kHazeClearFogEndMin, kHazeClearFogEndMax, fogEnd);
    const float moved = std::max(fogStart, kHazeEdgeFraction * fogEnd);
    return fogStart + (moved - fogStart) * clearW;
}

float FogEdgeScale(float fogStart, float fogEnd)
{
    if (!(fogEnd > fogStart) || !(fogEnd > 0.0f))
        return 1.0f;
    const float start = FogEdgeStart(fogStart, fogEnd);
    return (fogEnd - fogStart) / (fogEnd - start);
}

AirMedium ComputeAirMedium(const AirInputs& in, const AirParts& parts, float gainKnob)
{
    AirMedium m;
    const float fog = std::clamp(in.fog, 0.0f, 1.0f);
    const float rain = std::clamp(in.rain, 0.0f, 1.0f);
    const float air = kHazeSigma * (1.0f + kHazeFogSigmaGain * fog + kHazeRainSigmaGain * rain);
    m.fogStart = in.fogStart;
    m.fogEnd = in.fogEnd;
    if (parts.haze)
    {
        m.hazeSigma = air;
        m.fogEdgeScale = FogEdgeScale(in.fogStart, in.fogEnd);
        m.fogStart = FogEdgeStart(in.fogStart, in.fogEnd);
    }
    if (parts.valleyMist)
    {
        // Radiation mist forms under a clear sky; an overcast keeps 40% of it.
        const float sky = std::clamp(in.skyThrough, 0.0f, 1.0f);
        const float morning = MorningWeight(in.sunElevationDeg, in.sunEast) * (0.4f + 0.6f * sky);
        const float weight = std::max(morning, SmoothStep(kMistFogFrom, kMistFogFull, fog));
        m.valleyMist = kValleyMistSigma * weight;
    }
    const float w = BeamMistWeight(in);
    m.beamMistWeight = w;
    if (parts.beams)
    {
        const float scale = kBeamMistClear + (kBeamMistMisty - kBeamMistClear) * w +
                            kBeamMistHeavyFog * std::max(fog - kBeamFogFull, 0.0f);
        m.beamMist = kBeamMistSigma * scale;
        m.beamAir = air;
        m.beamGain = gainKnob >= 0.0f ? gainKnob : kBeamGainClear + (kBeamGainMisty - kBeamGainClear) * w;
        m.beamHighpass = kBeamHighpassClear + (kBeamHighpassMisty - kBeamHighpassClear) * w;
    }
    return m;
}

float BeamShaft(float lit, float open, float meanLit, float meanOpen, float conf, float highpass, float gain)
{
    float v = std::clamp(lit / std::max(open, 1e-7f), 0.0f, 1.0f);
    const float local = std::clamp(meanLit / std::max(meanOpen, 1e-7f), 0.0f, 1.0f);
    // No depth match: clamped to the mean share and no boost.
    const float low = std::min(v, local);
    v = low + (v - low) * conf;
    const float boost = (gain - 1.0f) * conf;
    const float s = std::max(v - highpass * local, (1.0f - highpass) * v) + boost * std::max(v - local, 0.0f);
    return s * open;
}

float ExpOpticalLength(float yA, float yB, float len, float scaleHeight)
{
    const float x = (yB - yA) / scaleHeight;
    // (1 - exp(-x)) / x, its series near 0.
    const float f = std::fabs(x) < 1e-3f ? 1.0f - 0.5f * x : (1.0f - std::exp(-x)) / x;
    return len * std::exp(-yA / scaleHeight) * f;
}

float LayerOpticalLength(float yA, float yB, float len, float top, float fall)
{
    const float lo = std::min(yA, yB);
    const float hi = std::max(yA, yB);
    const float dy = hi - lo;
    if (dy < 0.5f) // level: the density at the middle height
        return len * std::min(1.0f, std::exp(-(0.5f * (lo + hi) - top) / fall));
    const float below = std::clamp((top - lo) / dy, 0.0f, 1.0f);
    const float a = std::max(lo, top);
    const float above = a < hi ? fall * (std::exp(-(a - top) / fall) - std::exp(-(hi - top) / fall)) / dy : 0.0f;
    return len * (below + above);
}

LocalValley LocalValleyAt(const std::function<float(float x, float z)>& heightAt, float x, float z)
{
    float m = heightAt(x, z);
    float sum = 0.0f;
    for (int k = 0; k < 8; ++k)
    {
        const float a = static_cast<float>(k) * 0.785398f;
        const float h = heightAt(x + std::cos(a) * kValleyNearRadius, z + std::sin(a) * kValleyNearRadius);
        m = std::min(m, h);
        sum += h;
    }
    for (int k = 0; k < 6; ++k)
    {
        const float a = static_cast<float>(k) * 1.047198f + 0.5f;
        const float h = heightAt(x + std::cos(a) * kValleyFarRadius, z + std::sin(a) * kValleyFarRadius);
        m = std::min(m, h);
        sum += h;
    }
    LocalValley v;
    v.low = std::max(m, 0.0f);
    v.depth = sum / 14.0f - v.low;
    return v;
}

float LocalLowGround(const std::function<float(float x, float z)>& heightAt, float x, float z)
{
    return LocalValleyAt(heightAt, x, z).low;
}

float ValleyDepthWeight(float depth)
{
    return SmoothStep(kValleyDepthFrom, kValleyDepthFull, depth);
}

float ValleyMistTop(float lowGround, float n1, float n2)
{
    return lowGround + kValleyMistTop * (0.45f + 0.9f * n1) + (n2 - 0.5f) * 12.0f;
}

float ValleyMistDensity(float n1, float n2)
{
    return 0.35f + 1.15f * (n1 * 0.7f + n2 * 0.3f);
}

float AirOpticalDepth(const AirMedium& m, const float eye[3], const float target[3], float fromDistance, float mistTop,
                      float mistDensity)
{
    const float dx = target[0] - eye[0], dy = target[1] - eye[1], dz = target[2] - eye[2];
    const float d = std::sqrt(dx * dx + dy * dy + dz * dz);
    const float t0 = std::max(fromDistance, 0.0f);
    if (d <= t0)
        return 0.0f;
    const float len = d - t0;
    const float yA = eye[1] + dy / d * t0;
    const float yB = target[1];
    float tau = 0.0f;
    if (m.hazeSigma > 0.0f)
        tau += m.hazeSigma * ExpOpticalLength(std::max(yA, kSeaFloor), std::max(yB, kSeaFloor), len, kHazeScaleHeight);
    if (m.valleyMist > 0.0f)
        tau += m.valleyMist * mistDensity * LayerOpticalLength(yA, yB, len, mistTop, kValleyMistFall);
    return tau;
}

} // namespace Poseidon::remaster
