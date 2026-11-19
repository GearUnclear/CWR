// Wreck and building fire follow-ups (2026-11-18, the wrecks-after shooter's
// faults 1, 3, 5 and 6): an object's own smoke column whatever the grass fire
// round it does, building draws new each game, a wreck's char and hot shell,
// and a car's smoke over its timeline (the wildfire consult's tenth answer).
#include <Poseidon/World/Simulation/FireObjects.hpp>
#include <Poseidon/World/Simulation/FirePlume.hpp>
#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <vector>

using namespace Poseidon::firespread;

namespace
{
WreckTimeline Timeline(WreckClass cls, std::uint32_t seed, float loadL = 0.0f)
{
    WreckConfig c;
    c.cls = cls;
    c.fuelCargoL = loadL;
    c.ammo = cls == WreckClass::Armour;
    return WreckTimelineOf(c, seed);
}
// A wreck's smoke a second averaged over [t0, t1] (s).
float MeanSmoke(const WreckTimeline& w, float t0, float t1, float D)
{
    double sum = 0.0;
    int n = 0;
    for (float t = t0; t <= t1; t += 10.0f, ++n)
        sum += WreckStateAt(w, t, D).smoke;
    return n ? static_cast<float>(sum / n) : 0.0f;
}
// Its column's optical depth seen side-on 30 m downwind (a 12.5 m/s wind):
// its smoke over the wind and the plume's thickness there (FirePlume.hpp).
float SideTau30(const WreckState& s, float wind)
{
    const PlumeLaw l = MakePlumeLaw(std::max(s.bodyMW + s.poolMW, 0.05f), wind, 0.85f * s.bodyFlameM + 0.5f, false);
    return PlumeSideTau(l, 30.0f, s.smoke, wind, 1.0f);
}
} // namespace

TEST_CASE("followups: an object fire's column has its own puffs, unbroken", "[firespread]")
{
    // a burning house (25 MW, its smoke 75 units leaving at 7 m, born 5.6 m),
    // a car (3 MW, 14 units), a truck (8 MW, 48) and a tanker's pool (240 MW,
    // 720), on the worst day's wind and the typical's, seen from 30 m
    struct Src
    {
        float mw, smoke, baseM, bornM;
    };
    const Src srcs[] = {{25.0f, 75.0f, 7.0f, 5.6f},
                        {3.0f, 14.0f, 3.9f, 2.2f},
                        {8.0f, 48.0f, 6.0f, 3.5f},
                        {240.0f, 720.0f, 19.6f, 8.0f}};
    for (float wind : {12.5f, 4.5f})
        for (const Src& s : srcs)
        {
            const PlumeLaw l = MakePlumeLaw(s.mw, wind, s.baseM, false);
            const float pxPerM = 960.0f / (30.0f * 0.75f);
            const ObjectColumnPlan p = PlanObjectColumn(l, s.smoke, s.bornM, wind, pxPerM);
            CHECK(p.lod == Catch::Approx(1.0f));
            CHECK(p.rate > 0.5f);
            CHECK(p.rate <= kObjColumnRateMax + 1e-3f);
            // side-on, its puffs lie behind each other, never a row of discs
            for (float x : {30.0f, 100.0f, 300.0f})
                CHECK(ObjectColumnOverlap(l, x, p.bornM, p.perMetre) >= 1.0f);
            // the budget cut to a quarter: fewer puffs, born larger, the column holds near its source
            const ObjectColumnPlan q = PlanObjectColumn(l, s.smoke, s.bornM, wind, pxPerM, 0.25f);
            CHECK(q.rate < p.rate);
            CHECK(q.bornM >= p.bornM);
            CHECK(ObjectColumnOverlap(l, 30.0f, q.bornM, q.perMetre) >= 1.0f);
            // its puffs thin by its own puffs a metre; by the whole grass
            // fire's (30 puffs a second, the bug: a house drew one in
            // hundreds of them) as many or more died on their way
            const PlumePoint own = PlumeAt(l, 100.0f, p.bornM, p.perMetre, 1.0f, 1.0f, kObjPuffOverlap);
            const PlumePoint grass = PlumeAt(l, 100.0f, p.bornM, 30.0f / wind, 1.0f, 1.0f);
            CHECK(own.survival >= grass.survival);
            CHECK(own.survival > 0.5f);
        }
    // a house's column 1 km off: fewer, larger puffs
    const PlumeLaw house = MakePlumeLaw(25.0f, 12.5f, 7.0f, false);
    const ObjectColumnPlan nearP = PlanObjectColumn(house, 75.0f, 5.6f, 12.5f, 960.0f / (30.0f * 0.75f));
    const ObjectColumnPlan farP = PlanObjectColumn(house, 75.0f, 5.6f, 12.5f, 960.0f / (1000.0f * 0.75f));
    CHECK(farP.lod > 1.5f);
    CHECK(farP.rate < nearP.rate);
    CHECK(farP.bornM > nearP.bornM);
    CHECK(ObjectColumnOverlap(house, 100.0f, farP.bornM, farP.perMetre) >= 1.0f);
    // a thin smoke (a shell smoking at the end) sends fewer, wider puffs
    const ObjectColumnPlan thin = PlanObjectColumn(house, 2.0f, 5.6f, 12.5f, 960.0f / (30.0f * 0.75f));
    CHECK(thin.rate < nearP.rate);
    CHECK(thin.bornM > nearP.bornM);
}

