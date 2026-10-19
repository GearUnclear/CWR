#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
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
    // under half at two thirds of it, where the classic linear fog is half
    // (with kSetFogBoost, look director round 2: under half already at a
    // third of it).
    FogInputs in = ClearNoon();
    in.viewDistance = 1500.0f;
    in.fog = 0.3f;
    in.fogEnd = WeatherVisibility(in.fog, 0.0f, in.viewDistance);
    in.fogStart = in.fogEnd * 0.4f;
    const FogMedium m = ComputeFogMedium(in, LightingTuning{}, true);
    CHECK(m.weatherSigma > 0.0f);
    const float sigma = FogExtinctionAt(m, 0.0f, 0.0f);
    const float left = std::exp(-sigma * in.fogEnd);
    CHECK(left < 0.25f);
    CHECK(left > 0.001f);
    CHECK(std::exp(-sigma * 0.66f * in.fogEnd) < 0.5f);
    CHECK(std::exp(-sigma * 0.33f * in.fogEnd) < 0.5f);
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
