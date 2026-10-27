#include <Poseidon/Graphics/Remaster/TerrainDetail.hpp>

#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster::terrain_detail
{

namespace
{

constexpr float kTwoPi = 6.2831853f;

float Fract(float x)
{
    return x - std::floor(x);
}

float Smoothstep(float a, float b, float x)
{
    const float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

std::array<float, 2> Hash2(float x, float y)
{
    // fract(p.xyx * (0.1031, 0.1030, 0.0973)); p3 += dot(p3, p3.yzx + 33.33);
    // fract((p3.xx + p3.yz) * p3.zy)
    float p0 = Fract(x * 0.1031f);
    float p1 = Fract(y * 0.1030f);
    float p2 = Fract(x * 0.0973f);
    const float d = p0 * (p1 + 33.33f) + p1 * (p2 + 33.33f) + p2 * (p0 + 33.33f);
    p0 += d;
    p1 += d;
    p2 += d;
    return {Fract((p0 + p1) * p2), Fract((p0 + p2) * p1)};
}

float CellAngle(float cellX, float cellY)
{
    return kTwoPi * Hash2(cellX + 17.0f + kZoom, cellY + 17.0f + kZoom)[0];
}

std::array<float, 2> CellJitter(float cellX, float cellY)
{
    const std::array<float, 2> h = Hash2(cellX + kZoom, cellY + kZoom);
    return {(h[0] - 0.5f) * kJitterTexels, (h[1] - 0.5f) * kJitterTexels};
}

std::array<float, 2> CellSample(float px, float py, float cellX, float cellY)
{
    const float cx = (cellX + 0.5f) * kCellTexels;
    const float cy = (cellY + 0.5f) * kCellTexels;
    const float rx = (px - cx) * kZoom;
    const float ry = (py - cy) * kZoom;
    const float a = CellAngle(cellX, cellY);
    const float ca = std::cos(a);
    const float sa = std::sin(a);
    const std::array<float, 2> jit = CellJitter(cellX, cellY);
    // GLSL mat2(cos, sin, -sin, cos) * rel: columns (cos, sin) and (-sin, cos).
    return {cx + ca * rx - sa * ry + jit[0], cy + sa * rx + ca * ry + jit[1]};
}

CellBlend CellsAround(float px, float py)
{
    CellBlend b;
    const float gx = px / kCellTexels - 0.5f;
    const float gy = py / kCellTexels - 0.5f;
    b.i0 = std::floor(gx);
    b.j0 = std::floor(gy);
    const float fx = gx - b.i0;
    const float fy = gy - b.j0;
    b.w = {(1.0f - fx) * (1.0f - fy), fx * (1.0f - fy), (1.0f - fx) * fy, fx * fy};
    return b;
}

float VariancePreservingBlend(const std::array<float, 4>& w, const std::array<float, 4>& h)
{
    float acc = 0.0f;
    float w2 = 0.0f;
    for (int k = 0; k < 4; ++k)
    {
        acc += w[k] * h[k];
        w2 += w[k] * w[k];
    }
    return acc / std::sqrt(std::max(w2, 1e-4f));
}

float SoftLimit(float m)
{
    return m / std::sqrt(1.0f + m * m / (kLimit * kLimit));
}

float DistanceFade(float distance)
{
    return 1.0f - Smoothstep(kFadeStart, kMaxDistance, distance);
}

float FootprintLod(float l1, float l2)
{
    const float pmax = std::max(l1, l2);
    const float pmin = std::max(std::min(l1, l2), 1e-6f);
    return std::log2(std::max(pmax / std::min(kMaxAniso, pmax / pmin), 1e-6f));
}

float NearFade(float baseLod)
{
    return Smoothstep(kNearFromLod, kNearToLod, baseLod + std::log2(kZoom));
}

float AntiTileWeight(float lod, float distance)
{
    return kAntiTile * std::clamp(lod, 0.0f, 1.0f) * DistanceFade(distance);
}

float KeptBoost(float lod, float distance)
{
    const float boost = std::clamp(std::exp2(lod * 0.5f * kKeptRate), 1.0f, kKeptCap);
    const float t = Smoothstep(kKeptFadeStart, kMaxDistance, distance);
    return boost + (1.0f - boost) * t;
}

std::array<float, 3> FrameConstants(const LightingTuning& tuning, bool gateOn)
{
    if (!gateOn || !FeatureOn(tuning.features, Feature::TerrainDetail))
        return {0.0f, 0.0f, 0.0f};
    return {std::max(tuning.detailStrength, 0.0f), kKeptRate, kAntiTile};
}

} // namespace Poseidon::remaster::terrain_detail