TEST_CASE("followups: buildings draw anew each game, repeatably when asked", "[firespread]")
{
    // a deterministic world: a fixed sequence by the game's count of rolls,
    // whatever the clock says
    CHECK(BuildingRunSeedFor(1, true, 12345ULL) == BuildingRunSeedFor(1, true, 987654321ULL));
    CHECK(BuildingRunSeedFor(1, true, 0ULL) != BuildingRunSeedFor(2, true, 0ULL));
    // in play: from the clock
    CHECK(BuildingRunSeedFor(1, false, 12345ULL) != BuildingRunSeedFor(1, false, 12346ULL));
    std::set<std::uint32_t> seeds;
    for (std::uint64_t e = 0; e < 1000; ++e)
        seeds.insert(BuildingRunSeedFor(1, false, 1700000000000000000ULL + e * 1000ULL));
    CHECK(seeds.size() == 1000);
    // one run seed repeats every draw exactly
    CHECK(BuildingDrawFor(0x1234567U, 100, 77U) == BuildingDrawFor(0x1234567U, 100, 77U));
    // Cancon's eight houses passed by the worst day's front (chance 0.298
    // each): with the old fixed seed every game drew the same u for each; now
    // each house catches in about 0.298 of the games, and in about 94% of
    // the games at least one of the eight does
    const std::uint32_t houses[8] = {0x9a3f11c2U, 0x1b7e0d45U, 0x5c21aa90U, 0x77e4b301U,
                                     0x2d90fe17U, 0x63aa0c58U, 0x0f4d71e9U, 0xc8b25a36U};
    constexpr int kGames = 4000;
    int any = 0;
    int caught[8] = {};
    for (int g = 0; g < kGames; ++g)
    {
        const std::uint32_t run = BuildingRunSeedFor(static_cast<std::uint32_t>(g + 1), false,
                                                     0x5deece66dULL * static_cast<std::uint64_t>(g + 7));
        bool one = false;
        for (int h = 0; h < 8; ++h)
            if (BuildingDrawFor(houses[h], 100, run) < 0.298f)
            {
                caught[h]++;
                one = true;
            }
        any += one ? 1 : 0;
    }
    for (int h = 0; h < 8; ++h)
        CHECK(static_cast<float>(caught[h]) / kGames == Catch::Approx(0.298f).margin(0.03f));
    CHECK(static_cast<float>(any) / kGames == Catch::Approx(1.0f - std::pow(0.702f, 8.0f)).margin(0.02f));
}

