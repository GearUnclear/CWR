#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// The volumetric fog and atmosphere (the look lane "fog";
// Graphics/Remaster/VolumetricFog.hpp, EngineGL33_PostFog.cpp): the froxel
// grid and its exponential slices, the medium's weather and time rules, the
// classic fog's moved edge, the fog floor, the Air stage's plan and gate, and
// the shaders' agreement with the C++ mirrors.

using namespace Poseidon::remaster;
namespace gl = Poseidon::gl33lighting;

namespace
{

int CountOf(const std::string& text, const std::string& needle)
{
    int n = 0;
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
        ++n;
    return n;
}

FogInputs ClearNoon()
{
    FogInputs in;
    in.sunElevationDeg = 60.0f;
    in.sunAzimuthDeg = 180.0f;
    in.viewDistance = 3000.0f;
    in.fogStart = 900.0f;
    in.fogEnd = 2980.0f;
    return in;
}

// The forward fog factor as finishFragment takes it: the classic linear
// factor times the edge scale, clamped at 1.
float ForwardFog(float d, float start, float end, float scale)
{
    const float classic = std::clamp(1.0f - (d - start) / (end - start), 0.0f, 1.0f);
    return std::min(classic * scale, 1.0f);
}

} // namespace

TEST_CASE("Volumetric fog: a 1080p grid of 12 px cells and 64 slices, coarser and finer by quality", "[remaster][fog]")
{
    const FogGrid g = FogGridFor(1920, 1080, 1.0f);
    CHECK(g.cellsX == 160);
    CHECK(g.cellsY == 90);
    CHECK(g.slices == 64);
    CHECK(g.cellPixels == 12);
    const FogGrid lo = FogGridFor(1920, 1080, 0.0f);
    CHECK(lo.cellPixels == 16);
    CHECK(lo.slices == 48);
    CHECK(lo.cellsY == 68); // rounded up: the last row covers the bottom edge
    const FogGrid hi = FogGridFor(1920, 1080, 2.0f);
    CHECK(hi.cellPixels == 8);
    CHECK(hi.slices == 96);
    // Out-of-range knob values clamp to the ends; every grid writes whole
    // eight-slice draws.
    CHECK(FogGridFor(1920, 1080, 7.0f).slices == 96);
    CHECK(FogGridFor(1920, 1080, -3.0f).slices == 48);
    for (float q : {0.0f, 1.0f, 2.0f})
        CHECK(FogGridFor(1280, 720, q).slices % kFogSlicesPerDraw == 0);
    CHECK(FogGridFor(0, 1080, 1.0f).slices == 0);
}

TEST_CASE("Volumetric fog: the slices are exponential, about a metre deep at the eye, and invert", "[remaster][fog]")
{
    for (float zFar : {400.0f, 1500.0f, 4000.0f, 6000.0f})
        for (int slices : {48, 64, 96})
        {
            CAPTURE(zFar, slices);
            const float k = FogDepthK(zFar, slices);
            const float first = FogSliceDepth(1.0f / static_cast<float>(slices), zFar, k);
            CHECK(first == Catch::Approx(kFogFirstSliceMetres * 64.0f / static_cast<float>(slices)).epsilon(0.02));
            CHECK(FogSliceDepth(0.0f, zFar, k) == Catch::Approx(0.0f).margin(1e-4));
            CHECK(FogSliceDepth(1.0f, zFar, k) == Catch::Approx(zFar).epsilon(1e-4));
            float last = -1.0f;
            for (int i = 0; i <= 32; ++i)
            {
                const float t = static_cast<float>(i) / 32.0f;
                const float z = FogSliceDepth(t, zFar, k);
                CHECK(z > last); // deeper slice by slice
                last = z;
                CHECK(FogSliceCoord(z, zFar, k) == Catch::Approx(t).margin(1e-4));
            }
        }
    // A short range is already finer than a metre per slice: even slices.
    CHECK(FogDepthK(30.0f, 64) < 1e-2f);
    CHECK(FogSliceDepth(0.5f, 30.0f, FogDepthK(30.0f, 64)) == Catch::Approx(15.0f).epsilon(1e-3));
    CHECK(FogSliceCoord(1e6f, 1000.0f, 5.0f) == 1.0f);
}

TEST_CASE("Volumetric fog: the weather visibility is the classic engine's tactical range", "[remaster][fog]")
{
    // Landscape.cpp: no fog and no rain see the view distance; heavy rain
    // 350 m; full fog 5 % of 900 m.
    CHECK(WeatherVisibility(0.0f, 0.0f, 3000.0f) == Catch::Approx(3000.0f));
    CHECK(WeatherVisibility(0.0f, 1.0f, 3000.0f) == Catch::Approx(350.0f));
    CHECK(WeatherVisibility(1.0f, 0.0f, 3000.0f) == Catch::Approx(45.0f));
    float last = 1e9f;
    for (int i = 0; i <= 10; ++i)
    {
        const float v = WeatherVisibility(static_cast<float>(i) / 10.0f, 0.0f, 1500.0f);
        CHECK(v <= last);
        last = v;
    }
}

TEST_CASE("Volumetric fog: the weather fog reaches the classic visibility's opacity at sea level", "[remaster][fog]")
{
    // A level line of sight at the fog floor through setFog 0.3 with VD 1500:
    // what is left of a surface at the classic fog's end is a small share, and
    // about half at two thirds of it, where the classic linear fog is half
    // (look fix round 4: about half at a third of it, with the mist and the
    // humid air on top, and the first 150 m keep more than 60 %; look fix
    // round 5: a light mist, about two thirds at a third of it).
    FogInputs in = ClearNoon();
    in.viewDistance = 1500.0f;
    in.fog = 0.3f;
    in.fogEnd = WeatherVisibility(in.fog, 0.0f, in.viewDistance);
    in.fogStart = in.fogEnd * 0.4f;
    const FogMedium m = ComputeFogMedium(in, LightingTuning{}, true);
    CHECK(m.weatherSigma > 0.0f);
    const float sigma = FogExtinctionAt(m, 0.0f, 0.0f);
    const float left = std::exp(-sigma * in.fogEnd);
    CHECK(left < 0.45f);
    CHECK(left > 0.001f);
    CHECK(std::exp(-sigma * 0.66f * in.fogEnd) < 0.6f);
    CHECK(std::exp(-sigma * 0.33f * in.fogEnd) < 0.8f);
    CHECK(std::exp(-sigma * 0.33f * in.fogEnd) > 0.55f);
    CHECK(std::exp(-sigma * 150.0f) > 0.6f);
    // It thins upward from the fog floor: a hilltop 600 m up sees farther.
    CHECK(FogExtinctionAt(m, 600.0f, 0.0f) < 0.6f * sigma);
}

TEST_CASE("Volumetric fog: clear noon is thin haze and blue air; grey sky, rain and fog thicken it", "[remaster][fog]")
{
    const LightingTuning t{};
    const FogMedium noon = ComputeFogMedium(ClearNoon(), t, true);
    CHECK(noon.hazeSigma == Catch::Approx(kHazeClear));
    CHECK(noon.rayleigh == Catch::Approx(kRayleighScale));
    CHECK(noon.weatherSigma == 0.0f);
    CHECK(noon.mistSigma == 0.0f);   // no morning, no night, a clear sky
    CHECK(noon.groundSigma == 0.0f); // and no humid ground air
    // Far hills 3 km out at noon sit in visible air, not in a wash (the full
    // density; the near thinning leaves them more: the look director's 8 km
    // summer visibility).
    const float noonLeft = std::exp(-FogExtinctionAt(noon, 50.0f, 0.0f) * 3000.0f);
    CHECK(noonLeft > 0.12f);
    CHECK(noonLeft < 0.45f);

    // A broken sky (the blue sky with its sunlit cumulus) keeps nearly the
    // clear air; a grey deck thickens the haze and brings the mist.
    FogInputs broken = ClearNoon();
    broken.skyThrough = 0.5f;
    CHECK(ComputeFogMedium(broken, t, true).hazeSigma < 1.25f * noon.hazeSigma);
    FogInputs grey = ClearNoon();
    grey.skyThrough = 0.0f;
    const FogMedium overcast = ComputeFogMedium(grey, t, true);
    CHECK(overcast.hazeSigma > 2.0f * noon.hazeSigma);
    CHECK(overcast.mistSigma > 0.0f);
    CHECK(overcast.groundSigma > 0.0f);

    FogInputs wet = grey;
    wet.rain = 1.0f;
    const FogMedium rain = ComputeFogMedium(wet, t, true);
    CHECK(rain.weatherSigma > 0.0f);
    // Rain's air is mostly its weather fog (look director round 2): the
    // whole is thicker than the dry deck's haze.
    CHECK(rain.hazeSigma + rain.weatherSigma > overcast.hazeSigma);
    CHECK(rain.mistSigma > overcast.mistSigma);
    CHECK(rain.weatherHeight == Catch::Approx(kRainHazeHeight));

    // The knobs: density scales every part, 0 clears the air.
    LightingTuning thick;
    thick.fogDensity = 2.0f;
    CHECK(ComputeFogMedium(wet, thick, true).hazeSigma == Catch::Approx(2.0f * rain.hazeSigma));
    LightingTuning none;
    none.fogDensity = 0.0f;
    const FogMedium clear = ComputeFogMedium(wet, none, true);
    CHECK(FogExtinctionAt(clear, 0.0f, 0.0f) == 0.0f);
    LightingTuning noMist;
    noMist.fogMist = 0.0f;
    CHECK(ComputeFogMedium(wet, noMist, true).mistSigma == 0.0f);

    // A dry evening toward a low sun: no humid air, no mist, nothing to veil
    // the near field.
    FogInputs evening = ClearNoon();
    evening.sunElevationDeg = 9.0f;
    evening.sunAzimuthDeg = 280.0f;
    const FogMedium low = ComputeFogMedium(evening, t, true);
    CHECK(low.groundSigma == 0.0f);
    CHECK(low.mistSigma == 0.0f);
}

