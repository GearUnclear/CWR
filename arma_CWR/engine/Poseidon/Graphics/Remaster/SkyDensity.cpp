#include <Poseidon/Graphics/Remaster/SkyDensity.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{

constexpr float kPi = 3.14159265f;
constexpr float kDeg = 180.0f / kPi;

float Clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

float Smoothstep(float a, float b, float x)
{
    const float t = Clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}

std::uint8_t ToByte(float v)
{
    return static_cast<std::uint8_t>(Clamp01(v) * 255.0f + 0.5f);
}

// 5-tap binomial blur of one plane in place: u (x) wraps, v (y) clamps.
void BlurPlane(std::vector<float>& p, int w, int h)
{
    static const float kW[5] = {1.0f / 16, 4.0f / 16, 6.0f / 16, 4.0f / 16, 1.0f / 16};
    std::vector<float> tmp(p.size());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            float s = 0.0f;
            for (int k = -2; k <= 2; ++k)
                s += kW[k + 2] * p[static_cast<size_t>(y) * w + ((x + k) % w + w) % w];
            tmp[static_cast<size_t>(y) * w + x] = s;
        }
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            float s = 0.0f;
            for (int k = -2; k <= 2; ++k)
            {
                const int yy = std::min(std::max(y + k, 0), h - 1);
                s += kW[k + 2] * tmp[static_cast<size_t>(yy) * w + x];
            }
            p[static_cast<size_t>(y) * w + x] = s;
        }
}

} // namespace

SkyUv SkyDomeUv(float x, float y, float z)
{
    const float len = std::sqrt(x * x + y * y + z * z);
    SkyUv uv;
    if (!(len > 0.0f))
        return uv;
    const float az = std::atan2(x, z) * kDeg;
    const float e = std::asin(std::max(-1.0f, std::min(1.0f, y / len))) * kDeg;
    uv.u = az / 180.0f;
    uv.v = SkyDomeV(e);
    return uv;
}

float SkyDomeV(float elevationDeg)
{
    const float* e = kSkyDomeRingElevationDeg;
    const float* v = kSkyDomeRingV;
    if (elevationDeg <= e[0])
        return v[0];
    for (int i = 1; i < kSkyDomeRings; ++i)
    {
        if (elevationDeg < e[i])
            return v[i - 1] + (v[i] - v[i - 1]) * (elevationDeg - e[i - 1]) / (e[i] - e[i - 1]);
    }
    return v[kSkyDomeRings - 1];
}

float SkyDomeElevationDeg(float vIn)
{
    const float* e = kSkyDomeRingElevationDeg;
    const float* v = kSkyDomeRingV;
    // v falls as the elevation rises.
    if (vIn >= v[0])
        return e[0];
    for (int i = 1; i < kSkyDomeRings; ++i)
    {
        if (vIn > v[i])
            return e[i - 1] + (e[i] - e[i - 1]) * (vIn - v[i - 1]) / (v[i] - v[i - 1]);
    }
    return e[kSkyDomeRings - 1];
}

bool SkyDensityMap::Valid() const
{
    return width > 0 && height > 0 && rgba.size() == static_cast<size_t>(width) * height * 4;
}

float SkyDensityMap::R(int x, int y) const
{
    return rgba[(static_cast<size_t>(y) * width + x) * 4 + 0] / 255.0f;
}

float SkyDensityMap::G(int x, int y) const
{
    return rgba[(static_cast<size_t>(y) * width + x) * 4 + 1] / 255.0f;
}

float SkyDensityMap::B(int x, int y) const
{
    return rgba[(static_cast<size_t>(y) * width + x) * 4 + 2] / 255.0f;
}

SkyDensityMap BuildSkyDensityMap(const float* rgb, int w, int h)
{
    SkyDensityMap map;
    if (!rgb || w <= 0 || h <= 0)
        return map;
    const size_t n = static_cast<size_t>(w) * h;
    std::vector<float> feature(n), dark(n);
    std::vector<float> deckRow(static_cast<size_t>(h));
    // Whiteness (r + g) / 2b: 1 for grey and white, below 1 for blue.
    auto whiteness = [](const float* c) { return (c[0] + c[1]) / std::max(2.0f * c[2], 1e-3f); };
    for (int y = 0; y < h; ++y)
    {
        const float* row = rgb + static_cast<size_t>(y) * w * 3;
        float qMin = 1e9f;
        float sat = 0.0f;
        for (int x = 0; x < w; ++x)
        {
            const float* c = row + x * 3;
            const float mx = std::max(c[0], std::max(c[1], c[2]));
            const float mn = std::min(c[0], std::min(c[1], c[2]));
            sat += (mx - mn) / std::max(mx, 1e-3f);
            qMin = std::min(qMin, whiteness(c));
        }
        sat /= static_cast<float>(w);
        deckRow[static_cast<size_t>(y)] = 1.0f - Smoothstep(0.08f, 0.16f, sat);
        for (int x = 0; x < w; ++x)
        {
            const float* c = row + x * 3;
            const size_t i = static_cast<size_t>(y) * w + x;
            feature[i] = Smoothstep(0.03f, 0.30f, whiteness(c) - qMin);
            const float lum = 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];
            dark[i] = Clamp01(1.2f - lum * 1.3f);
        }
    }
    BlurPlane(feature, w, h);
    BlurPlane(dark, w, h);
    map.width = w;
    map.height = h;
    map.rgba.resize(n * 4);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const size_t i = static_cast<size_t>(y) * w + x;
            std::uint8_t* o = &map.rgba[i * 4];
            o[0] = ToByte(feature[i]);
            o[1] = ToByte(deckRow[static_cast<size_t>(y)]);
            o[2] = ToByte(dark[i]);
            o[3] = 255;
        }
    return map;
}