TEST_CASE("followups: a car's smoke is densest in its tyre phase, then a smoking shell", "[firespread]")
{
    const float D = 1.9f; // a jeep
    for (std::uint32_t seed = 1; seed < 200; ++seed)
    {
        const WreckTimeline w = Timeline(WreckClass::Car, seed);
        // its tyres catch 5 to 12 min in (the consult: 5 to 20 min, the first tyres)
        CHECK(w.tyreCatchSec >= 120.0f + kTyreCatchMinSec - 1.0f);
        CHECK(w.tyreCatchSec <= w.tyreSec);
        const float fuel = MeanSmoke(w, 30.0f, w.fuelSec, D);
        const float body = MeanSmoke(w, w.fuelSec + 60.0f, w.tyreCatchSec, D);
        const float tyres = MeanSmoke(w, w.tyreSec, w.flameEndSec - 200.0f, D);
        // the tyre phase (20 to 45 min) the densest black smoke of its fire
        CHECK(tyres > body);
        CHECK(tyres >= 0.75f * fuel);
        const float t = 0.5f * (w.tyreSec + w.flameEndSec - 200.0f);
        CHECK(SideTau30(WreckStateAt(w, t, D), 12.5f) > SideTau30(WreckStateAt(w, 60.0f, D), 12.5f));
        CHECK(WreckStateAt(w, t, D).black >= 0.99f);
        // a smoking shell after its flames, greying, to its smoke's end (1 to 2 h)
        const WreckState after = WreckStateAt(w, w.flameEndSec + 600.0f, D);
        CHECK(after.smoke > 0.1f * tyres);
        CHECK(after.smoke < tyres);
        CHECK(after.black < 0.9f);
        CHECK(WreckStateAt(w, w.smokeEndSec - 300.0f, D).smoke > 0.0f);
        CHECK(WreckStateAt(w, w.smokeEndSec + 1.0f, D).smoke <= 0.0f);
    }
    // a truck's six to ten tyres: an hour of black smoke
    for (std::uint32_t seed = 1; seed < 60; ++seed)
    {
        const WreckTimeline t = Timeline(WreckClass::Truck, seed);
        for (float at = 900.0f; at < t.flameEndSec - 200.0f; at += 300.0f)
        {
            const WreckState s = WreckStateAt(t, at, 3.9f);
            CHECK(s.black >= 0.9f);
            CHECK(s.smoke >= 0.5f * MeanSmoke(t, 600.0f, 1200.0f, 3.9f));
        }
        CHECK(t.flameEndSec - t.tyreCatchSec >= 2700.0f);
    }
}

TEST_CASE("followups: a wreck chars as it burns and its shell glows after its flames", "[firespread]")
{
    for (int cls = 0; cls < kWreckClasses; ++cls)
        for (std::uint32_t seed = 1; seed < 60; ++seed)
        {
            const WreckClass c = static_cast<WreckClass>(cls);
            const WreckTimeline w = Timeline(c, seed, c == WreckClass::Tanker ? 3000.0f : 0.0f);
            const float D = c == WreckClass::Car ? 1.9f : 3.9f;
            float last = 0.0f;
            for (float t = 0.0f; t < w.flameEndSec + 600.0f; t += 30.0f)
            {
                const float k = WreckStateAt(w, t, D).charK;
                CHECK(k >= last - 1e-4f);
                CHECK(k <= 1.0f);
                last = k;
            }
            CHECK(WreckStateAt(w, w.flameEndSec, D).charK == Catch::Approx(1.0f));
            CHECK(WreckStateAt(w, w.hotEndSec + 3600.0f, D).charK == Catch::Approx(1.0f));
            // hot in its flames
            CHECK(WreckStateAt(w, 600.0f, D).shellGlow > 0.3f);
            // still hot after them, cooling to nothing by its hot time's end
            const float g0 = WreckStateAt(w, w.flameEndSec + 60.0f, D).shellGlow;
            const float g1 = WreckStateAt(w, w.flameEndSec + 0.3f * (w.hotEndSec - w.flameEndSec), D).shellGlow;
            CHECK(g0 > 0.4f);
            CHECK(g1 > 0.05f);
            CHECK(g1 < g0);
            CHECK(WreckStateAt(w, w.hotEndSec, D).shellGlow == 0.0f);
            // its fire's record lives until its shell has cooled
            CHECK_FALSE(WreckStateAt(w, w.hotEndSec - 60.0f, D).done);
            CHECK(WreckStateAt(w, w.hotEndSec + 1.0f, D).done);
        }
    // a car: about half charred when its fuel phase ends, burnt out with its flames
    for (std::uint32_t seed = 1; seed < 200; ++seed)
    {
        const WreckTimeline w = Timeline(WreckClass::Car, seed);
        const float k = WreckStateAt(w, w.fuelSec, 1.9f).charK;
        CHECK(k >= 0.2f);
        CHECK(k <= 0.7f);
        CHECK(WreckStateAt(w, w.bodyHoldSec, 1.9f).charK >= 0.75f);
        // its shell glows on for an hour after its flames (its hot time 2 to 4 h)
        CHECK(WreckStateAt(w, w.flameEndSec + 3600.0f, 1.9f).shellGlow > 0.0f);
    }
    // armour: its hull hot 6 to 12 h
    const WreckTimeline a = Timeline(WreckClass::Armour, 5);
    CHECK(WreckStateAt(a, a.flameEndSec + 2.0f * 3600.0f, 4.1f).shellGlow > 0.0f);
}