TEST_CASE("Volumetric fog: a misty morning is a weather event the date seeds", "[remaster][fog]")
{
    CHECK(FogDayOfYear(1985, 1, 1) == 0);
    CHECK(FogDayOfYear(1985, 7, 5) == 185);
    CHECK(FogDayOfYear(1984, 3, 1) == 60); // a leap year
    int misty = 0;
    for (int d = 0; d < 365; ++d)
    {
        const float e = FogMistEvent(1985, d, 1.0f, 0.0f);
        CHECK(e == FogMistEvent(1985, d, 1.0f, 0.0f)); // the same day every run
        if (e > 0.0f)
        {
            ++misty;
            CHECK(e >= 0.6f);
            CHECK(e <= 1.0f);
            // A grey sky and rain only add misty days.
            CHECK(FogMistEvent(1985, d, 0.0f, 1.0f) > 0.0f);
        }
    }
    // About kMistChanceClear of clear days.
    CHECK(misty > 110);
    CHECK(misty < 180);
}

TEST_CASE("Volumetric fog: the valley mist fills on a misty morning, pools under its top and drops off above",
          "[remaster][fog]")
{
    // An eastern sun from 6 degrees under the horizon, full around sunrise,
    // gone by 24 degrees; a western sun gives no morning.
    CHECK(FogMorningWeight(-10.0f, 70.0f) == 0.0f);
    CHECK(FogMorningWeight(2.0f, 70.0f) == Catch::Approx(1.0f));
    CHECK(FogMorningWeight(16.0f, 70.0f) > 0.0f);
    CHECK(FogMorningWeight(16.0f, 70.0f) < 1.0f);
    CHECK(FogMorningWeight(25.0f, 70.0f) == 0.0f);
    CHECK(FogMorningWeight(2.0f, 290.0f) == 0.0f);

    // A misty and a clear day of the same summer.
    int mistyDay = -1, clearDay = -1;
    for (int d = 150; d < 250 && (mistyDay < 0 || clearDay < 0); ++d)
    {
        if (FogMistEvent(1985, d, 1.0f, 0.0f) > 0.0f)
            mistyDay = mistyDay < 0 ? d : mistyDay;
        else
            clearDay = clearDay < 0 ? d : clearDay;
    }
    REQUIRE(mistyDay >= 0);
    REQUIRE(clearDay >= 0);
    FogInputs dawn = ClearNoon();
    dawn.sunElevationDeg = 1.0f;
    dawn.sunAzimuthDeg = 65.0f;
    dawn.dayOfYear = clearDay;
    const FogMedium clearDawn = ComputeFogMedium(dawn, LightingTuning{}, true);
    CHECK(clearDawn.mistSigma == 0.0f);
    CHECK(clearDawn.groundSigma == 0.0f);
    dawn.dayOfYear = mistyDay;
    const FogMedium m = ComputeFogMedium(dawn, LightingTuning{}, true);
    CHECK(m.mistEvent > 0.0f);
    CHECK(m.mistSigma == Catch::Approx(kMistSigma * m.mistEvent));
    CHECK(m.morning == Catch::Approx(1.0f));
    CHECK(m.groundSigma > 0.0f);
    // Inside the layer the full density; above its top it falls off.
    const float floorY = 40.0f;
    const float layer = 30.0f;
    const float inside = FogExtinctionAt(m, floorY + 10.0f, floorY, 1e4f, layer);
    const float ridge = FogExtinctionAt(m, floorY + 200.0f, floorY, 1e4f, layer);
    CHECK(inside > 4.0f * ridge);
    CHECK(inside ==
          Catch::Approx(FogExtinctionAt(m, floorY + layer * m.mistLayer - 1.0f, floorY, 1e4f, layer)).epsilon(0.05));
    // No layer (a plain, the open sea): no mist (the humid ground air stays;
    // with kMistSigma 2e-3 it is a larger share of the layer's density).
    CHECK(FogExtinctionAt(m, floorY + 5.0f, floorY, 1e4f, 0.0f) < 0.35f * inside);
    // The same hour in the evening light of that day keeps under a third.
    FogInputs dusk = dawn;
    dusk.sunAzimuthDeg = 295.0f;
    const FogMedium e = ComputeFogMedium(dusk, LightingTuning{}, true);
    CHECK(e.mistSigma > 0.15f * m.mistSigma);
    CHECK(e.mistSigma < 0.35f * m.mistSigma);
}

TEST_CASE("Volumetric fog: the mist layer follows the local relief: valleys hold one, the open sea none",
          "[remaster][fog]")
{
    // A valley along z at x = 32 with slopes rising 8 m per 50 m cell, a flat
    // plain at 30 m on the right and the sea on the left edge.
    const int n = 64;
    const float cell = 50.0f;
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            h[static_cast<size_t>(z) * n + x] = x < 20    ? -20.0f
                                                : x >= 50 ? 30.0f
                                                          : 30.0f + 8.0f * std::fabs(static_cast<float>(x - 32));
    const std::vector<float> floorY = BuildFogFloor(h.data(), n, cell);
    const std::vector<float> layer = BuildFogLayer(h.data(), floorY, n, cell);
    REQUIRE(layer.size() == h.size());
    auto at = [&](int x, int z) { return layer[static_cast<size_t>(z) * n + x]; };
    CHECK(at(32, 32) > 10.0f);               // the valley holds a layer
    CHECK(at(32, 32) <= kMistLayerMax);      // never deeper than the cap
    CHECK(at(2, 32) == Catch::Approx(0.0f)); // the open sea none
    // Nothing for a floor of another size.
    CHECK(BuildFogLayer(h.data(), std::vector<float>(3), n, cell).empty());
}

TEST_CASE("Volumetric fog: AI sight takes the mist and the humid air the player sees", "[remaster][fog]")
{
    const float a[3] = {500.0f, 45.0f, 500.0f};
    const float b[3] = {1100.0f, 45.0f, 500.0f};
    SetFogSight(nullptr, nullptr);
    CHECK_FALSE(FogSightActive());
    CHECK(FogSightTransmittance(a, b) == 1.0f);
    // A flat valley floor at 40 m under a 30 m layer, on a misty morning.
    auto map = std::make_shared<FogSightMap>();
    map->size = 64;
    map->cellSize = 50.0f;
    map->ground.assign(64 * 64, 40.0f);
    map->floorY.assign(64 * 64, 40.0f);
    map->layer.assign(64 * 64, 30.0f);
    FogMedium m;
    m.mistSigma = kMistSigma;
    m.groundSigma = 0.0f;
    SetFogSight(&m, map);
    CHECK(FogSightActive());
    // 600 m through the layer: over half of it gone (kMistSigma 2e-3); the
    // same line 300 m up, clear.
    CHECK(FogSightTransmittance(a, b) < 0.45f);
    const float up[2][3] = {{500.0f, 340.0f, 500.0f}, {1100.0f, 340.0f, 500.0f}};
    CHECK(FogSightTransmittance(up[0], up[1]) > 0.95f);
    // A clear medium hands nothing over.
    FogMedium dry;
    SetFogSight(&dry, map);
    CHECK_FALSE(FogSightActive());
    CHECK(FogSightTransmittance(a, b) == 1.0f);
    SetFogSight(nullptr, nullptr);
}

// ---- AI sight is the simulation's (round 4 "ailink") ----------------------------

namespace
{

// A misty morning in fog: the weather and the time the simulation and the
// renderer both read (WorldFogInputs).
FogInputs MistyMorning()
{
    FogInputs in = ClearNoon();
    in.sunElevationDeg = 3.0f;
    in.sunAzimuthDeg = 80.0f;
    in.skyThrough = 0.3f;
    in.fog = 0.3f;
    in.viewDistance = 1500.0f;
    in.fogStart = 450.0f;
    in.fogEnd = 1500.0f;
    return in;
}

// A valley 64 x 64 cells of 50 m: walls at 90 m, a floor at 20 m between
// x 20 and 44 (the mist pools there).
std::vector<float> ValleyHeights(int n)
{
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            h[static_cast<size_t>(z) * n + x] = (x >= 20 && x <= 44) ? 20.0f : 90.0f;
    return h;
}

} // namespace

