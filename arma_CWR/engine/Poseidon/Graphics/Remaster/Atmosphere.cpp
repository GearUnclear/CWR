#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kRg = atmo::kGroundRadius;
constexpr float kRt = atmo::kTopRadius;

struct V3
{
    float x, y, z;
};
V3 Add(V3 a, V3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
V3 Mul(V3 a, float s)
{
    return {a.x * s, a.y * s, a.z * s};
}
float Dot(V3 a, V3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
float Len(V3 a)
{
    return std::sqrt(Dot(a, a));
}

float Clamp(float v, float lo, float hi)
{
    return std::min(hi, std::max(lo, v));
}

// Ray-sphere distances from radius r along zenith cosine mu. The discriminants
// are written as (r mu)^2 plus a difference of squares of nearby radii, so they
// keep their precision in float near the ground (r^2 (mu^2 - 1) + R^2 cancels
// to a few metres of error there). The GPU pass uses the same forms.
float DistanceToTop(float r, float mu)
{
    const float rmu = r * mu;
    const float disc = rmu * rmu + (kRt - r) * (kRt + r);
    return std::max(0.0f, -rmu + std::sqrt(std::max(disc, 0.0f)));
}

float DistanceToBottom(float r, float mu)
{
    const float rmu = r * mu;
    const float disc = rmu * rmu - (r - kRg) * (r + kRg);
    return std::max(0.0f, -rmu - std::sqrt(std::max(disc, 0.0f)));
}

bool HitsGround(float r, float mu)
{
    const float rmu = r * mu;
    return mu < 0.0f && rmu * rmu - (r - kRg) * (r + kRg) >= 0.0f;
}

struct Medium
{
    float scatR[3];
    float scatM;
    float ext[3];
};

Medium SampleMedium(float h, float haze)
{
    const float hh = std::max(h, 0.0f);
    const float rr = std::exp(-hh / atmo::kRayleighScaleHeight);
    const float rm = std::exp(-hh / atmo::kMieScaleHeight);
    const float ro = std::max(0.0f, 1.0f - std::fabs(h - atmo::kOzoneCentre) / atmo::kOzoneHalfWidth);
    Medium m;
    m.scatM = atmo::kMieScattering * haze * rm;
    for (int c = 0; c < 3; ++c)
    {
        m.scatR[c] = atmo::kRayleighScattering[c] * rr;
        m.ext[c] = m.scatR[c] + atmo::kMieExtinction * haze * rm + atmo::kOzoneAbsorption[c] * ro;
    }
    return m;
}

// Bilinear on a table whose texels sit on the grid points 0..n-1 (the
// transmittance and multiple-scattering tables).
Rgb GridLookup(const std::vector<float>& lut, int w, int h, float fx, float fy)
{
    const int x0 = std::min(static_cast<int>(fx), w - 2);
    const int y0 = std::min(static_cast<int>(fy), h - 2);
    const float tx = fx - static_cast<float>(x0);
    const float ty = fy - static_cast<float>(y0);
    float out[3];
    for (int c = 0; c < 3; ++c)
    {
        const float a = lut[(y0 * w + x0) * 3 + c];
        const float b = lut[(y0 * w + x0 + 1) * 3 + c];
        const float e = lut[((y0 + 1) * w + x0) * 3 + c];
        const float f = lut[((y0 + 1) * w + x0 + 1) * 3 + c];
        out[c] = (a + (b - a) * tx) * (1.0f - ty) + (e + (f - e) * tx) * ty;
    }
    return Rgb(out[0], out[1], out[2]);
}

void BuildTransmittance(std::vector<float>& lut, float haze)
{
    constexpr int W = kTransmittanceWidth;
    constexpr int H = kTransmittanceHeight;
    lut.assign(static_cast<size_t>(W) * H * 3, 0.0f);
    const float hTop = std::sqrt(kRt * kRt - kRg * kRg);
    for (int j = 0; j < H; ++j)
    {
        const float xR = static_cast<float>(j) / (H - 1);
        const float rho = hTop * xR;
        const float r = std::sqrt(rho * rho + kRg * kRg);
        for (int i = 0; i < W; ++i)
        {
            const float xMu = static_cast<float>(i) / (W - 1);
            const float dMin = kRt - r;
            const float dMax = rho + hTop;
            const float d = dMin + xMu * (dMax - dMin);
            float mu = d == 0.0f ? 1.0f : (hTop * hTop - rho * rho - d * d) / (2.0f * r * d);
            mu = Clamp(mu, -1.0f, 1.0f);
            const float len = DistanceToTop(r, mu);
            constexpr int kSteps = 64;
            const float dt = len / kSteps;
            double od[3] = {0, 0, 0};
            for (int s = 0; s < kSteps; ++s)
            {
                const float t = (static_cast<float>(s) + 0.5f) * dt;
                const float rs = std::sqrt(r * r + t * t + 2.0f * r * mu * t);
                const Medium m = SampleMedium(rs - kRg, haze);
                for (int c = 0; c < 3; ++c)
                    od[c] += static_cast<double>(m.ext[c]) * dt;
            }
            for (int c = 0; c < 3; ++c)
                lut[(static_cast<size_t>(j) * W + i) * 3 + c] = static_cast<float>(std::exp(-od[c]));
        }
    }
}

void BuildMultiScatter(std::vector<float>& lut, const AtmosphereTables& tables, float haze)
{
    constexpr int N = kMultiScatterSize;
    lut.assign(static_cast<size_t>(N) * N * 3, 0.0f);
    constexpr int kDirs = 64;
    constexpr int kSteps = 20;
    V3 dirs[kDirs];
    const float golden = kPi * (3.0f - std::sqrt(5.0f));
    for (int k = 0; k < kDirs; ++k)
    {
        const float y = 1.0f - 2.0f * (static_cast<float>(k) + 0.5f) / kDirs;
        const float rad = std::sqrt(std::max(0.0f, 1.0f - y * y));
        const float th = golden * static_cast<float>(k);
        dirs[k] = {rad * std::cos(th), y, rad * std::sin(th)};
    }
    const float isotropic = 1.0f / (4.0f * kPi);
    for (int j = 0; j < N; ++j)
    {
        const float h = std::max(10.0f, (kRt - kRg) * static_cast<float>(j) / (N - 1));
        const float r = kRg + std::min(h, kRt - kRg - 10.0f);
        for (int i = 0; i < N; ++i)
        {
            const float muS = -1.0f + 2.0f * static_cast<float>(i) / (N - 1);
            const V3 sun = {std::sqrt(std::max(0.0f, 1.0f - muS * muS)), muS, 0.0f};
            const V3 origin = {0.0f, r, 0.0f};
            double l2[3] = {0, 0, 0}, fms[3] = {0, 0, 0};
            for (const V3& dir : dirs)
            {
                const float mu = dir.y;
                const bool ground = HitsGround(r, mu);
                const float tMax = ground ? DistanceToBottom(r, mu) : DistanceToTop(r, mu);
                const float dt = tMax / kSteps;
                float tr[3] = {1, 1, 1};
                float lp[3] = {0, 0, 0}, lf[3] = {0, 0, 0};
                for (int s = 0; s < kSteps; ++s)
                {
                    const float t = (static_cast<float>(s) + 0.3f) * dt;
                    const V3 p = Add(origin, Mul(dir, t));
                    const float rp = Len(p);
                    const Medium m = SampleMedium(rp - kRg, haze);
                    const float muSp = Dot(Mul(p, 1.0f / rp), sun);
                    const Rgb ts = AtmosphereTransmittance(tables, rp, muSp);
                    const float tsc[3] = {ts.r, ts.g, ts.b};
                    for (int c = 0; c < 3; ++c)
                    {
                        const float sig = m.scatR[c] + m.scatM;
                        const float ext = std::max(m.ext[c], 1e-12f);
                        const float stepT = std::exp(-ext * dt);
                        const float src = sig * isotropic * tsc[c];
                        lp[c] += tr[c] * (src - src * stepT) / ext;
                        lf[c] += tr[c] * (sig - sig * stepT) / ext;
                        tr[c] *= stepT;
                    }
                }
                if (ground)
                {
                    const V3 g = Add(origin, Mul(dir, tMax));
                    const float rg = Len(g);
                    const float muG = Dot(Mul(g, 1.0f / rg), sun);
                    const Rgb tg = AtmosphereTransmittance(tables, rg, muG);
                    const float tgc[3] = {tg.r, tg.g, tg.b};
                    for (int c = 0; c < 3; ++c)
                        lp[c] += tr[c] * tgc[c] * std::max(muG, 0.0f) * atmo::kGroundAlbedo / kPi;
                }
                for (int c = 0; c < 3; ++c)
                {
                    l2[c] += lp[c];
                    fms[c] += lf[c];
                }
            }
            for (int c = 0; c < 3; ++c)
            {
                const double second = l2[c] / kDirs;
                const double f = std::min(fms[c] / kDirs, 0.99);
                lut[(static_cast<size_t>(j) * N + i) * 3 + c] = static_cast<float>(second / (1.0 - f));
            }
        }
    }
}

V3 SunDirection(float sunElevationRad)
{
    return {std::cos(sunElevationRad), std::sin(sunElevationRad), 0.0f};
}

V3 SkyViewDirection(float elevation, float azimuth)
{
    return {std::cos(elevation) * std::cos(azimuth), std::sin(elevation), std::cos(elevation) * std::sin(azimuth)};
}

float SmoothStep(float e0, float e1, float x)
{
    const float t = Clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

bool AtmosphereTables::Valid() const
{
    return haze >= 0.0f &&
           transmittance.size() == static_cast<size_t>(kTransmittanceWidth) * kTransmittanceHeight * 3 &&
           multiScatter.size() == static_cast<size_t>(kMultiScatterSize) * kMultiScatterSize * 3;
}

AtmosphereTables BuildAtmosphereTables(float haze)
{
    AtmosphereTables t;
    BuildTransmittance(t.transmittance, haze);
    t.haze = haze;
    // The multiple-scattering march reads the finished transmittance table.
    std::vector<float> ms;
    BuildMultiScatter(ms, t, haze);
    t.multiScatter = std::move(ms);
    return t;
}

Rgb AtmosphereTransmittance(const AtmosphereTables& t, float r, float mu)
{
    if (HitsGround(r, mu))
        return Rgb(0.0f, 0.0f, 0.0f);
    const float hTop = std::sqrt(kRt * kRt - kRg * kRg);
    const float rho = std::sqrt(std::max((r - kRg) * (r + kRg), 0.0f));
    const float d = DistanceToTop(r, mu);
    const float dMin = kRt - r;
    const float dMax = rho + hTop;
    const float xMu = dMax > dMin ? (d - dMin) / (dMax - dMin) : 0.0f;
    const float xR = rho / hTop;
    return GridLookup(t.transmittance, kTransmittanceWidth, kTransmittanceHeight,
                      Clamp(xMu, 0.0f, 1.0f) * (kTransmittanceWidth - 1),
                      Clamp(xR, 0.0f, 1.0f) * (kTransmittanceHeight - 1));
}

Rgb AtmosphereMultiScatter(const AtmosphereTables& t, float height, float muS)
{
    return GridLookup(t.multiScatter, kMultiScatterSize, kMultiScatterSize,
                      Clamp(muS * 0.5f + 0.5f, 0.0f, 1.0f) * (kMultiScatterSize - 1),
                      Clamp(height / (kRt - kRg), 0.0f, 1.0f) * (kMultiScatterSize - 1));
}

float AtmosphereRayleighPhase(float cosTheta)
{
    return 3.0f / (16.0f * kPi) * (1.0f + cosTheta * cosTheta);
}

float AtmosphereMiePhase(float cosTheta)
{
    // The broad lobe (Cornette-Shanks) and the aureole (Henyey-Greenstein),
    // each normalised over the sphere.
    const float g = atmo::kMieG;
    const float k = 3.0f / (8.0f * kPi) * (1.0f - g * g) / (2.0f + g * g);
    const float broad =
        k * (1.0f + cosTheta * cosTheta) / std::pow(std::max(1.0f + g * g - 2.0f * g * cosTheta, 1e-6f), 1.5f);
    const float ga = atmo::kMieAureoleG;
    const float peak =
        (1.0f - ga * ga) / (4.0f * kPi * std::pow(std::max(1.0f + ga * ga - 2.0f * ga * cosTheta, 1e-7f), 1.5f));
    return broad + (peak - broad) * atmo::kMieAureoleShare;
}

float SkyViewRowElevation(int row)
{
    const float v = (static_cast<float>(row) + 0.5f) / kSkyViewSize;
    const float x = 2.0f * v - 1.0f;
    return (x < 0.0f ? -1.0f : 1.0f) * x * x * (kPi * 0.5f);
}

float SkyViewColumnAzimuth(int col)
{
    const float u = (static_cast<float>(col) + 0.5f) / kSkyViewSize;
    return kPi * u * u;
}

namespace
{
// The sky-view march along `dir` (the sun frame of SunDirection): Rayleigh
// single scattering with its phase plus multiple scattering into lr, Mie single
// scattering without its phase into lm.
void MarchSkyView(const AtmosphereTables& t, V3 dir, float sunElevationRad, float lr[3], float lm[3]);
} // namespace

Rgb SkyViewTexel(const AtmosphereTables& t, int region, int col, int row, float sunElevationRad)
{
    const V3 dir = SkyViewDirection(SkyViewRowElevation(row), SkyViewColumnAzimuth(col));
    float lr[3] = {0, 0, 0}, lm[3] = {0, 0, 0};
    MarchSkyView(t, dir, sunElevationRad, lr, lm);
    return region == 0 ? Rgb(lr[0], lr[1], lr[2]) : Rgb(lm[0], lm[1], lm[2]);
}

Rgb SkyRadianceToward(const AtmosphereTables& t, float elevationRad, float azimuthFromSunRad, float sunElevationRad)
{
    const V3 dir = SkyViewDirection(elevationRad, azimuthFromSunRad);
    float lr[3] = {0, 0, 0}, lm[3] = {0, 0, 0};
    MarchSkyView(t, dir, sunElevationRad, lr, lm);
    const float phase = AtmosphereMiePhase(Dot(dir, SunDirection(sunElevationRad)));
    return Rgb(lr[0] + lm[0] * phase, lr[1] + lm[1] * phase, lr[2] + lm[2] * phase);
}

namespace
{
void MarchSkyView(const AtmosphereTables& t, V3 dir, float sunElevationRad, float lr[3], float lm[3])
{
    const float r0 = kRg + std::max(kSkyViewerHeight, 1.0f);
    const V3 origin = {0.0f, r0, 0.0f};
    const V3 sun = SunDirection(sunElevationRad);
    const float mu = dir.y;
    const bool ground = HitsGround(r0, mu);
    const float tMax = ground ? DistanceToBottom(r0, mu) : DistanceToTop(r0, mu);
    const float phaseR = AtmosphereRayleighPhase(Dot(dir, sun));
    float tr[3] = {1, 1, 1};
    float tPrev = 0.0f;
    for (int k = 0; k < kSkyViewSteps; ++k)
    {
        // Steps grow quadratically with distance: dense near the eye, where the
        // haze is.
        const float f1 = static_cast<float>(k + 1) / kSkyViewSteps;
        const float tNext = tMax * f1 * f1;
        const float dt = tNext - tPrev;
        const float tt = tPrev + 0.3f * dt;
        tPrev = tNext;
        const V3 p = Add(origin, Mul(dir, tt));
        const float rp = Len(p);
        const Medium m = SampleMedium(rp - kRg, t.haze);
        const float muSp = Dot(Mul(p, 1.0f / rp), sun);
        const Rgb ts = AtmosphereTransmittance(t, rp, muSp);
        const Rgb ms = AtmosphereMultiScatter(t, rp - kRg, muSp);
        const float tsc[3] = {ts.r, ts.g, ts.b};
        const float msc[3] = {ms.r, ms.g, ms.b};
        for (int c = 0; c < 3; ++c)
        {
            const float ext = std::max(m.ext[c], 1e-12f);
            const float stepT = std::exp(-ext * dt);
            const float srcR = m.scatR[c] * phaseR * tsc[c] + (m.scatR[c] + m.scatM) * msc[c];
            const float srcM = m.scatM * tsc[c];
            lr[c] += tr[c] * (srcR - srcR * stepT) / ext;
            lm[c] += tr[c] * (srcM - srcM * stepT) / ext;
            tr[c] *= stepT;
        }
    }
}
} // namespace

bool SkyViewTable::Valid() const
{
    return rgba.size() == static_cast<size_t>(kSkyViewRegions) * kSkyViewSize * kSkyViewSize * 4;
}

Rgb SkyViewTable::At(int x, int y) const
{
    const float* p = &rgba[(static_cast<size_t>(y) * kSkyViewRegions * kSkyViewSize + x) * 4];
    return Rgb(p[0], p[1], p[2]);
}

float SkyViewTable::AlphaAt(int x, int y) const
{
    return rgba[(static_cast<size_t>(y) * kSkyViewRegions * kSkyViewSize + x) * 4 + 3];
}

Rgb SampleSkyView(const SkyViewTable& table, float elevationRad, float azimuthFromSunRad, float cosToSun)
{
    constexpr int W = kSkyViewSize;
    constexpr int H = kSkyViewSize;
    if (!table.Valid())
        return Rgb();
    const float phi = Clamp(azimuthFromSunRad, 0.0f, kPi);
    const float u = std::sqrt(phi / kPi);
    const float l = Clamp(elevationRad, -kPi * 0.5f, kPi * 0.5f);
    const float v = 0.5f + 0.5f * (l < 0.0f ? -1.0f : 1.0f) * std::sqrt(std::fabs(l) / (kPi * 0.5f));
    // Texel centres at (i + 0.5) / n, as a linear-filtered texture.
    const float fx = Clamp(u * W - 0.5f, 0.0f, W - 1.0f);
    const float fy = Clamp(v * H - 0.5f, 0.0f, H - 1.0f);
    const int x0 = std::min(static_cast<int>(fx), W - 2);
    const int y0 = std::min(static_cast<int>(fy), H - 2);
    const float tx = fx - static_cast<float>(x0);
    const float ty = fy - static_cast<float>(y0);
    auto bilinear = [&](int region)
    {
        const int base = region * W;
        const Rgb a = table.At(base + x0, y0), b = table.At(base + x0 + 1, y0);
        const Rgb e = table.At(base + x0, y0 + 1), f = table.At(base + x0 + 1, y0 + 1);
        return (a + (b + a * -1.0f) * tx) * (1.0f - ty) + (e + (f + e * -1.0f) * tx) * ty;
    };
    const Rgb rayleigh = bilinear(0);
    const Rgb mie = bilinear(1);
    return rayleigh + mie * AtmosphereMiePhase(cosToSun);
}

Rgb SkyViewRowMean(const SkyViewTable& table, int row)
{
    const float l = SkyViewRowElevation(row);
    const V3 sun = SunDirection(table.sunElevationRad);
    Rgb sum;
    for (int a = 0; a < kSkyViewMeanTaps; ++a)
    {
        const float phi = (static_cast<float>(a) + 0.5f) / kSkyViewMeanTaps * kPi;
        const V3 dir = SkyViewDirection(l, phi);
        sum = sum + SampleSkyView(table, l, phi, Dot(dir, sun));
    }
    return sum * (1.0f / kSkyViewMeanTaps);
}

float SkyViewRowGlow(float sunElevationRad, int row)
{
    const float l = SkyViewRowElevation(row);
    const V3 sun = SunDirection(sunElevationRad);
    float sum = 0.0f;
    for (int a = 0; a < kSkyViewMeanTaps; ++a)
    {
        const float phi = (static_cast<float>(a) + 0.5f) / kSkyViewMeanTaps * kPi;
        const float c = std::max(Dot(SkyViewDirection(l, phi), sun), 0.0f);
        const float c2 = c * c;
        sum += c2 * c2 * c2;
    }
    return sum * (1.0f / kSkyViewMeanTaps);
}

void BuildSkyViewTable(const AtmosphereTables& t, float sunElevationRad, SkyViewTable& out)
{
    constexpr int W = kSkyViewSize;
    constexpr int stride = kSkyViewRegions * W;
    out.rgba.assign(static_cast<size_t>(stride) * W * 4, 0.0f);
    out.sunElevationRad = sunElevationRad;
    out.haze = t.haze;
    auto put = [&out](int x, int y, const Rgb& c, float a)
    {
        float* p = &out.rgba[(static_cast<size_t>(y) * stride + x) * 4];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
        p[3] = a;
    };
    for (int row = 0; row < W; ++row)
        for (int col = 0; col < W; ++col)
        {
            put(col, row, SkyViewTexel(t, 0, col, row, sunElevationRad), 1.0f);
            put(W + col, row, SkyViewTexel(t, 1, col, row, sunElevationRad), 1.0f);
        }
    for (int row = 0; row < W; ++row)
    {
        const Rgb mean = SkyViewRowMean(out, row);
        const float glow = SkyViewRowGlow(sunElevationRad, row);
        for (int col = 0; col < W; ++col)
            put(2 * W + col, row, mean, glow);
    }
}

bool SkyTableNeedsRebuild(bool built, float builtSunElevationRad, float builtHaze, float sunElevationRad, float haze)
{
    if (!built || builtHaze != haze)
        return true;
    return std::fabs(sunElevationRad - builtSunElevationRad) * (180.0f / kPi) >= kSkyTableRebuildDeg;
}

float SkyDayFade(float sunElevationDeg)
{
    return SmoothStep(kSkyNightFadeEndDeg, kSkyNightFadeTopDeg, sunElevationDeg);
}

float SkyOvercastCloudFloor(float skyThrough)
{
    // Weather::SetSky: sky-through 0.7 at overcast 0.5 is oblacno.pac, 0.1 at
    // 0.66 is zatazeno.pac, linear in between.
    return Clamp((0.7f - skyThrough) / 0.6f, 0.0f, 1.0f);
}

float SkyCloudSunShare(float sunElevationDeg)
{
    return 0.7f * SmoothStep(-3.0f, 2.0f, sunElevationDeg);
}

} // namespace Poseidon::remaster