TEST_CASE("followups: a wreck's char and glow reach the shader in its char class", "[firespread]")
{
    // its char in 128 steps, its glow in 64, its glow's light (on the scene's
    // scale: by day a few millionths, by night up to a tenth or so) log-packed
    for (float c : {0.0f, 0.13f, 0.5f, 0.999f, 1.0f})
        for (float g : {0.0f, 0.02f, 0.37f, 1.0f})
            for (float light : {0.0f, 1e-6f, 3e-4f, 0.02f, 0.5f})
            {
                const float cls = WreckBurnClass(c, g, light);
                CHECK(cls >= 3.0f);
                CHECK(cls < 4.0f);
                float c2 = -1.0f, g2 = -1.0f, l2 = -1.0f;
                WreckBurnUnpack(cls, c2, g2, l2);
                CHECK(c2 == Catch::Approx(c).margin(0.5f / 127.0f + 1e-6f));
                CHECK(g2 == Catch::Approx(g).margin(0.5f / 63.0f + 1e-6f));
                if (light <= 0.0f)
                    CHECK(l2 == 0.0f);
                else
                    CHECK(l2 == Catch::Approx(light).epsilon(0.05f));
            }
    // by day the full glow (a panel in its flames) is at most a faint red
    // tint beside the sun (the ground's embers' scale: pi L lux on a white
    // surface's 1.6 over 160000 lux a rig unit, the gain 1 by day)
    const float day = kWreckGlowNits * 3.14159265f * 1.6f / 160000.0f;
    CHECK(day < 0.1f); // the sun on the rig's scale is about 1
    // by night it is a few times the ground's embers' veins at most (12
    // cd/m2, EngineGL33_LightingShadersSurface.cpp kEmberNits): at fifty
    // times them the night camera burnt the shell out flat
    CHECK(kWreckGlowNits <= 4.0f * 12.0f);
    CHECK(WreckGlowNits(1.0f) == Catch::Approx(kWreckGlowNits));
    // just after its flames (a car's shell about 0.45) an orange-red, still a
    // dull red at 2 h (about 0.13), faint at its end
    CHECK(WreckGlowNits(0.45f) > 8.0f);
    CHECK(WreckGlowNits(0.45f) < 25.0f);
    CHECK(WreckGlowNits(0.13f) > 0.3f * WreckGlowNits(0.45f));
    CHECK(WreckGlowNits(0.13f) < WreckGlowNits(0.45f));
    CHECK(WreckGlowNits(0.03f) < 0.15f * kWreckGlowNits);
    CHECK(WreckGlowNits(0.0f) == 0.0f);
}