TEST_CASE("Volumetric fog: AI sight equals the drawn fog's air for the same inputs and knobs", "[remaster][fog]")
{
    const std::vector<float> h = ValleyHeights(64);
    const std::shared_ptr<const FogSightMap> map = BuildFogSightMap(h.data(), 64, 50.0f);
    REQUIRE(map);
    REQUIRE(map->Valid());
    // Across the valley floor, 1.7 m over it, 500 m.
    const float a[3] = {1100.0f, 21.7f, 1000.0f};
    const float b[3] = {1100.0f, 21.7f, 1500.0f};
    for (float density : {1.0f, 0.5f, 2.0f})
        for (float mist : {1.0f, 0.5f, 0.0f})
        {
            LightingTuning t;
            t.fogDensity = density;
            t.fogMist = mist;
            const FogInputs in = MistyMorning();
            UpdateFogSightMedium(in, t, map);
            // The renderer's medium while its fog draws.
            const FogMedium drawn = ComputeFogMedium(in, t, true);
            const FogMedium& ai = FogSightMedium();
            CHECK(ai.groundSigma == drawn.groundSigma);
            CHECK(ai.groundHeight == drawn.groundHeight);
            CHECK(ai.mistSigma == drawn.mistSigma);
            CHECK(ai.mistLayer == drawn.mistLayer);
            CHECK(ai.mistFall == drawn.mistFall);
            CHECK(FogSightActive());
            CHECK(FogSightTransmittance(a, b) == FogSightTransmittanceThrough(drawn, *map, a, b));
        }
    // The knobs move AI sight as they move the drawn fog: thicker with
    // fogDensity, thinner without the mist.
    auto sightWith = [&](float density, float mist)
    {
        LightingTuning t;
        t.fogDensity = density;
        t.fogMist = mist;
        UpdateFogSightMedium(MistyMorning(), t, map);
        return FogSightTransmittance(a, b);
    };
    const float base = sightWith(1.0f, 1.0f);
    CHECK(base < 0.9f); // the misty valley takes a real share
    CHECK(sightWith(2.0f, 1.0f) < base);
    CHECK(sightWith(1.0f, 0.0f) > base);
    SetFogSight(nullptr, nullptr);
}

// The pure helper only: UpdateFogSightMedium never receives gate state, so
// this case alone cannot catch a leak. The end-to-end pin (the simulation
// step on a real landscape, an engine reporting each render path state, and
// the engine's call order) is World/test_fog_sight_link.cpp ([aisight]).
TEST_CASE("Volumetric fog: AI sight does not follow the render path", "[remaster][fog]")
{
    const std::vector<float> h = ValleyHeights(64);
    const std::shared_ptr<const FogSightMap> map = BuildFogSightMap(h.data(), 64, 50.0f);
    REQUIRE(map);
    const float a[3] = {1100.0f, 21.7f, 1000.0f};
    const float b[3] = {1100.0f, 21.7f, 1500.0f};
    const FogInputs in = MistyMorning();
    UpdateFogSightMedium(in, LightingTuning{}, map);
    const float normal = FogSightTransmittance(a, b);
    REQUIRE(normal < 0.9f);

    // Post effects off and a vehicle interior: the fog's gate is off, the
    // renderer keeps the classic forward fog; AI sight is the same.
    LightingTuning noPost;
    noPost.postEffects = false;
    GateInputs gi;
    gi.fog = in.fog;
    gi.skyThrough = in.skyThrough;
    gi.sunElevationDeg = in.sunElevationDeg;
    gi.sunAzimuthDeg = in.sunAzimuthDeg;
    CHECK_FALSE(ComputeFeatureGates(gi, noPost)[Feature::VolFog].on);
    UpdateFogSightMedium(in, noPost, map);
    CHECK(FogSightActive());
    CHECK(FogSightTransmittance(a, b) == normal);

    GateInputs interior = gi;
    interior.interior = true;
    CHECK_FALSE(ComputeFeatureGates(interior, LightingTuning{})[Feature::VolFog].on);
    UpdateFogSightMedium(in, LightingTuning{}, map);
    CHECK(FogSightTransmittance(a, b) == normal);

    // The fog's quality knob and the other render-only knobs change nothing.
    LightingTuning coarse;
    coarse.fogQuality = 0.0f;
    coarse.fogSun = 0.3f;
    coarse.fogLights = 0.0f;
    UpdateFogSightMedium(in, coarse, map);
    CHECK(FogSightTransmittance(a, b) == normal);
    SetFogSight(nullptr, nullptr);
}

TEST_CASE("Volumetric fog: AI sight works without a renderer", "[remaster][fog]")
{
    // A dedicated server or a headless client: no GL, no terrain horizon map,
    // the lighting tuning the base engine reports (classic, default knobs).
    // The sight map comes straight from the heights, sampled as the horizon
    // map samples them.
    const int n = 40;
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            h[static_cast<size_t>(z) * n + x] =
                20.0f + 3.0f * std::fabs(static_cast<float>(x - 20)) + 0.5f * static_cast<float>(z % 7);
    const std::shared_ptr<const FogSightMap> map = BuildFogSightMap(h.data(), n, 25.0f, 16);
    REQUIRE(map);
    TerrainHorizonOptions options;
    options.maxSize = 16;
    options.threads = 1;
    TerrainHorizonMap horizon;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, 25.0f, options, horizon));
    CHECK(map->size == horizon.size);
    CHECK(map->cellSize == horizon.cellSize);
    CHECK(map->ground == horizon.height);
    CHECK(map->floorY == BuildFogFloor(horizon.height.data(), horizon.size, horizon.cellSize));
    CHECK(BuildFogSightMap(nullptr, n, 25.0f) == nullptr);
    CHECK(BuildFogSightMap(h.data(), 1, 25.0f) == nullptr);

    LightingTuning headless;
    headless.remastered = false;
    UpdateFogSightMedium(MistyMorning(), headless, map);
    CHECK(FogSightActive());
    const float from[3] = {500.0f, 22.0f, 100.0f};
    const float to[3] = {500.0f, 22.0f, 800.0f};
    CHECK(FogSightTransmittance(from, to) < 1.0f);
    CHECK(FogSightTransmittance(from, to) ==
          FogSightTransmittanceThrough(ComputeFogMedium(MistyMorning(), LightingTuning{}, true), *map, from, to));
    // No island: classic sight.
    UpdateFogSightMedium(MistyMorning(), headless, nullptr);
    CHECK_FALSE(FogSightActive());
    CHECK(FogSightTransmittance(from, to) == 1.0f);
    SetFogSight(nullptr, nullptr);
}

TEST_CASE("Volumetric fog: off, the forward fog is the classic one; on, it keeps only the view-distance edge",
          "[remaster][fog]")
{
    FogInputs in = ClearNoon();
    in.fogStart = 600.0f;
    in.fogEnd = 1500.0f;
    const FogMedium off = ComputeFogMedium(in, LightingTuning{}, false);
    CHECK(off.fogEdgeScale == 1.0f);
    CHECK(FogExtinctionAt(off, 0.0f, 0.0f) == 0.0f);
    CHECK(off.zFar == Catch::Approx(1500.0f));
    const FogMedium on = ComputeFogMedium(in, LightingTuning{}, true);
    CHECK(on.fogEdgeScale > 1.0f);
    // The scaled factor is the classic linear fog from kFogEdgeStart of the
    // end: no fog before it, the full fog at the end, a straight line between.
    const float edge = kFogEdgeStart * in.fogEnd;
    for (float d : {100.0f, 600.0f, 1000.0f, edge - 1.0f})
        CHECK(ForwardFog(d, in.fogStart, in.fogEnd, on.fogEdgeScale) == Catch::Approx(1.0f));
    CHECK(ForwardFog(in.fogEnd, in.fogStart, in.fogEnd, on.fogEdgeScale) == Catch::Approx(0.0f).margin(1e-5));
    const float mid = 0.5f * (edge + in.fogEnd);
    CHECK(ForwardFog(mid, in.fogStart, in.fogEnd, on.fogEdgeScale) == Catch::Approx(0.5f).epsilon(1e-3));
    // An empty range keeps the classic factor.
    CHECK(VolFogEdgeScale(500.0f, 500.0f) == 1.0f);
    CHECK(VolFogEdgeScale(1400.0f, 1500.0f) == 1.0f);
}

TEST_CASE("Volumetric fog: the fog floor is the lowest ground nearby, smooth, and never below the sea",
          "[remaster][fog]")
{
    // A valley along z at x = 32 (40 m deep per cell of slope), on 50 m cells,
    // and a sea on the left edge.
    const int n = 64;
    const float cell = 50.0f;
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            h[static_cast<size_t>(z) * n + x] = x < 4 ? -20.0f : 30.0f + 8.0f * std::fabs(static_cast<float>(x - 32));
    const std::vector<float> floorY = BuildFogFloor(h.data(), n, cell);
    REQUIRE(floorY.size() == h.size());
    auto at = [&](int x, int z) { return floorY[static_cast<size_t>(z) * n + x]; };
    // In and beside the valley the floor is the valley's bottom (30 m), on
    // the slopes far up it stays low, so the mist pools along the valley.
    CHECK(at(32, 32) == Catch::Approx(30.0f).margin(1.0f));
    CHECK(at(36, 32) == Catch::Approx(30.0f).margin(2.0f));
    CHECK(at(50, 32) < h[static_cast<size_t>(32) * n + 50] - 60.0f);
    // The sea holds its mist at the water: never below 0.
    for (float v : floorY)
        CHECK(v >= 0.0f);
    CHECK(at(2, 10) == Catch::Approx(0.0f).margin(1e-3));
    // Nothing for an empty grid.
    CHECK(BuildFogFloor(nullptr, n, cell).empty());
    CHECK(BuildFogFloor(h.data(), 1, cell).empty());
}

