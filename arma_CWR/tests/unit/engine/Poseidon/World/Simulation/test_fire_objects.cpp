// Object fires and wrecks (fire-objects, 2026-11-18): the wrecks' timelines by
// class, their embers' schedule, the fireball's size and the build-to table's
// ignition chances on the two fire days (the wildfire consult's tenth answer),
// driven through the same pure draws the game runs (FireObjects.hpp).
#include <Poseidon/World/Simulation/FireObjects.hpp>
#include <Poseidon/World/Simulation/FirePlume.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Poseidon::firespread;

namespace
{
float Hash01Test(std::uint32_t s)
{
    ObjRand r(s * 2654435761U + 1U);
    return r.U();
}

WreckTimeline CarTimeline(std::uint32_t seed, bool fuel = true)
{
    WreckConfig c;
    c.cls = WreckClass::Car;
    c.fuelBurning = fuel;
    return WreckTimelineOf(c, seed);
}

// One wreck's first ignition (s; 1e9 none) on a synthetic ground: geometry 0
// the middle of an 8 m bare road (grass from 4 m off its centre line, the car
// along it), 1 a car on bare ground with cured grass all round from g off its
// sides (the nearest grass within 3 m of its middle: g 0 to 2.1 m), wind of
// `wind` m/s from a random direction; the draws are the game's
// (WreckFireballDraws, WreckBodyFire, WreckPoolFire, ObjectFireDraws).
float FirstIgnition(int geometry, float m1h, float wind, float tEnd, std::uint32_t seed)
{
    ObjRand r(seed * 7919U + 13U);
    const float g = 2.1f * r.U();
    const float wa = 6.2831853f * r.U();
    const float wx = wind * std::cos(wa), wz = wind * std::sin(wa);
    ObjectFire body;
    body.halfL = 2.1f;
    body.halfW = 0.9f;
    if (geometry != 0)
    {
        const float a = 6.2831853f * r.U();
        body.ux = std::cos(a);
        body.uz = std::sin(a);
    }
    auto burnable = [&](float x, float z)
    { return geometry == 0 ? std::fabs(z) >= 4.0f : ObjectOutside(body, x, z) >= g; };
    const WreckDay day = WreckDayOf(m1h);
    const WreckTimeline w = CarTimeline(seed * 31U + 5U);
    float first = 1e9f, now = 0.0f;
    auto light = [&](float x, float z, float chance, float delay, ObjectIgniteKind) -> bool
    {
        if (!burnable(x, z))
            return false;
        if (r.U() < chance)
            first = std::min(first, now + delay);
        return true;
    };
    auto area = [&](float, float, float) { first = std::min(first, now); };
    const float kg = WreckFireballKg(WreckClass::Car, 0.0f, r.U(), 40.0f + 40.0f * r.U(), 0.0f);
    WreckFireballDraws(w, day, body, 0.5f * 5.8f * std::cbrt(kg), r, light, area);
    const float D = std::sqrt(16.0f * body.halfL * body.halfW / 3.14159265f);
    for (now = 0.0f; now < tEnd && now < first; now += 0.5f)
    {
        const WreckState s = WreckStateAt(w, now, D);
        ObjectFire b = body;
        WreckBodyFire(w, s, day, b);
        ObjectFireDraws(b, 0.5f, wx, wz, r, light);
        if (s.poolAreaM2 > 0.0f)
        {
            ObjectFire p = body;
            WreckPoolFire(w, s, day, body.halfL, body.halfW, p);
            ObjectFireDraws(p, 0.5f, wx, wz, r, light);
        }
    }
    return first;
}

struct Share
{
    float by = 0.0f, median = 0.0f;
};
Share Trials(int geometry, float m1h, float wind, float tEnd, float by, int n, std::uint32_t seed0)
{
    std::vector<float> lit;
    int hits = 0;
    for (int i = 0; i < n; ++i)
    {
        const float f = FirstIgnition(geometry, m1h, wind, tEnd, seed0 + static_cast<std::uint32_t>(i));
        if (f <= by)
            hits++;
        if (f < tEnd)
            lit.push_back(f);
    }
    std::sort(lit.begin(), lit.end());
    Share s;
    s.by = static_cast<float>(hits) / static_cast<float>(n);
    s.median = lit.empty() ? 0.0f : lit[lit.size() / 2];
    return s;
}
} // namespace

