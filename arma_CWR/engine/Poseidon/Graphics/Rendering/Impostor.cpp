#include <Poseidon/Graphics/Rendering/Impostor.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Poseidon::render::impostor
{

const Settings& Get()
{
    static const Settings s = []
    {
        Settings out;
        const char* on = std::getenv("POSEIDON_XP_IMPOSTOR");
        out.on = on && on[0] == '1' && on[1] == '\0';
        if (const char* d = std::getenv("POSEIDON_XP_IMPOSTOR_DIST"))
        {
            const float v = static_cast<float>(std::atof(d));
            if (v >= 50.0f)
                out.distance = v;
        }
        if (const char* far = std::getenv("POSEIDON_XP_IMPOSTOR_FAR"))
            out.far = far[0] == '1' && far[1] == '\0';
        if (const char* l = std::getenv("POSEIDON_XP_IMPOSTOR_LOD0"))
            out.lod0 = !(l[0] == '0' && l[1] == '\0');
        return out;
    }();
    return s;
}

FramePick PickFrames(float dirX, float dirZ, int frames)
{
    FramePick p;
    if (frames <= 0)
        return p;
    const float len2 = dirX * dirX + dirZ * dirZ;
    if (!(len2 > 1e-12f))
        return p;
    constexpr float kTwoPi = 6.28318530717958647692f;
    float f = std::atan2(dirX, dirZ) / kTwoPi * static_cast<float>(frames);
    f = std::fmod(f, static_cast<float>(frames));
    if (f < 0.0f)
        f += static_cast<float>(frames);
    const float f0 = std::floor(f);
    p.f0 = static_cast<int>(f0) % frames;
    p.f1 = (p.f0 + 1) % frames;
    p.blend = f - f0;
    return p;
}

void Downsample(const Image& albedo, const Image& normal, Image& outAlbedo, Image& outNormal,
                const std::uint8_t fill[4], const std::uint8_t fillN[4])
{
    const int w = std::max(albedo.w / 2, 1);
    const int h = std::max(albedo.h / 2, 1);
    outAlbedo.w = outNormal.w = w;
    outAlbedo.h = outNormal.h = h;
    outAlbedo.rgba.assign(static_cast<size_t>(w) * h * 4, 0);
    outNormal.rgba.assign(static_cast<size_t>(w) * h * 4, 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            float sa = 0.0f;
            float c[3] = {0, 0, 0};
            float n[4] = {0, 0, 0, 0};
            int taps = 0;
            for (int dy = 0; dy < 2; ++dy)
                for (int dx = 0; dx < 2; ++dx)
                {
                    const int sx = std::min(x * 2 + dx, albedo.w - 1);
                    const int sy = std::min(y * 2 + dy, albedo.h - 1);
                    const size_t i = (static_cast<size_t>(sy) * albedo.w + sx) * 4;
                    const float a = albedo.rgba[i + 3] / 255.0f;
                    sa += a;
                    for (int k = 0; k < 3; ++k)
                        c[k] += a * albedo.rgba[i + k];
                    for (int k = 0; k < 4; ++k)
                        n[k] += a * normal.rgba[i + k];
                    ++taps;
                }
            const size_t o = (static_cast<size_t>(y) * w + x) * 4;
            if (sa > 1e-6f)
            {
                for (int k = 0; k < 3; ++k)
                    outAlbedo.rgba[o + k] = static_cast<std::uint8_t>(std::clamp(c[k] / sa + 0.5f, 0.0f, 255.0f));
                for (int k = 0; k < 4; ++k)
                    outNormal.rgba[o + k] = static_cast<std::uint8_t>(std::clamp(n[k] / sa + 0.5f, 0.0f, 255.0f));
                outAlbedo.rgba[o + 3] =
                    static_cast<std::uint8_t>(std::clamp(sa / static_cast<float>(taps) * 255.0f + 0.5f, 0.0f, 255.0f));
            }
            else
            {
                for (int k = 0; k < 3; ++k)
                    outAlbedo.rgba[o + k] = fill[k];
                outAlbedo.rgba[o + 3] = 0;
                for (int k = 0; k < 4; ++k)
                    outNormal.rgba[o + k] = fillN[k];
            }
        }
}

float Coverage(const Image& albedo, float ref)
{
    const size_t n = static_cast<size_t>(albedo.w) * albedo.h;
    if (n == 0)
        return 0.0f;
    const int r = static_cast<int>(std::ceil(ref * 255.0f));
    size_t covered = 0;
    for (size_t i = 0; i < n; ++i)
        covered += albedo.rgba[i * 4 + 3] >= r ? 1 : 0;
    return static_cast<float>(covered) / static_cast<float>(n);
}

namespace
{
float CoverageScaled(const Image& albedo, float ref, float scale)
{
    const size_t n = static_cast<size_t>(albedo.w) * albedo.h;
    if (n == 0)
        return 0.0f;
    size_t covered = 0;
    for (size_t i = 0; i < n; ++i)
        covered += (albedo.rgba[i * 4 + 3] / 255.0f) * scale >= ref ? 1 : 0;
    return static_cast<float>(covered) / static_cast<float>(n);
}
} // namespace

float KeepCoverage(Image& albedo, float ref, float target)
{
    float lo = 0.25f;
    float hi = 8.0f;
    float best = 1.0f;
    float bestErr = std::fabs(CoverageScaled(albedo, ref, 1.0f) - target);
    for (int it = 0; it < 14; ++it)
    {
        const float mid = 0.5f * (lo + hi);
        const float c = CoverageScaled(albedo, ref, mid);
        const float err = std::fabs(c - target);
        if (err < bestErr)
        {
            bestErr = err;
            best = mid;
        }
        if (c < target)
            lo = mid;
        else
            hi = mid;
    }
    if (best != 1.0f)
    {
        const size_t n = static_cast<size_t>(albedo.w) * albedo.h;
        for (size_t i = 0; i < n; ++i)
        {
            const float a = albedo.rgba[i * 4 + 3] * best;
            albedo.rgba[i * 4 + 3] = static_cast<std::uint8_t>(std::clamp(a + 0.5f, 0.0f, 255.0f));
        }
    }
    return best;
}

void BuildChain(const Image& albedo0, const Image& normal0, const std::uint8_t fill[4], const std::uint8_t fillN[4],
                std::vector<Image>& albedoChain, std::vector<Image>& normalChain)
{
    albedoChain.clear();
    normalChain.clear();
    albedoChain.push_back(albedo0);
    normalChain.push_back(normal0);
    const float target = Coverage(albedo0, kAlphaRef);
    while (albedoChain.back().w > 1 || albedoChain.back().h > 1)
    {
        Image a;
        Image n;
        // Halve the level above (already kept), then keep this one against
        // level 0's coverage: the target never drifts with the chain.
        Downsample(albedoChain.back(), normalChain.back(), a, n, fill, fillN);
        KeepCoverage(a, kAlphaRef, target);
        albedoChain.push_back(std::move(a));
        normalChain.push_back(std::move(n));
    }
}

} // namespace Poseidon::render::impostor