TEST_CASE("Volumetric fog: the Air stage applies it with the world depth and replaces the legacy shafts",
          "[remaster][fog][PostStages]")
{
    PostFrameInputs in;
    in.fog = true;
    in.sunShafts = 1.0f;
    in.shaftWeight = 0.5f;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Resolve));
    CHECK(plan.Runs(PostStage::Air));
    CHECK_FALSE(plan.Runs(PostStage::Shafts));
    CHECK(plan.Runs(PostStage::Composite));
    // The fog alone asks for the pass.
    PostFrameInputs alone;
    alone.fog = true;
    CHECK(PlanPostStages(alone).Runs(PostStage::Air));
    CHECK(PlanPostStages(alone).Runs(PostStage::Composite));
    // A vehicle interior: no fog. An optics overlay (glow off): the fog
    // still runs, its apply leaves the overlay's mask as drawn (look director
    // round 2: the mist popped away whenever the player aimed).
    for (bool depth : {false, true})
        for (bool glow : {false, true})
        {
            PostFrameInputs f = in;
            f.worldDepth = depth;
            f.glow = glow;
            const PostStagePlan p = PlanPostStages(f);
            CHECK(p.Runs(PostStage::Air) == depth);
        }
    // Without the fog the shafts run as before.
    in.fog = false;
    plan = PlanPostStages(in);
    CHECK_FALSE(plan.Runs(PostStage::Air));
    CHECK(plan.Runs(PostStage::Shafts));
}

TEST_CASE("Volumetric fog: it supersedes B8's air pass, whose gates it turns off while it runs",
          "[remaster][fog][FeatureGates]")
{
    GateInputs in;
    in.sunElevationDeg = 8.0f;
    in.sunAzimuthDeg = 90.0f; // a morning sun in view: B8's beams and mist would run
    in.sunViewAngleDeg = 20.0f;
    LightingTuning t{};
    const FeatureGates on = ComputeFeatureGates(in, t);
    CHECK(on[Feature::VolFog].on);
    for (Feature f : {Feature::AirBeams, Feature::AirHaze, Feature::ValleyMist})
    {
        CAPTURE(GetFeatureInfo(f).key);
        CHECK_FALSE(on[f].on);
        CHECK(std::string(on[f].reason) == "volFog runs in its place");
    }
    t.features.volFog = 0.0f;
    const FeatureGates off = ComputeFeatureGates(in, t);
    CHECK(off[Feature::AirHaze].on);
    CHECK(off[Feature::ValleyMist].on);
    CHECK(off[Feature::AirBeams].on);
    // With the gates forced on (gates=0) the fog still takes their place.
    LightingTuning forced{};
    forced.gates = false;
    CHECK_FALSE(ComputeFeatureGates(in, forced)[Feature::AirHaze].on);
}

TEST_CASE("Volumetric fog: the switch has landed with its knobs; its gate needs the post pass and a drawn world",
          "[remaster][fog][FeatureGates]")
{
    const FeatureInfo& info = GetFeatureInfo(Feature::VolFog);
    CHECK(info.landed);
    CHECK(std::string(info.key) == "volFog");
    CHECK(std::string(info.gpuSection) == "air");
    for (const char* knob : {"fogDensity", "fogQuality", "fogMist", "fogSun", "fogLights"})
    {
        CAPTURE(knob);
        const LightingKey* k = FindLightingKey(knob);
        REQUIRE(k != nullptr);
        CHECK(std::string(k->parent) == "volFog");
        CHECK(KeyDefault(*k) == 1.0f);
    }
    LightingTuning t{};
    GateInputs in;
    CHECK(ComputeFeatureGates(in, t)[Feature::VolFog].on);
    GateInputs interior = in;
    interior.interior = true;
    CHECK_FALSE(ComputeFeatureGates(interior, t)[Feature::VolFog].on);
    GateInputs optics = in;
    optics.optics = true;
    // A gunner's optic overlay keeps the fog (the apply leaves its mask alone).
    CHECK(ComputeFeatureGates(optics, t)[Feature::VolFog].on);
    GateInputs nv = in;
    nv.nightVision = true;
    CHECK_FALSE(ComputeFeatureGates(nv, t)[Feature::VolFog].on);
    LightingTuning noPost = t;
    noPost.postEffects = false;
    CHECK_FALSE(ComputeFeatureGates(in, noPost)[Feature::VolFog].on);
    LightingTuning off = t;
    off.features.volFog = 0.0f;
    CHECK_FALSE(ComputeFeatureGates(in, off)[Feature::VolFog].on);
    // Night, rain, fog, noon: always on (it replaces the distance fog).
    GateInputs night = in;
    night.nightEffect = 1.0f;
    night.sunElevationDeg = -30.0f;
    night.fog = 0.5f;
    night.rainDensity = 1.0f;
    CHECK(ComputeFeatureGates(night, t)[Feature::VolFog].on);
}