TEST_CASE("object fires: the two fire days as numbers", "[firespread]")
{
    // the build's pig on the tenth answer's two days
    CHECK(EmberPig(kDayTypicalM1h) == Catch::Approx(0.630f).margin(0.002f));
    CHECK(EmberPig(kDayWorstM1h) == Catch::Approx(0.956f).margin(0.002f));
    CHECK(DayChance(0.02f, 0.12f, kDayTypicalM1h) == Catch::Approx(0.02f).margin(1e-4f));
    CHECK(DayChance(0.02f, 0.12f, kDayWorstM1h) == Catch::Approx(0.12f).margin(1e-4f));
    CHECK(DayTime(55.0f, 4.0f, kDayTypicalM1h) == Catch::Approx(55.0f).margin(0.01f));
    CHECK(DayTime(55.0f, 4.0f, kDayWorstM1h) == Catch::Approx(4.0f).margin(0.01f));
    // a damper day lights less, never more; past the grass's extinction nothing
    CHECK(DayChance(0.02f, 0.12f, 0.10f) < 0.02f);
    const WreckDay wet = WreckDayOf(0.13f);
    CHECK(wet.pig == 0.0f);
    CHECK(wet.heatMeanSec == 0.0f);
    CHECK(wet.debris == 0.0f);
}

TEST_CASE("wrecks: a car or jeep burns on the tenth answer's timeline", "[firespread]")
{
    for (std::uint32_t seed = 1; seed < 200; ++seed)
    {
        const WreckTimeline w = CarTimeline(seed);
        const float D = 3.1f;
        // its fuel 2 to 5 min with 3 to 5 m flames (a slow wander of 8% on it)
        REQUIRE(w.fuelSec >= 120.0f);
        REQUIRE(w.fuelSec <= 300.0f);
        const float early = WreckStateAt(w, 60.0f, D).bodyFlameM;
        CHECK(early >= 3.0f * 0.9f);
        CHECK(early <= 5.0f * 1.1f);
        // then 2 to 3 m from every opening
        const float mid = WreckStateAt(w, 600.0f, D).bodyFlameM;
        CHECK(mid >= 2.0f * 0.9f * 0.6f);
        CHECK(mid <= 3.0f * 1.1f);
        // flaming 30 to 45 min, smoke 1 to 2 h, the shell hot 2 to 4 h
        CHECK(w.flameEndSec >= 1800.0f);
        CHECK(w.flameEndSec <= 2700.0f);
        CHECK(w.smokeEndSec >= 3600.0f);
        CHECK(w.smokeEndSec <= 7200.0f + 1e-3f);
        CHECK(w.hotEndSec >= 7200.0f);
        CHECK(w.hotEndSec <= 14400.0f);
        const WreckState after = WreckStateAt(w, w.flameEndSec + 1.0f, D);
        CHECK(after.bodyFlameM <= 0.31f);
        CHECK(after.emberRate == 0.0f);
        CHECK(WreckStateAt(w, w.flameEndSec + 600.0f, D).smoking);
        CHECK(WreckStateAt(w, w.smokeEndSec + 1.0f, D).smoke <= 0.0f);
        // its heat: 3 to 6 MW burning its fuel, 1.5 to 3 MW after (Heskestad)
        const float fuelMW = WreckStateAt(w, 60.0f, D).bodyMW;
        CHECK(fuelMW >= 2.5f);
        CHECK(fuelMW <= 7.0f);
    }
    // without its fuel, its peak comes 10 to 20 min on
    const WreckTimeline dry = CarTimeline(7, false);
    CHECK(dry.fuelSec == 0.0f);
    CHECK(dry.bodyPeakSec >= 600.0f);
    CHECK(dry.bodyPeakSec <= 1200.0f);
    CHECK(WreckStateAt(dry, 60.0f, 3.1f).bodyFlameM < WreckStateAt(dry, dry.bodyPeakSec, 3.1f).bodyFlameM);
}

