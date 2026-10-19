#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>

#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Poseidon::remaster
{

namespace
{

float Clamp01(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

float Smooth(float e0, float e1, float x)
{
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

// Rayleigh scattering of air at sea level, green (1/m; Atmosphere.hpp).
constexpr float kRayleighGreen = 13.558e-6f;
constexpr float kRayleighHeight = 8000.0f;

// A uniform number in [0, 1) from a date and a salt (a 32-bit integer hash):
// the same day gives the same number on every run and machine.
float Hash01(int year, int day, std::uint32_t salt)
{
    std::uint32_t h = static_cast<std::uint32_t>(year) * 0x9E3779B1u ^ static_cast<std::uint32_t>(day) * 0x85EBCA77u ^
                      salt * 0xC2B2AE3Du;
    h ^= h >> 16;
    h *= 0x7FEB352Du;
    h ^= h >> 15;
    h *= 0x846CA68Bu;
    h ^= h >> 16;
    return static_cast<float>(h >> 8) * (1.0f / 16777216.0f);
}

// A box average over a square window of radius r cells, across then down
// (running sums: the cost does not grow with the radius).
std::vector<float> BoxAverage(const std::vector<float>& in, int size, int r)
{
    std::vector<float> tmp(in.size()), out(in.size());
    for (int z = 0; z < size; ++z)
    {
        const float* row = in.data() + static_cast<size_t>(z) * size;
        double sum = 0.0;
        int lo = 0, hi = -1; // the window [lo, hi]
        for (int x = 0; x < size; ++x)
        {
            const int wantHi = std::min(size - 1, x + r), wantLo = std::max(0, x - r);
            while (hi < wantHi)
                sum += row[++hi];
            while (lo < wantLo)
                sum -= row[lo++];
            tmp[static_cast<size_t>(z) * size + x] = static_cast<float>(sum / (hi - lo + 1));
        }
    }
    for (int x = 0; x < size; ++x)
    {
        double sum = 0.0;
        int lo = 0, hi = -1;
        for (int z = 0; z < size; ++z)
        {
            const int wantHi = std::min(size - 1, z + r), wantLo = std::max(0, z - r);
            while (hi < wantHi)
                sum += tmp[static_cast<size_t>(++hi) * size + x];
            while (lo < wantLo)
                sum -= tmp[static_cast<size_t>(lo++) * size + x];
            out[static_cast<size_t>(z) * size + x] = static_cast<float>(sum / (hi - lo + 1));
        }
    }
    return out;
}

// The mist's density at a height over its floor, for a layer this deep.
float MistShape(const FogMedium& m, float y, float floorY, float layer)
{
    const float top = floorY + layer * m.mistLayer;
    if (layer * m.mistLayer <= 0.5f)
        return 0.0f;
    const float above = y - top;
    return above <= 0.0f ? 1.0f : std::exp(-above / std::max(m.mistFall, 1.0f));
}

} // namespace

FogGrid FogGridFor(int w, int h, float quality)
{
    FogGrid g;
    if (w <= 0 || h <= 0)
        return g;
    const int q = std::clamp(static_cast<int>(std::lround(quality)), 0, 2);
    static constexpr int kCell[3] = {16, 12, 8};
    static constexpr int kSlices[3] = {48, 64, 96};
    g.cellPixels = kCell[q];
    g.slices = kSlices[q];
    g.cellsX = std::max(1, (w + g.cellPixels - 1) / g.cellPixels);
    g.cellsY = std::max(1, (h + g.cellPixels - 1) / g.cellPixels);
    return g;
}

int FogNearSteps(float quality)
{
    static constexpr int kSteps[3] = {12, 20, 32};
    return kSteps[std::clamp(static_cast<int>(std::lround(quality)), 0, 2)];
}

float FogSliceDepth(float t, float zFar, float k)
{
    if (k < 1e-3f)
        return zFar * t;
    return zFar * (std::exp(k * t) - 1.0f) / (std::exp(k) - 1.0f);
}

float FogSliceCoord(float z, float zFar, float k)
{
    if (zFar <= 0.0f)
        return 0.0f;
    if (k < 1e-3f)
        return Clamp01(z / zFar);
    return Clamp01(std::log(1.0f + std::max(z, 0.0f) * (std::exp(k) - 1.0f) / zFar) / k);
}

float FogDepthK(float zFar, int slices)
{
    if (slices <= 0 || zFar <= 0.0f)
        return 1e-3f;
    const float first = kFogFirstSliceMetres * 64.0f / static_cast<float>(slices);
    const float t = 1.0f / static_cast<float>(slices);
    if (zFar * t <= first)
        return 1e-3f; // even slices are already this fine
    // FogSliceDepth(t) falls as k grows: bisect for the first slice's depth.
    float lo = 1e-3f, hi = 30.0f;
    for (int i = 0; i < 60; ++i)
    {
        const float mid = 0.5f * (lo + hi);
        if (FogSliceDepth(t, zFar, mid) > first)
            lo = mid;
        else
            hi = mid;
    }
    return 0.5f * (lo + hi);
}

float WeatherVisibility(float fog, float rain, float viewDistance)
{
    // Landscape.cpp's tactical range with the night factor at 1.
    const float fogVisibility = 1.0f - Clamp01(fog) * 0.95f;
    const float r = Clamp01(rain);
    const float noFog = (1.0f - r) * viewDistance + kRainVisibility * r;
    const float defaultDistance = std::min(900.0f, noFog) * fogVisibility;
    const float currentDistance = noFog * fogVisibility;
    float range = defaultDistance;
    constexpr float kThreshold = 0.6f;
    if (fogVisibility > kThreshold)
    {
        const float i = (fogVisibility - kThreshold) / (1.0f - kThreshold);
        range = i * currentDistance + (1.0f - i) * defaultDistance;
    }
    return std::max(range, 0.1f);
}

float FogGreySky(float skyThrough)
{
    return Clamp01((0.65f - skyThrough) / 0.5f);
}

float FogMorningWeight(float sunElevationDeg, float sunAzimuthDeg)
{
    const bool east = sunAzimuthDeg > 0.0f && sunAzimuthDeg < 180.0f;
    if (!east)
        return 0.0f;
    return Smooth(-6.0f, -2.0f, sunElevationDeg) * (1.0f - Smooth(8.0f, 24.0f, sunElevationDeg));
}

float FogMistEvent(int year, int dayOfYear, float skyThrough, float rain)
{
    const float chance = std::min(
        kMistChanceClear + kMistChanceOvercast * (1.0f - Clamp01(skyThrough)) + kMistChanceRain * Clamp01(rain), 0.9f);
    if (Hash01(year, dayOfYear, 1u) >= chance)
        return 0.0f;
    return 0.6f + 0.4f * Hash01(year, dayOfYear, 2u);
}

int FogDayOfYear(int year, int month, int day)
{
    static constexpr int kBefore[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    const int m = std::clamp(month, 1, 12);
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return kBefore[m - 1] + (leap && m > 2 ? 1 : 0) + std::max(day, 1) - 1;
}

float VolFogEdgeScale(float fogStart, float fogEnd)
{
    const float range = fogEnd - fogStart;
    if (range <= 1e-3f || fogEnd <= 0.0f)
        return 1.0f;
    const float edge = fogEnd * (1.0f - kFogEdgeStart);
    return std::max(1.0f, range / std::max(edge, 1e-3f));
}

FogMedium ComputeFogMedium(const FogInputs& in, const LightingTuning& t, bool applied)
{
    FogMedium m;
    m.zFar = std::max(in.fogEnd, 50.0f);
    if (!applied)
        return m;
    const float density = std::max(t.fogDensity, 0.0f);
    const float overcast = 1.0f - Clamp01(in.skyThrough);
    const float rain = Clamp01(in.rain);
    const float fog = Clamp01(in.fog);
    const float night = Clamp01(in.nightEffect);
    const float morning = FogMorningWeight(in.sunElevationDeg, in.sunAzimuthDeg);
    const float event = FogMistEvent(in.year, in.dayOfYear, in.skyThrough, rain);
    m.morning = morning;
    m.mistEvent = event;
    m.skyThrough = Clamp01(in.skyThrough);
    m.night = night;

    // Haze: thin under a broken sky (the blue sky and its white cumulus keep
    // their clear air), thick under a grey deck and in rain.
    // (Look director round 2: rain's own weather fog carries most of its
    // air; once the air took the sky's real colour, a grey deck's haze times
    // 1 + 2 rain on top erased a hill 300 m away.)
    // (Look fix "plate": the grey gain 3.5 to kHazeGreyGain; a dry overcast
    // erased every ridge past 1.5 km.)
    // (Look fix round 4 "mist": setFog's own fog carries its air as rain's
    // does; the grey haze on top of a setFog 0.3 mist under a deck erased a
    // rock 300 m out.)
    const float wetAir = std::max(rain, Smooth(0.0f, 0.3f, fog) * (1.0f - night));
    m.hazeSigma = kHazeClear * density *
                  (1.0f + kHazeGreyGain * Smooth(0.5f, 0.95f, overcast) * (1.0f - 0.7f * wetAir)) *
                  (1.0f + 0.5f * rain);
    m.rayleigh = kRayleighScale * density;
    // The humid air near the ground: a misty morning, the night, a grey sky,
    // rain and fog (a foggy morning most: the light falls through it in
    // shafts); never the clear air of a dry day (the veil it laid over near
    // vehicles toward a low sun).
    const float groundWeight = std::min(std::max(morning * std::max(event, Smooth(0.0f, 0.15f, fog)), 0.35f * night) +
                                            0.3f * Smooth(0.5f, 1.0f, overcast) + 0.5f * rain + 0.5f * fog,
                                        1.5f);
    m.groundSigma = kGroundHaze * groundWeight * density;

    // Weather fog: the part of the classic tactical visibility the weather
    // takes away, at the fog floor: half of a surface is gone a little before
    // two thirds of the classic fog end (where its linear fog is half), and
    // the forward fog's edge hides the rest.
    const float vd = std::max(in.viewDistance, 100.0f);
    const float visibility = WeatherVisibility(fog, rain, vd);
    m.weatherSigma = kWeatherExtinction * std::max(0.0f, 1.0f / visibility - 1.0f / vd) * density *
                     (1.0f + kNightFogBoost * night * Smooth(0.0f, 0.15f, fog));
    const float wFog = fog / std::max(fog + rain, 1e-4f);
    m.weatherSigma *= 1.0f + (kSetFogBoost - 1.0f) * wFog * Smooth(0.0f, 0.15f, fog);
    // Rain's part keeps kRainFogShare of it (setFog's all).
    m.weatherSigma *= wFog + kRainFogShare * (1.0f - wFog);
    m.weatherHeight = kWeatherFogHeight * wFog + kRainHazeHeight * (1.0f - wFog);

    // Valley mist: the day's misty morning (and a little of it that evening
    // and night), a grey sky, rain and fog.
    const bool west = !(in.sunAzimuthDeg > 0.0f && in.sunAzimuthDeg < 180.0f);
    const float evening = west ? Smooth(6.0f, 0.0f, in.sunElevationDeg) : 0.0f;
    const float weight =
        std::min(event * (morning + 0.3f * std::max(evening, night)) + 0.2f * Smooth(0.5f, 1.0f, overcast) +
                     0.5f * rain + (kMistFogWeight + (kMistFogWeightNight - kMistFogWeight) * night) * fog,
                 2.0f);
    m.mistSigma = kMistSigma * weight * std::max(t.fogMist, 0.0f) * density;
    m.mistLayer = 1.0f + 1.5f * fog + 0.5f * rain;
    m.mistFall = kMistLayerFall * (1.0f + fog);
    m.mistNoise = 0.8f - 0.5f * fog;

    // Dust over an arid map, laid by rain.
    m.dustSigma = kDustHaze * Clamp01(in.aridity) * density * (1.0f - 0.7f * rain);
    m.dustHeight = kDustHeight;
    m.sun = std::max(t.fogSun, 0.0f);
    m.lobe = kFogLobeGain * m.sun;
    const float grey = std::max({Smooth(0.5f, 0.95f, overcast), rain, Smooth(0.0f, 0.2f, fog)});
    m.hazeNearTo = kHazeNearTo + (kHazeNearToGrey - kHazeNearTo) * grey;
    m.hazeNearKeep = kHazeNearKeep + (kHazeNearKeepGrey - kHazeNearKeep) * grey;
    m.sky = 1.0f;
    m.lights = std::max(t.fogLights, 0.0f);
    m.fogEdgeScale = VolFogEdgeScale(in.fogStart, in.fogEnd);
    return m;
}

std::vector<float> BuildFogFloor(const float* heights, int size, float cellSize, float radius, float blur)
{
    std::vector<float> out;
    if (!heights || size <= 1 || cellSize <= 0.0f)
        return out;
    const size_t n = static_cast<size_t>(size) * static_cast<size_t>(size);
    const int r = std::max(1, static_cast<int>(std::lround(radius / cellSize)));
    const int b = std::max(0, static_cast<int>(std::lround(blur / cellSize)));
    std::vector<float> tmp(n);
    out.assign(n, 0.0f);
    // The lowest ground in a square window: across, then down.
    for (int z = 0; z < size; ++z)
        for (int x = 0; x < size; ++x)
        {
            float m = heights[static_cast<size_t>(z) * size + x];
            for (int k = std::max(0, x - r); k <= std::min(size - 1, x + r); ++k)
                m = std::min(m, heights[static_cast<size_t>(z) * size + k]);
            tmp[static_cast<size_t>(z) * size + x] = m;
        }
    for (int z = 0; z < size; ++z)
        for (int x = 0; x < size; ++x)
        {
            float m = tmp[static_cast<size_t>(z) * size + x];
            for (int k = std::max(0, z - r); k <= std::min(size - 1, z + r); ++k)
                m = std::min(m, tmp[static_cast<size_t>(k) * size + x]);
            // Water holds its mist at its surface.
            out[static_cast<size_t>(z) * size + x] = std::max(m, 0.0f);
        }
    if (b == 0)
        return out;
    // A box blur, so the layer's top runs smooth.
    return BoxAverage(out, size, b);
}

std::vector<float> BuildFogLayer(const float* heights, const std::vector<float>& floorY, int size, float cellSize)
{
    std::vector<float> out;
    const size_t n = static_cast<size_t>(std::max(size, 0)) * static_cast<size_t>(std::max(size, 0));
    if (!heights || size <= 1 || cellSize <= 0.0f || floorY.size() != n)
        return out;
    // The mean ground around each cell, the sea counted at its surface.
    std::vector<float> ground(n);
    for (size_t i = 0; i < n; ++i)
        ground[i] = std::max(heights[i], 0.0f);
    const int r = std::max(1, static_cast<int>(std::lround(kFogReliefRadius / cellSize)));
    const std::vector<float> mean = BoxAverage(ground, size, r);
    out.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
        const float relief = std::max(mean[i] - floorY[i], 0.0f);
        out[i] = std::min(kMistReliefShare * relief, kMistLayerMax) * Smooth(kMistReliefFrom, kMistReliefFull, relief);
    }
    return out;
}

float FogGroundAirTop(const FogMedium& m, float y, float floorY, float layer)
{
    const float top = floorY + std::max(layer, 0.0f) * m.mistLayer + kGroundAirTop;
    return 1.0f - Smooth(top, top + kGroundAirFade, y);
}

float FogHazeNearKeep(const FogMedium& m, float dist)
{
    return m.hazeNearKeep + (1.0f - m.hazeNearKeep) * Smooth(kHazeNearFrom, m.hazeNearTo, dist);
}

float FogSightExtinctionAt(const FogMedium& m, float y, float floorY, float aboveGround, float layer)
{
    float sigma = m.groundSigma *
                  std::exp(-std::max(aboveGround, 0.0f) / std::max(m.groundHeight, 1.0f) -
                           std::max(y - floorY, 0.0f) / kGroundHazePool) *
                  FogGroundAirTop(m, y, floorY, layer);
    sigma += m.mistSigma * MistShape(m, y, floorY, layer);
    return sigma;
}

float FogExtinctionAt(const FogMedium& m, float y, float floorY, float aboveGround, float layer)
{
    const float above = std::max(y - floorY, 0.0f);
    float sigma = m.hazeSigma * std::exp(-std::max(y, -50.0f) / m.hazeHeight);
    sigma += m.rayleigh * kRayleighGreen * std::exp(-std::max(y, -50.0f) / kRayleighHeight);
    sigma += m.dustSigma * std::exp(-std::max(aboveGround, 0.0f) / std::max(m.dustHeight, 1.0f));
    sigma += m.weatherSigma * std::exp(-above / std::max(m.weatherHeight, 1.0f));
    sigma += FogSightExtinctionAt(m, y, floorY, aboveGround, layer);
    return sigma;
}

float FogAridity(const std::vector<float>& groundTint, const std::vector<float>& heights)
{
    const size_t n = heights.size();
    if (n == 0 || groundTint.size() != n * 2)
        return 0.0f;
    double sum = 0.0;
    size_t land = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (heights[i] <= 0.5f)
            continue;
        sum += groundTint[i * 2];
        ++land;
    }
    if (land == 0)
        return 0.0f;
    return Smooth(kAridRedFrom, kAridRedFull, static_cast<float>(sum / static_cast<double>(land)));
}

float FogStarKeep(const FogMedium& m, float eyeY, float floorY, float aboveGround, float layer)
{
    // Straight up: the weather fog's layer above the eye, the humid air and
    // the mist around it over their depths.
    const float overFloor = std::max(eyeY - floorY, 0.0f);
    float tau = m.weatherSigma * m.weatherHeight * std::exp(-overFloor / std::max(m.weatherHeight, 1.0f));
    tau += FogSightExtinctionAt(m, eyeY, floorY, aboveGround, layer) *
           std::max(m.groundHeight, std::max(layer, 0.0f) * m.mistLayer);
    return 1.0f - Smooth(kStarHideFrom, kStarHideFull, tau);
}

// ---- AI sight ------------------------------------------------------------------

bool FogSightMap::Valid() const
{
    const size_t n = static_cast<size_t>(std::max(size, 0)) * static_cast<size_t>(std::max(size, 0));
    return size > 1 && cellSize > 0.0f && ground.size() == n && floorY.size() == n && layer.size() == n;
}

namespace
{

struct FogSightState
{
    bool on = false;
    FogMedium medium;
    std::shared_ptr<const FogSightMap> map;
};

FogSightState& Sight()
{
    static FogSightState s;
    return s;
}

// Bilinear sample of a grid at world x, z (clamped to its edge).
float SampleGrid(const FogSightMap& map, const std::vector<float>& grid, float x, float z)
{
    const float last = static_cast<float>(map.size - 1);
    const float gx = std::clamp(x / map.cellSize, 0.0f, last), gz = std::clamp(z / map.cellSize, 0.0f, last);
    const int x0 = std::min(static_cast<int>(gx), map.size - 2), z0 = std::min(static_cast<int>(gz), map.size - 2);
    const float fx = gx - static_cast<float>(x0), fz = gz - static_cast<float>(z0);
    auto at = [&](int xi, int zi) { return grid[static_cast<size_t>(zi) * map.size + xi]; };
    const float a = at(x0, z0) + (at(x0 + 1, z0) - at(x0, z0)) * fx;
    const float b = at(x0, z0 + 1) + (at(x0 + 1, z0 + 1) - at(x0, z0 + 1)) * fx;
    return a + (b - a) * fz;
}

} // namespace

std::shared_ptr<const FogSightMap> BuildFogSightMap(const float* heights, int range, float grid, int maxSize)
{
    auto sight = std::make_shared<FogSightMap>();
    DownsampleTerrainGrid(heights, range, grid, maxSize, sight->ground, sight->size, sight->cellSize);
    if (sight->size < 2)
        return nullptr;
    sight->floorY = BuildFogFloor(sight->ground.data(), sight->size, sight->cellSize);
    sight->layer = BuildFogLayer(sight->ground.data(), sight->floorY, sight->size, sight->cellSize);
    if (!sight->Valid())
        return nullptr;
    return sight;
}

void SetFogSight(const FogMedium* m, std::shared_ptr<const FogSightMap> map)
{
    FogSightState& s = Sight();
    s.on = m != nullptr && map && map->Valid() && (m->mistSigma > 0.0f || m->groundSigma > 0.0f);
    s.medium = m ? *m : FogMedium{};
    s.map = std::move(map);
}

void UpdateFogSightMedium(const FogInputs& in, const LightingTuning& t, std::shared_ptr<const FogSightMap> map)
{
    const FogMedium m = ComputeFogMedium(in, t, true);
    SetFogSight(&m, std::move(map));
}

const FogMedium& FogSightMedium()
{
    return Sight().medium;
}

std::shared_ptr<const FogSightMap> FogSightMapInUse()
{
    return Sight().map;
}

bool FogSightActive()
{
    return Sight().on;
}

float FogSightTransmittanceThrough(const FogMedium& m, const FogSightMap& map, const float from[3], const float to[3])
{
    if (!map.Valid())
        return 1.0f;
    const float dx = to[0] - from[0], dy = to[1] - from[1], dz = to[2] - from[2];
    const float len = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 1e-3f)
        return 1.0f;
    constexpr int kSamples = 8;
    float tau = 0.0f;
    for (int i = 0; i < kSamples; ++i)
    {
        const float t = (static_cast<float>(i) + 0.5f) / kSamples;
        const float x = from[0] + dx * t, y = from[1] + dy * t, z = from[2] + dz * t;
        const float ground = SampleGrid(map, map.ground, x, z);
        const float floorY = SampleGrid(map, map.floorY, x, z);
        const float layer = SampleGrid(map, map.layer, x, z);
        const float near = kMistNearKeep + (1.0f - kMistNearKeep) * Smooth(kMistNearFrom, kMistNearTo, len * t);
        tau += FogSightExtinctionAt(m, y, floorY, y - ground, layer) * near;
    }
    return std::exp(-tau * len / kSamples);
}

float FogSightTransmittance(const float from[3], const float to[3])
{
    const FogSightState& s = Sight();
    if (!s.on || !s.map)
        return 1.0f;
    return FogSightTransmittanceThrough(s.medium, *s.map, from, to);
}

namespace
{
struct DrawnFogState
{
    bool drawn = false;
    FogMedium medium;
};
DrawnFogState& Drawn()
{
    static DrawnFogState s;
    return s;
}
} // namespace

void SetDrawnFogMedium(const FogMedium* m)
{
    DrawnFogState& d = Drawn();
    d.drawn = m != nullptr;
    d.medium = m ? *m : FogMedium{};
}

bool GetDrawnFogMedium(FogMedium& out)
{
    const DrawnFogState& d = Drawn();
    if (d.drawn)
        out = d.medium;
    return d.drawn;
}

} // namespace Poseidon::remaster