TEST_CASE("Volumetric fog: the shaders read the slices and the medium the way VolumetricFog.cpp lays them out",
          "[remaster][fog][Shaders]")
{
    const std::string inject = gl::PostShaderSource(gl::PostShader::FogInject);
    const std::string integrate = gl::PostShaderSource(gl::PostShader::FogIntegrate);
    const std::string apply = gl::PostShaderSource(gl::PostShader::FogApply);
    REQUIRE_FALSE(inject.empty());
    // The injection reads the rig and the world's cloud and canopy terms from
    // the pasted pieces, once each.
    CHECK(CountOf(inject, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(inject, gl::kRemasterTerrainCloudsGLSL) == 1);
    // FogSliceDepth in the injection and the integration, FogSliceCoord in
    // the apply and the reprojection.
    const std::string depth = "fogDepth.x * (exp(fogDepth.y * t) - 1.0) * fogDepth.z";
    CHECK(CountOf(inject, depth) == 1);
    CHECK(CountOf(integrate, depth) == 1);
    CHECK(CountOf(apply, "log(1.0 + max(z, 0.0) * (exp(fogDepth.y) - 1.0) / fogDepth.x) / fogDepth.y") == 1);
    CHECK(CountOf(inject, "log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w") == 1);
    // Eight slices per draw, eight outputs.
    for (const std::string* s : {&inject, &integrate})
        for (int i = 0; i < kFogSlicesPerDraw; ++i)
            CHECK(CountOf(*s, "layout(location = " + std::to_string(i) + ") out vec4 fog" + std::to_string(i) + ";") ==
                  1);
    // The physical Rayleigh air of Atmosphere.hpp, at sea level.
    CHECK(CountOf(inject, "vec3(5.802e-6, 13.558e-6, 33.1e-6)") == 1);
    // The near field: the low beam the shading uses (FrameLights.hpp).
    const std::string nearPass = gl::PostShaderSource(gl::PostShader::FogNear);
    REQUIRE_FALSE(nearPass.empty());
    CHECK(CountOf(nearPass, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamCutLow = -0.005;") == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamCutHigh = 0.025;") == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamHotBand = -0.03;") == 1);
    CHECK(FogNearSteps(1.0f) == 20);
    CHECK(FogNearSteps(9.0f) == 32);
    // The eye's near thinning and the ground air's pooling in the shaders are
    // the C++ ones.
    CHECK(CountOf(inject, "const float kHazeNearFrom = 200.0;") == 1);
    CHECK(kHazeNearFrom == 200.0f);
    CHECK(CountOf(inject, "const float kGroundAirTop = 20.0;") == 1);
    CHECK(kGroundAirTop == 20.0f);
    CHECK(CountOf(inject, "const float kGroundAirFade = 30.0;") == 1);
    CHECK(kGroundAirFade == 30.0f);
    CHECK(CountOf(inject, "const float kDustNearKeep = 0.3;") == 1);
    CHECK(kDustNearKeep == 0.3f);
    CHECK(CountOf(inject, "const float kDustNearFrom = 100.0;") == 1);
    CHECK(kDustNearFrom == 100.0f);
    CHECK(CountOf(inject, "const float kDustNearTo = 1500.0;") == 1);
    CHECK(kDustNearTo == 1500.0f);
    CHECK(CountOf(inject, "overFloor / 80.0") == 1);
    CHECK(kGroundHazePool == 80.0f);
    CHECK(CountOf(inject, "const float kMistNearKeep = 0.15;") == 1);
    CHECK(kMistNearKeep == 0.15f);
    // The apply reads the rig (the sun's glow past the far end) once, and
    // closes small gaps of nearer geometry before it looks the volume up.
    CHECK(CountOf(apply, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(apply, "float zg = fogGapZ(zRaw);") == 1);
    // The night glow's program has its four modes.
    const std::string glow = gl::PostShaderSource(gl::PostShader::FogGlow);
    REQUIRE_FALSE(glow.empty());
    CHECK(CountOf(glow, "int mode = int(glowCtl.x + 0.5);") == 1);
    // The forward fog keeps only the edge through litHaze.x (the surfaces
    // lane's share reads the same factor).
    CHECK(CountOf(gl::RemasterFragmentCommon(), "r0.rgb = mix(fogC, r0.rgb, min(vFogTC * (1.0 + litHaze.x), 1.0));") ==
          1);
}

namespace
{

// A mirror of the injection and its history, to see what a way of sampling
// the cells leaves in the volume over time. A grid of cells looks at a smooth
// medium: a field over the cells' footprint plus a depth term whose gradient
// differs from cell to cell, as at the mist's top where the far slices are
// hundreds of metres deep. Each frame a cell takes the weighted mean of the
// medium at its taps (offsets in cells, and in slices from the slice's
// centre) and keeps 0.88 of its last frame's value (kFogHistoryShare 0.12,
// read at the cell's centre and reprojected for the camera's pan, which
// slides the medium across the grid). The residual is a cell's value less the
// medium at its centre, so the change the camera explains is taken out.
//   pump   the swing over frames of a 5 x 5 block's mean residual (about 60
//          px at 1080p, what the apply's lookup and the eye average): a
//          residual of one sign over whole regions pumps the far haze;
//   travel the correlation between one frame's change of the residual and
//          the last frame's change, at the best cell lag other than zero
//          (less any at lag zero): a pattern that slides through the fog
//          reads as wind, however small its block mean;
//   still  the largest change of a cell's residual from one frame to the
//          next: with the camera held, nothing should move at all.
struct FogTap
{
    double x = 0.0, y = 0.0, z = 0.0, w = 1.0;
};

struct FogSamplingResidual
{
    double pump = 0.0;
    double travel = 0.0;
    double still = 0.0;
};

// tapsOf(cellX, cellY, frame, taps) fills the cell's taps for that frame.
template <class TapsOf>
FogSamplingResidual MeasureFogSampling(TapsOf tapsOf, double panCellsPerFrame)
{
    constexpr int kN = 40, kFrames = 240, kWarm = 80, kLag = 2, kEdge = 4;
    constexpr double kShare = 0.12, kTwoPi = 6.283185307179586;
    auto medium = [](double x, double y, double z)
    {
        const double g = 1.0 + 0.5 * std::sin(0.21 * x) * std::cos(0.17 * y);
        return std::sin(kTwoPi * (0.083 * x + 0.047 * y)) + 0.6 * std::cos(kTwoPi * (0.031 * x - 0.058 * y)) +
               g * (0.9 * z + 0.4 * z * z);
    };
    auto idx = [](int x, int y) { return static_cast<std::size_t>(y) * kN + static_cast<std::size_t>(x); };
    std::vector<double> hist(kN * kN, 0.0), next(kN * kN, 0.0), res(kN * kN, 0.0), prevRes(kN * kN, 0.0),
        change(kN * kN, 0.0), prevChange(kN * kN, 0.0);
    std::vector<double> lagSum((2 * kLag + 1) * (2 * kLag + 1), 0.0), blocks;
    std::vector<FogTap> taps;
    int lagFrames = 0;
    FogSamplingResidual out;
    for (int frame = 0; frame < kFrames; ++frame)
    {
        const double pan = panCellsPerFrame * frame;
        for (int y = 0; y < kN; ++y)
            for (int x = 0; x < kN; ++x)
            {
                taps.clear();
                tapsOf(x, y, frame, taps);
                double v = 0.0;
                for (const FogTap& t : taps)
                    v += t.w * medium(x + pan + t.x, y + t.y, t.z);
                double h = v;
                if (frame > 0)
                {
                    // Last frame's cell under this cell's centre: the pan moved
                    // the medium by panCellsPerFrame cells (linear, clamped).
                    const double px = std::min(x + panCellsPerFrame, kN - 1.0);
                    const int x0 = static_cast<int>(std::floor(px));
                    const int x1 = std::min(x0 + 1, kN - 1);
                    const double f = px - x0;
                    const double hp = hist[idx(x0, y)] * (1.0 - f) + hist[idx(x1, y)] * f;
                    h = hp + kShare * (v - hp);
                }
                next[idx(x, y)] = h;
                res[idx(x, y)] = h - medium(x + pan, y, 0.0);
            }
        hist.swap(next);
        if (frame > 0)
        {
            for (std::size_t i = 0; i < res.size(); ++i)
                change[i] = res[i] - prevRes[i];
            if (frame >= kWarm)
            {
                for (int y = kEdge; y < kN - kEdge; ++y)
                    for (int x = kEdge; x < kN - kEdge; ++x)
                        out.still = std::max(out.still, std::fabs(change[idx(x, y)]));
                if (frame > kWarm)
                {
                    ++lagFrames;
                    for (int dy = -kLag; dy <= kLag; ++dy)
                        for (int dx = -kLag; dx <= kLag; ++dx)
                        {
                            double sa = 0.0, sb = 0.0, saa = 0.0, sbb = 0.0, sab = 0.0;
                            int n = 0;
                            for (int y = kEdge; y < kN - kEdge; ++y)
                                for (int x = kEdge; x < kN - kEdge; ++x)
                                {
                                    const double a = change[idx(x, y)], b = prevChange[idx(x + dx, y + dy)];
                                    sa += a;
                                    sb += b;
                                    saa += a * a;
                                    sbb += b * b;
                                    sab += a * b;
                                    ++n;
                                }
                            const double va = saa - sa * sa / n, vb = sbb - sb * sb / n;
                            const double rho =
                                (va < 1e-20 || vb < 1e-20) ? 0.0 : (sab - sa * sb / n) / std::sqrt(va * vb);
                            lagSum[static_cast<std::size_t>((dy + kLag) * (2 * kLag + 1) + dx + kLag)] += rho;
                        }
                }
            }
            prevChange.swap(change);
        }
        prevRes = res;
        if (frame >= kWarm)
        {
            double block = 0.0;
            for (int y = 18; y < 23; ++y)
                for (int x = 18; x < 23; ++x)
                    block += res[idx(x, y)] / 25.0;
            blocks.push_back(block);
        }
    }
    double mean = 0.0;
    for (double b : blocks)
        mean += b / static_cast<double>(blocks.size());
    double var = 0.0;
    for (double b : blocks)
        var += (b - mean) * (b - mean) / static_cast<double>(blocks.size());
    out.pump = std::sqrt(var);
    const double atZero = std::max(lagSum[static_cast<std::size_t>(kLag * (2 * kLag + 1) + kLag)] / lagFrames, 0.0);
    double best = 0.0;
    for (int dy = -kLag; dy <= kLag; ++dy)
        for (int dx = -kLag; dx <= kLag; ++dx)
            if (dx != 0 || dy != 0)
                best = std::max(best,
                                lagSum[static_cast<std::size_t>((dy + kLag) * (2 * kLag + 1) + dx + kLag)] / lagFrames);
    out.travel = std::max(best - atZero, 0.0);
    return out;
}

} // namespace

TEST_CASE("Volumetric fog: the injection samples fixed points of each cell, so its history can neither pump nor travel",
          "[remaster][fog][Shaders]")
{
    // The dawn valley (2026-10-23), twice. One jittered point per cell at the
    // frame's golden-ratio offset: the history's residual of the jitter had
    // the same sign over whole regions, and the far haze and the valley mist
    // pumped as one sheet. Then that offset turned per cell by an
    // interleaved-gradient pattern (d91a08e0): the pump went, but the pattern
    // is a ramp over the cells, so with the frame's step the next frame's
    // field was this frame's read one cell over, and the residual slid
    // through the fog about a cell per frame: wind. Now each cell's two taps
    // are fixed (VolumetricFog.hpp kFogTapZ, kFogTapXY) and the history is
    // read at the cell's centre through a fixed blur (kFogHistoryBlur).
    const std::string inject = gl::PostShaderSource(gl::PostShader::FogInject);
    REQUIRE_FALSE(inject.empty());
    CHECK(CountOf(inject, "const float kFogTapZ[2] = float[2](0.25, 0.75);") == 1);
    CHECK(kFogTapZ[0] == 0.25f);
    CHECK(kFogTapZ[1] == 0.75f);
    CHECK(CountOf(inject, "const vec2 kFogTapXY[2] = vec2[2](vec2(-0.25, -0.25), vec2(0.25, 0.25));") == 1);
    CHECK(kFogTapXY[0][0] == -0.25f);
    CHECK(kFogTapXY[0][1] == -0.25f);
    CHECK(kFogTapXY[1][0] == 0.25f);
    CHECK(kFogTapXY[1][1] == 0.25f);
    CHECK(CountOf(inject, "const float kFogHistoryBlur = 0.25;") == 1);
    CHECK(kFogHistoryBlur == 0.25f);
    // Each tap weighted by the metres of its half of the slice, on the column
    // through its own point of the cell.
    CHECK(CountOf(inject, "float w = fogSliceDepth((s + kFogTapZ[i] + 0.25) / fogGrid.z) - "
                          "fogSliceDepth((s + kFogTapZ[i] - 0.25) / fogGrid.z);") == 1);
    CHECK(CountOf(inject, "acc += w * fogCell(g_tapDir[i] * fogSliceDepth((s + kFogTapZ[i]) / fogGrid.z));") == 1);
    CHECK(CountOf(inject, "vec2 ndcI = (gl_FragCoord.xy + kFogTapXY[i]) / fogGrid.xy * 2.0 - 1.0;") == 1);
    CHECK(CountOf(inject, "vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;") == 1);
    // The history at the cell's centre (read at the sample point it was a
    // leaky sum of the offsets, not their average), through the fixed blur.
    CHECK(CountOf(inject, "vec3 rel = dirZ * fogSliceDepth((s + 0.5) / fogGrid.z);") == 1);
    CHECK(CountOf(inject, "vec3 tq = vec3(puv, pt), te = vec3(kFogHistoryBlur) / fogGrid.xyz;") == 1);
    CHECK(CountOf(inject, "cur = mix(hist, cur, fogFrame.z);") == 1);
    // Nothing in the injection moves with the frame: no jitter is read and no
    // per-cell pattern is left.
    CHECK(CountOf(inject, "fogDepth.w") == 0);
    CHECK(CountOf(inject, "fogFrame.xy") == 0);
    CHECK(CountOf(inject, "fogCellNoise") == 0);
    CHECK(CountOf(inject, "g_cellJitter") == 0);

    // The mirror, at the dawn valley's slow pan (0.9 px a frame, 0.075 of a
    // 12 px cell) and with the camera held.
    auto fract = [](double x) { return x - std::floor(x); };
    auto gradient = [&](double x, double y) { return fract(52.9829189 * fract(x * 0.06711056 + y * 0.00583715)); };
    auto frameWide = [&](int, int, int frame, std::vector<FogTap>& taps)
    {
        taps.push_back({fract(0.7548776662 * frame) - 0.5, fract(0.5698402910 * frame) - 0.5,
                        fract(0.6180339887 * frame) - 0.5, 1.0});
    };
    auto perCell = [&](int x, int y, int frame, std::vector<FogTap>& taps)
    {
        const double px = x + 0.5, py = y + 0.5, slice = 40.0;
        taps.push_back({fract(fract(0.7548776662 * frame) + gradient(px + 17.0, py + 59.0)) - 0.5,
                        fract(fract(0.5698402910 * frame) + gradient(px + 83.0, py + 29.0)) - 0.5,
                        fract(0.6180339887 * frame + gradient(px, py) + 0.618034 * slice) - 0.5, 1.0});
    };
    auto fixedTaps = [&](int, int, int, std::vector<FogTap>& taps)
    {
        for (int i = 0; i < 2; ++i)
            taps.push_back({kFogTapXY[i][0], kFogTapXY[i][1], kFogTapZ[i] - 0.5, 0.5});
    };
    constexpr double kPan = 0.075, kPumpBar = 0.01, kTravelBar = 0.2;

    // The first failure: one offset for the whole frame pumps (the block's
    // mean swings by about 4% of the medium's slice-deep change), and does
    // not travel.
    const FogSamplingResidual wideHeld = MeasureFogSampling(frameWide, 0.0);
    CAPTURE(wideHeld.pump, wideHeld.travel, wideHeld.still);
    CHECK(wideHeld.pump > 2.0 * kPumpBar);
    CHECK(wideHeld.travel < kTravelBar);
    // The second: the per-cell gradient pattern no longer pumps, but its
    // residual is the last frame's read about one cell over, held or panning.
    const FogSamplingResidual cellHeld = MeasureFogSampling(perCell, 0.0);
    const FogSamplingResidual cellPan = MeasureFogSampling(perCell, kPan);
    CAPTURE(cellHeld.pump, cellHeld.travel, cellPan.travel);
    CHECK(cellHeld.pump < kPumpBar);
    CHECK(cellHeld.travel > 2.0 * kTravelBar);
    CHECK(cellPan.travel > 2.0 * kTravelBar);
    // Both move something with the camera held.
    CHECK(wideHeld.still > 0.05);
    CHECK(cellHeld.still > 0.05);
    // The fixed taps: with the camera held the volume is exactly still; under
    // the pan everything that changes moves with the camera.
    const FogSamplingResidual fixedHeld = MeasureFogSampling(fixedTaps, 0.0);
    const FogSamplingResidual fixedPan = MeasureFogSampling(fixedTaps, kPan);
    CAPTURE(fixedHeld.pump, fixedHeld.travel, fixedHeld.still, fixedPan.travel);
    CHECK(fixedHeld.still == 0.0);
    CHECK(fixedHeld.pump < 1e-12);
    CHECK(fixedHeld.travel < kTravelBar);
    CHECK(fixedPan.travel < 0.05);
}

TEST_CASE("Volumetric fog: round 3 medium rules (clear near air, dust, hilltops, night fog, stars)", "[remaster][fog]")
{
    LightingTuning t;
    FogInputs clear;
    clear.sunElevationDeg = 60.0f;
    clear.skyThrough = 1.0f;
    const FogMedium noon = ComputeFogMedium(clear, t, true);
    // Clear air: the near thinning reaches the whole only far out.
    CHECK(noon.hazeNearTo == Catch::Approx(kHazeNearTo));
    CHECK(noon.hazeNearKeep == Catch::Approx(kHazeNearKeep));
    CHECK(noon.dustSigma == 0.0f);
    // A grey deck: thick near air.
    FogInputs grey = clear;
    grey.skyThrough = 0.0f;
    const FogMedium deck = ComputeFogMedium(grey, t, true);
    CHECK(deck.hazeNearTo == Catch::Approx(kHazeNearToGrey));
    CHECK(deck.hazeNearKeep == Catch::Approx(kHazeNearKeepGrey));
    // A desert: dust, laid by rain.
    FogInputs desert = clear;
    desert.aridity = 1.0f;
    const FogMedium dry = ComputeFogMedium(desert, t, true);
    CHECK(dry.dustSigma == Catch::Approx(kDustHaze));
    desert.rain = 1.0f;
    CHECK(ComputeFogMedium(desert, t, true).dustSigma < 0.5f * kDustHaze);
    // Dust thins with the height over the ground, not over the sea.
    CHECK(FogExtinctionAt(dry, 400.0f, 300.0f, 2.0f, 0.0f) > FogExtinctionAt(dry, 400.0f, 300.0f, 300.0f, 0.0f));
    // setFog at night is denser than by day.
    FogInputs fogDay = clear;
    fogDay.fog = 0.35f;
    FogInputs fogNight = fogDay;
    fogNight.nightEffect = 1.0f;
    fogNight.sunElevationDeg = -30.0f;
    const FogMedium day = ComputeFogMedium(fogDay, t, true);
    const FogMedium night = ComputeFogMedium(fogNight, t, true);
    CHECK(night.weatherSigma == Catch::Approx(day.weatherSigma * (1.0f + kNightFogBoost)));
    // The humid ground air lies in the low ground: gone on a hilltop.
    CHECK(FogGroundAirTop(night, 10.0f, 0.0f, 10.0f) == Catch::Approx(1.0f));
    CHECK(FogGroundAirTop(night, 200.0f, 0.0f, 10.0f) == 0.0f);
    // The stars: all on a clear night, none in setFog.
    FogInputs clearNight = fogNight;
    clearNight.fog = 0.0f;
    CHECK(FogStarKeep(ComputeFogMedium(clearNight, t, true), 50.0f, 0.0f, 50.0f, 0.0f) == Catch::Approx(1.0f));
    CHECK(FogStarKeep(night, 2.0f, 0.0f, 2.0f, 0.0f) == 0.0f);
    // Aridity: red-heavy land is a desert, green land and the sea are not.
    const std::vector<float> heights = {10.0f, 10.0f, -5.0f, -5.0f};
    CHECK(FogAridity({1.3f, 0.7f, 1.3f, 0.7f, 0.6f, 1.0f, 0.6f, 1.0f}, heights) == Catch::Approx(1.0f));
    CHECK(FogAridity({0.8f, 0.9f, 0.8f, 0.9f, 1.4f, 0.5f, 1.4f, 0.5f}, heights) == 0.0f);
    CHECK(FogAridity({}, heights) == 0.0f);
}

TEST_CASE("Volumetric fog: a grey day fades its ridges into the deck's air, with no horizon plate", "[remaster][fog]")
{
    // How grey the sky is for the air's colour: none under a broken sky's
    // blue gaps, all under a full deck.
    CHECK(FogGreySky(1.0f) == 0.0f);
    CHECK(FogGreySky(0.65f) == 0.0f);
    CHECK(FogGreySky(0.4f) == Catch::Approx(0.5f));
    CHECK(FogGreySky(0.15f) == Catch::Approx(1.0f));
    CHECK(FogGreySky(0.0f) == Catch::Approx(1.0f));
    // The ring samples above the classic horizon polygon's top (3.4 degrees).
    CHECK(std::sin(kFogRingGreyFromDeg * 3.14159265f / 180.0f) > kFogHorizonFillTop);

    // A dry full deck thickens the clear haze by 1 + kHazeGreyGain: the
    // ridges from 1 to 3 km fade one behind the other instead of one wash
    // (look fix "plate": the gain was 3.5, everything past 1.5 km was gone).
    const LightingTuning t{};
    const FogMedium noon = ComputeFogMedium(ClearNoon(), t, true);
    FogInputs grey = ClearNoon();
    grey.skyThrough = 0.0f;
    grey.viewDistance = 3500.0f;
    const FogMedium overcast = ComputeFogMedium(grey, t, true);
    CHECK(overcast.hazeSigma == Catch::Approx(noon.hazeSigma * (1.0f + kHazeGreyGain)));
    const float sigmaGrey = FogExtinctionAt(overcast, 100.0f, 0.0f, 100.0f, 0.0f);
    CHECK(std::exp(-sigmaGrey * 1000.0f) > 0.2f);  // a ridge 1 km out keeps a fifth or more
    CHECK(std::exp(-sigmaGrey * 2000.0f) > 0.05f); // one at 2 km is still there
    CHECK(std::exp(-sigmaGrey * 3000.0f) < 0.15f); // one at 3 km is mostly air

    // Rain takes kRainFogShare of the classic tactical visibility's extinction;
    // setFog keeps all of it.
    FogInputs wet = grey;
    wet.rain = 0.8f;
    const FogMedium rain = ComputeFogMedium(wet, t, true);
    const float vis = WeatherVisibility(0.0f, 0.8f, 3500.0f);
    CHECK(vis == Catch::Approx(980.0f));
    CHECK(rain.weatherSigma ==
          Catch::Approx(kWeatherExtinction * (1.0f / vis - 1.0f / 3500.0f) * kRainFogShare).epsilon(1e-4));
    // A hill 300 m away in rain 0.8 keeps over 40 % of its contrast (the
    // shaders' near thinning leaves it more), the classic fog end (where the
    // AI's tactical sight ends) under 15 %, 2 km nearly nothing.
    const float sigmaRain = FogExtinctionAt(rain, 30.0f, 0.0f, 30.0f, 0.0f);
    CHECK(std::exp(-sigmaRain * 300.0f) > 0.4f);
    CHECK(std::exp(-sigmaRain * vis) < 0.15f);
    CHECK(std::exp(-sigmaRain * 2000.0f) < 0.02f);
    FogInputs foggy = grey;
    foggy.fog = 0.3f;
    const float visFog = WeatherVisibility(0.3f, 0.0f, 3500.0f);
    CHECK(ComputeFogMedium(foggy, t, true).weatherSigma ==
          Catch::Approx(kWeatherExtinction * (1.0f / visFog - 1.0f / 3500.0f) * kSetFogBoost).epsilon(1e-4));
}

TEST_CASE("Volumetric fog: the air's light takes the sun's phase at each column's own angle to the sun",
          "[remaster][fog][Shaders]")
{
    // A column straight at the ring's sample: the ring's colour, unchanged
    // (far ground still melts into the sky behind it).
    CHECK(FogRingPhaseGain(0.4f, 0.6f, 3.0f, 3.0f) == Catch::Approx(1.0f));
    // The haze over ground under the horizon toward a low sun is farther
    // from the sun than the ring's sample above the land: less of the sun's
    // forward light (dawn_valley's bay, look fix "dawn").
    const float gBay = FogRingPhaseGain(0.2f, 0.8f, 0.7f, 3.2f);
    CHECK(gBay < 0.5f);
    CHECK(gBay > 0.2f); // the sky's own part stays
    // Away from the sun the phase is flat: next to no change.
    CHECK(FogRingPhaseGain(0.6f, 0.4f, 0.05f, 0.055f) == Catch::Approx(1.0f).margin(0.02));
    // Haze in front of land right under the sun glows, capped.
    CHECK(FogRingPhaseGain(0.2f, 0.8f, 50.0f, 3.0f) == Catch::Approx(kFogRingPhaseMax));
    CHECK(kFogRingPhaseMax >= 1.0f);
    CHECK(kFogRingPhaseMax <= 2.0f);
    CHECK(FogRingPhaseGain(0.0f, 0.0f, 1.0f, 1.0f) >= 0.0f);

    const std::string ring = gl::PostShaderSource(gl::PostShader::FogRing);
    const std::string inject = gl::PostShaderSource(gl::PostShader::FogInject);
    REQUIRE_FALSE(ring.empty());
    REQUIRE_FALSE(inject.empty());
    // The ring's second row holds where it measured the colour.
    CHECK(CountOf(ring, "vec3 value = gl_FragCoord.y < 1.0 ? best : vec3(bestEl, 0.0, 0.0);") == 1);
    CHECK(CountOf(inject, "texture(fogRing, vec2(u, 0.25))") == 1);
    CHECK(CountOf(inject, "texture(fogRing, vec2(u, 0.75))") == 1);
    CHECK(CountOf(inject, "const float kFogRingPhaseMax = 1.5;") == 1);
    CHECK(kFogRingPhaseMax == 1.5f);
    CHECK(CountOf(inject, "vec3 colSky = pr + pm * phaseCol;") == 1);
    CHECK(CountOf(inject, "ring += w * vec4(a.rgb * gain * a.a, a.a);") == 1);
}

TEST_CASE("Volumetric fog: the ring reads the deck over the horizon fill, the apply lays the air over the fill",
          "[remaster][fog][Shaders]")
{
    const std::string ring = gl::PostShaderSource(gl::PostShader::FogRing);
    const std::string apply = gl::PostShaderSource(gl::PostShader::FogApply);
    REQUIRE_FALSE(ring.empty());
    REQUIRE_FALSE(apply.empty());
    CHECK(kFogRingGreyFromDeg == 3.8f);
    CHECK(CountOf(ring, "const float kRingGreyFromDeg = 3.8;") == 1);
    CHECK(CountOf(ring, "max(found + radians(0.3), radians(kRingGreyFromDeg))") == 1);
    CHECK(kFogHorizonFillTop == 0.06f);
    CHECK(CountOf(apply, "const float kApplyFillTop = 0.06;") == 1);
    // Grey (fogSkyWeather.z, FogGreySky) or wet, the fill below the polygon's
    // top is the air entirely; the deck above sinks into it.
    CHECK(CountOf(apply, "float greyFill = max(wetSky, clamp(fogSkyWeather.z, 0.0, 1.0));") == 1);
    CHECK(CountOf(apply, "float hb = exp(-max(sW - kApplyFillTop, 0.0) / 0.09);") == 1);
    CHECK(CountOf(apply, "base = mix(base, airAt, hb * greyFill);") == 1);
}

TEST_CASE("Volumetric fog: an optic overlay's darkening leaves a near-black world channel alone",
          "[remaster][fog][Shaders]")
{
    // Look fix round 5 "mist": world channels at code 0 or 1 gave a ratio of
    // 0 or 0.5 under the old 1e-5 floor with no overlay there (black and green
    // specks over a fogged tree line).
    const float code1 = std::pow(1.0f / 255.0f, 2.2f);
    CHECK(FogSnapRatio(0.0f, 0.0f) == Catch::Approx(1.0f));
    CHECK(FogSnapRatio(code1, code1) == Catch::Approx(1.0f));
    CHECK(FogSnapRatio(0.3f, 0.3f) == Catch::Approx(1.0f));
    // A black mask over the darkest world still darkens it whole.
    CHECK(kFogSnapRatioEps < 0.05f * code1);
    CHECK(FogSnapRatio(0.0f, code1) < 0.05f);
    CHECK(FogSnapRatio(0.0f, 0.2f) < 1e-5f);
    // A half-dark mask, half; brighter than the world (a reticle) is 1 (the
    // apply adds the difference back).
    CHECK(FogSnapRatio(0.1f, 0.2f) == Catch::Approx(0.5f).epsilon(1e-4));
    CHECK(FogSnapRatio(0.4f, 0.2f) == Catch::Approx(1.0f));
    const std::string apply = gl::PostShaderSource(gl::PostShader::FogApply);
    REQUIRE_FALSE(apply.empty());
    CHECK(CountOf(apply, "const float kSnapRatioEps = 1e-7;") == 1);
    CHECK(kFogSnapRatioEps == 1e-7f);
    CHECK(CountOf(apply, "clamp((over + kSnapRatioEps) / (lin0 + kSnapRatioEps), 0.0, 1.0)") == 1);
    CHECK(CountOf(apply, "max(lin0, vec3(1e-5))") == 0);
}

TEST_CASE("Volumetric fog: the composite's occlusion acts on the surface's light under the fog, not on the air",
          "[remaster][fog][Shaders]")
{
    // The apply writes the surface's share of the pixel's light after the fog
    // on a second target (an optic overlay's untouched mask: 1), and the
    // composite scales the surface target's sky and sun shares by it before
    // the occlusion and the contact shadow: darkening the whole pixel by that
    // share of the occlusion is darkening the surface under the fog (look fix
    // "plate": dark rims at the foot of fogged trees).
    const std::string apply = gl::PostShaderSource(gl::PostShader::FogApply);
    CHECK(CountOf(apply, "layout(location = 0) out vec4 fragColor;") == 1);
    CHECK(CountOf(apply, "layout(location = 1) out vec4 fogVeil;") == 1);
    CHECK(CountOf(apply, "fogVeil = vec4(1.0);") == 1);
    CHECK(CountOf(apply, "float surfL = dot(base * T, kApplyLuma);") == 1);
    CHECK(CountOf(apply, "fogVeil = vec4(allL > 1e-6 ? clamp(surfL / allL, 0.0, 1.0) : clamp(T, 0.0, 1.0));") == 1);
    const std::string composite = gl::PostShaderSource(gl::PostShader::Composite);
    REQUIRE_FALSE(composite.empty());
    CHECK(CountOf(composite, "uniform sampler2D fogVeilTex;") == 1);
    CHECK(CountOf(composite, "float veil = surfParams2.y > 0.5 ? textureLod(fogVeilTex, vUV, 0.0).r : 1.0;") == 1);
    const size_t veiled = composite.find("s.r *= veil;");
    const size_t keep = composite.find("keep = max(1.0 - clamp(s.r, 0.0, 1.0) * (1.0 - ao)");
    REQUIRE(veiled != std::string::npos);
    REQUIRE(keep != std::string::npos);
    CHECK(veiled < keep);
    CHECK(CountOf(composite, "s.b *= veil;") == 1);
    // A free unit: not a post unit, not a material or upload unit.
    CHECK(gl::kFogVeilUnit == gl::kFogFloorUnit);
    for (int u : gl::kPostUnits)
        CHECK(u != gl::kFogVeilUnit);
    CHECK(gl::kFogVeilUnit != 1);
    CHECK(gl::kFogVeilUnit != 7);
}

TEST_CASE("Volumetric fog: clear noon air is nearly clear for the first kilometre, the far ridges keep theirs",
          "[remaster][fog]")
{
    // Look fix "veil": the clear haze's optical depth along a level ray 300 m
    // up, with the eye's near thinning (the injection's hazeKeep).
    LightingTuning t;
    const FogMedium noon = ComputeFogMedium(ClearNoon(), t, true);
    auto tau = [](const FogMedium& m, float to)
    {
        const float sigma = m.hazeSigma * std::exp(-300.0f / m.hazeHeight);
        double sum = 0.0;
        for (float s = 0.5f; s < to; s += 1.0f)
            sum += sigma * FogHazeNearKeep(m, s);
        return static_cast<float>(sum);
    };
    // The round-2 thinning (0.35 at the eye, the whole at 2.5 km).
    FogMedium before = noon;
    before.hazeNearKeep = 0.35f;
    before.hazeNearTo = 2500.0f;
    CHECK(FogHazeNearKeep(noon, 0.0f) == Catch::Approx(kHazeNearKeep));
    CHECK(FogHazeNearKeep(noon, kHazeNearTo) == Catch::Approx(1.0f));
    // The first few hundred metres are nearly clear: a dark wood 500 m out
    // takes under 2.5 % of the horizon's light (it took 8 %, a milky veil).
    CHECK(tau(noon, 500.0f) < 0.025f);
    CHECK(tau(before, 500.0f) > 0.07f);
    CHECK(tau(noon, 1000.0f) < 0.08f);
    // The depth is in the distance: ridges 3 and 5 km out keep most of their air.
    CHECK(tau(noon, 3000.0f) > 0.6f);
    CHECK(tau(noon, 5000.0f) > 0.8f * tau(before, 5000.0f));
    // A grey deck keeps its thick near air.
    FogInputs grey = ClearNoon();
    grey.skyThrough = 0.0f;
    const FogMedium deck = ComputeFogMedium(grey, t, true);
    CHECK(FogHazeNearKeep(deck, 0.0f) == Catch::Approx(kHazeNearKeepGrey));
    CHECK(FogHazeNearKeep(deck, kHazeNearToGrey) == Catch::Approx(1.0f));
    // The shaders' thinning is this one, read from the medium.
    const std::string inject = gl::PostShaderSource(gl::PostShader::FogInject);
    CHECK(CountOf(inject, "float hazeKeep = mix(fogWind.w, 1.0, smoothstep(kHazeNearFrom, fogWind.z, dist));") == 1);
}

TEST_CASE("Volumetric fog: the air's colour along the screen's edges is the ring's, not the model's",
          "[remaster][fog][Shaders]")
{
    // Look fix "veil": a column takes the ring's measured sky as surely as the
    // freshest of its nine taps (the taps past the screen's edge, never seen,
    // no longer count as doubt), and the model only where the ring's
    // measurements in that azimuth have aged.
    const std::string inject = gl::PostShaderSource(gl::PostShader::FogInject);
    REQUIRE_FALSE(inject.empty());
    CHECK(CountOf(inject, "ringSure = max(ringSure, a.a * (1.0 - 0.1 * abs(float(k))));") == 1);
    CHECK(CountOf(inject, "g_air = mix(g_air, ring.rgb, clamp(ringSure, 0.0, 1.0) * (1.0 - fogNightAir.w)) * "
                          "fogLightK.y;") == 1);
    CHECK(CountOf(inject, "clamp(ring.a, 0.0, 1.0)") == 0);
}

TEST_CASE("Volumetric fog: a day's setFog is a light mist, the foggy night keeps its density", "[remaster][fog]")
{
    // Look fix round 4 "mist": setFog's own fog by day is kSetFogBoost of the
    // classic extinction (0.75), the night's is round 3's (2 x 2.8).
    FogInputs day = ClearNoon();
    day.viewDistance = 1500.0f;
    day.fog = 0.3f;
    day.fogEnd = WeatherVisibility(day.fog, 0.0f, day.viewDistance);
    const float classic = kWeatherExtinction * (1.0f / day.fogEnd - 1.0f / day.viewDistance);
    const FogMedium d = ComputeFogMedium(day, LightingTuning{}, true);
    CHECK(kSetFogBoost < 1.0f);
    CHECK(d.weatherSigma == Catch::Approx(classic * kSetFogBoost).epsilon(1e-4));
    FogInputs night = day;
    night.nightEffect = 1.0f;
    night.sunElevationDeg = -30.0f;
    const FogMedium n = ComputeFogMedium(night, LightingTuning{}, true);
    CHECK(n.weatherSigma == Catch::Approx(classic * 2.0f * 2.8f).epsilon(1e-3));
    // Under a grey deck the grey haze does not stack on a day's setFog (as on
    // rain's fog): the rock 150 m out keeps more than 60 % at the fog floor.
    FogInputs grey = day;
    grey.skyThrough = 0.09f;
    const FogMedium g = ComputeFogMedium(grey, LightingTuning{}, true);
    CHECK(g.hazeSigma < kHazeClear * (1.0f + 0.5f * kHazeGreyGain));
    FogInputs dryGrey = grey;
    dryGrey.fog = 0.0f;
    CHECK(ComputeFogMedium(dryGrey, LightingTuning{}, true).hazeSigma > g.hazeSigma);
    CHECK(std::exp(-FogExtinctionAt(g, 0.0f, 0.0f, 1.75f, 0.0f) * 150.0f) > 0.6f);
    CHECK(std::exp(-FogExtinctionAt(g, 0.0f, 0.0f, 1.75f, 0.0f) * g.zFar) > 0.08f);
    // Look fix round 5 "mist": a light mist, not dense fog. The rock 300 m out
    // keeps about two thirds of itself and the tree line 500 m out half, at
    // the fog floor, while the classic fog end (742 m) is still faded; the
    // clear haze gives way to setFog's own mist by day.
    const float sig = FogExtinctionAt(g, 0.0f, 0.0f, 1.75f, 0.0f);
    CHECK(std::exp(-sig * 300.0f) > 0.6f);
    CHECK(std::exp(-sig * 500.0f) > 0.45f);
    CHECK(std::exp(-sig * day.fogEnd) < 0.45f);
    CHECK(kSetFogBoost <= 0.5f);
    CHECK(kMistFogWeight < kMistFogWeightNight);
    FogInputs clearAir = grey;
    clearAir.fog = 0.0f;
    clearAir.skyThrough = 1.0f;
    FogInputs mistAir = clearAir;
    mistAir.fog = 0.3f;
    CHECK(ComputeFogMedium(mistAir, LightingTuning{}, true).hazeSigma ==
          Catch::Approx(ComputeFogMedium(clearAir, LightingTuning{}, true).hazeSigma * (1.0f - kSetFogHazeCut)));
    FogInputs mistNight = mistAir;
    mistNight.nightEffect = 1.0f;
    mistNight.sunElevationDeg = -30.0f;
    FogInputs clearNight = mistNight;
    clearNight.fog = 0.0f;
    CHECK(ComputeFogMedium(mistNight, LightingTuning{}, true).hazeSigma ==
          Catch::Approx(ComputeFogMedium(clearNight, LightingTuning{}, true).hazeSigma));
}

TEST_CASE("Volumetric fog: the morning shafts between crowns are strong enough to read", "[fog][remaster]")
{
    // Look fix round 4 "sun": at 14 the shafts of pine_rays barely read; the
    // near pass's soft cap was not the limit (the beams are linear in the
    // gain there), so the gain itself rose. The beams' gain at fogSun 1.
    CHECK(kFogBeamGain >= 30.0f);
    CHECK(kFogBeamGain <= 40.0f);
    CHECK(kFogBeamReach == Catch::Approx(180.0f));
}
