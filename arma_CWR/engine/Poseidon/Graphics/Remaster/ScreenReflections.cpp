#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{

float Smoothstep(float e0, float e1, float x)
{
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

SsrSurface SsrSurfaceOfStencil(std::uint8_t stencil)
{
    switch (stencil & ssrtag::kSurfaceBits)
    {
        case ssrtag::kTerrain:
            return SsrSurface::Terrain;
        case ssrtag::kWater:
            return SsrSurface::Water;
        case ssrtag::kRoad:
            return SsrSurface::Road;
        default:
            return SsrSurface::None;
    }
}

bool SsrCoveredStencil(std::uint8_t stencil)
{
    return (stencil & ssrtag::kCover) != 0;
}

SsrStencil SsrStencilForDraw(const render::RenderPassDescriptor& d, bool landLayer, bool tagging)
{
    using render::DepthMode;
    using render::ShaderFamily;
    SsrStencil s;
    if (!tagging)
        return s;
    if (d.depth == DepthMode::Shadow)
    {
        // The projected shadows count in the low bits; the tags above them do
        // not stop a shadow from landing on a tagged surface.
        s.shadowCompareMask = ssrtag::kShadowBits;
        return s;
    }
    std::uint8_t surface = ssrtag::kNone;
    if (d.shader == ShaderFamily::Water)
        surface = ssrtag::kWater;
    else if ((d.shader == ShaderFamily::Detail && landLayer) || d.shader == ShaderFamily::Grass)
        surface = ssrtag::kTerrain;
    else if (d.shader == ShaderFamily::Normal && d.surface == render::SurfaceMode::OnSurface)
        surface = ssrtag::kRoad;
    if (surface != ssrtag::kNone)
    {
        s.ref = surface;
        return s;
    }
    if (d.depth == DepthMode::Normal)
    {
        // A blended draw that writes depth (alpha textures: bush cards,
        // glass) covers the surface without hiding it; an opaque one hides it.
        if (d.blend != render::BlendMode::Opaque)
        {
            s.ref = ssrtag::kCover;
            s.writeMask = ssrtag::kCover | ssrtag::kShadowBits;
        }
        return s;
    }
    // No depth write: the pixel still shows the surface behind.
    s.writeMask = ssrtag::kShadowBits;
    return s;
}

std::uint8_t SsrReplace(std::uint8_t stored, const SsrStencil& s)
{
    return static_cast<std::uint8_t>((stored & ~s.writeMask) | (s.ref & s.writeMask));
}

std::uint8_t SsrShadowDraw(std::uint8_t stored, const SsrStencil& s, bool& passed)
{
    passed = (stored & s.shadowCompareMask) == 0;
    if (!passed)
        return stored;
    return static_cast<std::uint8_t>(stored == 0xFF ? 0xFF : stored + 1);
}

float SsrSelfTolerance(float depth)
{
    return kSsrSelfBase + kSsrSelfPerM * depth;
}

float SsrCoverOpacity(const float panel[3], const float behind[3])
{
    float d = 0.0f;
    for (int c = 0; c < 3; ++c)
        d = std::max(d, std::fabs(panel[c] - behind[c]));
    return Smoothstep(kSsrCoverSeeFrom, kSsrCoverSeeTo, d);
}

SsrRowHit SsrMarchRow(const float* sceneZ, const std::uint8_t* stencil, int n, float z0, float z1, float rz,
                      float jitter, const float* planeDist)
{
    SsrRowHit out;
    if (!sceneZ || n < 2)
        return out;
    // Screen position of t along the row (texel centres 0.5 .. n - 0.5) and
    // the perspective-correct ray depth there.
    const float s0 = 0.5f;
    const float dS = static_cast<float>(n - 1);
    const float k0 = 1.0f / z0, k1 = 1.0f / z1;
    auto at = [&](float t) { return std::clamp(static_cast<int>(s0 + dS * t), 0, n - 1); };
    auto rayZ = [&](float t) { return 1.0f / (k0 + (k1 - k0) * t); };
    auto travelTo = [&](float z, float t) { return std::fabs(rz) > 1e-3f ? std::fabs((z - z0) / rz) : t * dS; };
    const float tc = 1.0f;
    const float span = dS;
    const int steps = static_cast<int>(std::clamp(span * tc / kSsrStride, 4.0f, static_cast<float>(kSsrSteps)));
    const float dt = tc / static_cast<float>(steps);
    const float startT = kSsrStartSkip / std::max(span, 1e-3f);
    // The binary refine between two march positions (ssrRefine).
    auto refine = [&](float a, float b)
    {
        for (int j = 0; j < kSsrRefine; ++j)
        {
            const float m = 0.5f * (a + b);
            if (rayZ(m) >= sceneZ[at(m)])
                b = m;
            else
                a = m;
        }
        return b;
    };
    float tPrev = 0.0f;
    float zPrev = z0;
    bool behind = false;
    float behindT = 0.0f;
    float behindTravel = 0.0f;
    for (int i = 0; i < kSsrSteps; ++i)
    {
        if (i >= steps)
            break;
        const float t = std::min(dt * (static_cast<float>(i) + jitter) + startT, tc);
        const float z = rayZ(t);
        const int x = at(t);
        const float sz = sceneZ[x];
        const float thick = kSsrThickBase + kSsrThickPerM * sz;
        bool cand = std::max(zPrev, z) >= sz && std::min(zPrev, z) <= sz + thick;
        const bool covered = stencil && SsrCoveredStencil(stencil[x]);
        // The first uncovered texel after the panel: what is next to it, and
        // whether it stands in front of the ray (the panel is set in it).
        if (out.cover && out.afterT < 0.0f && !covered)
        {
            out.afterT = t;
            out.afterInFront = sz < kSsrSkyDepth && std::min(zPrev, z) > sz + thick;
        }
        if (cand)
        {
            if (stencil && SsrSurfaceOfStencil(stencil[x]) == SsrSurface::Water)
                cand = false; // water never reflects itself
            else if (planeDist && planeDist[x] < SsrSelfTolerance(sz))
                cand = false; // the ray's own surface
            else if (covered)
            {
                // A blended panel: kept (the first one), and the ray goes on.
                if (!out.cover)
                {
                    out.cover = true;
                    out.coverT = refine(tPrev, t);
                    out.coverTravel = travelTo(rayZ(out.coverT), out.coverT);
                }
                cand = false;
            }
        }
        const bool occluded = !cand && sz < kSsrSkyDepth && std::min(zPrev, z) > sz + thick;
        out.hidden += occluded ? 1 : 0;
        if (occluded && !behind && travelTo(z, t) < kSsrBehindNear)
        {
            behind = true;
            behindT = t;
            behindTravel = travelTo(z, t);
        }
        if (cand)
        {
            const float b = refine(tPrev, t);
            out.kind = SsrHit::Surface;
            out.t = b;
            out.travel = travelTo(rayZ(b), b);
            return out;
        }
        tPrev = t;
        zPrev = z;
    }
    out.t = tc;
    if (behind)
    {
        out.kind = SsrHit::BehindNear;
        out.t = behindT;
        out.travel = behindTravel;
    }
    else if (sceneZ[at(tc)] >= kSsrSkyDepth)
        out.kind = SsrHit::Sky;
    return out;
}

float SsrHiddenTrust(int hiddenSteps)
{
    return 1.0f - Smoothstep(kSsrHiddenFrom, kSsrHiddenTo, static_cast<float>(hiddenSteps));
}

float SsrUndersideFade(float travel)
{
    return 1.0f - Smoothstep(0.5f * kSsrBehindNear, kSsrBehindNear, travel);
}

float SsrEdgeFade(float u, float v)
{
    const float e = std::min(std::min(u, 1.0f - u), std::min(v, 1.0f - v));
    return Smoothstep(0.0f, kSsrEdgeFade, e);
}

float SsrBlurSpread(float pixelsH)
{
    return std::clamp(0.5f * pixelsH, 0.0f, kSsrBlurMaxSpread);
}

SsrStreak SsrStreakTaps(float pxV, float pxH, float maxLod)
{
    SsrStreak s;
    const float span = std::clamp(pxV, 0.0f, kSsrStreakMaxPx);
    const float n = std::clamp(std::ceil(span / std::max(pxH, 1.0f)), 1.0f, static_cast<float>(kSsrStreakTaps));
    s.taps = static_cast<int>(n);
    s.stepPx = span / n;
    s.lod = std::clamp(std::log2(std::max(std::max(pxH, s.stepPx), 1.0f)), 0.0f, std::max(maxLod, 0.0f));
    return s;
}

int SsrBlurTaps(float spread)
{
    return static_cast<int>(std::clamp(std::ceil(spread), 1.0f, static_cast<float>(kSsrBlurMaxTaps)));
}

float SsrBlurTapWeight(int k, int taps)
{
    const float x = static_cast<float>(k) / static_cast<float>(std::max(taps, 1));
    return std::exp(-2.0f * x * x);
}

float SsrBlurDepthWeight(float tz, float z)
{
    const float dz = (tz - z) / (0.03f * z + 0.1f);
    return 1.0f / (1.0f + dz * dz);
}

float SsrBlurSpreadWeight(float tapSpread, float spread)
{
    const float s = std::clamp(tapSpread / std::max(spread, 1e-4f), 0.0f, 1.0f);
    return s * s;
}

void SsrBlurRow(const float* value, const std::uint8_t* surface, const float* z, const float* spread, int n, float* out)
{
    for (int i = 0; i < n; ++i)
    {
        const float r = std::min(spread[i], kSsrBlurMaxSpread);
        if (surface[i] == 0 || r < kSsrBlurMinSpread)
        {
            out[i] = value[i];
            continue;
        }
        float sum = value[i];
        float wsum = 1.0f;
        const int taps = SsrBlurTaps(r);
        for (int k = 1; k <= taps; ++k)
        {
            const float o = r * static_cast<float>(k) / static_cast<float>(taps);
            for (const float sign : {-1.0f, 1.0f})
            {
                const int q = std::clamp(i + static_cast<int>(std::floor(sign * o + 0.5f)), 0, n - 1);
                if (surface[q] != surface[i])
                    continue;
                const float w =
                    SsrBlurTapWeight(k, taps) * SsrBlurDepthWeight(z[q], z[i]) * SsrBlurSpreadWeight(spread[q], r);
                sum += value[q] * w;
                wsum += w;
            }
        }
        out[i] = sum / wsum;
    }
}

SsrWaterBlend SsrWaterComposite(float skyShare, float ssrWater, float fade)
{
    SsrWaterBlend b;
    const float x = std::min(std::clamp(skyShare, 0.0f, 1.0f) * std::max(ssrWater, 0.0f), kSsrWaterSwapMax);
    const float f = std::clamp(fade, 0.0f, 1.0f);
    b.keep = 1.0f - f * x;
    b.swap = f * x;
    return b;
}

float SsrWaterRatio(float r, float s)
{
    return std::clamp(r / std::max(s, 1e-4f), 0.0f, kSsrWaterRatioMax);
}

} // namespace Poseidon::remaster