std::vector<float> MixSkyTexels(const std::vector<float>& a, const std::vector<float>& b, float f)
{
    if (a.size() != b.size() || f <= 0.0f)
        return a;
    std::vector<float> out(a.size());
    for (size_t i = 0; i < a.size(); ++i)
        out[i] = a[i] * (1.0f - f) + b[i] * f;
    return out;
}

float SkyDeckWeight(float skyThrough)
{
    return 1.0f - Smoothstep(0.35f, 0.75f, skyThrough);
}

float SkyCloudDensity(float r, float g, float b, float deckWeight)
{
    const float deck = g * deckWeight;
    const float cloud = std::max(r, deck);
    const float thin = 0.75f + 0.25f * b;
    const float thick = 0.5f + 0.5f * b;
    return cloud * (thin + (thick - thin) * deck);
}

float SkyShadowCover(float density)
{
    return Smoothstep(kSkyShadowThreshold - kSkyShadowEdge, kSkyShadowThreshold + kSkyShadowEdge, density);
}

namespace
{

// SkyShadowCover's mean over each map row (the dome turned through every
// azimuth) at a deck weight.
std::vector<float> RowCover(const SkyDensityMap& map, float deckWeight)
{
    std::vector<float> rows(static_cast<size_t>(map.height), 0.0f);
    for (int y = 0; y < map.height; ++y)
    {
        double row = 0.0;
        for (int x = 0; x < map.width; ++x)
            row += SkyShadowCover(SkyCloudDensity(map.R(x, y), map.G(x, y), map.B(x, y), deckWeight));
        rows[static_cast<size_t>(y)] = static_cast<float>(row / map.width);
    }
    return rows;
}

// The ground within kSkyShadowCoverageRadius of the anchor, on equal-area
// rings: each point's deck point (along the sun, as domeCloudCover projects
// it) seen from the anchor, its row's cover read as the shaders' linear
// filter reads the map.
float CoverageFromRows(const std::vector<float>& rows, float sunElevationDeg)
{
    constexpr int kRings = 16;
    constexpr int kAngles = 32;
    const int h = static_cast<int>(rows.size());
    if (h <= 0)
        return 0.0f;
    const float e = std::max(0.0f, sunElevationDeg) / kDeg;
    // Horizontal run from a ground point to its deck point (the shader's
    // max(sun.y, 0.06) included).
    const float lead = kSkyShadowDeckAltitude * std::cos(e) / std::max(std::sin(e), 0.06f);
    double sum = 0.0;
    for (int i = 0; i < kRings; ++i)
    {
        const float r = kSkyShadowCoverageRadius * std::sqrt((static_cast<float>(i) + 0.5f) / kRings);
        for (int j = 0; j < kAngles; ++j)
        {
            const float a = 2.0f * kPi * (static_cast<float>(j) + 0.5f) / kAngles;
            const float sx = r * std::cos(a) + lead;
            const float sz = r * std::sin(a);
            const float elev = std::atan2(kSkyShadowDeckAltitude, std::sqrt(sx * sx + sz * sz)) * kDeg;
            const float y = SkyDomeV(elev) * static_cast<float>(h) - 0.5f;
            const int y0 = std::max(0, std::min(h - 1, static_cast<int>(std::floor(y))));
            const int y1 = std::min(h - 1, y0 + 1);
            const float t = std::max(0.0f, std::min(1.0f, y - static_cast<float>(y0)));
            sum += rows[static_cast<size_t>(y0)] * (1.0f - t) + rows[static_cast<size_t>(y1)] * t;
        }
    }
    return static_cast<float>(sum / (kRings * kAngles));
}

} // namespace

float SkyShadowCoverage(const SkyDensityMap& map, float deckWeight, float sunElevationDeg)
{
    if (!map.Valid())
        return 0.0f;
    return CoverageFromRows(RowCover(map, deckWeight), sunElevationDeg);
}

SkyShadowCoverageTable BuildSkyShadowCoverageTable(const SkyDensityMap& map)
{
    SkyShadowCoverageTable table;
    if (!map.Valid())
        return table;
    for (int d = 0; d < SkyShadowCoverageTable::kDeckSteps; ++d)
    {
        const std::vector<float> rows =
            RowCover(map, static_cast<float>(d) / static_cast<float>(SkyShadowCoverageTable::kDeckSteps - 1));
        for (int s = 0; s < SkyShadowCoverageTable::kSunSteps; ++s)
            table.cover[d][s] = CoverageFromRows(rows, 90.0f * static_cast<float>(s) /
                                                           static_cast<float>(SkyShadowCoverageTable::kSunSteps - 1));
    }
    return table;
}