TEST_CASE("wrecks: trucks, tankers and armour burn by their class", "[firespread]")
{
    for (std::uint32_t seed = 1; seed < 100; ++seed)
    {
        WreckConfig c;
        c.cls = WreckClass::Truck;
        const WreckTimeline t = WreckTimelineOf(c, seed);
        // 4 to 7 m flames for 1 to 1.5 h
        CHECK(t.flameEndSec >= 3600.0f);
        CHECK(t.flameEndSec <= 5400.0f);
        for (float at = 1200.0f; at < 0.5f * t.flameEndSec; at += 300.0f)
        {
            const float L = WreckStateAt(t, at, 5.0f).bodyFlameM;
            CHECK(L >= 4.0f * 0.92f);
            CHECK(L <= 7.0f * 1.1f);
        }
        CHECK(t.emberK == 2.0f);

        c.cls = WreckClass::Tanker;
        c.fuelCargoL = 5000.0f;
        const WreckTimeline k = WreckTimelineOf(c, seed);
        // its tank opened: 20 to 25 m flames over a 100 m2 pool for 5 to 15 min, then as a truck
        CHECK(k.poolAreaM2 == Catch::Approx(100.0f).margin(0.5f));
        CHECK(k.poolFlameM >= 20.0f * 0.98f);
        CHECK(k.poolFlameM <= 25.0f * 1.02f);
        CHECK(k.poolSec >= 300.0f);
        CHECK(k.poolSec <= 900.0f);
        const WreckState ks = WreckStateAt(k, 60.0f, 5.0f);
        CHECK(ks.poolMW == Catch::Approx(240.0f).margin(1.0f));
        CHECK(ks.black == 1.0f);
        CHECK(WreckStateAt(k, k.poolSec + 1.0f, 5.0f).poolFlameM == 0.0f);
        CHECK(k.flameEndSec >= 3600.0f);

        c.cls = WreckClass::Armour;
        c.ammo = true;
        const WreckTimeline a = WreckTimelineOf(c, seed);
        // 1 to 3 h, the hull hot 6 to 12 h, propellant jets 5 to 20 m for 2 to 10 s from 2 to 30 min
        CHECK(a.flameEndSec >= 3600.0f);
        CHECK(a.flameEndSec <= 10800.0f);
        CHECK(a.hotEndSec >= 21600.0f);
        CHECK(a.hotEndSec <= 43200.0f);
        CHECK(a.jets >= 3);
        CHECK(a.jets <= 6);
        for (int i = 0; i < a.jets; ++i)
        {
            CHECK(a.jetM[i] >= 5.0f);
            CHECK(a.jetM[i] <= 20.0f);
            CHECK(a.jetSec[i] >= 2.0f);
            CHECK(a.jetSec[i] <= 10.0f);
            CHECK(a.jetAt[i] >= 120.0f);
            const WreckState js = WreckStateAt(a, a.jetAt[i] + 0.5f * a.jetSec[i], 4.0f);
            CHECK(js.jet == i);
            CHECK(js.jetM > 0.5f * a.jetM[i]);
        }
        const float steady = WreckStateAt(a, 0.4f * a.flameEndSec, 4.0f).bodyFlameM;
        CHECK(steady >= 3.0f * 0.9f);
        CHECK(steady <= 6.0f * 1.1f);
    }
    // the classes from config
    CHECK(WreckClassOf(10000.0f, false, 40.0f, 7.5f, 7000.0f) == WreckClass::Tanker);
    CHECK(WreckClassOf(0.0f, true, 60.0f, 6.0f, 12000.0f) == WreckClass::Armour);
    CHECK(WreckClassOf(0.0f, false, 150.0f, 5.5f, 7000.0f) == WreckClass::Armour);
    CHECK(WreckClassOf(0.0f, false, 40.0f, 7.4f, 6000.0f) == WreckClass::Truck);
    CHECK(WreckClassOf(0.0f, false, 30.0f, 4.0f, 1500.0f) == WreckClass::Car);
}