float SkyShadowCoverageTable::At(float deckWeight, float sunElevationDeg) const
{
    const float fd = Clamp01(deckWeight) * static_cast<float>(kDeckSteps - 1);
    const float fs = Clamp01(sunElevationDeg / 90.0f) * static_cast<float>(kSunSteps - 1);
    const int d0 = std::min(kDeckSteps - 2, static_cast<int>(fd));
    const int s0 = std::min(kSunSteps - 2, static_cast<int>(fs));
    const float td = fd - static_cast<float>(d0), ts = fs - static_cast<float>(s0);
    const float lo = cover[d0][s0] * (1.0f - ts) + cover[d0][s0 + 1] * ts;
    const float hi = cover[d0 + 1][s0] * (1.0f - ts) + cover[d0 + 1][s0 + 1] * ts;
    return lo * (1.0f - td) + hi * td;
}

float LitCloudLowSun(float sunElevationDeg)
{
    return 1.0f - Smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, sunElevationDeg);
}

float LitCloudStepAngleDeg(int k)
{
    const float fk = static_cast<float>(k);
    return kLitCloudStepDeg * fk * (0.5f + 0.5f * fk);
}

LitCloudShade LitCloudTerms(float sunElevationDeg, float cosViewSun, float density, float towardSun, float feature,
                            float dark, float deck, float skyThrough)
{
    // Overcast flattens the direct part: the light inside a deck is diffuse.
    const float sunY = std::sin(sunElevationDeg / kDeg);
    const float direct = Smoothstep(-0.06f, 0.04f, sunY) * (0.45f + 0.55f * Clamp01(skyThrough));
    const float relief = std::clamp((density - towardSun) * kLitCloudRelief, -0.35f, 0.8f);
    float gain = 1.0f + relief * direct;
    gain *= 1.0f + (0.55f - dark) * 0.7f * deck;
    gain *= 1.0f - 0.35f * std::pow(std::max(cosViewSun, 0.0f), 6.0f) * Smoothstep(0.35f, 0.9f, feature) * direct;
    const float warm = Clamp01(relief) * direct;
    const float rim = Clamp01((density - towardSun) * 3.0f) * (1.0f - 0.5f * density);
    const float hg = 0.008f / std::pow(std::max(1.64f - 1.6f * cosViewSun, 1e-4f), 1.5f);
    const float silver = hg * rim * kLitCloudSilver * direct;
    // Every term fades with the low sun, the darkening against the sun and the
    // bellies included.
    const float low = LitCloudLowSun(sunElevationDeg);
    return LitCloudShade{1.0f + (gain - 1.0f) * low, warm * low, silver * low};
}

float SkyDomeTurnU(float cloudsPosition)
{
    // In double: the position grows without bound over a long session.
    const double u = static_cast<double>(cloudsPosition) * static_cast<double>(kSkyTurnDegPerCloudsPos) / 180.0;
    const double f = u - std::floor(u);
    return static_cast<float>(f >= 1.0 ? 0.0 : f);
}

void SkyShadowAnchor::Snap(float camX, float camZ)
{
    valid = true;
    x = camX;
    z = camZ;
    prevWeight = 0.0f;
    prevAngle = 0.0f;
    lastCamX = camX;
    lastCamZ = camZ;
}

void SkyShadowAnchor::Update(float camX, float camZ, float dt, bool hold)
{
    const float jx = camX - lastCamX, jz = camZ - lastCamZ;
    if (!valid || jx * jx + jz * jz >= kSkyShadowAnchorJump * kSkyShadowAnchorJump)
    {
        Snap(camX, camZ);
        return;
    }
    lastCamX = camX;
    lastCamZ = camZ;
    dt = dt < 0.0f ? 0.0f : dt;
    if (hold)
    {
        prevWeight = 0.0f;
        return;
    }
    if (prevWeight > 0.0f)
        prevWeight = std::max(0.0f, prevWeight - dt / kSkyShadowFadeSeconds);
    const float dx = camX - x, dz = camZ - z;
    const float d2 = dx * dx + dz * dz;
    if (d2 <= kSkyShadowAnchorRadius * kSkyShadowAnchorRadius)
        return;
    // Move the anchor kSkyShadowAnchorRadius toward the camera: the old one is
    // then exactly that far behind, at the opposite angle, and the shadows
    // cross-fade from it. A fade still running ends here.
    const float d = std::sqrt(d2);
    const float ux = dx / d, uz = dz / d;
    x += ux * kSkyShadowAnchorRadius;
    z += uz * kSkyShadowAnchorRadius;
    prevAngle = std::atan2(-uz, -ux);
    prevWeight = 1.0f;
}

float SkyShadowAnchor::PrevX() const
{
    return x + kSkyShadowAnchorRadius * std::cos(prevAngle);
}

float SkyShadowAnchor::PrevZ() const
{
    return z + kSkyShadowAnchorRadius * std::sin(prevAngle);
}

} // namespace Poseidon::remaster