TEST_CASE("wrecks: embers at the build's rate only through the fuel phase", "[firespread]")
{
    for (std::uint32_t seed = 1; seed < 100; ++seed)
    {
        const WreckTimeline w = CarTimeline(seed);
        CHECK(w.emberFuelSec >= 120.0f);
        CHECK(w.emberFuelSec <= 180.0f);
        CHECK(WreckStateAt(w, 0.5f * w.emberFuelSec, 3.0f).emberRate == Catch::Approx(kEmberFuelRate));
        CHECK(WreckStateAt(w, w.emberFuelSec + 1.0f, 3.0f).emberRate == Catch::Approx(kEmberAfterRate));
        CHECK(WreckStateAt(w, w.flameEndSec + 1.0f, 3.0f).emberRate == 0.0f);
    }
    CHECK(kEmberFuelRate == Catch::Approx(0.07f));
    CHECK(kEmberAfterRate == Catch::Approx(0.005f));
    // each lights with the build's pig
    const WreckDay typ = WreckDayOf(kDayTypicalM1h);
    ObjectFire f;
    WreckBodyFire(CarTimeline(3), WreckStateAt(CarTimeline(3), 30.0f, 3.0f), typ, f);
    CHECK(f.emberChance == Catch::Approx(0.630f).margin(0.002f));
    CHECK(f.emberRate == Catch::Approx(kEmberFuelRate));
}

TEST_CASE("wrecks: their fireballs' size and reach", "[firespread]")
{
    // a car's 5 to 10 kg: 10 to 12 m across for about 1 s (CCPS: 5.8 M^(1/3), 0.45 M^(1/3))
    for (float u : {0.0f, 0.5f, 1.0f})
    {
        const float kg = WreckFireballKg(WreckClass::Car, 0.0f, u, 60.0f, 0.0f);
        CHECK(kg >= 5.0f);
        CHECK(kg <= 10.0f);
        const float d = 5.8f * std::cbrt(kg);
        CHECK(d >= 9.9f);
        CHECK(d <= 12.6f);
        CHECK(0.45f * std::cbrt(kg) == Catch::Approx(1.0f).margin(0.25f));
        // a tanker's 500 to 1000 kg: 46 to 58 m, about 4 s
        const float tk = WreckFireballKg(WreckClass::Tanker, 0.0f, u, 0.0f, 5000.0f);
        CHECK(5.8f * std::cbrt(tk) >= 45.9f);
        CHECK(5.8f * std::cbrt(tk) <= 58.1f);
    }
    CHECK(WreckFireballKg(WreckClass::Car, 0.0f, 0.5f, 0.0f, 0.0f) == 0.0f); // an empty tank
    // a car fireball over cured grass lights it about half the time on the typical day, nearly always on the worst
    for (int day = 0; day < 2; ++day)
    {
        const WreckDay d = WreckDayOf(day == 0 ? kDayTypicalM1h : kDayWorstM1h);
        int lit = 0;
        const int n = 4000;
        for (int i = 0; i < n; ++i)
        {
            ObjRand r(static_cast<std::uint32_t>(i) * 2654435761U + 7U);
            bool any = false;
            FireballContact(0.0f, 0.0f, 5.0f, d.fireballFull, r,
                            [&](float, float, float, float, ObjectIgniteKind)
                            {
                                any = true;
                                return true;
                            });
            lit += any ? 1 : 0;
        }
        const float share = static_cast<float>(lit) / static_cast<float>(n);
        CHECK(share == Catch::Approx(day == 0 ? 0.5f : 0.97f).margin(0.03f));
    }
    // its debris: 3 to 10 pieces, 5 to 30 m
    const WreckDay d = WreckDayOf(kDayTypicalM1h);
    ObjRand r(99U);
    ObjectFire body;
    for (int i = 0; i < 50; ++i)
    {
        int pieces = 0;
        WreckFireballDraws(
            CarTimeline(static_cast<std::uint32_t>(i) + 1U), d, body, 5.5f, r,
            [&](float x, float z, float, float, ObjectIgniteKind kind)
            {
                if (kind == ObjectIgniteKind::Debris)
                {
                    pieces++;
                    const float dist = std::sqrt(x * x + z * z);
                    CHECK(dist >= kDebrisMinM - 1e-3f);
                    CHECK(dist <= kDebrisMaxM + 1e-3f);
                }
                return true;
            },
            [&](float, float, float) {});
        CHECK(pieces >= kDebrisMin);
        CHECK(pieces <= kDebrisMax);
    }
    // a tanker's: everything out to 1.6 (typical) to 2 (worst) radii at once, fuel rain to 2 radii, 1 to 5 pieces 100
    // to 200 m
    WreckConfig tc;
    tc.cls = WreckClass::Tanker;
    tc.fuelCargoL = 5000.0f;
    const WreckTimeline tw = WreckTimelineOf(tc, 5U);
    for (int day = 0; day < 2; ++day)
    {
        const WreckDay dd = WreckDayOf(day == 0 ? kDayTypicalM1h : kDayWorstM1h);
        float ring = 0.0f;
        int rain = 0, frag = 0;
        WreckFireballDraws(
            tw, dd, body, 26.0f, r,
            [&](float x, float z, float, float, ObjectIgniteKind kind)
            {
                const float dist = std::sqrt(x * x + z * z);
                if (kind == ObjectIgniteKind::Rain)
                {
                    rain++;
                    CHECK(dist >= 26.0f - 1e-3f);
                    CHECK(dist <= 52.0f + 1e-3f);
                }
                if (kind == ObjectIgniteKind::Fragment)
                {
                    frag++;
                    CHECK(dist >= 100.0f - 1e-3f);
                    CHECK(dist <= 200.0f + 1e-3f);
                }
                return true;
            },
            [&](float, float, float rr) { ring = rr; });
        CHECK(ring == Catch::Approx((day == 0 ? 1.6f : 2.0f) * 26.0f).margin(0.01f));
        CHECK(rain == kRainDrops);
        CHECK(frag >= 1);
        CHECK(frag <= 5);
    }
}

TEST_CASE("wrecks: does the grass light (the build-to table, both days)", "[firespread]")
{
    const int n = 1500;
    // grass within 3 m of a wreck: 85 to 95% within 1 to 3 min (typical), for certain within seconds (worst)
    const Share nearTyp = Trials(1, kDayTypicalM1h, 4.5f, 180.0f, 180.0f, n, 1000U);
    CHECK(nearTyp.by >= 0.85f);
    CHECK(nearTyp.by <= 0.96f);
    const Share nearWorst = Trials(1, kDayWorstM1h, 12.5f, 60.0f, 30.0f, n, 2000U);
    CHECK(nearWorst.by >= 0.97f);
    CHECK(nearWorst.median <= 5.0f);
    // and the two days differ in how soon, not only how often: on the typical day a brand, the fireball's touch or
    // a spill smoulders before it flames (half a minute and more), on the worst it flames at once
    CHECK(nearTyp.median >= 20.0f);
    CHECK(nearTyp.median <= 90.0f);
    // the middle of an 8 m bare road: 20 to 40% within 2 to 15 min (typical), 80 to 95% within 10 to 60 s (worst)
    const Share roadTyp = Trials(0, kDayTypicalM1h, 4.5f, 2700.0f, 2700.0f, n, 3000U);
    CHECK(roadTyp.by >= 0.20f);
    CHECK(roadTyp.by <= 0.40f);
    CHECK(roadTyp.median >= 120.0f);
    CHECK(roadTyp.median <= 900.0f);
    const Share roadWorst60 = Trials(0, kDayWorstM1h, 12.5f, 300.0f, 60.0f, n, 4000U);
    const Share roadWorst300 = Trials(0, kDayWorstM1h, 12.5f, 300.0f, 300.0f, n, 4000U);
    CHECK(roadWorst60.by >= 0.75f);
    CHECK(roadWorst300.by <= 0.95f);
    CHECK(roadWorst300.median <= 60.0f);
}

TEST_CASE("object fires: the heat lights the nearest ground in reach", "[firespread]")
{
    // a thin strip of grass 1 m off one long side of a burning car, within its 2 m reach, at a 50 s mean
    ObjectFire f;
    f.halfL = 2.1f;
    f.halfW = 0.9f;
    f.heatReachM = 2.0f;
    f.heatMeanSec = 50.0f;
    ObjRand r(4242U);
    double sum = 0.0;
    int lit = 0;
    for (int trial = 0; trial < 400; ++trial)
    {
        float t = 0.0f, first = -1.0f;
        while (first < 0.0f && t < 2000.0f)
        {
            ObjectFireDraws(f, 0.5f, 0.0f, 0.0f, r,
                            [&](float, float z, float, float, ObjectIgniteKind)
                            {
                                if (z >= 1.9f && z <= 2.4f)
                                {
                                    first = t;
                                    return true;
                                }
                                return false;
                            });
            t += 0.5f;
        }
        if (first >= 0.0f)
        {
            sum += first;
            lit++;
        }
    }
    CHECK(lit >= 395);
    // the try finds the strip most times it fires: its mean within 1.5 times the heat's
    CHECK(sum / lit < 75.0);
    // nothing outside its reach
    f.heatReachM = 0.5f;
    bool far = false;
    for (int i = 0; i < 2000; ++i)
        ObjectFireDraws(f, 0.5f, 0.0f, 0.0f, r,
                        [&](float, float z, float, float, ObjectIgniteKind)
                        {
                            far = far || z > 1.41f;
                            return false;
                        });
    CHECK_FALSE(far);
}

TEST_CASE("object fires: wreck and building smoke is black", "[firespread]")
{
    const PlumeRgb soot = PlumeAlbedo(1.0f, 0.0f, 1.0f);
    CHECK(soot.r == Catch::Approx(kPlumeSootRgb.r).margin(1e-4f));
    const PlumeRgb grass = PlumeAlbedo(0.0f, 0.0f);
    const PlumeRgb woody = PlumeAlbedo(1.0f, 2000.0f);
    const PlumeRgb black2k = PlumeAlbedo(1.0f, 2000.0f, 1.0f);
    CHECK(black2k.r < woody.r);
    CHECK(black2k.r < 0.5f);
    CHECK(grass.r > 0.7f);
    // the stock's albedo is unchanged with no black share
    CHECK(PlumeAlbedo(0.3f, 500.0f, 0.0f).r == Catch::Approx(PlumeAlbedo(0.3f, 500.0f).r));
}

TEST_CASE("wrecks: the fireball drawn through the flame pass", "[firespread]")
{
    // a car's 5.7 m ball lasts about 0.9 s (CCPS 0.45 M^(1/3)), a tanker's 26 m about 4 s
    for (float R : {5.7f, 26.0f})
    {
        const float T = kBallSecPerM * R;
        CHECK(FireballAt(R, 0.05f * T).on);
        CHECK_FALSE(FireballAt(R, 1.01f * T).on);
        CHECK(FireballAt(R, 0.0f).radiusM == Catch::Approx(kBallStartShare * R));
        CHECK(FireballAt(R, 0.5f * T).radiusM == Catch::Approx(R));
        CHECK(FireballAt(R, 0.5f * T).heat == Catch::Approx(1.0f));
        CHECK(FireballAt(R, 0.95f * T).heat < 0.1f);
        CHECK(FireballAt(R, 0.9f * T).riseM > FireballAt(R, 0.1f * T).riseM);
        CHECK(FireballAt(R, 0.9f * T).soot > FireballAt(R, 0.1f * T).soot);
    }
    CHECK(kBallSecPerM * 5.7f == Catch::Approx(0.88f).margin(0.05f));
    CHECK(kBallSecPerM * 26.0f == Catch::Approx(4.0f).margin(0.1f));
}

TEST_CASE("wrecks: the game's 3000 l fuel truck spills a full pool", "[firespread]")
{
    // Truck5tRefuel carries 3000 l: what its fireball does not take (300 to 600 kg) pours out over 100 m2
    for (std::uint32_t seed = 1; seed < 40; ++seed)
    {
        WreckConfig c;
        c.cls = WreckClass::Tanker;
        c.fuelCargoL = 3000.0f;
        const WreckTimeline k = WreckTimelineOf(c, seed);
        CHECK(k.poolAreaM2 == Catch::Approx(100.0f).margin(0.5f));
        CHECK(k.poolFlameM >= 20.0f * 0.98f);
        CHECK(k.poolFlameM <= 25.0f * 1.02f);
        CHECK(k.poolSec >= 300.0f);
        CHECK(k.poolSec <= 900.0f);
        const float kg = WreckFireballKg(WreckClass::Tanker, 0.0f, Hash01Test(seed), 0.0f, 3000.0f);
        CHECK(5.8f * std::cbrt(kg) >= 38.0f);
        CHECK(5.8f * std::cbrt(kg) <= 50.0f);
    }
}
