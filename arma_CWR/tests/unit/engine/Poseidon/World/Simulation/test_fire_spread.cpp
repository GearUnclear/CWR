#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Simulation/FireBuildingShell.hpp>
#include <Poseidon/World/Simulation/FirePlume.hpp>
#include <Poseidon/World/Simulation/FireSpread.hpp>
#include <Poseidon/World/Simulation/FireShell.hpp>
#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

// FIRE-SPREAD (explore lane): the wildland fire core. The surface physics is
// checked against Rothermel's equations worked by hand for Anderson's fuel
// model 1 (short grass) at 6% fine fuel moisture: reaction intensity about
// 825 Btu/ft2/min, no-wind rate about 4.6 ft/min, and about 104 ft/min
// (0.53 m/s, 94 ch/h) in a 5 mi/h midflame wind. The grid is checked for the
// shape a fire takes in wind, for bare ground and wet fuel stopping it, and
// for its caps.

using namespace Poseidon::firespread;
using Catch::Approx;

namespace
{
Moisture Dry()
{
    Moisture m;
    m.m1h = 0.06f;
    m.m10h = 0.07f;
    m.m100h = 0.08f;
    m.mLive = 1.0f;
    return m;
}
constexpr float kMph5 = 2.2352f; // 5 mi/h in m/s
} // namespace

TEST_CASE("Rothermel surface fire for fuel model 1 matches the hand worked numbers", "[firespread]")
{
    const SurfaceFire s = SolveSurface(1, Dry());
    REQUIRE(s.burns);
    CHECK(s.irBtu == Approx(825.0f).epsilon(0.05));
    CHECK(s.r0Ftmin == Approx(4.6f).epsilon(0.06));
    CHECK(s.residenceSec == Approx(60.0f * 384.0f / 3500.0f).epsilon(0.01));
    const HeadFire h = SolveHead(s, kMph5, 0.0f, 0.0f, 0.0f);
    CHECK(h.rateMps == Approx(0.527f).epsilon(0.08));
    CHECK(h.dirX == Approx(1.0f));
    // Alexander (1985) on the 10 m wind (here the midflame wind itself, a WAF of 1).
    const float u = kMph5 * 3.6f;
    CHECK(h.lengthToBreadth == Approx(1.0f + 8.729f * std::pow(1.0f - std::exp(-0.030f * u), 2.155f)).epsilon(0.01));
}

TEST_CASE("fuel at or past its moisture of extinction does not burn", "[firespread]")
{
    Moisture m = Dry();
    m.m1h = 0.12f; // fuel model 1 extinction moisture
    m.m10h = 0.13f;
    m.m100h = 0.14f;
    CHECK_FALSE(SolveSurface(1, m).burns);
    CHECK(SolveSurface(3, m).burns); // tall grass goes out at 25%
    CHECK_FALSE(SolveSurface(0, m).burns);
}

TEST_CASE("wind and slope speed the head fire, backing fire is slow", "[firespread]")
{
    for (int fm : {1, 2, 3, 4, 5, 8, 9, 10})
    {
        const SurfaceFire s = SolveSurface(fm, Dry());
        INFO("fuel model " << fm);
        REQUIRE(s.burns);
        const float r0 = SolveHead(s, 0.0f, 0.0f, 0.0f, 0.0f).rateMps;
        const float r2 = SolveHead(s, 2.0f, 0.0f, 0.0f, 0.0f).rateMps;
        const float r4 = SolveHead(s, 4.0f, 0.0f, 0.0f, 0.0f).rateMps;
        CHECK(r2 > r0);
        CHECK(r4 > r2);
        const float up = SolveHead(s, 0.0f, 0.0f, 0.3f, 0.0f).rateMps;
        CHECK(up > 2.0f * r0);
        const HeadFire h = SolveHead(s, 4.0f, 0.0f, 0.0f, 0.0f);
        const float back = RateToward(h, -1.0f, 0.0f);
        const float flank = RateToward(h, 0.0f, 1.0f);
        CHECK(back < flank);
        CHECK(flank < h.rateMps);
        CHECK(RateToward(h, 1.0f, 0.0f) == Approx(h.rateMps));
    }
}

TEST_CASE("slope alone points the head upslope", "[firespread]")
{
    const SurfaceFire s = SolveSurface(1, Dry());
    const HeadFire h = SolveHead(s, 0.0f, 0.0f, 0.0f, 0.25f);
    CHECK(h.dirZ == Approx(1.0f).margin(1e-4));
    // phi_s = 5.275 beta^-0.3 tan^2: about 3.7 at 30% for fuel model 1.
    const float r0 = SolveHead(s, 0.0f, 0.0f, 0.0f, 0.0f).rateMps;
    const float r30 = SolveHead(s, 0.0f, 0.0f, 0.3f, 0.0f).rateMps;
    CHECK(r30 / r0 == Approx(4.7f).epsilon(0.08));
}

TEST_CASE("flame length, torching and length to breadth follow the published forms", "[firespread]")
{
    CHECK(FlameLengthM(1000.0f) == Approx(0.0775f * std::pow(1000.0f, 0.46f)));
    CHECK(FlameLengthM(1000.0f) == Approx(1.86f).epsilon(0.01));
    CHECK(TorchingIntensityKwM(4.0f, 1.0f) == Approx(1347.0f).epsilon(0.01));
    // FBP eq. 79 (forest, shrub): 1 at no wind, about 5.0 at 40 km/h (11 m/s),
    // below 1 + 8.729 however strong. FBP grass: 1.1 U^0.464, 6.06 at 40 km/h, at most 8.
    CHECK(LengthToBreadth(0.0f) == Approx(1.0f));
    CHECK(LengthToBreadth(11.0f) == Approx(4.99f).epsilon(0.01));
    CHECK(LengthToBreadth(100.0f) < 9.73f);
    CHECK(LengthToBreadth(100.0f) > LengthToBreadth(11.0f));
    CHECK(LengthToBreadth(0.0f, true) == Approx(1.0f));
    CHECK(LengthToBreadth(11.0f, true) == Approx(6.06f).epsilon(0.01));
    CHECK(LengthToBreadth(100.0f, true) == Approx(kGrassMaxLB));
    // The grass head never runs past kGrassMaxRateMps.
    CHECK(SolveHead(SolveSurface(3, Dry()), 30.0f, 0.0f, 0.0f, 0.0f, 1.0f, true).rateMps <= kGrassMaxRateMps);
    // Torching trees: one alone flames to kTorchRatio tree heights, more together higher (N^0.4), at most the stand's.
    CHECK(TorchRatio(1) == Approx(1.5f));
    CHECK(TorchRatio(3) == Approx(1.5f * std::pow(3.0f, 0.4f)));
    CHECK(TorchRatio(20) == Approx(kTorchRatioMax));
}

namespace
{
struct Extent
{
    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    int burnt = 0;
};
Extent Burned(const FireGrid& g)
{
    Extent e;
    std::vector<BurningView> b;
    g.CollectBurning(b);
    std::vector<std::pair<float, float>> d;
    g.CollectBurnt(d);
    for (const auto& v : b)
        d.emplace_back(v.x, v.z);
    for (const auto& p : d)
    {
        e.minX = std::min(e.minX, p.first);
        e.maxX = std::max(e.maxX, p.first);
        e.minZ = std::min(e.minZ, p.second);
        e.maxZ = std::max(e.maxZ, p.second);
    }
    e.burnt = static_cast<int>(d.size());
    return e;
}
} // namespace

TEST_CASE("a grass fire in wind runs downwind as an ellipse at about the head rate", "[firespread]")
{
    Params p;
    p.moisture = Dry();
    p.spotting = false;
    p.buildUpPerMin = 0.0f; // the equilibrium shape
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.Ignite(4.0f, 4.0f));
    const float wind = 5.0f; // 10 m wind, east
    const float seconds = 180.0f;
    for (int k = 0; k < static_cast<int>(seconds / 0.05f); ++k)
        g.Tick(0.05f, wind, 0.0f);
    const Extent e = Burned(g);
    const float head = e.maxX - 4.0f, back = 4.0f - e.minX, side = 0.5f * (e.maxZ - e.minZ);
    INFO("head " << head << " back " << back << " side " << side << " cells " << e.burnt);
    // Expected head rate: fuel model 1, midflame 0.36 x 5 m/s.
    const float rate = SolveHead(SolveSurface(1, Dry()), 0.36f * wind, 0.0f, 0.0f, 0.0f).rateMps;
    CHECK(head > 0.6f * rate * seconds);
    CHECK(head < 1.4f * rate * seconds + 16.0f);
    CHECK(head > 3.0f * back);
    CHECK(side < head);
    // (grass at 5 m/s: length to breadth 4.2, the flanks 0.01 m/s, under a
    // cell in 3 minutes; never narrower than the back)
    CHECK(side >= back);
}

TEST_CASE("bare ground and wet fuel stop a fire, caps hold", "[firespread]")
{
    Params p;
    p.moisture = Dry();
    // Grass west of x = 40, bare beyond (a road or a ploughed strip).
    FireGrid g(
        p,
        [](float x, float)
        {
            Fuel f;
            f.model = x < 40.0f ? 1 : 0;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.Ignite(4.0f, 4.0f));
    for (int k = 0; k < 4000; ++k)
        g.Tick(0.05f, 6.0f, 0.0f);
    CHECK(Burned(g).maxX < 40.0f);
    CHECK_FALSE(g.Ignite(100.0f, 0.0f));

    Params wet;
    wet.moisture = Dry();
    wet.moisture.m1h = 0.30f;
    FireGrid w(
        wet,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    CHECK_FALSE(w.Ignite(0.0f, 0.0f));

    Params cap;
    cap.moisture = Dry();
    cap.maxFront = 50;
    FireGrid c(
        cap,
        [](float, float)
        {
            Fuel f;
            f.model = 3;
            return f;
        },
        [](float, float) { return 0.0f; });
    c.IgniteArea(0.0f, 0.0f, 200.0f);
    CHECK(c.GetStats().front <= 50);
    for (int k = 0; k < 400; ++k)
        c.Tick(0.05f, 8.0f, 0.0f);
    CHECK(c.GetStats().front <= 50);
    CHECK(c.GetStats().deferred > 0);
}

TEST_CASE("a long frame never runs more than the step cap", "[firespread]")
{
    Params p;
    p.moisture = Dry();
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    g.Ignite(0.0f, 0.0f);
    g.Tick(10.0f, 3.0f, 0.0f);
    CHECK(g.GetStats().steps == p.maxStepsPerTick);
}

TEST_CASE("the front sweeps across a cell: the parts light in turn and never jump", "[firespread]")
{
    BurningView v;
    v.crossSec = 8.0f;
    v.life = 20.0f;
    v.frontX = 1.0f; // the front comes from the west
    const float cell = 8.0f;
    // Early on the western edge flames, the eastern one not yet.
    v.age = 2.0f;
    CHECK(PartStrength(v, -3.5f, 0.0f, cell) > 0.3f);
    CHECK(PartStrength(v, 3.5f, 0.0f, cell) == 0.0f);
    // Across the front's way the parts are alike.
    CHECK(PartStrength(v, -3.5f, -3.0f, cell) == Approx(PartStrength(v, -3.5f, 3.0f, cell)));
    // Late on the west has died down while the east still burns.
    v.age = 19.0f;
    CHECK(PartStrength(v, -3.5f, 0.0f, cell) == 0.0f);
    CHECK(PartStrength(v, 3.5f, 0.0f, cell) > 0.1f);
    // A frame's step never changes a part by much (no popping).
    float worst = 0.0f;
    for (float x = -4.0f; x <= 4.0f; x += 0.5f)
    {
        float prev = 0.0f;
        for (float a = -1.0f; a < 22.0f; a += 1.0f / 60.0f)
        {
            v.age = a;
            const float s = PartStrength(v, x, 0.0f, cell);
            worst = std::max(worst, std::fabs(s - prev));
            prev = s;
        }
    }
    CHECK(worst < 0.05f);
    // An ignition (no front) lights the whole cell at once.
    v.crossSec = 0.0f;
    v.frontX = 0.0f;
    v.age = 1.0f;
    CHECK(PartStrength(v, -3.5f, 0.0f, cell) == Approx(PartStrength(v, 3.5f, 0.0f, cell)));
}

TEST_CASE("the views move smoothly between the simulation's steps", "[firespread]")
{
    Params p;
    p.moisture = Dry();
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.Ignite(4.0f, 4.0f));
    // Frames of 1/60 s: the display clock advances by the frame, and no
    // cell's strength jumps from one frame to the next.
    std::vector<BurningView> a, b;
    double prevClock = g.DisplayTime();
    float worstStrength = 0.0f, worstClock = 0.0f;
    bool sawFront = false;
    for (int k = 0; k < 60 * 90; ++k)
    {
        g.CollectBurning(a);
        g.Tick(1.0f / 60.0f, 5.0f, 0.0f);
        g.CollectBurning(b);
        worstClock = std::max(worstClock, static_cast<float>(std::fabs(g.DisplayTime() - prevClock - 1.0 / 60.0)));
        prevClock = g.DisplayTime();
        for (const BurningView& vb : b)
        {
            sawFront = sawFront || vb.crossSec > 0.0f;
            for (const BurningView& va : a)
                if (va.x == vb.x && va.z == vb.z)
                    worstStrength = std::max(worstStrength, std::fabs(vb.strength - va.strength));
        }
    }
    CHECK(sawFront);
    CHECK(worstClock < 1e-3f);
    CHECK(worstStrength < 0.05f);
}

TEST_CASE("the flames' shape follows the fire model: tilt, strip and torching", "[firespread]")
{
    // Tilt from the flame Froude number: no wind stands up, more wind leans
    // over, a taller flame stands straighter, never past the cap.
    CHECK(FlameTiltRad(0.0f, 2.0f) == Approx(0.0f));
    CHECK(FlameTiltRad(4.0f, 2.6f) == Approx(std::atan(1.4f * 4.0f / std::sqrt(9.81f * 2.6f))));
    CHECK(FlameTiltRad(4.0f, 2.6f) > FlameTiltRad(2.0f, 2.6f));
    CHECK(FlameTiltRad(4.0f, 20.0f) < FlameTiltRad(4.0f, 2.6f));
    CHECK(FlameTiltRad(100.0f, 0.1f) <= kFlameTiltMax);

    // A point flames for the residence time after the front reaches it, its
    // flame tallest at the strip's leading edge.
    BurningView v;
    v.residenceSec = 6.6f;
    v.crossSec = 4.0f;
    v.frontX = 1.0f;
    v.life = 11.0f;
    const float cell = 8.0f;
    v.age = 0.2f;
    CHECK(SurfacePart(v, 3.5f, 0.0f, cell).heat == 0.0f); // the far edge: not reached yet
    float prev = 2.0f;
    for (float a = 1.0f; a < 6.6f; a += 0.5f)
    {
        v.age = a;
        const PartFlame f = SurfacePart(v, -4.0f, 0.0f, cell);
        CHECK(f.length > 0.0f);
        CHECK(f.length <= 1.0f);
        CHECK(f.length < prev);
        prev = f.length;
    }
    v.age = 6.7f;
    CHECK(SurfacePart(v, -4.0f, 0.0f, cell).heat == 0.0f); // burnt out
    CHECK(SurfacePart(v, 4.0f, 0.0f, cell).heat > 0.0f);   // the far edge still flames

    // A torching crown: nothing before the delay or after its time, full in between.
    CHECK(TorchEnvelope(0.0f) == 0.0f);
    CHECK(TorchEnvelope(kTorchDelaySec + kTorchSec + 0.1f) == 0.0f);
    CHECK(TorchEnvelope(kTorchDelaySec + kTorchRiseSec + 1.0f) == Approx(1.0f));
}

TEST_CASE("a lone torching tree flames to kTorchRatio times its height, the grass keeps Byram's", "[firespread]")
{
    Params p;
    p.moisture = Dry();
    p.moisture.m1h = 0.04f;
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    FireGrid g(
        p,
        [](float x, float z)
        {
            Fuel f;
            f.model = 1;
            if (x > 40.0f && x < 48.0f && z > 0.0f && z < 8.0f)
            {
                f.canopy = true;
                f.canopyBaseM = 1.5f;
                f.treeHeightM = 9.0f;
                f.treeOffX = 1.0f;
            }
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.Ignite(4.0f, 4.0f));
    float torchPeak = 0.0f, grassMax = 0.0f, treeH = 0.0f;
    std::vector<BurningView> b;
    for (int k = 0; k < 60 * 120; ++k)
    {
        g.Tick(1.0f / 60.0f, 11.0f, 0.0f);
        g.CollectBurning(b);
        for (const BurningView& v : b)
        {
            if (v.torching)
            {
                torchPeak = std::max(torchPeak, v.torchM);
                treeH = v.treeH;
            }
            grassMax = std::max(grassMax, v.flameMaxM);
        }
    }
    // The debug view of a cell there (fireSpreadCell) tells its tree.
    BurningView at;
    CellState st = CellState::Unknown;
    REQUIRE(g.ViewAt(44.0f, 4.0f, at, &st));
    INFO("state " << static_cast<int>(st) << " ext " << g.GetStats().extMaxX << " ign " << g.GetStats().ignitions);
    CHECK(g.GetStats().torching > 0);
    CHECK(treeH == Approx(9.0f));
    CHECK(at.treeH == Approx(9.0f));
    CHECK(st != CellState::Unknown);
    // A lone tree: its flame stands on the crown (kTorchFootShare up) and reaches kTorchRatio tree heights.
    CHECK(torchPeak == Approx((kTorchRatio - kTorchFootShare) * 9.0f).epsilon(0.02));
    // Short grass at 4% in an 11 m/s wind: Byram's 0.5 to 3 m, never a crown's.
    CHECK(grassMax > 0.5f);
    CHECK(grassMax < 3.5f);
}

// ---- The realism audit's spread fixes (2026-11-16) ----

namespace
{
// Short grass everywhere at 4%, with fuelAt deciding the trees.
Params Worst()
{
    Params p;
    p.moisture = Dry();
    p.moisture.m1h = 0.04f;
    p.moisture.m10h = 0.05f;
    p.moisture.m100h = 0.06f;
    p.moisture.mLive = 0.9f;
    return p;
}
} // namespace

TEST_CASE("embers come only from torching conifers standing in their cell", "[firespread]")
{
    // A canopy over every cell (trees within 9 m) but no tree standing in
    // any: nothing torches, nothing spots.
    {
        Params p = Worst();
        p.buildUpPerMin = 0.0f;
        FireGrid g(
            p,
            [](float, float)
            {
                Fuel f;
                f.model = 1;
                f.canopy = true;
                f.canopyBaseM = 1.5f;
                return f;
            },
            [](float, float) { return 0.0f; });
        REQUIRE(g.IgniteLine(-60.0f, 0.0f, 60.0f, 0.0f) > 0);
        for (int k = 0; k < 4 * 60; ++k)
            g.Tick(0.25f, 0.0f, 11.0f);
        CHECK(g.GetStats().torching == 0);
        CHECK(g.GetStats().embers == 0);
        CHECK(g.GetStats().spots == 0);
    }
    // Trees standing in one cell in five: conifers torch and throw a few
    // embers each (about kSpotEmbersPerTree), broadleaves never.
    for (bool conifer : {true, false})
    {
        INFO("conifer " << conifer);
        Params p = Worst();
        p.buildUpPerMin = 0.0f;
        FireGrid g(
            p,
            [conifer](float x, float z)
            {
                Fuel f;
                f.model = 1;
                const int i = static_cast<int>(std::floor(x / 8.0f)), j = static_cast<int>(std::floor(z / 8.0f));
                if (((i * 7 + j * 3) % 5 + 5) % 5 == 0)
                {
                    f.canopy = true;
                    f.canopyBaseM = 1.5f;
                    f.treeHeightM = 12.0f;
                    f.conifer = conifer;
                }
                return f;
            },
            [](float, float) { return 0.0f; });
        REQUIRE(g.IgniteLine(-200.0f, 0.0f, 200.0f, 0.0f) > 0);
        for (int k = 0; k < 4 * 60; ++k)
            g.Tick(0.25f, 0.0f, 11.0f);
        const Stats& s = g.GetStats();
        INFO("torching " << s.torching << " embers " << s.embers << " spots " << s.spots << " spotmax " << s.spotMaxM);
        if (conifer)
        {
            CHECK(s.torching > 20);
            CHECK(s.embers > 0);
            // About half an ember a tree over its torch, never more than one a tree on average.
            CHECK(static_cast<float>(s.embers) < 1.0f * static_cast<float>(s.torching));
            CHECK(static_cast<float>(s.embers) > 0.2f * static_cast<float>(s.torching));
            CHECK(s.spotMaxM <= AlbiniTorchSpotMaxM(12.0f, 11.0f) + 1e-3f);
        }
        else
        {
            CHECK(s.torching == 0);
            CHECK(s.embers == 0);
        }
    }
}

TEST_CASE("a fire from a point builds up to its equilibrium rate, a long line starts at it", "[firespread]")
{
    // The FBP form: 1 - exp(-a t), t in minutes.
    CHECK(BuildUp(0.0f, 0.115f) == Approx(0.0f));
    CHECK(BuildUp(60.0f, 0.115f) == Approx(1.0f - std::exp(-0.115f)).epsilon(1e-4));
    CHECK(BuildUp(8.7f * 60.0f, 0.115f) == Approx(0.632f).epsilon(0.01));
    CHECK(BuildUp(5.0f, 0.0f) == Approx(1.0f));
    // The run is the rate's integral (checked by summing it).
    for (float age0 : {0.0f, 30.0f, 400.0f})
    {
        double sum = 0.0;
        for (int k = 0; k < 9000; ++k)
            sum += BuildUp(age0 + (k + 0.5f) * 0.01f, kBuildUpPerMin) * 0.01;
        CHECK(BuildUpRun(age0, 90.0f, kBuildUpPerMin) == Approx(static_cast<float>(sum)).epsilon(1e-3));
    }
    CHECK(BuildUpRun(0.0f, 60.0f, 0.0f) == Approx(60.0f));

    // A point ignition and a 160 m line in the same grass and wind (no
    // spotting): after 90 s the line's head has run about its equilibrium
    // rate's distance, the point's about the build-up's share of it.
    auto headAfter = [](bool line)
    {
        Params p = Worst();
        p.spotting = false;
        p.fuelNoise = 0.0f;
        FireGrid g(
            p,
            [](float, float)
            {
                Fuel f;
                f.model = 1;
                return f;
            },
            [](float, float) { return 0.0f; });
        if (line)
            g.IgniteLine(-80.0f, 4.0f, 80.0f, 4.0f);
        else
            g.Ignite(4.0f, 4.0f);
        for (int k = 0; k < 4 * 90; ++k)
            g.Tick(0.25f, 0.0f, 11.0f);
        // (the drawn front: a cell catches as the front enters its square)
        float head = 0.0f;
        for (float z = 4.0f; z < 400.0f; z += 0.25f)
            if (g.DrawnAgeAt(4.0f, z) >= 0.0f)
                head = z - 4.0f;
        return head;
    };
    const float rate = SolveHead(SolveSurface(1, Worst().moisture), 0.0f, 0.36f * 11.0f, 0.0f, 0.0f, 0.36f).rateMps;
    const float line = headAfter(true), point = headAfter(false);
    INFO("rate " << rate << " line " << line << " point " << point);
    CHECK(line == Approx(rate * 90.0f).margin(12.0f));
    CHECK(point == Approx(rate * BuildUpRun(0.0f, 90.0f, kBuildUpPerMin)).margin(12.0f));
    CHECK(point < 0.5f * line);
}

TEST_CASE("spot distances: lognormal throws under Albini's cap for a torching tree", "[firespread]")
{
    // Albini (1979): a few hundred metres to about a kilometre for trees of
    // 10 to 15 m in 30 to 40 km/h winds; more with wind and height; none in calm.
    const float cap = AlbiniTorchSpotMaxM(12.0f, 11.0f);
    INFO("cap " << cap);
    CHECK(cap > 300.0f);
    CHECK(cap < 1000.0f);
    CHECK(AlbiniTorchSpotMaxM(12.0f, 5.0f) < cap);
    CHECK(AlbiniTorchSpotMaxM(18.0f, 11.0f) > cap);
    CHECK(AlbiniTorchSpotMaxM(12.0f, 0.0f) == 0.0f);
    // Most throws are short: the median kSpotMedianShare of the cap, nine in
    // ten under twice that, none past the cap.
    std::vector<float> d;
    std::uint32_t r = 12345u;
    auto uni = [&r]()
    {
        r = r * 1664525u + 1013904223u;
        return (static_cast<float>(r >> 8) + 1.0f) / static_cast<float>(1u << 24);
    };
    for (int k = 0; k < 20000; ++k)
        d.push_back(SpotDistanceM(cap, uni(), uni()));
    std::sort(d.begin(), d.end());
    CHECK(d[d.size() / 2] == Approx(kSpotMedianShare * cap).epsilon(0.05));
    CHECK(d[d.size() * 9 / 10] < 3.0f * kSpotMedianShare * cap);
    CHECK(d.back() <= cap);
}

namespace
{
// The drawn burn (DrawnAgeAt at least 0) sampled every stepM over a window:
// its area (m2) and extent (m).
struct Drawn
{
    double area = 0.0;
    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
};
Drawn DrawnBurn(const FireGrid& g, float x0, float z0, float x1, float z1, float stepM)
{
    Drawn d;
    for (float z = z0; z <= z1; z += stepM)
        for (float x = x0; x <= x1; x += stepM)
            if (g.DrawnAgeAt(x, z) >= 0.0f)
            {
                d.area += stepM * stepM;
                d.minX = std::min(d.minX, x);
                d.maxX = std::max(d.maxX, x);
                d.minZ = std::min(d.minZ, z);
                d.maxZ = std::max(d.maxZ, z);
            }
    return d;
}
} // namespace

TEST_CASE("the front is Huygens on the model's own ellipse, not a lattice (the audit's C3)", "[firespread]")
{
    // The compare scene's ten seed cells (three blobs 30 m apart), short
    // grass at 4%, an 11 m/s wind north, no build-up, no spotting, no fuel
    // noise: at 61 s the drawn burn is the union of the model's ellipses from
    // the seed points (the rear-focus ellipse at the FBP grass length to
    // breadth, 6.06): its area within 6%, its head and its widest point
    // within 3 m. (The 16-neighbour lattice ran its flanks at a third of the
    // ellipse's divergence and had 94 cells here.)
    Params p = Worst();
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    int seeds = 0;
    for (float x : {4770.0f, 4800.0f, 4830.0f})
        seeds += g.IgniteArea(x, 9990.0f, 6.0f);
    REQUIRE(seeds == 10);
    std::vector<BurningView> b;
    g.CollectBurning(b);
    std::vector<std::pair<float, float>> pts;
    for (const BurningView& v : b)
        pts.emplace_back(v.x, v.z);
    REQUIRE(pts.size() == 10);
    for (int k = 0; k < 4 * 61; ++k)
        g.Tick(0.25f, 0.0f, 11.0f);
    const float t = static_cast<float>(g.DisplayTime());
    const HeadFire h = SolveHead(SolveSurface(1, Worst().moisture), 0.0f, 0.36f * 11.0f, 0.0f, 0.0f, 0.36f, true);
    const Drawn d = DrawnBurn(g, 4700.0f, 9940.0f, 4900.0f, 10160.0f, 0.5f);
    Drawn e;
    for (float z = 9940.0f; z <= 10160.0f; z += 0.5f)
        for (float x = 4700.0f; x <= 4900.0f; x += 0.5f)
        {
            float best = 1e9f;
            for (const auto& s : pts)
                best = std::min(best, Gauge(x - s.first, z - s.second, h.rateMps, h.dirX, h.dirZ, h.eccentricity));
            if (best <= t)
            {
                e.area += 0.25;
                e.minX = std::min(e.minX, x);
                e.maxX = std::max(e.maxX, x);
                e.maxZ = std::max(e.maxZ, z);
            }
        }
    INFO("drawn area " << d.area << " x " << d.minX << ".." << d.maxX << " head " << d.maxZ << "; exact " << e.area
                       << " x " << e.minX << ".." << e.maxX << " head " << e.maxZ << " (ignitions "
                       << g.GetStats().ignitions << ")");
    CHECK(d.area == Approx(e.area).epsilon(0.06));
    CHECK(d.maxZ == Approx(e.maxZ).margin(3.0f));
    CHECK(d.minX == Approx(e.minX).margin(3.0f));
    CHECK(d.maxX == Approx(e.maxX).margin(3.0f));
}

TEST_CASE("a young grass fire is a fat teardrop growing into a head (the consult's look targets)", "[firespread]")
{
    // A point in short grass at 4%, an 11 m/s wind north, no spotting, the
    // game's build-up: the fire science consult's targets (2026-11-16) for
    // 1 and 2 minutes: 6 to 20 m and 20 to 60 m downwind, 4 to 8 m and 8 to
    // 20 m wide; the drawn burn measured (a metre's slack for its edge noise).
    Params p = Worst();
    p.spotting = false;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.Ignite(4.0f, 4.0f));
    float len[3] = {}, wide[3] = {};
    const int at[3] = {4 * 20, 4 * 60, 4 * 120};
    for (int k = 1, m = 0; k <= 4 * 120; ++k)
    {
        g.Tick(0.25f, 0.0f, 11.0f);
        if (k == at[m])
        {
            const Drawn d = DrawnBurn(g, -60.0f, -30.0f, 70.0f, 120.0f, 0.25f);
            len[m] = d.maxZ - d.minZ;
            wide[m] = d.maxX - d.minX;
            ++m;
        }
    }
    INFO("20 s " << len[0] << " x " << wide[0] << ", 1 min " << len[1] << " x " << wide[1] << ", 2 min " << len[2]
                 << " x " << wide[2]);
    // At 20 s it is still round (a young fire's length to breadth grows from 1 with its build-up).
    CHECK(wide[0] > 0.6f * len[0]);
    CHECK(len[1] >= 6.0f);
    CHECK(len[1] <= 21.0f);
    CHECK(wide[1] >= 3.0f);
    CHECK(wide[1] <= 10.0f);
    CHECK(len[2] >= 20.0f);
    CHECK(len[2] <= 66.0f);
    CHECK(wide[2] >= 8.0f);
    CHECK(wide[2] <= 22.0f);
    CHECK(len[2] > 2.5f * wide[2]);
}

TEST_CASE("an olive torches only in a strong surface fire, a broadleaf never", "[firespread]")
{
    // Olives in every cell under a mild fire (5 m/s, 6%: about 350 kW/m) never torch.
    Params p;
    p.moisture = Dry();
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            f.canopy = true;
            f.canopyBaseM = 1.0f;
            f.treeHeightM = 6.0f;
            f.conifer = false;
            f.olive = true;
            return f;
        },
        [](float, float) { return 0.0f; });
    g.IgniteLine(-60.0f, 4.0f, 60.0f, 4.0f);
    for (int k = 0; k < 4 * 90; ++k)
        g.Tick(0.25f, 0.0f, 5.0f);
    CHECK(g.GetStats().torching == 0);
    // In a strong one (11 m/s, 4%: about 2000 kW/m) about half of them do.
    Params q = Worst();
    q.spotting = false;
    q.buildUpPerMin = 0.0f;
    FireGrid h(
        q,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            f.canopy = true;
            f.canopyBaseM = 1.0f;
            f.treeHeightM = 6.0f;
            f.conifer = false;
            f.olive = true;
            return f;
        },
        [](float, float) { return 0.0f; });
    h.IgniteLine(-60.0f, 4.0f, 60.0f, 4.0f);
    for (int k = 0; k < 4 * 40; ++k)
        h.Tick(0.25f, 0.0f, 11.0f);
    const Stats& s = h.GetStats();
    INFO("torching " << s.torching << " of " << s.ignitions);
    CHECK(s.torching > 0.25f * s.ignitions);
    CHECK(s.torching < 0.75f * s.ignitions);
}

TEST_CASE("flames keep the tempo of their own size (the consult's licking speeds)", "[firespread]")
{
    // Visible features climb at 1.5 sqrt(L) m/s, a flame licks at 2.3 / sqrt(L) Hz
    // (the wildfire consult, 2026-11-17): a 1 m grass flame about 2.3 times a
    // second, a 20 m crown flame about once in 2 s.
    CHECK(FeatureRiseMps(1.0f) == Approx(1.5f));
    CHECK(FeatureRiseMps(20.0f) == Approx(6.7f).margin(0.05f));
    CHECK(LickHz(1.0f) == Approx(2.3f));
    CHECK(LickHz(12.0f) == Approx(0.66f).margin(0.01f));
    CHECK(LickHz(20.0f) == Approx(0.51f).margin(0.01f));
    // A wind quickens it a little, never more than 1.3 times, never slows it.
    CHECK(LickWindFactor(0.0f) == Approx(1.0f));
    CHECK(LickWindFactor(4.4f) == Approx(1.17f).margin(0.01f));
    CHECK(LickWindFactor(100.0f) == Approx(kLickWindMax));
    // A lone 11.3 m pine's column (crown 5.65 m across, flame 14.2 m) puffs
    // at about 0.6 Hz, the lower of the pool fire's and the flame's own rates.
    const float puff = ColumnPuffHz(5.65f, 14.2f);
    CHECK(puff == Approx(0.61f).margin(0.02f));
    CHECK(puff <= LickHz(14.2f));
    CHECK(puff >= 0.5f);
    CHECK(puff <= 0.7f);
}

TEST_CASE("bushes change a cell's fuel only by their cover (the consult's cover rule)", "[firespread]")
{
    // A plant covers pi r^2 of its 64 m2 cell. One 2.4 m thicket (r 1.2 m,
    // 4.5 m2) leaves the ground's own fuel to carry the fire: it flares as a
    // plant instead of turning the cell into brush.
    const float cell = 64.0f;
    const float one = 3.14159265f * 1.2f * 1.2f;
    CHECK(CoverModel(1, one, 0.0f, 0.0f, cell, 2.4f) == 1);
    // A clump of thickets over a third of it: grass with open shrub.
    CHECK(CoverModel(1, 0.4f * cell, 0.0f, 0.0f, cell, 2.4f) == 2);
    // Not a leafy or reed clump: those stay the ground's.
    CHECK(CoverModel(1, 0.0f, 0.4f * cell, 0.0f, cell, 2.4f) == 1);
    CHECK(CoverModel(1, 0.0f, 0.0f, 0.4f * cell, cell, 2.4f) == 1);
    // A stand over two thirds: reeds tall grass, leafy bushes green brush,
    // thickets dormant brush, tall thickets chaparral.
    CHECK(CoverModel(1, 0.0f, 0.0f, 0.8f * cell, cell, 0.0f) == 3);
    CHECK(CoverModel(1, 0.1f * cell, 0.7f * cell, 0.0f, cell, 2.4f) == 5);
    CHECK(CoverModel(1, 0.8f * cell, 0.0f, 0.0f, cell, 1.5f) == 6);
    CHECK(CoverModel(1, 0.8f * cell, 0.0f, 0.0f, cell, 2.4f) == 4);
}

TEST_CASE("a burning bush flares to its class's height and timing (the consult's bush flare)", "[firespread]")
{
    // A 2.76 m thicket (krovi): a flame twice its height, peak in 3 s + 2 s a
    // metre, full 10 s + 10 s a metre, stem flames after (at most 150 s), the
    // 3 m layout or more of main tongues and half as many again of fill.
    const FlareShape a = FlareShapeOf(PlantClass::Thicket, 2.76f);
    CHECK(a.flameM == Approx(5.52f));
    CHECK(a.peakSec == Approx(8.52f));
    CHECK(a.fullSec == Approx(37.6f));
    CHECK(a.stemSec == Approx(112.8f));
    CHECK(a.mainN == 7);
    CHECK(a.fillN == 11);
    CHECK(FlareShapeOf(PlantClass::Thicket, 5.0f).stemSec == Approx(kFlareStemMaxSec));
    // A leafy bush (ker listnac): weak, 1.15 h, from its windward lower half.
    const FlareShape b = FlareShapeOf(PlantClass::Leafy, 3.0f);
    CHECK(b.flameM == Approx(3.45f));
    CHECK(b.peakSec + b.fullSec + b.downSec == Approx(16.0f));
    CHECK(b.stemSec == 0.0f);
    CHECK(b.leeShare == 0.0f);
    CHECK(b.mainFoot1 <= 0.5f);
    // A reed clump (ker rakosi): a fast, tall flash, many thin clear tongues.
    const FlareShape r = FlareShapeOf(PlantClass::Reed, 2.5f);
    CHECK(r.flameM == Approx(3.75f));
    CHECK(r.peakSec == Approx(3.5f));
    CHECK(r.fullSec == Approx(15.0f));
    CHECK(r.mainN > a.mainN);
    CHECK(r.widthK < 1.0f);
    CHECK(r.woody == 0.0f);
    CHECK(r.absorb < a.absorb);
    // An olive: a partial flare in its crown, 1.35 h, slow to build.
    const FlareShape o = FlareShapeOf(PlantClass::Olive, 3.5f);
    CHECK(o.flameM == Approx(4.725f));
    CHECK(o.peakSec == Approx(12.0f));
    CHECK(o.mainFoot0 >= 0.5f);
    // Every class's flame stands within the consult's share of its height.
    for (const float h : {1.7f, 2.4f, 3.2f})
    {
        CHECK(FlareShapeOf(PlantClass::Thicket, h).flameM / h == Approx(2.0f));
        CHECK(FlareShapeOf(PlantClass::Leafy, h).flameM / h >= 1.0f);
        CHECK(FlareShapeOf(PlantClass::Leafy, h).flameM / h <= 1.3f);
        CHECK(FlareShapeOf(PlantClass::Reed, h).flameM / h == Approx(1.5f));
        CHECK(FlareShapeOf(PlantClass::Olive, h).flameM / h >= 1.2f);
        CHECK(FlareShapeOf(PlantClass::Olive, h).flameM / h <= 1.5f);
    }
}

TEST_CASE("burnt ground keeps flaming behind the front (the consult's lingering)", "[firespread]")
{
    // The drawn band: the model's own residence for grass, the heavier
    // fuel's slower burning in shrub and litter (never shorter than the model's).
    CHECK(DrawResidenceSec(1, 6.6f) == Approx(6.6f));
    CHECK(DrawResidenceSec(3, 15.4f) == Approx(15.4f));
    CHECK(DrawResidenceSec(4, 13.7f) == Approx(kShrubBandSec));
    CHECK(DrawResidenceSec(9, 10.0f) == Approx(kLitterBandSec));
    CHECK(DrawResidenceSec(10, 12.0f) == Approx(kTimberBandSec));
    CHECK(DrawResidenceSec(10, 60.0f) == Approx(60.0f));
    // A point flames full at first, eases to kBandEndShare by the band's end,
    // then falls as exp(-(t - tr) / tr) without a jump: a third of the
    // band's end after one more tr, a seventh after two.
    const float tr = 6.6f;
    CHECK(LingerFlame(0.0f, tr).length == 0.0f);
    CHECK(LingerFlame(2.0f, tr).length > 0.85f);
    CHECK(LingerFlame(2.0f, tr).cover == 1.0f);
    CHECK(LingerFlame(tr - 0.001f, tr).length == Approx(LingerFlame(tr + 0.001f, tr).length).margin(0.01f));
    CHECK(LingerFlame(tr - 0.001f, tr).heat == Approx(LingerFlame(tr + 0.001f, tr).heat).margin(0.01f));
    CHECK(LingerFlame(2.0f * tr, tr).length / kBandEndShare == Approx(0.368f).margin(0.01f));
    CHECK(LingerFlame(3.0f * tr, tr).length / kBandEndShare == Approx(0.135f).margin(0.01f));
    // The tail breaks up: half the points alight at 1.5 tr, a fifth at
    // 2.5 tr, none by kTailEnd tr.
    CHECK(LingerFlame(1.5f * tr, tr).cover == Approx(0.5f));
    CHECK(LingerFlame(2.5f * tr, tr).cover == Approx(0.2f));
    CHECK(LingerFlame(kTailEnd * tr, tr).cover == 0.0f);
    // The consult's test 2 for the band and tail alone: grass (model 1) 30 to
    // 50% of the ground still flaming 10 s after the front; under conifers
    // (model 10) about 80% at 10 s, 40 to 60% at 30 s, 10 to 15% at 60 s.
    CHECK(LingerFlame(10.0f, tr).cover >= 0.3f);
    CHECK(LingerFlame(10.0f, tr).cover <= 0.5f);
    const float tt = DrawResidenceSec(10, 12.0f);
    CHECK(LingerFlame(10.0f, tt).cover == 1.0f);
    CHECK(LingerFlame(30.0f, tt).cover >= 0.4f);
    CHECK(LingerFlame(30.0f, tt).cover <= 0.6f);
    CHECK(LingerFlame(60.0f, tt).cover >= 0.1f - 1e-4f);
    CHECK(LingerFlame(60.0f, tt).cover <= 0.15f);
}

TEST_CASE("flamelets live a lognormal time (the consult's residual flamelets)", "[firespread]")
{
    // Median 40 s, log sigma 0.7: about 66% alight at 30 s, 28% at 60 s, 6%
    // at 2 min (sampled on an even grid of the two draws).
    int n = 0, at30 = 0, at60 = 0, at120 = 0;
    std::vector<float> lives;
    for (int i = 0; i < 200; ++i)
        for (int j = 0; j < 200; ++j)
        {
            const float life = FlameletLifeSec((i + 0.5f) / 200.0f, (j + 0.5f) / 200.0f);
            lives.push_back(life);
            ++n;
            at30 += life > 30.0f;
            at60 += life > 60.0f;
            at120 += life > 120.0f;
        }
    std::sort(lives.begin(), lives.end());
    CHECK(lives[lives.size() / 2] == Approx(kFlameletMedianSec).margin(1.5f));
    CHECK(static_cast<float>(at30) / n == Approx(0.66f).margin(0.03f));
    CHECK(static_cast<float>(at60) / n == Approx(0.28f).margin(0.03f));
    CHECK(static_cast<float>(at120) / n == Approx(0.06f).margin(0.02f));
    CHECK(lives.back() <= kFlameletMaxSec);
    // About 8 a model 1 grass cell, twice in grass with shrub litter.
    CHECK(kFlameletsPerCell * FlameletScale(1) == Approx(8.0f));
    CHECK(FlameletScale(2) == Approx(2.0f));
    CHECK(FlameletScale(0) == 0.0f);
}

TEST_CASE("tongue feet are pinned to their fuel (the consult's seventh round)", "[firespread]")
{
    // A head cell crossed at 1.9 m/s (model 1 grass): its surface tongues at
    // one moment and later ones (in the band, then the tail) stand on the very
    // same feet, every one inside the cell; the tail only drops tongues.
    BurningView v;
    v.x = 4004.0f;
    v.z = 10004.0f;
    v.flameMaxM = 2.6f;
    v.crossSec = 8.0f / 1.9f;
    v.frontX = 0.0f;
    v.frontZ = 1.0f;
    v.residenceSec = 6.6f;
    v.model = 1;
    const float tr = DrawResidenceSec(1, 6.6f);
    std::vector<SurfaceTongue> now;
    std::map<std::uint32_t, std::pair<float, float>> feet;
    int compared = 0;
    for (const float age : {1.0f, 1.5f, 1.53f, 3.0f, 7.0f, 12.0f, 20.0f})
    {
        v.age = age;
        CellSurfaceTongues(v, tr, 8.0f, 12345U, 0.0f, 1.0f, 20.0f, 0.0f, now);
        CHECK_FALSE(now.empty());
        for (const SurfaceTongue& s : now)
        {
            CHECK(std::fabs(s.ox) <= 4.0f);
            CHECK(std::fabs(s.oz) <= 4.0f);
            const auto [it, fresh] = feet.emplace(s.key, std::make_pair(s.ox, s.oz));
            if (!fresh)
            {
                CHECK(it->second.first == s.ox);
                CHECK(it->second.second == s.oz);
                ++compared;
            }
        }
    }
    CHECK(compared > 50);
    // Past the tail's end nothing stands.
    v.age = v.crossSec + kTailEnd * tr + 0.1f;
    CellSurfaceTongues(v, tr, 8.0f, 12345U, 0.0f, 1.0f, 20.0f, 0.0f, now);
    CHECK(now.empty());
}

TEST_CASE("far flames keep their light and are drawn to the horizon at night (fix 08)", "[firespread]")
{
    // A head cell crossed at 1.9 m/s with its whole square alight: near, every
    // tongue stands (keep 1, gain 1); far off the level of detail drops
    // tongues, and the kept ones carry the dropped ones' light (keep x gain x
    // widen, a tongue's light being its width's), so the cell's light stays
    // within a few percent of the near one (the realism audit's C13: the old
    // level of detail and its fade through the heat kept about a seventh of
    // it at 1 km).
    BurningView v;
    v.x = 4004.0f;
    v.z = 10004.0f;
    v.flameMaxM = 2.6f;
    v.crossSec = 8.0f / 1.9f;
    v.frontX = 0.0f;
    v.frontZ = 1.0f;
    v.residenceSec = 6.6f;
    v.model = 1;
    v.age = v.crossSec + 0.5f;
    const float tr = DrawResidenceSec(1, 6.6f);
    std::vector<SurfaceTongue> tongues;
    auto light = [&](float dist, double& kept)
    {
        const SurfaceGrid g = CellSurfaceTongues(v, tr, 8.0f, 777U, 0.0f, 1.0f, dist, 0.0f, tongues);
        double sum = 0.0;
        kept = 0.0;
        for (const SurfaceTongue& s : tongues)
        {
            sum += static_cast<double>(s.keep) * g.gain * g.widen;
            kept += s.keep;
        }
        return sum;
    };
    double keptNear = 0.0;
    const double nearLight = light(20.0f, keptNear);
    REQUIRE(keptNear > 40.0);
    CHECK(nearLight == Approx(keptNear));
    for (const float dist : {500.0f, 1000.0f, 2000.0f})
    {
        CAPTURE(dist);
        double kept = 0.0;
        const double farLight = light(dist, kept);
        if (dist >= 1000.0f)
            CHECK(kept < 0.8 * keptNear);
        CHECK(farLight == Approx(nearLight).epsilon(0.2));
    }
    // A share of a level's tongues kept on average: 2^-lod at whole levels.
    CHECK(FlameLodMeanKeep(0.0f) == Approx(1.0f));
    CHECK(FlameLodMeanKeep(1.0f) == Approx(0.5f));
    CHECK(FlameLodMeanKeep(1.5f) == Approx(0.375f));
    CHECK(FlameLodMeanKeep(3.0f) == Approx(0.125f));
    // The level of detail is worked in the reference view's pixels: a 6 times
    // zoom at 1.3 km thins as at about 220 m.
    CHECK(FlameLodDistance(1300.0f, kLodRefLeft) == Approx(1300.0f));
    CHECK(FlameLodDistance(1300.0f, kLodRefLeft / 5.83f) == Approx(223.0f).epsilon(0.01));
    // By day flames to 3 km and the debris behind the front to 200 m; at
    // night both to the camera's far clip (the horizon), at most 8 km; dusk
    // between. The last part of a range fades out.
    CHECK(FlameDrawRangeM(0.0f, 4500.0f) == Approx(kFlameDrawDayM));
    CHECK(FlameLingerRangeM(0.0f, 4500.0f) == Approx(kLingerDayM));
    CHECK(FlameDrawRangeM(1.0f, 4500.0f) == Approx(4500.0f));
    CHECK(FlameLingerRangeM(1.0f, 4500.0f) == Approx(4500.0f));
    CHECK(FlameLingerRangeM(1.0f, 20000.0f) == Approx(kFlameDrawNightMaxM));
    CHECK(FlameLingerRangeM(0.35f, 4500.0f) > kLingerDayM);
    CHECK(FlameLingerRangeM(0.35f, 4500.0f) < 4500.0f);
    CHECK(FlameDrawRangeM(0.0f, 1200.0f) == Approx(1200.0f));
    CHECK(FlameRangeFade(100.0f, 3000.0f) == Approx(1.0f));
    CHECK(FlameRangeFade(3000.0f, 3000.0f) == Approx(0.0f).margin(1e-6));
    CHECK(FlameRangeFade(2800.0f, 3000.0f) > 0.0f);
    CHECK(FlameRangeFade(2800.0f, 3000.0f) < FlameRangeFade(2600.0f, 3000.0f));
    CHECK(kLingerDayM >= 150.0f);
    CHECK(kFlameDrawNightMaxM >= 2000.0f);
}

TEST_CASE("residual flamelets are low wide tufts, not streaks (the consult's seventh round)", "[firespread]")
{
    // A 0.25 m flamelet in an 11 m/s wind leans by the tilt law in the wind
    // at its own mid height (about 3.2 m/s over burnt ground): past 65
    // degrees, so it is held at 65; a lull stands it up to 40, a gust presses
    // it to 70; in a light wind it stands more upright and a lull never
    // raises it.
    CHECK(TuftTiltRad(11.0f, 0.25f, 0.0f, 0.0f) == Approx(kTuftTiltMax));
    CHECK(TuftTiltRad(11.0f, 0.25f, 1.0f, 0.0f) == Approx(kTuftTiltLull));
    CHECK(TuftTiltRad(11.0f, 0.25f, 0.0f, 1.0f) == Approx(kTuftTiltGust));
    const float light = TuftTiltRad(1.0f, 0.25f, 0.0f, 0.0f);
    CHECK(light < kTuftTiltLull);
    CHECK(TuftTiltRad(1.0f, 0.25f, 1.0f, 0.0f) == Approx(light));
    // Its body 0.4 to 0.7 of its length wide (kTuftWiden of a tongue's frame),
    // and a tail tongue becomes one as it shrinks under kTuftFromM.
    CHECK(TuftShare(0.2f) == Approx(1.0f));
    CHECK(TuftShare(kTuftFromM) == Approx(0.0f));
    CHECK(kTuftWiden * FlameFrameWidth(2, 1.0f) > 2.0f * FlameFrameWidth(2, 1.0f));
    // The band's tips swing over 1.26 sqrt(L) s (1 to 2 s for 1 to 2.5 m
    // flames), never faster than once a second; by at most 0.15 L.
    CHECK(SwayPeriodSec(1.0f) == Approx(1.26f));
    CHECK(SwayPeriodSec(2.5f) == Approx(1.99f).margin(0.01f));
    CHECK(SwayPeriodSec(0.2f) == Approx(1.0f));
    CHECK(kSwayBandMax <= 0.15f);
    CHECK(kSwayTailMax <= 0.2f);
    CHECK(kSwayTuftMax <= 0.3f);
}
// ---- The plume (the realism audit's fix 2; FirePlume.hpp) ----

TEST_CASE("the plume rises and spreads as the consult's bent-over plume", "[firespread]")
{
    // The scene's 120 m grass head: about 250 MW in an 11 m/s wind, smoke
    // leaving at half a 2.6 m flame (consult eighth 1).
    const PlumeLaw l = MakePlumeLaw(250.0f, 11.0f, 1.3f, true);
    // Centreline: 30 m (20 to 35) at 100 m, 145 m (100 to 200) at 1 km, 230 m
    // (200 to 300) at 2 km; the rise flattens at about 2 km.
    CHECK(PlumeCentreM(l, 100.0f) >= 20.0f);
    CHECK(PlumeCentreM(l, 100.0f) <= 35.0f);
    CHECK(PlumeCentreM(l, 1000.0f) == Approx(145.0f).margin(15.0f));
    CHECK(PlumeCentreM(l, 2000.0f) == Approx(230.0f).margin(20.0f));
    CHECK(l.finalX == Approx(2100.0f).margin(300.0f));
    CHECK(PlumeRiseM(l, 4000.0f) == Approx(PlumeRiseM(l, l.finalX)));
    // Seen from the source the axis rises about 17 degrees at 100 m, 8 at
    // 1 km and 6.5 at 2 km.
    auto deg = [&](float x) { return std::atan2(PlumeCentreM(l, x), x) * 57.29578f; };
    CHECK(deg(100.0f) == Approx(16.0f).margin(3.0f));
    CHECK(deg(1000.0f) == Approx(8.0f).margin(1.0f));
    CHECK(deg(2000.0f) == Approx(6.5f).margin(1.0f));
    // Thickness about 1.2 times the rise: 150 to 200 m at 1 km, 250 to 300 m
    // at 2 km; the underside 40 to 70 m up at 1 km and about 100 m at 2 km,
    // and on the ground for the first 100 m (the eye downwind stands in it).
    const float t1 = PlumeTopM(l, 1000.0f) - PlumeBottomM(l, 1000.0f);
    const float t2 = PlumeTopM(l, 2000.0f) - PlumeBottomM(l, 2000.0f);
    CHECK(t1 >= 150.0f);
    CHECK(t1 <= 200.0f);
    CHECK(t2 >= 250.0f);
    CHECK(t2 <= 300.0f);
    CHECK(PlumeBottomM(l, 1000.0f) >= 40.0f);
    CHECK(PlumeBottomM(l, 1000.0f) <= 70.0f);
    CHECK(PlumeBottomM(l, 2000.0f) == Approx(100.0f).margin(15.0f));
    CHECK(PlumeBottomM(l, 50.0f) == Approx(0.0f));
    CHECK(PlumeBottomM(l, 100.0f) < 2.0f);
    // Width: the fire's 120 m plus the spread, 150 to 170 m at 100 m, 300 to
    // 400 m at 1 km, 450 to 600 m at 2 km.
    auto width = [&](float x) { return 120.0f + 2.0f * PlumeSpreadM(l, x); };
    CHECK(width(100.0f) >= 150.0f);
    CHECK(width(100.0f) <= 170.0f);
    CHECK(width(1000.0f) >= 300.0f);
    CHECK(width(1000.0f) <= 400.0f);
    CHECK(width(2000.0f) >= 450.0f);
    CHECK(width(2000.0f) <= 600.0f);
    // A weaker fire's plume lies lower, a lighter wind's stands taller, and a
    // torching crown's smoke leaves from its flame's top and never hugs.
    CHECK(PlumeCentreM(MakePlumeLaw(25.0f, 11.0f, 1.3f, true), 1000.0f) < PlumeCentreM(l, 1000.0f));
    CHECK(PlumeCentreM(MakePlumeLaw(250.0f, 3.0f, 1.3f, true), 1000.0f) > PlumeCentreM(l, 1000.0f));
    const PlumeLaw crown = MakePlumeLaw(28.0f, 11.0f, 20.0f, false);
    CHECK(crown.hugM == 0.0f);
    CHECK(PlumeBottomM(crown, 10.0f) >= 9.0f);
    // A lone torching 12 m pine's slug: about 35 m up at 100 m, 80 to 100 m
    // at 1 km (consult eighth 1).
    CHECK(PlumeCentreM(crown, 100.0f) == Approx(35.0f).margin(8.0f));
    CHECK(PlumeCentreM(crown, 1000.0f) == Approx(85.0f).margin(15.0f));
}

TEST_CASE("plume puffs thin as they grow but carry the plume's smoke", "[firespread]")
{
    const PlumeLaw l = MakePlumeLaw(250.0f, 11.0f, 1.3f, true);
    const float born = 5.0f, perMetre = 30.0f / 11.0f, smoke = 1.0f, k = 11.0f * 178.0f;
    const float mass = smoke / 30.0f;
    // They grow (no more 7 to 13 m discs at the source; tens of metres far out).
    CHECK(PlumePuffSizeM(l, 0.0f, born) == Approx(born));
    CHECK(PlumePuffSizeM(l, 1000.0f, born) > 50.0f);
    CHECK(PlumePuffSizeM(l, 2000.0f, born) > PlumePuffSizeM(l, 1000.0f, born));
    // All live near the source; the set thins downwind, never back up.
    CHECK(PlumeSurvival(l, 0.0f, born, perMetre) == Approx(1.0f));
    float last = 1.0f;
    for (float x = 25.0f; x <= 6000.0f; x += 25.0f)
    {
        const float s = PlumeSurvival(l, x, born, perMetre);
        CHECK(s <= last + 1e-4f);
        last = s;
    }
    CHECK(last < 0.05f);
    // Wherever the set has thinned, about kPuffOverlap puffs lie behind each
    // other side-on, each carrying its share of the plume's optical depth:
    // side-on 1 at 1 km, about 0.6 at 2 km, about 0.4 at 5 km (consult eighth 1).
    for (float x : {1000.0f, 2000.0f, 5000.0f})
    {
        const float live = PlumeSurvival(l, x, born, perMetre);
        const float s = PlumePuffSizeM(l, x, born);
        const float thick = PlumeTopM(l, x) - PlumeBottomM(l, x);
        const float overlap = perMetre * live * kPuffCover * s * s / thick;
        CHECK(overlap == Approx(kPuffOverlap).margin(0.05f));
        const float tau = PlumePuffTau(l, x, born, perMetre, mass, k);
        CHECK(overlap * tau == Approx(PlumeSideTau(l, x, smoke, 11.0f, k)).epsilon(0.01));
    }
    CHECK(PlumeSideTau(l, 1000.0f, smoke, 11.0f, k) == Approx(1.0f).margin(0.05f));
    CHECK(PlumeSideTau(l, 2000.0f, smoke, 11.0f, k) == Approx(0.63f).margin(0.08f));
    CHECK(PlumeSideTau(l, 5000.0f, smoke, 11.0f, k) == Approx(0.45f).margin(0.1f));
    // From above it is thinner than side-on (wider than thick): about half.
    const float top1 = PlumeTopTau(l, 1000.0f, smoke, 11.0f, 120.0f, k);
    CHECK(top1 < PlumeSideTau(l, 1000.0f, smoke, 11.0f, k));
    CHECK(top1 == Approx(0.5f).margin(0.15f));
    // PlumeAt (a puff's every step) is the same as the separate functions.
    for (float x : {0.0f, 30.0f, 150.0f, 900.0f, 2500.0f, 5500.0f})
    {
        const PlumePoint p = PlumeAt(l, x, born, perMetre, mass, k);
        CHECK(p.top == Approx(PlumeTopM(l, x)));
        CHECK(p.bottom == Approx(PlumeBottomM(l, x)).margin(1e-4));
        CHECK(p.spread == Approx(PlumeSpreadM(l, x)).margin(1e-4));
        CHECK(p.size == Approx(PlumePuffSizeM(l, x, born)));
        CHECK(p.survival == Approx(PlumeSurvival(l, x, born, perMetre)));
        CHECK(p.tau == Approx(PlumePuffTau(l, x, born, perMetre, mass, k)));
    }
}

TEST_CASE("plume smoke takes its fuel's colour and the sun's angle", "[firespread]")
{
    // Grass smoke leaves pale grey with a tan cast and is grey-white by 500 m;
    // woody smoke leaves dark and stays darker for a kilometre and more.
    const PlumeRgb grass = PlumeAlbedo(0.0f, 0.0f), wood = PlumeAlbedo(1.0f, 0.0f);
    CHECK(grass.r > 0.7f);
    CHECK(grass.r > grass.b); // tan
    CHECK(wood.r < 0.4f);
    CHECK(PlumeAlbedo(0.0f, 700.0f).g > grass.g);
    CHECK(PlumeAlbedo(1.0f, 1000.0f).g < PlumeAlbedo(0.0f, 1000.0f).g - 0.15f);
    CHECK(PlumeAlbedo(1.0f, 3000.0f).g > 0.7f);
    // Thin smoke glows when the sun is behind it; dense smoke shows its lit
    // side with the sun behind the eye and its shaded side looking sunward.
    CHECK(PlumePhase(0.95f, 0.9f) > 2.0f);
    CHECK(PlumePhase(0.95f, 0.9f) > 2.0f * PlumePhase(-0.95f, 0.9f));
    CHECK(PlumePhase(-0.95f, 0.05f) > PlumePhase(0.95f, 0.05f));
    CHECK(PlumePhase(0.95f, 0.05f) < 0.6f);
    // Light through the plume goes orange-brown; none of the sun's share at night.
    const PlumeRgb thru = PlumeSunThrough(1.0f);
    CHECK(thru.r > thru.g);
    CHECK(thru.g > thru.b);
    CHECK(PlumeSunShare(-5.0f) == Approx(0.0f));
    CHECK(PlumeSunShare(45.0f) > 0.7f);
    // The flames light the underside within tens of metres, fading out by
    // kPlumeGlowRangeM.
    CHECK(PlumeGlowKernel(0.0f) == Approx(1.0f));
    CHECK(PlumeGlowKernel(kPlumeGlowReachM) == Approx(0.5f));
    CHECK(PlumeGlowKernel(kPlumeGlowRangeM) == Approx(0.0f));
    CHECK(PlumeGlowKernel(60.0f) < 0.05f);
    CHECK(PlumeGlowKernel(30.0f) > 0.1f);
    // The flames' light on the underside shows from about sunset; by day it is a
    // faint warmth at most.
    CHECK(PlumeGlowNight(-10.0f) == Approx(1.0f));
    CHECK(PlumeGlowNight(45.0f) == Approx(kPlumeGlowDay));
    CHECK(PlumeGlowNight(0.0f) > 0.4f);
    CHECK(PlumeGlowNight(0.0f) < 1.0f);
    CHECK(kPlumeGlowRgb[0] > kPlumeGlowRgb[1]);
    CHECK(kPlumeGlowRgb[1] > kPlumeGlowRgb[2]);
}

TEST_CASE("a plant's own fire: scorch, bark char and what is left (the realism audit's fix 3)", "[firespread]")
{
    // Crown scorch over a fire that does not burn the crown (the consult's
    // eighth answer): 3 to 5 m at the 2000 kW/m head, 0.5 to 1 m on a 100 kW/m
    // flank, little more than the flames at a 10 kW/m back.
    CHECK(CrownScorchM(2060.0f) >= 3.0f);
    CHECK(CrownScorchM(2060.0f) <= 5.0f);
    CHECK(CrownScorchM(100.0f) >= 0.5f);
    CHECK(CrownScorchM(100.0f) <= 1.0f);
    CHECK(CrownScorchM(10.0f) < 0.5f);
    CHECK(CrownScorchM(0.0f) == Approx(kScorchMinM));
    // Bark char: 0.5 to 1 m windward and 2 to 4 m lee at the 2.6 m head; 0.3
    // to 0.6 m on a 0.65 m flank, higher on the side away; 0.2 to 0.4 m and
    // nearly even at the back; never the whole tree.
    float w = 0.0f, l = 0.0f;
    BarkCharM(2.6f, false, w, l);
    CHECK(w >= 0.5f);
    CHECK(w <= 1.0f);
    CHECK(l >= 2.0f);
    CHECK(l <= 4.0f);
    BarkCharM(0.65f, false, w, l);
    CHECK(w >= 0.3f);
    CHECK(l <= 0.65f);
    CHECK(l > w);
    BarkCharM(0.35f, false, w, l);
    CHECK(l >= 0.2f);
    CHECK(l <= 0.4f);
    CHECK(w / l > 0.8f);
    BarkCharM(30.0f, false, w, l);
    CHECK(l <= kBarkMaxM);

    // What is left. A flared thicket: a black skeleton at kThicketKeep (20 to
    // 30%) of its bulk, burning away through its rise and full flame, its
    // stems charred.
    const float hT = 2.76f;
    const FlareShape thicket = FlareShapeOf(PlantClass::Thicket, hT);
    const PlantLook t = PlantLookOf(PlantKind::Thicket, hT, 2000.0f, 2.6f, &thicket, false, 0.0f);
    CHECK(1.0f - t.goneShare >= 0.2f);
    CHECK(1.0f - t.goneShare <= 0.3f);
    CHECK(t.keptChar == Approx(1.0f));
    CHECK(t.burn0Sec >= 0.0f);
    CHECK(t.burn0Sec < thicket.peakSec);
    CHECK(t.burn1Sec > thicket.peakSec);
    CHECK(t.burn1Sec <= thicket.peakSec + thicket.fullSec);
    CHECK(t.frameTopM > hT);
    CHECK(t.sweepMax == Approx(1.0f));
    CHECK(t.scorchChar >= 0.8f); // a dark lattice in its flames
    CHECK(t.strands);            // its stems, not round clumps
    // A flared olive: a blackened frame with brown leaves, 50 to 70% burnt.
    const FlareShape olive = FlareShapeOf(PlantClass::Olive, 3.5f);
    const PlantLook o = PlantLookOf(PlantKind::Olive, 3.5f, 2000.0f, 2.6f, &olive, false, 0.0f);
    CHECK(o.goneShare >= 0.5f);
    CHECK(o.goneShare <= 0.7f);
    CHECK(o.keptChar < 0.3f);
    CHECK(o.scorchChar < 0.5f); // its leaves stay brown
    CHECK_FALSE(o.strands);     // clumps of brown leaves
    CHECK(o.frameTopM > 3.5f);
    // A reed clump: black stubble kReedStubbleM (0.3 to 0.5 m) high.
    const FlareShape reed = FlareShapeOf(PlantClass::Reed, 2.6f);
    const PlantLook r = PlantLookOf(PlantKind::Reed, 2.6f, 2000.0f, 2.6f, &reed, false, 0.0f);
    CHECK(r.keepBelowM >= 0.3f);
    CHECK(r.keepBelowM <= 0.5f);
    CHECK(r.goneShare > 0.85f);
    // A leafy bush's weak flare: its windward lower half, so 70 to 90% of its
    // bulk stays, brown.
    const FlareShape leafy = FlareShapeOf(PlantClass::Leafy, 2.8f);
    const PlantLook b = PlantLookOf(PlantKind::Leafy, 2.8f, 2000.0f, 2.6f, &leafy, false, 0.0f);
    CHECK(1.0f - b.sweepMax * b.goneShare >= 0.7f);
    CHECK(1.0f - b.sweepMax * b.goneShare <= 0.9f);
    CHECK(b.keptChar < 0.3f);
    // Only a torched pine loses every needle; its frame chars as the torch climbs.
    const PlantLook p = PlantLookOf(PlantKind::Pine, 12.0f, 2000.0f, 2.6f, nullptr, true, kTorchDelaySec);
    CHECK(p.goneShare == Approx(1.0f));
    CHECK(p.scorchChar >= 0.8f);
    CHECK(p.burn0Sec >= kTorchDelaySec);
    CHECK(p.burn1Sec <= kTorchDelaySec + kTorchSec);
    CHECK(p.frameTopM > 12.0f);
    // A pine or a broadleaf the head ran under: scorched brown 3 to 5 m up,
    // nothing burnt away, the trunk charred only at its foot.
    for (const PlantKind kind : {PlantKind::Pine, PlantKind::Broadleaf})
    {
        const PlantLook s = PlantLookOf(kind, 12.0f, 2000.0f, 2.6f, nullptr, false, 0.0f);
        CHECK(s.goneShare == Approx(0.0f));
        CHECK_FALSE(s.burn1Sec > s.burn0Sec);
        CHECK(s.scorchTopM >= 3.0f);
        CHECK(s.scorchTopM <= 5.0f);
        CHECK(s.barkLeeM <= 4.0f);
        CHECK(s.frameTopM == Approx(0.0f));
    }
    // A thicket the backing fire only singes, and a broadleaf on a flank.
    const PlantLook singe = PlantLookOf(PlantKind::Thicket, hT, 15.0f, 0.35f, nullptr, false, 0.0f);
    CHECK(singe.goneShare == Approx(0.0f));
    CHECK(singe.scorchTopM < 0.5f);
    CHECK(PlantLookOf(PlantKind::Broadleaf, 12.0f, 100.0f, 0.65f, nullptr, false, 0.0f).scorchTopM <= 1.0f);

    // The share of leaf clumps that go: the threshold under which that share
    // of the shader's value noise lies, monotonic, 0 and 1 at the ends.
    // A skeleton's band keeps its share: half of it either side of the median.
    CHECK(PlantStrandHalfWidth(0.0f) == Approx(0.0f));
    CHECK(PlantStrandHalfWidth(1.0f) == Approx(0.5f));
    CHECK(0.5f + PlantStrandHalfWidth(0.25f) == Approx(PlantNoiseThreshold(0.625f)));
    CHECK(PlantNoiseThreshold(0.0f) == Approx(0.0f));
    CHECK(PlantNoiseThreshold(0.5f) == Approx(0.5f));
    CHECK(PlantNoiseThreshold(1.0f) == Approx(1.0f));
    float last = -1.0f;
    for (int i = 0; i <= 20; ++i)
    {
        const float th = PlantNoiseThreshold(static_cast<float>(i) / 20.0f);
        CHECK(th > last);
        last = th;
    }
}

TEST_CASE("a plant is reached when the drawn flames reach it (the realism audit's fix 3)", "[firespread]")
{
    // DrawnReachSec is the surface tongues' own timing: every tongue of a
    // head cell lights at the cell's age DrawnReachSec gives for its foot.
    BurningView v;
    v.x = 4004.0f;
    v.z = 10004.0f;
    v.flameMaxM = 2.6f;
    v.crossSec = 8.0f / 1.9f;
    v.frontX = 0.0f;
    v.frontZ = 1.0f;
    v.residenceSec = 6.6f;
    v.model = 1;
    v.age = 3.0f;
    std::vector<SurfaceTongue> now;
    CellSurfaceTongues(v, DrawResidenceSec(1, 6.6f), 8.0f, 77U, 0.0f, 1.0f, 20.0f, 0.0f, now);
    REQUIRE_FALSE(now.empty());
    for (const SurfaceTongue& s : now)
        CHECK(s.age == Approx(v.age - DrawnReachSec(v, v.x + s.ox, v.z + s.oz, 8.0f)).margin(2e-3f));
    // The front's entry edge at 0, its exit edge at the crossing, a cell lit
    // all at once at 0 everywhere.
    CHECK(DrawnReachSec(v, v.x, v.z - 4.0f, 8.0f) == Approx(0.0f));
    CHECK(DrawnReachSec(v, v.x, v.z + 4.0f, 8.0f) == Approx(v.crossSec));
    CHECK(DrawnReachSec(v, v.x, v.z + 40.0f, 8.0f) == Approx(v.crossSec));
    v.crossSec = 0.0f;
    CHECK(DrawnReachSec(v, v.x + 3.0f, v.z + 3.0f, 8.0f) == Approx(0.0f));
}

TEST_CASE("the plant table finds every plant from its foot rebuilt to a millimetre", "[firespread]")
{
    // The shader rebuilds a plant's foot from the drawn object's origin to
    // about a millimetre; the table must find it whatever side of a bucket's
    // edge that lands, never confuse two plants, and stay within its probes.
    PlantTable t;
    t.Reset(8192);
    std::vector<std::pair<float, float>> feet;
    std::uint32_t h = 12345U;
    auto rnd = [&h]()
    {
        h = h * 1664525U + 1013904223U;
        return static_cast<float>(h >> 8) / 16777216.0f;
    };
    // Random feet over a few hundred metres, and feet right on bucket edges.
    for (int i = 0; i < 1200; ++i)
        feet.emplace_back(4600.0f + 300.0f * rnd(), 9900.0f + 300.0f * rnd());
    for (int i = 0; i < 200; ++i)
    {
        const float bx = 4700.0f + static_cast<float>(i), bz = 10050.0f + static_cast<float>(i % 7);
        feet.emplace_back(bx + (i % 2 ? 0.0005f : -0.0005f), bz + (i % 3 ? 0.0004f : 0.9995f));
    }
    std::vector<std::vector<int>> slots;
    for (const auto& [x, z] : feet)
    {
        int s[4];
        REQUIRE(t.Insert(x, z, s));
        slots.emplace_back(s, s + 4);
    }
    CHECK(kPlantLoadDiv * t.used <= t.slots);
    for (std::size_t i = 0; i < feet.size(); ++i)
    {
        const auto [x, z] = feet[i];
        for (const float ex : {-0.002f, 0.0f, 0.002f})
            for (const float ez : {-0.002f, 0.0f, 0.002f})
            {
                const int found = t.Find(x + ex, z + ez);
                REQUIRE(found >= 0);
                CHECK(std::find(slots[i].begin(), slots[i].end(), found) != slots[i].end());
            }
    }
    // Nothing stands where no plant does.
    CHECK(t.Find(100.0f, 100.0f) == -1);
    // A full table refuses rather than overflowing its probes or its load.
    PlantTable small;
    small.Reset(16);
    int s[4];
    int taken = 0;
    for (int i = 0; i < 40 && small.Insert(10.5f + static_cast<float>(i) * 3.0f, 20.5f, s); ++i)
        ++taken;
    CHECK(taken <= 16 / kPlantLoadDiv);
    CHECK(kPlantLoadDiv * small.used <= small.slots);
}

TEST_CASE("a burnt building keeps its plant-table slots however full the table gets", "[firespread]")
{
    // (end-state agent, 2026-11-18) Cancon's worst day: once the table was at
    // its largest, a refused plant cleared every building's slots and the
    // building's own write found no room, so a gutted house was drawn whole
    // again. A refusal must leave the table byte for byte, the buildings keep
    // their slots (and their data) through every growth, and a building finds
    // room even in a table the plants have filled.
    constexpr int kStart = 64, kMax = 256;
    PlantTable t;
    t.Reset(kStart);
    struct Foot
    {
        float x, z;
        int slots[4] = {-1, -1, -1, -1};
    };
    std::vector<Foot> blds, plants;
    blds.reserve(64);
    plants.reserve(4096);
    auto refs = [](std::vector<Foot>& v)
    {
        std::vector<PlantFootRef> r;
        for (Foot& f : v)
            r.push_back({f.x, f.z, f.slots});
        return r;
    };
    auto mark = [&t](const Foot& f, float m)
    {
        for (const int s : f.slots)
            if (s >= 0)
                t.Slot(s)[kPlantSlotFloats - 1] = m;
    };
    auto addBuilding = [&](float x, float z)
    {
        blds.push_back({x, z});
        Foot& f = blds.back();
        std::vector<PlantFootRef> others;
        for (std::size_t i = 0; i + 1 < blds.size(); ++i)
            others.push_back({blds[i].x, blds[i].z, blds[i].slots});
        bool grew = false;
        int dropped = 0;
        const bool ok = PlantTableAdd(t, {f.x, f.z, f.slots}, true, 0, kMax, others, refs(plants), grew, dropped);
        // every building's look written again after a move, as WriteBuildingLook does
        for (const Foot& b : blds)
            mark(b, 7777.0f);
        return ok;
    };
    // Three gutted houses first.
    REQUIRE(addBuilding(5394.6f, 2798.6f));
    REQUIRE(addBuilding(5381.8f, 2781.4f));
    REQUIRE(addBuilding(5410.6f, 2792.4f));
    // Then plants until the largest table refuses them.
    const int reserve = PlantBuildingReserve(0);
    CHECK(reserve == kPlantBuildingReserveMin);
    int refused = 0;
    std::uint32_t h = 777U;
    auto rnd = [&h]()
    {
        h = h * 1664525U + 1013904223U;
        return static_cast<float>(h >> 8) / 16777216.0f;
    };
    for (int i = 0; i < 2000 && refused < 20; ++i)
    {
        plants.push_back({5300.0f + 200.0f * rnd(), 2700.0f + 200.0f * rnd()});
        Foot& f = plants.back();
        const std::vector<float> before = t.data;
        const int usedBefore = t.used, slotsBefore = t.slots;
        bool grew = false;
        int dropped = 0;
        // no reserve here, so the plants fill the table to its load
        if (!PlantTableAdd(t, {f.x, f.z, f.slots}, false, 0, kMax, refs(blds), refs(plants), grew, dropped))
        {
            ++refused;
            // refused: the table byte for byte as it was, the plant without slots
            REQUIRE(t.slots == slotsBefore);
            REQUIRE(t.used == usedBefore);
            REQUIRE(t.data == before);
            CHECK(f.slots[0] == -1);
            plants.pop_back();
        }
        else if (grew)
        {
            for (const Foot& b : blds)
                mark(b, 7777.0f);
        }
        // the buildings are found from their feet, with their looks
        for (const Foot& b : blds)
        {
            const int s = t.Find(b.x, b.z);
            REQUIRE(s >= 0);
            CHECK(std::find(std::begin(b.slots), std::end(b.slots), s) != std::end(b.slots));
            CHECK(t.Slot(s)[kPlantSlotFloats - 1] == 7777.0f);
        }
    }
    CHECK(t.slots == kMax);
    CHECK(refused > 0);
    // A fourth house catches in the full table: it gets slots, the plants
    // make room, and every house is found with its look.
    REQUIRE(addBuilding(5420.3f, 2770.9f));
    CHECK(std::count_if(plants.begin(), plants.end(), [](const Foot& f) { return f.slots[0] < 0; }) > 0);
    for (const Foot& b : blds)
    {
        const int s = t.Find(b.x, b.z);
        REQUIRE(s >= 0);
        CHECK(t.Slot(s)[kPlantSlotFloats - 1] == 7777.0f);
    }
    CHECK(kPlantLoadDiv * t.used <= t.slots);
    // A plant half inserted never stays half inserted: a table one foot short
    // of its load refuses a foot on a bucket corner (four slots) whole.
    PlantTable tiny;
    tiny.Reset(16);
    int s4[4];
    REQUIRE(tiny.Insert(3.5f, 3.5f, s4));
    REQUIRE(tiny.Insert(7.5f, 7.5f, s4));
    REQUIRE(tiny.Insert(11.5f, 11.5f, s4));
    REQUIRE(tiny.Insert(15.5f, 15.5f, s4));
    const std::vector<float> was = tiny.data;
    const int usedWas = tiny.used;
    CHECK_FALSE(tiny.Insert(20.01f, 30.01f, s4));
    CHECK(tiny.used == usedWas);
    CHECK(tiny.data == was);
    CHECK(s4[0] == -1);
}

// ---- Burnt building shells (the end-state agent, 2026-11-18) ----

TEST_CASE("a house's wall tops are the eave line under its roof, not a porch roof or the ridge", "[firespread]")
{
    // A one-storey house 0..6.9 m: walls to 3.6 m, two roof planes from the
    // eaves to the ridge (each in two rows of faces), a small porch roof low
    // on its front at 2.4 m, a chimney cap at 7.2 m, the ground floor.
    std::vector<ShellFaceStat> f;
    const float eave = 3.6f;
    for (int side = 0; side < 2; ++side)
    {
        f.push_back({eave, 4.4f, 0.62f, 18.0f}); // the lower row of tiles
        f.push_back({5.2f, 6.0f, 0.62f, 18.0f}); // the upper row
    }
    f.push_back({2.4f, 2.6f, 0.9f, 2.0f});  // the porch roof (low: not the roof)
    f.push_back({7.2f, 7.2f, 1.0f, 0.3f});  // the chimney's cap
    f.push_back({0.0f, 0.0f, 1.0f, 60.0f}); // the floor
    for (int w = 0; w < 4; ++w)
        f.push_back({0.0f, 1.8f, 0.0f, 25.0f}); // the walls (upright)
    CHECK(ShellWallTopY(f, 0.0f, 6.9f) == Approx(eave));
    // A big porch roof below the eaves still does not pull the eave down past
    // a tenth of the roof's area.
    f.push_back({2.4f, 3.3f, 0.7f, 6.0f});
    CHECK(ShellWallTopY(f, 0.0f, 6.9f) == Approx(eave));
    // No roof face at all: the buildings builder's share of the height.
    const std::vector<ShellFaceStat> walls(4, ShellFaceStat{0.0f, 2.0f, 0.0f, 20.0f});
    CHECK(ShellWallTopY(walls, 0.0f, 5.0f) == Approx(3.0f));
}

TEST_CASE("a gutted shell's rubble is a shallow heap inside its walls, below their tops", "[firespread]")
{
    ShellRubbleFrame one;
    one.cx = 0.3f;
    one.cz = -0.2f;
    one.hx = 4.6f;
    one.hz = 3.7f;
    one.floorY = 0.0f;
    one.wallTopY = 3.6f;
    one.ridgeAlongX = true;
    one.seed = 12345U;
    ShellRubbleFrame two = one;
    two.wallTopY = 6.4f;
    float sum1 = 0.0f, sum2 = 0.0f, lo = 9.0f, hi = 0.0f;
    int n = 0;
    for (float x = -4.0f; x <= 4.0f; x += 0.37f)
        for (float z = -3.2f; z <= 3.2f; z += 0.41f)
        {
            const float d1 = ShellRubbleDepth(one, one.cx + x, one.cz + z);
            const float d2 = ShellRubbleDepth(two, two.cx + x, two.cz + z);
            REQUIRE(d1 >= 0.15f);
            REQUIRE(d1 <= 1.1f);
            REQUIRE(d1 <= one.wallTopY - one.floorY - 0.3f);
            REQUIRE(d2 <= 1.1f);
            lo = std::min(lo, d1);
            hi = std::max(hi, d1);
            sum1 += d1;
            sum2 += d2;
            ++n;
        }
    // the eleventh answer: one storey 0.3 to 0.6 m, two 0.5 to 1.0 m, uneven
    CHECK(sum1 / n > 0.3f);
    CHECK(sum1 / n < 0.65f);
    CHECK(sum2 / n > 0.5f);
    CHECK(sum2 / n < 1.0f);
    CHECK(hi - lo > 0.2f);
    // heaped along the walls and under the ridge more than in the open rooms
    float wall = 0.0f, ridge = 0.0f, mid = 0.0f;
    for (int k = 0; k < 40; ++k)
    {
        const float t = -3.5f + 0.18f * static_cast<float>(k);
        wall += ShellRubbleDepth(one, one.cx + t, one.cz + one.hz - 0.2f);
        ridge += ShellRubbleDepth(one, one.cx + t, one.cz);
        mid += ShellRubbleDepth(one, one.cx + t, one.cz + 0.5f * one.hz);
    }
    CHECK(wall > mid);
    CHECK(ridge > mid);
    // a roof point is brought inside the walls (the eaves' overhang too)
    float x = one.cx + 5.4f, z = one.cz - 4.4f;
    ShellRubbleInside(one, one.cx, one.cz, 5.4f, 4.4f, x, z);
    CHECK(std::fabs(x - one.cx) <= one.hx - kShellRubbleInsetM + 1e-4f);
    CHECK(std::fabs(z - one.cz) <= one.hz - kShellRubbleInsetM + 1e-4f);
}

TEST_CASE("a gutted shell collides, stops shots and sight only up to its wall tops", "[firespread]")
{
    const float cut = 3.6f;
    // a point below the cut stays, one above comes down onto it
    CHECK(ShellSquashY(1.0f, cut) == Approx(1.0f));
    CHECK(ShellSquashY(cut, cut) == Approx(cut));
    CHECK(ShellSquashY(6.9f, cut) == Approx(cut + 0.02f * 3.3f));
    CHECK(ShellSquashY(6.9f, cut) < cut + 0.1f);
    // the roof's prism and the chimney's box (wholly above) go; a wall
    // (from the ground) and a box from the ground to the ridge stay, cut
    CHECK(ShellComponentGoes(ShellSquashY(3.6f, cut), cut));
    CHECK(ShellComponentGoes(ShellSquashY(6.1f, cut), cut));
    CHECK_FALSE(ShellComponentGoes(0.0f, cut));
    CHECK_FALSE(ShellComponentGoes(3.0f, cut));
    // a timber shed's stub: its lowest half metre
    CHECK(ShellComponentGoes(ShellSquashY(2.0f, kShellStubM), kShellStubM));
    CHECK_FALSE(ShellComponentGoes(0.0f, kShellStubM));
}

// ---- The realism audit's fix 4 (2026-11-17): a real front, roads as barriers ----

TEST_CASE("the gauge is the rear-focus ellipse, a segment's minimum and the support speed agree", "[firespread]")
{
    const float R = 1.9f, e = EccentricityOf(6.06f);
    // Along each direction the gauge is the distance over RateToward's rate.
    HeadFire h;
    h.rateMps = R;
    h.dirX = 0.0f;
    h.dirZ = 1.0f;
    h.eccentricity = e;
    for (float deg = 0.0f; deg <= 180.0f; deg += 15.0f)
    {
        const float a = deg * 0.0174532925f;
        const float dx = std::sin(a), dz = std::cos(a);
        CHECK(Gauge(10.0f * dx, 10.0f * dz, R, 0.0f, 1.0f, e) == Approx(10.0f / RateToward(h, dx, dz)).epsilon(1e-4));
    }
    // A segment's minimum is no later than any point on it and equals the best of many.
    float best = 1e9f;
    for (float s = -20.0f; s <= 20.0f; s += 0.01f)
        best = std::min(best, Gauge(3.0f - s * 0.8f, 30.0f - s * 0.6f, R, 0.0f, 1.0f, e));
    CHECK(SegmentGauge(3.0f, 30.0f, 0.8f, 0.6f, 20.0f, R, 0.0f, 1.0f, e) == Approx(best).epsilon(1e-3));
    // The support speed: the head's rate at the head, b / t square to it
    // (sqrt((1 - e) / (1 + e)) of the head: a line fire's flank), the back's at the back.
    CHECK(FrontSpeed(R, 0.0f, 1.0f, e, 0.0f, 1.0f) == Approx(R));
    CHECK(FrontSpeed(R, 0.0f, 1.0f, e, 1.0f, 0.0f) == Approx(R * std::sqrt((1.0f - e) / (1.0f + e))).epsilon(1e-4));
    CHECK(FrontSpeed(R, 0.0f, 1.0f, e, 0.0f, -1.0f) == Approx(R * (1.0f - e) / (1.0f + e)).epsilon(1e-4));
    // The inverse of the build-up's run.
    for (float age : {0.0f, 20.0f, 300.0f})
        for (float run : {0.5f, 5.0f, 60.0f})
            CHECK(BuildUpRun(age, BuildUpTime(age, run, kBuildUpPerMin), kBuildUpPerMin) == Approx(run).epsilon(1e-3));
}

TEST_CASE("a line fire's sides diverge as the model's ellipse says and its head runs at the model's rate",
          "[firespread]")
{
    // A 160 m line, 11 m/s north, no build-up, no noise: past the line's
    // ends the sides open at atan(b / c) of the model's ellipse (9.6 deg for
    // the grass length to breadth 6.06; the consult and the checklist: 7 to
    // 12; the lattice gave about 3), and the head runs at the model's rate.
    Params p = Worst();
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(-80.0f, 4.0f, 80.0f, 4.0f) > 0);
    for (int k = 0; k < 4 * 120; ++k)
        g.Tick(0.25f, 0.0f, 11.0f);
    const HeadFire h = SolveHead(SolveSurface(1, Worst().moisture), 0.0f, 0.36f * 11.0f, 0.0f, 0.0f, 0.36f, true);
    const float t = static_cast<float>(g.DisplayTime());
    // The head on the line's middle.
    float head = 0.0f;
    for (float z = 4.0f; z < 400.0f; z += 0.25f)
        if (g.DrawnAgeAt(0.0f, z) >= 0.0f)
            head = z - 4.0f;
    // The east side's widest reach past the line's end (x 80).
    float widest = 0.0f, widestZ = 0.0f;
    for (float z = 4.0f; z < 300.0f; z += 1.0f)
        for (float x = 80.0f; x < 140.0f; x += 0.25f)
            if (g.DrawnAgeAt(x, z) >= 0.0f && x - 80.0f > widest)
            {
                widest = x - 80.0f;
                widestZ = z - 4.0f;
            }
    // The model's: the end's ellipse is widest b = R t sqrt(1 - e^2) / (1 + e)
    // out, c = R t e / (1 + e) on, so the side opens at atan(b / c).
    const float e = h.eccentricity, run = h.rateMps * t;
    const float b = run * std::sqrt(1.0f - e * e) / (1.0f + e), c = run * e / (1.0f + e);
    const float side = std::atan2(widest, c) * 57.29578f, model = std::atan2(b, c) * 57.29578f;
    INFO("head " << head << " (model " << run << ") widest " << widest << " at " << widestZ << " (model " << b << " at "
                 << c << "): side " << side << " deg (model " << model << ")");
    CHECK(head == Approx(run).margin(3.0f));
    CHECK(widest == Approx(b).epsilon(0.15));
    CHECK(side > 7.0f);
    CHECK(side < 12.0f);
}

TEST_CASE("the drawn front is continuous across cells: no stair steps, no square cells", "[firespread]")
{
    // A diagonal wind over a line fire: along a row crossing many cells the
    // arrival field never jumps at a cell's edge (neighbouring points a
    // little apart in time, as the front's speed says).
    Params p = Worst();
    p.spotting = false;
    p.buildUpPerMin = 0.0f;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(-80.0f, 4.0f, 80.0f, 4.0f) > 0);
    for (int k = 0; k < 4 * 60; ++k)
        g.Tick(0.25f, 4.0f, 10.0f);
    float worst = 0.0f, wx = 0.0f, wz = 0.0f, wa = 0.0f;
    int samples = 0;
    for (float z : {30.0f, 50.0f, 70.0f})
    {
        float prev = -1e9f;
        for (float x = -60.0f; x <= 60.0f; x += 0.25f)
        {
            const float a = g.DrawnAgeAt(x, z);
            if (a > -1e8f && prev > -1e8f)
            {
                if (std::fabs(a - prev) > worst)
                {
                    worst = std::fabs(a - prev);
                    wx = x;
                    wz = z;
                    wa = a;
                }
                samples++;
            }
            prev = a;
        }
    }
    INFO("samples " << samples << " worst jump " << worst << " s over 0.25 m at " << wx << "," << wz << " age " << wa);
    CHECK(samples > 900);
    // (the edge noise warps the field by up to a metre and a half over
    // metres; the head crosses that in under a second, a flank in seconds)
    CHECK(worst < 3.0f);
}

TEST_CASE("an ignition line's cell has a head phase with a normal window, then a low backing phase", "[firespread]")
{
    // The realism audit's C6 (fix 5 builds its light on this): the line's
    // cells no longer flame for minutes at full strength. The head leaves the
    // line in seconds (window: its crossing and the residence); the cell then
    // shows only the backing front creeping upwind (flames a few tenths of a
    // metre, tens of kW/m).
    Params p = Worst();
    p.spotting = false;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(-80.0f, 2.0f, 80.0f, 2.0f) > 0); // 2 m upwind of the cells' centres
    BurningView v;
    for (int k = 0; k < 4 * 2; ++k)
        g.Tick(0.25f, 0.0f, 11.0f);
    REQUIRE(g.ViewAt(4.0f, 4.0f, v));
    INFO("head phase: life " << v.life << " cross " << v.crossSec << " flame " << v.flameMaxM << " I " << v.intensityKwM
                             << " strength " << v.strength);
    CHECK_FALSE(v.backing);
    CHECK(v.life < 20.0f);
    CHECK(v.flameMaxM > 1.5f);
    CHECK(v.frontZ > 0.99f);
    for (int k = 0; k < 4 * 40; ++k)
        g.Tick(0.25f, 0.0f, 11.0f);
    REQUIRE(g.ViewAt(4.0f, 4.0f, v));
    INFO("backing phase: flame " << v.flameMaxM << " I " << v.intensityKwM << " front " << v.frontX << "," << v.frontZ);
    CHECK(v.backing);
    CHECK(v.flameMaxM < 0.5f);
    CHECK(v.intensityKwM < 60.0f);
    CHECK(v.frontZ < -0.99f);
    // Its points behind the line have not burnt yet; ahead of it they have.
    CHECK(FrontAgeAt(v, 0.0f, -3.5f, 8.0f) < 0.0f);
    CHECK(FrontAgeAt(v, 0.0f, 3.5f, 8.0f) > 30.0f);
}

namespace
{
// Grass with an east-west road along z = zRoad whose bare running surface is
// bareM wide (FuelAt's rule: a cell on it is bare, every cell near it keeps its line).
FireGrid::FuelQuery RoadFuel(float zRoad, float bareM)
{
    return [zRoad, bareM](float, float z)
    {
        Fuel f;
        f.model = 1;
        if (std::fabs(z - zRoad) < 14.0f)
        {
            f.roadOffX = 0.0f;
            f.roadOffZ = zRoad - z;
            f.roadDirX = 1.0f;
            f.roadDirZ = 0.0f;
            f.roadHalfM = 0.5f * bareM;
            if (std::fabs(z - zRoad) < 0.5f * bareM)
                f.model = 0;
        }
        return f;
    };
}
} // namespace

TEST_CASE("a road is a barrier: verges burn to the bare edge, flames cross only within reach", "[firespread]")
{
    // The fire science consult's ninth answer. A 160 m head runs north into
    // an east-west road 40 m on (no embers: spotting off).
    for (float bare : {5.6f, 2.2f})
    {
        INFO("bare " << bare);
        Params p = Worst();
        p.spotting = false;
        p.buildUpPerMin = 0.0f;
        p.fuelNoise = 0.0f;
        const float zRoad = 44.0f;
        FireGrid g(p, RoadFuel(zRoad, bare), [](float, float) { return 0.0f; });
        REQUIRE(g.IgniteLine(-80.0f, 4.0f, 80.0f, 4.0f) > 0);
        for (int k = 0; k < 4 * 90; ++k)
            g.Tick(0.25f, 0.0f, 11.0f);
        // The near verge burns up to the bare edge (just short of it has
        // burnt), the bare surface never does.
        const float edge = zRoad - 0.5f * bare;
        int verge = 0, road = 0, far = 0;
        for (float x = -60.0f; x <= 60.0f; x += 1.0f)
        {
            verge += g.DrawnAgeAt(x, edge - 0.6f) >= 0.0f ? 1 : 0;
            road += g.DrawnAgeAt(x, zRoad) >= 0.0f ? 1 : 0;
            far += g.DrawnAgeAt(x, zRoad + 20.0f) >= 0.0f ? 1 : 0;
        }
        INFO("verge " << verge << " road " << road << " far " << far);
        CHECK(verge > 110);
        CHECK(road == 0);
        if (bare > 3.0f)
            CHECK(far == 0); // a 5.6 m gap: the head's 2.6 m flames reach about 3 m (an 8 m road, 0.7 of it bare)
        else
            CHECK(far > 100); // a 2.2 m gap: flame contact, the head runs on
    }
}

TEST_CASE("embers cross a road at the consult's rates", "[firespread]")
{
    // The per-metre rate (the consult's checks: a head square on an 8 m
    // gap 17% a 100 m, 5.5 m 36%, 12 m 5%; a flank under 0.1%).
    auto per100 = [](float I, float gapEff, float reach)
    { return 1.0f - std::exp(-100.0f * RoadEmberRate(I, 11.0f, gapEff, reach, 0.945f)); };
    CHECK(per100(2000.0f, 8.0f, 3.1f) == Approx(0.17f).margin(0.02f));
    CHECK(per100(2000.0f, 5.5f, 3.1f) == Approx(0.36f).margin(0.03f));
    CHECK(per100(2000.0f, 12.0f, 3.1f) == Approx(0.05f).margin(0.01f));
    CHECK(per100(100.0f, GapAlongWindM(5.5f, 0.0f), 0.3f) < 0.001f);
    CHECK(RoadEmberRate(2000.0f, 4.0f, 8.0f, 3.1f, 0.945f) == 0.0f); // no embers under 5 m/s
    CHECK(GapAlongWindM(5.5f, 1.0f) == Approx(5.5f));
    CHECK(GapAlongWindM(5.5f, 0.0f) == Approx(5.5f / 0.3f));
    // On the grid: 100 m heads meeting a road with a 5.6 m bare surface
    // square on (the showcase's 8 m road) breach it as one or a few spot
    // fires beyond the far edge, about a third of them (the consult: 35 to
    // 45% for 5.5 m), never by flame.
    int breached = 0, spots = 0, draws = 0;
    const int runs = 40;
    for (int r = 0; r < runs; ++r)
    {
        Params p = Worst();
        p.buildUpPerMin = 0.0f;
        p.fuelNoise = 0.0f;
        p.seed = 1000u + static_cast<std::uint32_t>(r) * 7919u;
        const float zRoad = 36.0f;
        FireGrid g(p, RoadFuel(zRoad, 5.6f), [](float, float) { return 0.0f; });
        REQUIRE(g.IgniteLine(-50.0f, 4.0f, 50.0f, 4.0f) > 0);
        for (int k = 0; k < 4 * 70; ++k)
            g.Tick(0.25f, 0.0f, 11.0f);
        const Stats& s = g.GetStats();
        draws += s.roadDraws;
        spots += s.roadSpots;
        breached += s.roadSpots > 0 ? 1 : 0;
    }
    INFO("breached " << breached << " of " << runs << ", spot fires " << spots << ", cells drawing " << draws);
    CHECK(draws > 10 * runs);
    CHECK(breached >= 6);
    CHECK(breached <= 24);
}

TEST_CASE("the wind wanders 10 to 15 degrees over minutes, smoothly", "[firespread]")
{
    double sum = 0.0, sum2 = 0.0, worstRate = 0.0;
    double prev = WindWanderRad(0.0, 7u, kWindWanderDeg) * 57.29578;
    const int n = 4 * 3600 * 4;
    for (int k = 1; k <= n; ++k)
    {
        const double a = WindWanderRad(0.25 * k, 7u, kWindWanderDeg) * 57.29578;
        sum += a;
        sum2 += a * a;
        worstRate = std::max(worstRate, std::fabs(a - prev) / 0.25);
        prev = a;
    }
    const double mean = sum / n, sd = std::sqrt(sum2 / n - mean * mean);
    INFO("mean " << mean << " sd " << sd << " deg, fastest " << worstRate << " deg/s");
    CHECK(sd > 10.0);
    CHECK(sd < 15.0);
    CHECK(std::fabs(mean) < 3.0);
    CHECK(worstRate < 1.5);
    CHECK(WindWanderRad(100.0, 7u, 0.0f) == 0.0f);
}

TEST_CASE("fuel noise: patchy rates about the model's, fixed per cell", "[firespread]")
{
    double sum = 0.0, sum2 = 0.0;
    float lo = 1e9f, hi = 0.0f;
    bool fixed = true;
    const int n = 200;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
        {
            const float k = FuelNoiseFactor(i, j, 0x5eed1234u, 1.0f);
            const double l = std::log(k);
            sum += l;
            sum2 += l * l;
            lo = std::min(lo, k);
            hi = std::max(hi, k);
            fixed = fixed && k == FuelNoiseFactor(i, j, 0x5eed1234u, 1.0f);
        }
    const double mean = sum / (n * n), sd = std::sqrt(sum2 / (n * n) - mean * mean);
    INFO("log mean " << mean << " sd " << sd << " range " << lo << ".." << hi);
    CHECK(fixed);
    CHECK(std::fabs(mean) < 0.03);
    CHECK(sd > 0.07);
    CHECK(sd < 0.16);
    CHECK(lo > 0.6f);
    CHECK(hi < 1.6f);
    CHECK(FuelNoiseFactor(3, 4, 1u, 0.0f) == 1.0f);
}

TEST_CASE("an inner cell goes out once the drawn front has crossed its square", "[firespread]")
{
    // A line fire in grass under a wind that turns a little. A cell whose
    // neighbours are all caught burns on only until the drawn front (the
    // arrival between the centres) has crossed its whole square and its
    // residence is over, whatever its own source said of its far corner (a
    // narrow head's far side, reached backwards from it, used to keep such
    // cells alight for minutes in the black). The flanks' cells, whose
    // outer neighbours are caught only as the front enters them, burn on
    // while their slow front crosses. (The dance fix: the arrival between
    // the centres the cell burns out by is the model's own, CentreAgeAt;
    // its view draws each neighbour's at most as late, Cell::drawnArrival,
    // so its drawn front is never behind it.)
    Params p = Worst();
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(-60.0f, 4.0f, 60.0f, 4.0f) > 0);
    int checked = 0, slow = 0, drawnBehind = 0;
    for (int k = 0; k < 4 * 240; ++k)
    {
        const float a = 0.2f * std::sin(0.01f * static_cast<float>(k));
        g.Tick(0.25f, 11.0f * std::sin(a), 11.0f * std::cos(a));
        if (k % 40 != 39)
            continue;
        std::vector<BurningView> b;
        g.CollectBurning(b);
        for (const BurningView& v : b)
        {
            // (not the ignition line's cells nor those drawing its own field)
            if (v.exact || v.backing || std::fabs(v.z - 4.0f) < kSeedExactM + 8.0f + 4.0f)
                continue;
            bool enclosed = true;
            for (int dj = -1; dj <= 1 && enclosed; ++dj)
                for (int di = -1; di <= 1 && enclosed; ++di)
                {
                    const CellState s =
                        g.StateAt(v.x + 8.0f * static_cast<float>(di), v.z + 8.0f * static_cast<float>(dj));
                    enclosed = s == CellState::Burning || s == CellState::Burnt;
                }
            if (!enclosed)
                continue;
            // its square's last corner: the mean of the four centres round it
            // (the model's centre times, and as its view draws them)
            float centre[9];
            for (int dj = -1; dj <= 1; ++dj)
                for (int di = -1; di <= 1; ++di)
                    centre[(dj + 1) * 3 + di + 1] =
                        g.CentreAgeAt(v.x + 8.0f * static_cast<float>(di), v.z + 8.0f * static_cast<float>(dj));
            float last = 1e9f, drawnLast = 1e9f;
            for (int sz = 0; sz <= 1; ++sz)
                for (int sx = 0; sx <= 1; ++sx)
                {
                    const int c0 = sz * 3 + sx, c1 = c0 + 1, c2 = c0 + 3, c3 = c0 + 4;
                    last = std::min(last, 0.25f * (centre[c0] + centre[c1] + centre[c2] + centre[c3]));
                    drawnLast = std::min(drawnLast, 0.25f * (v.nbAge[c0] + v.nbAge[c1] + v.nbAge[c2] + v.nbAge[c3]));
                }
            ++checked;
            if (last > v.residenceSec + kEntryNoiseMaxSec + 1.0f)
                ++slow;
            if (drawnLast < last - 0.01f)
                ++drawnBehind;
        }
    }
    CHECK(checked > 200);
    CHECK(slow == 0);
    CHECK(drawnBehind == 0);
}

TEST_CASE("the fire's light follows its drawn flames (the showcase audit's fix 5)", "[firespread]")
{
    // A grass head cell crossed at 1.9 m/s: its flames, side on, about a
    // full 2.6 m head's wall over its 8 m (the consult: 100 m of head about
    // 750000 cd, so 1100 lux 10 m off and 0.75 lux 1 km off).
    BurningView v;
    v.x = 4004.0f;
    v.z = 10004.0f;
    v.y = 100.0f;
    v.flameMaxM = 2.6f;
    v.crossSec = 8.0f / 1.9f;
    v.frontX = 0.0f;
    v.frontZ = 1.0f;
    v.residenceSec = 6.6f;
    v.model = 1;
    const float tr = DrawResidenceSec(1, 6.6f);
    const float midflame = 4.0f;
    float peak = 0.0f, peakH = 0.0f;
    for (float a = 0.0f; a < 20.0f; a += 0.1f)
    {
        v.age = a;
        const CellLight l = CellFlameLight(v, tr, 8.0f, midflame);
        if (l.cd > peak)
        {
            peak = l.cd;
            peakH = l.heightM;
        }
    }
    // (the head's 2.6 m flames lean about 48 degrees: 1.7 m tall)
    CHECK(peakH > 1.2f);
    CHECK(peakH < 2.2f);
    CHECK(peak > 35000.0f);
    CHECK(peak < 90000.0f);
    const float head100 = peak * 100.0f / 8.0f;
    CHECK(head100 / 1.0e6f > 0.4f); // lux at 1 km
    CHECK(head100 / 1.0e6f < 1.2f);
    // Nothing before the front reaches it.
    v.age = -1.0f;
    CHECK(CellFlameLight(v, tr, 8.0f, midflame).cd == 0.0f);

    // The ignition line's cell (lit all at once, no front, a model window of
    // minutes): its light goes with its drawn band and tail (kTailEnd tr,
    // about 23 s), then only its flamelets; the ash's glow fades too.
    BurningView ig = v;
    ig.crossSec = 0.0f;
    ig.frontX = ig.frontZ = 0.0f;
    float igPeak = 0.0f, emberPeak = 0.0f;
    for (float a = 0.0f; a < 10.0f; a += 0.1f)
    {
        ig.age = a;
        const CellLight l = CellFlameLight(ig, tr, 8.0f, midflame);
        igPeak = std::max(igPeak, l.cd);
        emberPeak = std::max(emberPeak, l.ember);
    }
    CHECK(igPeak > 20000.0f);
    CHECK(emberPeak > 0.5f);
    ig.age = 30.0f;
    CHECK(CellFlameLight(ig, tr, 8.0f, midflame).cd < 0.05f * igPeak);
    ig.age = 150.0f;
    const CellLight late = CellFlameLight(ig, tr, 8.0f, midflame);
    CHECK(late.cd < 0.01f * igPeak);
    CHECK(late.ember < 0.01f);

    // Followed (UpdateLights) at 30 frames a second, the light never steps:
    // under a tenth of its peak from one frame to the next, a cell lit all at
    // once included; it takes a second or more to reach half its peak.
    float cur = 0.0f, worst = 0.0f, half = -1.0f;
    for (int f = 0; f < 900; ++f)
    {
        ig.age = f / 30.0f;
        const float next = LightFollow(cur, CellFlameLight(ig, tr, 8.0f, midflame).cd, 1.0f / 30.0f, kLightAttackSec,
                                       kLightReleaseSec);
        worst = std::max(worst, std::fabs(next - cur));
        if (half < 0.0f && next > 0.5f * igPeak)
            half = ig.age;
        cur = next;
    }
    CHECK(worst < 0.1f * igPeak);
    CHECK(half > 0.8f);

    // A torching 12 m pine: about 15 m by 3 to 4 m of flame at 10000 cd/m2
    // (the consult: 40 lux on a wall 100 m off), dimmer while it grows.
    const float torch = TorchLightCd(18.0f, 18.0f, 12.0f);
    CHECK(torch / 1.0e4f > 25.0f);
    CHECK(torch / 1.0e4f < 70.0f);
    CHECK(TorchLightCd(9.0f, 18.0f, 12.0f) < 0.5f * torch);
    CHECK(TorchLightCd(0.0f, 18.0f, 12.0f) == 0.0f);
    // A flaring 2.8 m thicket (5.5 m of flame at 45 degrees over 4.8 m).
    const float flare = FlareLightCd(5.5f, 4.8f, 0.785f);
    CHECK(flare > 50000.0f);
    CHECK(flare < 150000.0f);
}

TEST_CASE("the ground's halo lights the near field and ends smoothly (the showcase audit's fix 5)", "[firespread]")
{
    // A line of full head cells (60000 cd each, 8 m apart): level ground
    // 10 m off takes tens of lux from the halo (the block lights add about
    // 25 more), 30 m off nothing (the block lights carry it there), and the
    // halo falls smoothly, never a ring.
    auto line = [](float off)
    {
        float lux = 0.0f;
        for (int k = -20; k <= 20; ++k)
            lux += HaloLux(60000.0f, std::sqrt(off * off + (8.0f * k) * (8.0f * k)));
        return lux;
    };
    CHECK(line(10.0f) > 25.0f);
    CHECK(line(10.0f) < 90.0f);
    CHECK(line(3.0f) > 300.0f);
    CHECK(line(30.0f) == 0.0f);
    float prev = HaloLux(60000.0f, 0.0f);
    for (float r = 0.25f; r <= kHaloReachM + 1.0f; r += 0.25f)
    {
        const float h = HaloLux(60000.0f, r);
        CHECK(h <= prev);
        CHECK(prev - h < 0.12f * HaloLux(60000.0f, 0.0f));
        prev = h;
    }
    CHECK(HaloLux(60000.0f, kHaloReachM) == 0.0f);
}

TEST_CASE("the fire's lights are picked with hysteresis (the showcase audit's fix 5)", "[firespread]")
{
    LightPick pick;
    std::vector<LightCandidate> c;
    for (int i = 0; i < kFireLightsMax; ++i)
        c.push_back({static_cast<std::uint64_t>(i + 1), 10.0f + static_cast<float>(i)});
    pick.Update(c.data(), static_cast<int>(c.size()), 1.0f / 30.0f);
    for (const LightCandidate& k : c)
        CHECK(pick.Has(k.id));
    // A newcomer a little stronger than the weakest never takes its place.
    c.push_back({100, 14.0f});
    for (int f = 0; f < 300; ++f)
        pick.Update(c.data(), static_cast<int>(c.size()), 1.0f / 30.0f);
    CHECK_FALSE(pick.Has(100));
    CHECK(pick.Has(1));
    // One far stronger takes it, but only after kLightSwapSec.
    c.back().score = 40.0f;
    int frames = 0;
    while (!pick.Has(100) && frames < 300)
    {
        pick.Update(c.data(), static_cast<int>(c.size()), 1.0f / 30.0f);
        ++frames;
    }
    CHECK(pick.Has(100));
    CHECK_FALSE(pick.Has(1));
    CHECK(static_cast<float>(frames) / 30.0f >= kLightSwapSec - 0.05f);
    // A source that goes out leaves at once, and its place goes to the next.
    c[2].score = 0.0f;
    pick.Update(c.data(), static_cast<int>(c.size()), 1.0f / 30.0f);
    CHECK_FALSE(pick.Has(3));
    CHECK(pick.Has(1));
}

TEST_CASE("the fire's light flickers at a flame's tempo (the showcase audit's fix 5)", "[firespread]")
{
    // Zero mean, about 0.4 standard deviation, crossing its mean three to
    // six times a second (1.5 to 3 Hz), never the same for two seeds.
    for (std::uint32_t seed : {1U, 77U, 4242U})
    {
        double sum = 0.0, sum2 = 0.0;
        int crossings = 0;
        float prev = LightFlicker(0.0f, seed);
        const int n = 6000; // 60 s at 100 Hz
        for (int i = 1; i <= n; ++i)
        {
            const float v = LightFlicker(i * 0.01f, seed);
            sum += v;
            sum2 += static_cast<double>(v) * v;
            crossings += (v > 0.0f) != (prev > 0.0f) ? 1 : 0;
            prev = v;
        }
        const double mean = sum / n, sd = std::sqrt(sum2 / n - mean * mean);
        CHECK(std::fabs(mean) < 0.05);
        CHECK(sd > 0.3);
        CHECK(sd < 0.5);
        CHECK(crossings > 150);
        CHECK(crossings < 420);
    }
    CHECK(LightFlicker(1.234f, 1U) != LightFlicker(1.234f, 2U));
}

TEST_CASE("a torching tree's column: crown-wide, necking, its tip 1.5 h up, leaning at most 50 degrees (fix 6)",
          "[firespread]")
{
    // A lone 12 m pine (crown 6 m wide) in an 11 m/s wind: its tip stands 1.5
    // tree heights above the ground (vertically, not along the lean).
    const TorchColumn c = TorchColumnOf(12.0f, 6.0f, kTorchRatio, 11.0f);
    CHECK(c.tipM == Approx(18.0f));
    CHECK(c.crownBaseM == Approx(kTorchFootShare * 12.0f));
    // It leans by the tilt law in the wind at mid-crown (a little under the
    // 10 m wind) and the tree's full flame, never past 50 degrees, and less
    // than with the 10 m wind and the growing flame the drawing used.
    CHECK(c.windMps < 11.0f);
    CHECK(c.windMps > 9.0f);
    CHECK(c.tilt <= kTorchTiltMax);
    CHECK(c.tilt > 0.7f); // about 40 degrees at least
    CHECK(c.tilt < FlameTiltRad(11.0f, 0.5f * (kTorchRatio - kTorchFootShare) * 12.0f));
    // A group's taller flame leans less; in a light wind it stands up.
    CHECK(TorchColumnOf(12.0f, 6.0f, 2.2f, 11.0f).tilt < c.tilt - 0.08f);
    CHECK(TorchColumnOf(12.0f, 6.0f, kTorchRatio, 3.0f).tilt < 0.5f);
    CHECK(TorchColumnOf(12.0f, 6.0f, kTorchRatio, 40.0f).tilt == Approx(kTorchTiltMax));
    // Crown-wide in the crown, necking above its top to 0.5 to 0.7 of the
    // crown's width, tapering to nothing at the tip.
    CHECK(TorchColumnRadius(c, c.crownBaseM) == Approx(3.0f));
    CHECK(c.neckM > 12.0f);
    CHECK(c.neckM < c.tipM);
    const float neckW = 2.0f * TorchColumnRadius(c, c.neckM) / 6.0f;
    CHECK(neckW >= 0.5f);
    CHECK(neckW <= 0.7f);
    float prev = TorchColumnRadius(c, c.neckM);
    for (float z = c.neckM + 0.5f; z < c.tipM; z += 0.5f)
    {
        const float r = TorchColumnRadius(c, z);
        CHECK(r < prev);
        prev = r;
    }
    CHECK(TorchColumnRadius(c, c.tipM) == Approx(0.0f));
    // The axis rises steeply over the crown and bends over higher up (a
    // bent-over plume), its chord from mid-crown to the tip at the tilt.
    CHECK(TorchColumnOffset(c, c.axisM) == Approx(0.0f));
    CHECK(TorchColumnOffset(c, c.tipM) == Approx((c.tipM - c.axisM) * std::tan(c.tilt)));
    const float zHalf = 0.5f * (c.axisM + c.tipM);
    CHECK(TorchColumnOffset(c, zHalf) < 0.5f * TorchColumnOffset(c, c.tipM));
    // A group's taller flame (2 h) keeps the same neck; an olive's short one
    // (1.2 h) necks between the tree's top and its tip.
    CHECK(TorchColumnOf(12.0f, 6.0f, 2.0f, 11.0f).tipM == Approx(24.0f));
    const TorchColumn olive = TorchColumnOf(4.0f, 2.0f, kOliveTorchRatio, 11.0f);
    CHECK(olive.neckM > 4.0f);
    CHECK(olive.neckM < olive.tipM);
    // A volume's puff is a bulge climbing it, never a stretch of the body:
    // one number holds the wave's length (0.1 m), the bulge's size (steps of
    // kVolumeBulgeStep) and the phase at the foot; the phase wraps, and a
    // tongue without a bulge sends 0.
    CHECK(FlameSheet{}.volumeBulge == 0.0f);
    CHECK(VolumePuffCode(0.25f, 14.7f, 0.0f) == 0.0f);
    CHECK(VolumePuffCode(0.25f, 0.0f, 0.16f) == 0.0f);
    {
        const float code = VolumePuffCode(3.25f, 14.7f, 0.16f);
        const float whole = std::floor(code);
        CHECK(std::floor(whole / 2048.0f) == 8.0f);         // 0.16 in steps of 0.02
        CHECK(whole - 2048.0f * 8.0f == 147.0f);            // 14.7 m
        CHECK(code - whole == Approx(0.25f).margin(0.01f)); // the phase, wrapped
        CHECK(VolumePuffCode(-0.25f, 14.7f, 0.16f) - whole == Approx(0.75f).margin(0.01f));
        CHECK(VolumePuffCode(0.9999f, 14.7f, 0.16f) < whole + 1.0f);
        CHECK(VolumePuffCode(0.0f, 500.0f, 1.0f) == 15.0f * 2048.0f + 2047.0f);
    }
    // Big tongues are volumes; a ribbon is the default.
    CHECK(FlameSheet{}.volumeK == 0.0f);
    CHECK(kVolumeFromM <= 2.0f);
}

TEST_CASE("the burning bed: brightest at the front, gone over the back third (the realism audit's fix 7)",
          "[firespread]")
{
    CHECK(BedHeat(0.0f) == 0.0f);
    CHECK(BedHeat(1.0f) == 0.0f);
    CHECK(BedHeat(0.05f) == Approx(1.0f));
    CHECK(BedHeat(kBedFrontShare) == Approx(1.0f));
    CHECK(BedHeat(kBedBackFrom) == Approx(1.0f - kBedFrontDrop).margin(1e-3f));
    CHECK(BedHeat(0.83f) < 0.5f);
    CHECK(BedHeat(0.83f) > 0.2f);
    CHECK(BedHeat(0.99f) < 0.01f);
    for (float x = 0.05f; x < 0.95f; x += 0.05f)
        CHECK(BedHeat(x + 0.05f) <= BedHeat(x) + 1e-6f);
    // The bed is the procedural kind past the flipbook's styles.
    CHECK(static_cast<int>(FlameStyle::Bed) == kFlameStyles);
    CHECK(kFlameKinds == kFlameStyles + 1);
}

TEST_CASE("a head's rows stagger, so from downwind the tongues fill each other's gaps (the realism audit's fix 7)",
          "[firespread]")
{
    // A head cell in its band: each row's tongues stand half a spacing along
    // from the row before's (a straight grid lined every row up behind the
    // first: a picket fence seen end on), measured as the circular mean of
    // each row's positions over the spacing; the tongues' full lengths spread
    // about 0.17 L.
    BurningView v;
    v.x = 4004.0f;
    v.z = 10004.0f;
    v.flameMaxM = 2.6f;
    v.crossSec = 8.0f / 1.9f;
    v.frontX = 0.0f;
    v.frontZ = 1.0f;
    v.residenceSec = 6.6f;
    v.model = 1;
    v.age = 5.0f;
    const float tr = DrawResidenceSec(1, 6.6f);
    std::vector<SurfaceTongue> now;
    const SurfaceGrid grid = CellSurfaceTongues(v, tr, 8.0f, 777U, 0.0f, 1.0f, 20.0f, 0.0f, now);
    REQUIRE(now.size() > 40);
    std::map<int, std::pair<double, double>> phase; // by row: the sum of the positions' unit vectors on the spacing
    std::map<int, int> count;
    double sum = 0.0, sum2 = 0.0;
    for (const SurfaceTongue& s : now)
    {
        const int row = static_cast<int>(std::floor(s.oz / grid.a));
        const double a = 6.283185307 * static_cast<double>(s.ox) / static_cast<double>(grid.b);
        phase[row].first += std::cos(a);
        phase[row].second += std::sin(a);
        count[row]++;
        sum += s.lmax;
        sum2 += static_cast<double>(s.lmax) * s.lmax;
    }
    // (each row's phase is a mean over about ten tongues jittered a third of
    // a spacing either way, so one pair can be off by a tenth of a turn or
    // so: the mean over the pairs is the test)
    int pairs = 0;
    double turnsSum = 0.0;
    for (auto it = phase.begin(); std::next(it) != phase.end(); ++it)
    {
        const auto nx = std::next(it);
        if (count[it->first] < 6 || count[nx->first] < 6 || nx->first != it->first + 1)
            continue;
        const double d =
            std::atan2(nx->second.second, nx->second.first) - std::atan2(it->second.second, it->second.first);
        const double turns = std::fabs(std::remainder(d / 6.283185307, 1.0)); // 0 lined up, 0.5 staggered
        INFO("rows " << it->first << " turns " << turns);
        CHECK(turns > 0.15);
        turnsSum += turns;
        ++pairs;
    }
    REQUIRE(pairs >= 3);
    CHECK(turnsSum / pairs > 0.35);
    const double n = static_cast<double>(now.size());
    const double mean = sum / n, sd = std::sqrt(std::max(0.0, sum2 / n - mean * mean));
    CHECK(sd / mean == Approx(0.17).margin(0.03));
}

TEST_CASE("a slow front's band is deep enough to stand as an unbroken line (the realism audit's fix 7)", "[firespread]")
{
    // The drawn residence is lengthened only where rate x residence is under
    // kBandMinRows rows of tongues: the showcase's back (0.013 m/s, 0.26 m
    // flames, rows 0.25 m apart) burns a band 0.4 m deep instead of 0.09 m; a
    // flank (0.16 m/s, 0.8 m, rows 0.4 m) and a head (1.9 m/s, 2.6 m, rows 1 m)
    // keep the model's own residence; nothing past kBandMaxSec, a stopped front
    // never forever.
    const float tr = DrawResidenceSec(1, 6.6f);
    const float back = BandResidenceSec(tr, 0.013f, 0.26f);
    CHECK(back * 0.013f == Approx(kBandMinRows * 0.25f).margin(1e-3f));
    CHECK(back > tr);
    CHECK(BandResidenceSec(tr, 0.16f, 0.8f) == Approx(tr));
    CHECK(BandResidenceSec(tr, 1.9f, 2.6f) == Approx(tr));
    CHECK(BandResidenceSec(tr, 0.0f, 0.3f) == Approx(kBandMinRows * 0.25f / kBandMinRateMps));
    CHECK(BandResidenceSec(tr, 0.0f, 0.3f) <= kBandMaxSec);
    CHECK(BandResidenceSec(tr, 0.0f, 2.0f) == Approx(kBandMaxSec));
    CHECK(BandResidenceSec(40.0f, 0.0f, 0.3f) >= 40.0f);
}

TEST_CASE("sparks rise, drift and count as the consult's eighth round says", "[firespread]")
{
    // Rise inside a flame 3 sqrt(L): 4.8 m/s in a 2.6 m grass flame, 11 to 12
    // m/s in a 15 m torch (not 2 + 1.5 L, 24.5 m/s).
    CHECK(SparkRiseMps(2.6f) == Approx(4.84f).margin(0.05f));
    CHECK(SparkRiseMps(15.0f) == Approx(11.6f).margin(0.1f));
    // The wind at its height: about 0.5 of the 10 m wind within a metre of the
    // ground, 0.8 at 3 m, 1.0 at 10 m, 1.1 to 1.2 at 30 m.
    CHECK(SparkWindShare(0.5f) == Approx(0.5f).margin(0.08f));
    CHECK(SparkWindShare(1.0f) == Approx(0.55f).margin(0.08f));
    CHECK(SparkWindShare(3.0f) == Approx(0.8f).margin(0.05f));
    CHECK(SparkWindShare(10.0f) == Approx(1.0f));
    CHECK(SparkWindShare(30.0f) == Approx(1.15f).margin(0.07f));
    // In the air at once: a grass head 5 to 20 a metre near the eye (about 5
    // to 100 m), a flank's 0.65 m flame 0.5 to 2; grass sparks live 0.5 to 2
    // s, a flank's 0.3 to 1 s.
    CHECK(FrontSparksPerM(2.6f) >= 5.0f);
    CHECK(FrontSparksPerM(2.6f) * SparkNearBoost(10.0f) <= 20.0f);
    CHECK(FrontSparksPerM(2.6f) * SparkNearBoost(10.0f) > FrontSparksPerM(2.6f));
    CHECK(SparkNearBoost(60.0f) == Approx(1.0f));
    CHECK(FrontSparksPerM(0.65f) >= 0.5f);
    CHECK(FrontSparksPerM(0.65f) <= 2.0f);
    CHECK(FrontSparkLifeSec(2.6f, 0.0f) == Approx(0.5f));
    CHECK(FrontSparkLifeSec(2.6f, 1.0f) == Approx(2.0f));
    CHECK(FrontSparkLifeSec(0.65f, 1.0f) == Approx(1.0f));
    CHECK(FrontSparkLifeSec(0.65f, 0.0f) >= 0.2f);
    // A thicket's burst 50 to 150 (a 2.4 m bush, 4.8 m flames), a reed
    // clump's 100 to 300, a torching pine's 300 to 1000 at its peak.
    CHECK(FlareSparkPeak(SparkKind::Thicket, 4.8f) >= 50.0f);
    CHECK(FlareSparkPeak(SparkKind::Thicket, 4.8f) <= 150.0f);
    CHECK(FlareSparkPeak(SparkKind::Reed, 3.0f) >= 100.0f);
    CHECK(FlareSparkPeak(SparkKind::Reed, 3.0f) <= 300.0f);
    CHECK(TorchSparkPeak(13.9f) >= 300.0f);
    CHECK(TorchSparkPeak(13.9f) <= 1000.0f);
    CHECK(TorchSparkPeak(40.0f) == Approx(1000.0f));
    // Torch sparks mostly live 2 to 8 s, a few 10 to 30 s.
    const SparkKindShape torch = SparkKindShapeOf(SparkKind::Torch);
    CHECK(torch.lifeMin == Approx(2.0f));
    CHECK(torch.lifeMax == Approx(8.0f));
    CHECK(torch.longMax == Approx(30.0f));
    CHECK(torch.longShare <= 0.1f);
    // 2 to 5 cm across (a real ember is 2 to 20 mm; within 20 m an 8 to 18 cm
    // spark reads as a glowing ball).
    for (int k = 0; k < kSparkKinds; ++k)
    {
        const SparkKindShape s = SparkKindShapeOf(static_cast<SparkKind>(k));
        CAPTURE(k);
        CHECK(s.sizeMin >= 0.01f);
        CHECK(s.sizeMax <= 0.05f);
    }
    // Fewer far off, each standing for the rest; none past the far limit.
    CHECK(SparkKeep(50.0f, kFrontSparkFullM, kFrontSparkMaxM) == Approx(1.0f));
    CHECK(SparkKeep(200.0f, kFrontSparkFullM, kFrontSparkMaxM) == Approx(0.25f));
    CHECK(SparkKeep(kFrontSparkMaxM, kFrontSparkFullM, kFrontSparkMaxM) == Approx(0.0f));
    CHECK(SparkKeep(1000.0f, kTorchSparkFullM, kTorchSparkMaxM) > 0.0f);
    CHECK(SparkKeep(kTorchSparkMaxM, kTorchSparkFullM, kTorchSparkMaxM) == Approx(0.0f));
    // A spark's glow fades in, holds, and goes out as it dies, cooling from
    // its birth temperature to a dull red.
    CHECK(SparkPool::LightOverLife(0.0f, 2.0f) == Approx(0.0f));
    CHECK(SparkPool::LightOverLife(0.8f, 2.0f) == Approx(1.0f));
    CHECK(SparkPool::LightOverLife(1.99f, 2.0f) < 0.1f);
    CHECK(SparkPool::KelvinOverLife(0.0f, 2.0f, 1500.0f, 1050.0f) == Approx(1500.0f));
    CHECK(SparkPool::KelvinOverLife(2.0f, 2.0f, 1500.0f, 1050.0f) == Approx(1050.0f));
}

TEST_CASE("spark paths: a grass spark flattens, a torch spark streams up the column and bends over", "[firespread]")
{
    const SparkPool::GroundFn flat = [](float, float) { return 0.0f; };
    // A grass head's sparks (a 2.6 m flame leaning 55 degrees, its tips 1.5 m
    // up) in an 11 m/s wind from the south.
    {
        SparkPool pool(7U);
        SparkBirth b;
        b.kind = SparkKind::Front;
        b.y = 0.8f;
        b.tipY = 1.5f;
        b.riseMps = SparkRiseMps(2.6f);
        b.vz = 11.0f * SparkWindShare(0.8f);
        b.vy = b.riseMps;
        pool.Emit(b, 400);
        float sumY = 0.0f, sumZ = 0.0f, maxTop = 0.0f;
        int n = 0;
        for (int step = 0; step < 60; ++step)
        {
            pool.Step(0.05f, 0.0f, 11.0f, flat);
            for (const SparkPoint& p : pool.BuildPoints())
            {
                maxTop = std::max(maxTop, p.y);
                if (step == 19)
                {
                    sumY += p.y;
                    sumZ += p.z;
                    ++n;
                }
            }
        }
        REQUIRE(n > 50);
        // A second after birth: up to 1 to 3 m above the tips, 5 to 20 m
        // downwind; the path's mean climb 5 to 35 degrees above level.
        const float meanY = sumY / static_cast<float>(n), meanZ = sumZ / static_cast<float>(n);
        CHECK(meanY > 1.5f);
        CHECK(meanY < 1.5f + 3.0f);
        CHECK(meanZ > 4.0f);
        CHECK(meanZ < 15.0f);
        const float deg = std::atan2(meanY - 0.8f, meanZ) * 57.29578f;
        CHECK(deg > 5.0f);
        CHECK(deg < 35.0f);
        CHECK(maxTop < 1.5f + 6.0f);
        CHECK(pool.Alive() == 0); // all out within 3 s
    }
    // A torching pine's (a 15 m column from a crown 3 m up).
    {
        SparkPool pool(11U);
        SparkBirth b;
        b.kind = SparkKind::Torch;
        b.y = 3.0f;
        b.spreadXZ = 2.0f;
        b.spreadY = 2.0f;
        b.tipY = 3.0f + 15.0f * std::cos(0.9f);
        b.riseMps = SparkRiseMps(15.0f);
        b.vz = 11.0f * SparkWindShare(4.0f);
        b.vy = b.riseMps;
        pool.Emit(b, 600);
        // inside the column they climb at about 3 sqrt(L), never at 24 m/s
        for (int step = 0; step < 3; ++step)
            pool.Step(0.05f, 0.0f, 11.0f, flat);
        float vy = 0.0f;
        for (const SparkPoint& p : pool.BuildPoints())
            vy += p.vy;
        vy /= static_cast<float>(pool.Alive());
        CHECK(vy == Approx(SparkRiseMps(15.0f)).margin(2.0f));
        // 4 s on: out of the column, sinking, carried at the wind of their
        // height (1.0 to 1.2 of the 10 m wind), 15 to 40 m up, a stream tens
        // of metres long already
        for (int step = 0; step < 77; ++step)
            pool.Step(0.05f, 0.0f, 11.0f, flat);
        float y = 0.0f, z = 0.0f, vy4 = 0.0f, vz4 = 0.0f;
        const std::vector<SparkPoint>& pts = pool.BuildPoints();
        REQUIRE(pts.size() > 200);
        for (const SparkPoint& p : pts)
        {
            y += p.y;
            z += p.z;
            vy4 += p.vy;
            vz4 += p.vz;
        }
        const float k = 1.0f / static_cast<float>(pts.size());
        CHECK(y * k > 15.0f);
        CHECK(y * k < 40.0f);
        CHECK(vy4 * k < 0.0f);
        CHECK(vz4 * k > 11.0f * 0.95f);
        CHECK(z * k > 30.0f);
    }
}

TEST_CASE("sparks in the air at once match their emission rate times their life", "[firespread]")
{
    // Emitting a grass head's FrontSparksPerM a metre over 10 m of front at
    // its rate (the count over the mean life) leaves about that many in the air.
    const SparkPool::GroundFn deep = [](float, float) { return -1000.0f; };
    SparkPool pool(3U);
    const SparkKindShape front = SparkKindShapeOf(SparkKind::Front);
    const float want = 10.0f * FrontSparksPerM(2.6f);
    const float perSec = want / SparkMeanLifeSec(front);
    SparkBirth b;
    b.kind = SparkKind::Front;
    b.y = 0.5f;
    b.tipY = 1.5f;
    b.riseMps = SparkRiseMps(2.6f);
    long long samples = 0, alive = 0;
    for (int step = 0; step < 200; ++step)
    {
        pool.EmitExpected(b, perSec * 0.03f);
        pool.Step(0.03f, 0.0f, 11.0f, deep);
        if (step >= 100)
        {
            alive += pool.Alive();
            ++samples;
        }
    }
    const float mean = static_cast<float>(alive) / static_cast<float>(samples);
    CHECK(mean == Approx(want).epsilon(0.2f));
    // The pool caps itself and counts what it drops.
    SparkPool full(5U);
    full.Emit(b, SparkPool::kMaxSparks + 10);
    CHECK(full.Alive() == SparkPool::kMaxSparks);
    CHECK(full.Counts().dropped == 10);
    // A far spark stands for the ones left out: its light and its spread.
    SparkBirth far = b;
    far.weight = 4.0f;
    SparkPool one(9U);
    one.Emit(far, 1);
    one.Step(0.2f, 0.0f, 0.0f, deep);
    const SparkPoint p = one.BuildPoints().front();
    CHECK(p.area == Approx(4.0f));
    CHECK(p.light > 2.0f);
}

TEST_CASE("the wandering wind is the model's: the spread never depends on the frame rate", "[firespread]")
{
    // The same line fire under a 12 deg wander, one grid ticked at a steady
    // 0.25 s, the other at a ragged frame rate (16 to 120 ms frames): each step
    // turns the wind by the wander at its own sim time, so at the same sim
    // time both have caught the same cells at the same times. Without the
    // wander the fire runs elsewhere.
    auto make = [](float wanderDeg)
    {
        Params p = Worst();
        p.windWanderDeg = wanderDeg;
        auto g = std::make_unique<FireGrid>(
            p,
            [](float, float)
            {
                Fuel f;
                f.model = 1;
                return f;
            },
            [](float, float) { return 0.0f; });
        g->IgniteLine(-40.0f, 4.0f, 40.0f, 4.0f);
        return g;
    };
    auto steady = make(12.0f), ragged = make(12.0f), still = make(0.0f);
    const float frames[6] = {0.016f, 0.033f, 0.120f, 0.050f, 0.071f, 0.090f};
    for (int k = 0; k < 4 * 150; ++k)
    {
        steady->Tick(0.25f, 0.0f, 11.0f);
        still->Tick(0.25f, 0.0f, 11.0f);
    }
    for (int k = 0; ragged->GetStats().simTime < steady->GetStats().simTime - 1e-6; ++k)
        ragged->Tick(frames[k % 6], 0.0f, 11.0f);
    REQUIRE(ragged->GetStats().simTime == steady->GetStats().simTime);
    CHECK(ragged->GetStats().ignitions == steady->GetStats().ignitions);
    CHECK(ragged->GetStats().burning == steady->GetStats().burning);
    CHECK(ragged->GetStats().front == steady->GetStats().front);
    int same = 0, differ = 0, moved = 0;
    for (float z = -20.0f; z < 260.0f; z += 4.0f)
        for (float x = -120.0f; x < 120.0f; x += 4.0f)
        {
            (ragged->StateAt(x, z) == steady->StateAt(x, z) ? same : differ)++;
            if (steady->StateAt(x, z) != still->StateAt(x, z))
                ++moved;
        }
    CHECK(differ == 0);
    CHECK(same > 1000);
    CHECK(moved > 20);
    CHECK(steady->WanderRad(100.0) == ragged->WanderRad(100.0));
    // The old way, the caller turning the wind by the wander at its frame
    // clock, made the two pacings part.
    auto steadyOld = make(0.0f), raggedOld = make(0.0f);
    double clock = 0.0;
    for (int k = 0; k < 4 * 150; ++k)
    {
        clock += 0.25;
        const float a = WindWanderRad(clock, 0x77a2d1c5u, 12.0f);
        steadyOld->Tick(0.25f, -11.0f * std::sin(a), 11.0f * std::cos(a));
    }
    clock = 0.0;
    for (int k = 0; raggedOld->GetStats().simTime < steadyOld->GetStats().simTime - 1e-6; ++k)
    {
        clock += frames[k % 6];
        const float a = WindWanderRad(clock, 0x77a2d1c5u, 12.0f);
        raggedOld->Tick(frames[k % 6], -11.0f * std::sin(a), 11.0f * std::cos(a));
    }
    int differOld = 0;
    for (float z = -20.0f; z < 260.0f; z += 4.0f)
        for (float x = -120.0f; x < 120.0f; x += 4.0f)
            if (raggedOld->StateAt(x, z) != steadyOld->StateAt(x, z))
                ++differOld;
    CHECK(differOld > 0);
}

TEST_CASE("the char starts at the drawn front and grows behind it, never ahead of the flames", "[firespread]")
{
    // The burn map's edge ramp: 0 at and ahead of the front, rising to 1 four
    // metres behind it (2 m texels, 2 texels).
    const float W = 4.0f;
    CHECK(BurnEdgeRamp(-1.0f, W) == 0.0f);
    CHECK(BurnEdgeRamp(0.0f, W) == 0.0f);
    CHECK(BurnEdgeRamp(2.0f, W) == Approx(0.5f));
    CHECK(BurnEdgeRamp(4.0f, W) == 1.0f);
    CHECK(BurnEdgeRamp(9.0f, W) == 1.0f);
    // Read bilinearly from 2 m texels wherever they fall, the map in front of
    // the front stays under the ground shader's cover start (burn 0.03; the
    // scorch value 110 of 255 at the ramp's top) half a metre ahead, and
    // barely over it at the front itself. The old ramp centred on the front
    // read half the scorch there.
    const float scorch = 110.0f / 255.0f;
    float worstFront = 0.0f, worstAhead = 0.0f;
    for (int k = 0; k < 40; ++k)
    {
        const float s0 = -2.0f + 0.05f * static_cast<float>(k); // a texel centre, metres behind the front
        auto read = [&](float s)
        {
            // the texel centres either side of s, and the linear read between them
            const float lo = s0 + 2.0f * std::floor((s - s0) / 2.0f), w = (s - lo) / 2.0f;
            return scorch * ((1.0f - w) * BurnEdgeRamp(lo, W) + w * BurnEdgeRamp(lo + 2.0f, W));
        };
        worstFront = std::max(worstFront, read(0.0f));
        worstAhead = std::max(worstAhead, read(-0.5f));
    }
    CHECK(worstFront <= scorch / (2.0f * W) + 1e-4f);
    CHECK(worstFront < 0.06f);
    CHECK(worstAhead <= 0.031f);
    CHECK(scorch * std::clamp(0.5f + 0.0f / W, 0.0f, 1.0f) > 0.2f);
}

TEST_CASE("the burn map's burnt interior is black wall to wall at every texel size (the lattice)", "[firespread]")
{
    // The owner's helicopter view of the worst day at 40 min: a burn past
    // about 2 km takes 6 m texels under the 8 m cells, and the interior's fill
    // painted lround(8 / 6) = 1 texel a cell from its square's first, so every
    // fourth row and column of the black stayed 0: read bilinearly, a trough
    // down to bare grass every 24 m. Every texel now goes to the cell its
    // centre lies in, so a block of burnt-out cells is black wall to wall at
    // every texel the window takes (2 m up to 1 km across, 4 m to 2 km, 6 m to
    // 3 km, coarser on) and nothing outside the block is painted.
    const float cellM = 8.0f;
    const int size = 512; // UpdateBurnMap's window
    const int ci0 = 571, cj0 = 318, nx = 61, nz = 47;
    std::vector<std::pair<float, float>> centres;
    for (int b = 0; b < nz; ++b)
        for (int a = 0; a < nx; ++a)
            centres.emplace_back((static_cast<float>(ci0 + a) + 0.5f) * cellM,
                                 (static_cast<float>(cj0 + b) + 0.5f) * cellM);
    const float xLo = static_cast<float>(ci0) * cellM, xHi = static_cast<float>(ci0 + nx) * cellM;
    const float zLo = static_cast<float>(cj0) * cellM, zHi = static_cast<float>(cj0 + nz) * cellM;
    auto at = [&](const std::vector<unsigned char>& m, int i, int j)
    { return static_cast<float>(m[static_cast<std::size_t>(j) * size + static_cast<std::size_t>(i)]); };
    // the map at world (x, z) as the ground shader reads it: bilinear between texel centres (0..1)
    auto read = [&](const std::vector<unsigned char>& m, float x0, float z0, float T, float x, float z)
    {
        const float fx = (x - x0) / T - 0.5f, fz = (z - z0) / T - 0.5f;
        const int ix = static_cast<int>(std::floor(fx)), iz = static_cast<int>(std::floor(fz));
        const float tx = fx - static_cast<float>(ix), tz = fz - static_cast<float>(iz);
        return ((at(m, ix, iz) * (1.0f - tx) + at(m, ix + 1, iz) * tx) * (1.0f - tz) +
                (at(m, ix, iz + 1) * (1.0f - tx) + at(m, ix + 1, iz + 1) * tx) * tz) /
               255.0f;
    };
    for (const float T : {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 16.0f, 22.0f})
    {
        CAPTURE(T);
        // the window round the block as UpdateBurnMap places it (its first texel on a multiple of the texel)
        const float half = 0.5f * static_cast<float>(size) * T;
        const float x0 = T * std::floor((0.5f * (xLo + xHi) - half) / T);
        const float z0 = T * std::floor((0.5f * (zLo + zHi) - half) / T);
        std::vector<unsigned char> m(static_cast<std::size_t>(size) * size, 0);
        PaintBurntInterior(centres, cellM, x0, z0, T, size, m.data());
        int inside = 0, wrong = 0;
        for (int j = 0; j < size; ++j)
            for (int i = 0; i < size; ++i)
            {
                const float wx = x0 + (static_cast<float>(i) + 0.5f) * T;
                const float wz = z0 + (static_cast<float>(j) + 0.5f) * T;
                const bool in = wx >= xLo && wx < xHi && wz >= zLo && wz < zHi;
                inside += in ? 1 : 0;
                wrong += at(m, i, j) != (in ? 255.0f : 0.0f) ? 1 : 0;
            }
        CHECK(inside > 0);
        CHECK(wrong == 0);
        // read anywhere a texel or more inside the block: black, no trough
        float lowest = 1.0f;
        for (float z = zLo + T; z <= zHi - T; z += 0.75f)
            for (float x = xLo + T; x <= xHi - T; x += 0.75f)
                lowest = std::min(lowest, read(m, x0, z0, T, x, z));
        CHECK(lowest > 0.999f);
        // The old fill, for the cause: the same texels at 2, 4 and 8 m (the
        // near look of a burn under 2 km unchanged), the lattice at 6 m.
        std::vector<unsigned char> old(static_cast<std::size_t>(size) * size, 0);
        for (const auto& p : centres)
        {
            const int i0 = static_cast<int>(std::ceil((p.first - 0.5f * cellM - x0) / T - 0.5f));
            const int j0 = static_cast<int>(std::ceil((p.second - 0.5f * cellM - z0) / T - 0.5f));
            const int n = std::max(1, static_cast<int>(std::lround(cellM / T)));
            for (int j = j0; j < j0 + n; ++j)
                for (int i = i0; i < i0 + n; ++i)
                    if (i >= 0 && j >= 0 && i < size && j < size)
                        old[static_cast<std::size_t>(j) * size + static_cast<std::size_t>(i)] = 255;
        }
        if (T == 2.0f || T == 4.0f || T == 8.0f)
            CHECK(old == m);
        if (T == 6.0f)
        {
            // one texel row and column in four left out: about 7 of every 16 texels
            int gaps = 0;
            for (int j = 0; j < size; ++j)
                for (int i = 0; i < size; ++i)
                    gaps += at(m, i, j) == 255.0f && at(old, i, j) == 0.0f ? 1 : 0;
            CHECK(static_cast<float>(gaps) > 0.38f * static_cast<float>(inside));
            CHECK(static_cast<float>(gaps) < 0.49f * static_cast<float>(inside));
            float oldLowest = 1.0f;
            for (float z = zLo + T; z <= zHi - T; z += 0.75f)
                for (float x = xLo + T; x <= xHi - T; x += 0.75f)
                    oldLowest = std::min(oldLowest, read(old, x0, z0, T, x, z));
            CHECK(oldLowest < 0.05f);
        }
    }
}

TEST_CASE("the drawn front never moves back: ground once reached stays burnt, its flames never relight (the dance)",
          "[firespread]")
{
    // The owner on the jeep wreck's day time-lapse: the burnt ground round a
    // bush went back to grass and black again, the flames jumped about and
    // back. A burning cell's views drew the front through its head NOW (the
    // wandering wind swung the ellipse round its anchor), the build-up's
    // share now behind its centre (it shrank the ages there) and its uncaught
    // neighbours' predicted arrivals (they move both ways), so a point's
    // drawn age went back: in a harness of this test before the fix, a point
    // fire like the first case unburnt 294 lattice points in 10 min and relit
    // 331 that had burnt out, the line fire at real speed 64 and 84 in 5 min.
    // Now a point's drawn age (as the tongues' lattice and the burn map's
    // texels read it) never goes back: a point reached stays reached, its
    // age never falls, and a point past its flames never lights again.
    struct Case
    {
        int kind; // 0 a point, 1 a line, 2 an area
        int model;
        float m1h, windX, windZ, simSec;
    };
    for (const Case cs : {Case{0, 1, 0.075f, -3.2f, 3.1f, 240.0f}, Case{1, 1, 0.04f, -8.8f, 8.8f, 150.0f},
                          Case{2, 3, 0.075f, -3.2f, 3.1f, 240.0f}})
    {
        CAPTURE(cs.kind);
        Params p;
        p.moisture.m1h = cs.m1h;
        p.moisture.m10h = cs.m1h + 0.01f;
        p.moisture.m100h = cs.m1h + 0.02f;
        p.moisture.mLive = 1.0f;
        p.windWanderDeg = 15.0f;
        const int model = cs.model;
        FireGrid g(
            p,
            [model](float, float)
            {
                Fuel f;
                f.model = static_cast<std::uint8_t>(model);
                return f;
            },
            [](float, float) { return 0.0f; });
        if (cs.kind == 0)
            g.Ignite(1.3f, 2.7f);
        else if (cs.kind == 1)
            g.IgniteLine(-40.0f, 4.0f, 40.0f, 4.0f);
        else
            g.IgniteArea(1.3f, 2.7f, 6.0f);
        const float cellM = p.cellM;
        std::map<std::array<int, 4>, float> last; // (cell, point) -> its age at the last look
        std::vector<BurningView> views, edges;
        int looks = 0, reached = 0, back = 0, unburnt = 0, relit = 0;
        float worst = 0.0f;
        double t = 0.0;
        for (int k = 1; t < cs.simSec; ++k)
        {
            g.Tick(0.25f, cs.windX, cs.windZ);
            t += 0.25;
            if (k % 4 != 0)
                continue;
            g.CollectBurning(views);
            g.CollectBurntEdges(edges);
            views.insert(views.end(), edges.begin(), edges.end());
            for (const BurningView& v : views)
            {
                const int ci = static_cast<int>(std::floor(v.x / cellM)),
                          cj = static_cast<int>(std::floor(v.z / cellM));
                const float tail = kTailEnd * DrawResidenceSec(v.model, v.residenceSec);
                // the tongues' lattice nodes (2 m, FrontAgeField) and the burn map's 2 m texel centres (FrontAgeAt)
                for (int q = 0; q < 25 + 16; ++q)
                {
                    const bool node = q < 25;
                    const int u = node ? q % 5 : (q - 25) % 4, w = node ? q / 5 : (q - 25) / 4;
                    const float ox = node ? -4.0f + 2.0f * static_cast<float>(u) : -3.0f + 2.0f * static_cast<float>(u);
                    const float oz = node ? -4.0f + 2.0f * static_cast<float>(w) : -3.0f + 2.0f * static_cast<float>(w);
                    const float a = node ? FrontAgeField(v, ox, oz, cellM) : FrontAgeAt(v, ox, oz, cellM);
                    const std::array<int, 4> key{ci, cj, node ? u : 10 + u, node ? w : 10 + w};
                    const auto it = last.find(key);
                    looks++;
                    if (it != last.end() && it->second > 0.0f && a > -1e8f)
                    {
                        reached++;
                        unburnt += a <= 0.0f ? 1 : 0;
                        relit += it->second >= tail && a < tail ? 1 : 0;
                        if (a < it->second - 0.01f)
                        {
                            back++;
                            worst = std::max(worst, it->second - a);
                        }
                    }
                    last[key] = a;
                }
            }
        }
        CAPTURE(looks, reached, worst);
        CHECK(reached > 2000);
        CHECK(unburnt == 0);
        CHECK(relit == 0);
        CHECK(back == 0);
    }
}

TEST_CASE("the burn map keeps its char when the next map reads less, its window moves or coarsens (the dance)",
          "[firespread]")
{
    // LatchBurnMap: every texel of the map just painted at least what the map
    // before read at its centre, so ground once burnt stays burnt through the
    // edge ramp's reads, a moved window and a coarser texel.
    const int n = 64;
    auto idx = [&](int i, int j) { return static_cast<std::size_t>(j) * static_cast<std::size_t>(n) + i; };
    BurnWindow w0;
    w0.x0 = 1000.0f;
    w0.z0 = 2000.0f;
    w0.texelM = 2.0f;
    w0.size = n;
    std::vector<unsigned char> prev(static_cast<std::size_t>(n) * n, 0), now(prev.size(), 0);
    // a burnt block of 8 x 8 texels (16 m) at texels 20..27, its last column at scorch
    for (int j = 20; j < 28; ++j)
        for (int i = 20; i < 28; ++i)
            prev[idx(i, j)] = i == 27 ? 110 : 255;
    // the same window: the next paint reads its edge a little less, and a new texel on
    now[idx(27, 24)] = 60;
    now[idx(28, 24)] = 90;
    LatchBurnMap(prev.data(), w0, w0, now.data());
    CHECK(now[idx(27, 24)] == 110);
    CHECK(now[idx(28, 24)] == 90);
    CHECK(now[idx(24, 24)] == 255);
    CHECK(now[idx(19, 24)] == 0);
    // the window moved 5 texels east and 3 north (the fire neared its edge): the block shifted with it
    BurnWindow w1 = w0;
    w1.x0 += 5.0f * w0.texelM;
    w1.z0 += 3.0f * w0.texelM;
    std::vector<unsigned char> moved(prev.size(), 0);
    LatchBurnMap(prev.data(), w0, w1, moved.data());
    int same = 0;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
        {
            const int oi = i + 5, oj = j + 3;
            const unsigned char want = oi < n && oj < n ? prev[idx(oi, oj)] : 0;
            same += moved[idx(i, j)] == want ? 1 : 0;
        }
    CHECK(same == n * n);
    // a coarser texel (4 m, the fire outgrew 1 km): each reads the old map at
    // its centre, the block's inside still black, its rim between
    BurnWindow w2 = w0;
    w2.texelM = 4.0f;
    w2.x0 = 4.0f * std::floor((w0.x0 - 40.0f) / 4.0f);
    w2.z0 = 4.0f * std::floor((w0.z0 - 40.0f) / 4.0f);
    std::vector<unsigned char> coarse(prev.size(), 0);
    LatchBurnMap(prev.data(), w0, w2, coarse.data());
    auto coarseAt = [&](float x, float z)
    {
        const int i = static_cast<int>(std::floor((x - w2.x0) / w2.texelM));
        const int j = static_cast<int>(std::floor((z - w2.z0) / w2.texelM));
        return coarse[idx(i, j)];
    };
    // the block spans x 1040..1056, z 2040..2056; coarse texel centres at 1042, 1046, 1050, 1054
    CHECK(coarseAt(1042.0f, 2046.0f) == 255);
    CHECK(coarseAt(1046.0f, 2046.0f) == 255);
    CHECK(coarseAt(1050.0f, 2050.0f) == 255);
    CHECK(coarseAt(1054.0f, 2046.0f) == 183); // half on the black, half on the scorched last column
    CHECK(coarseAt(1038.0f, 2046.0f) == 0);   // off the block
    CHECK(coarseAt(1058.0f, 2046.0f) == 0);
    // a window elsewhere reads nothing, and no map before (a cleared fire) leaves the paint as it is
    BurnWindow away = w0;
    away.x0 += 5000.0f;
    std::vector<unsigned char> none(prev.size(), 7);
    LatchBurnMap(prev.data(), w0, away, none.data());
    CHECK(std::all_of(none.begin(), none.end(), [](unsigned char c) { return c == 7; }));
    LatchBurnMap(prev.data(), BurnWindow(), w0, none.data());
    CHECK(std::all_of(none.begin(), none.end(), [](unsigned char c) { return c == 7; }));
}

TEST_CASE("the char's depth behind an ignition line's backing front is the distance from it (follow-up 13)",
          "[firespread]")
{
    // Shot 06's dark band: on the line's head side a point the head passed
    // seconds after the ignition has the head's speed, so its age times that
    // speed put it tens of metres behind the front and the burn map charred
    // it black, though the backing front creeping upwind is a metre or two
    // away and its flames stand there. FrontDepthAt measures from the nearest
    // edge of the seed's burnt area instead.
    Params p = Worst();
    p.spotting = false;
    p.fuelNoise = 0.0f;
    FireGrid g(
        p,
        [](float, float)
        {
            Fuel f;
            f.model = 1;
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(-80.0f, 2.0f, 80.0f, 2.0f) > 0); // 2 m upwind of the cells' centres
    for (int k = 0; k < 4 * 30; ++k)
        g.Tick(0.25f, 0.0f, 11.0f);
    BurningView v;
    REQUIRE(g.ViewAt(4.0f, 4.0f, v));
    REQUIRE(v.exact);
    const float cellM = g.GetParams().cellM;
    // The backing front on x = 4 (the warped field's own crossing).
    float zb = 1e9f;
    for (float z = 2.0f; z > -4.0f; z -= 0.01f)
        if (FrontAgeAt(v, 0.0f, z - v.z, cellM) <= 0.0f)
        {
            zb = z + 0.01f;
            break;
        }
    REQUIRE(zb < 2.0f);
    INFO("backing front at z " << zb);
    for (const float behind : {0.3f, 1.0f, 2.5f, 5.0f})
    {
        float speed = 0.0f;
        const float oz = zb + behind - v.z;
        const float a = FrontAgeAt(v, 0.0f, oz, cellM, &speed);
        const float d = FrontDepthAt(v, 0.0f, oz, a, speed);
        CAPTURE(behind, a, speed, a * speed, d);
        REQUIRE(a > 0.0f);
        // (the edge noise stretches the field by up to about a third over a metre)
        CHECK(d > 0.6f * behind);
        CHECK(d < 1.4f * behind + 0.2f);
    }
    // Past the line on its head side the old measure was tens of metres.
    {
        float speed = 0.0f;
        const float a = FrontAgeAt(v, 0.0f, 3.0f - v.z, cellM, &speed);
        CHECK(a * speed > 10.0f);
        CHECK(FrontDepthAt(v, 0.0f, 3.0f - v.z, a, speed) < 3.0f - zb + 1.0f);
    }
    // Not reached: nothing; a view off the seed keeps the field's own measure.
    CHECK(FrontDepthAt(v, 0.0f, zb - 1.0f - v.z, -5.0f, 0.02f) == 0.0f);
    BurningView far = v;
    far.exact = false;
    CHECK(FrontDepthAt(far, 0.0f, 0.0f, 10.0f, 0.5f) == Approx(5.0f));
}

// ---- Buildings (buildings builder; the wildfire consult, 2026-11-18, tenth answer) ----

TEST_CASE("a building's bare cell: the cells beside it run their ground on into it, each part drawn by one",
          "[firespread]")
{
    // Every point of a built cell's square (here the cell at the origin)
    // belongs to exactly one of its four neighbours, the one on its side of
    // the square's diagonals; nothing is drawn past a side that borders no
    // built cell, nor in the corners.
    const float cellM = 8.0f;
    int claimed = 0, twice = 0, none = 0;
    for (int i = 0; i < 40; ++i)
        for (int j = 0; j < 40; ++j)
        {
            const float px = -3.9f + 7.8f * static_cast<float>(i) / 39.0f + 0.013f,
                        pz = -3.9f + 7.8f * static_cast<float>(j) / 39.0f + 0.007f;
            int n = 0;
            // its neighbours to the west (their +x side toward it), east, south and north
            n += BuiltRunOn(1u, px + cellM, pz, cellM) ? 1 : 0;
            n += BuiltRunOn(2u, px - cellM, pz, cellM) ? 1 : 0;
            n += BuiltRunOn(4u, px, pz + cellM, cellM) ? 1 : 0;
            n += BuiltRunOn(8u, px, pz - cellM, cellM) ? 1 : 0;
            claimed += n == 1 ? 1 : 0;
            twice += n > 1 ? 1 : 0;
            none += n == 0 ? 1 : 0;
        }
    CHECK(twice == 0);
    CHECK(none == 0);
    CHECK(claimed == 1600);
    CHECK(BuiltRunOn(0u, 2.0f, 1.0f, cellM));        // inside its own square, always
    CHECK_FALSE(BuiltRunOn(0u, 5.0f, 0.0f, cellM));  // past a side with no built cell
    CHECK_FALSE(BuiltRunOn(15u, 5.0f, 5.0f, cellM)); // a corner (the diagonal cell)

    // The grid: a house's footprint and apron bare and built, the grass round
    // it burns, its burning neighbours know their built sides, and the spread
    // goes round it as round any bare ground.
    Params p;
    p.moisture = Dry();
    FireGrid g(
        p,
        [](float x, float z)
        {
            Fuel f;
            f.model = 1;
            if (x > 36.0f && x < 52.0f && z > -4.0f && z < 12.0f)
            {
                f.model = 0;
                f.built = true;
            }
            return f;
        },
        [](float, float) { return 0.0f; });
    REQUIRE(g.IgniteLine(4.0f, -40.0f, 4.0f, 60.0f) > 0);
    std::uint8_t sides = 0;
    for (int k = 0; k < 3000; ++k)
    {
        g.Tick(0.05f, 6.0f, 0.0f);
        std::vector<BurningView> b;
        g.CollectBurning(b);
        for (const BurningView& v : b)
            if (std::fabs(v.x - 36.0f) < 0.1f && std::fabs(v.z - 4.0f) < 0.1f)
                sides |= v.builtSides;
    }
    CHECK((sides & 1u) != 0); // the cell west of the house: its east side borders it
    CHECK(g.StateAt(44.0f, 4.0f) != CellState::Burning);
    CHECK(g.StateAt(60.0f, 4.0f) != CellState::Unburnt); // the fire went round it
}

TEST_CASE("the two fire days and the chance a building catches as a fire passes it (the tenth answer)", "[firespread]")
{
    // The build's ember ignition chance on the two days, and the day's index.
    CHECK(EmberIgnitionChance(0.075f) == Approx(0.63f).margin(0.01f));
    CHECK(EmberIgnitionChance(0.035f) == Approx(0.96f).margin(0.01f));
    const float pigT = EmberIgnitionChance(0.075f), pigW = EmberIgnitionChance(0.035f);
    CHECK(FireDayIndex(pigT, 4.5f) == Approx(0.0f).margin(0.02f));
    CHECK(FireDayIndex(pigW, 12.5f) == Approx(1.0f).margin(0.02f));
    struct Row
    {
        BuildingKind kind;
        bool veg;
        float tLo, tHi, wLo, wHi;
    };
    const Row rows[] = {
        {BuildingKind::StoneHouse, false, 0.01f, 0.03f, 0.05f, 0.15f},
        {BuildingKind::StoneHouse, true, 0.05f, 0.10f, 0.20f, 0.40f},
        {BuildingKind::Shed, false, 0.10f, 0.20f, 0.50f, 0.80f},
        {BuildingKind::Barn, false, 0.10f, 0.20f, 0.60f, 0.90f},
    };
    for (const Row& r : rows)
    {
        const float t = BuildingPassChance(r.kind, r.veg, pigT, 4.5f),
                    w = BuildingPassChance(r.kind, r.veg, pigW, 12.5f);
        CAPTURE(static_cast<int>(r.kind), r.veg, t, w);
        CHECK(t >= r.tLo);
        CHECK(t <= r.tHi);
        CHECK(w >= r.wLo);
        CHECK(w <= r.wHi);
    }
    // A church and a fuel station as a stone house, a masonry block half;
    // bunkers never; a damp day less, a wet one none.
    CHECK(BuildingPassChance(BuildingKind::Church, false, pigW, 12.5f) ==
          Approx(BuildingPassChance(BuildingKind::StoneHouse, false, pigW, 12.5f)));
    CHECK(BuildingPassChance(BuildingKind::Masonry, false, pigW, 12.5f) ==
          Approx(0.5f * BuildingPassChance(BuildingKind::StoneHouse, false, pigW, 12.5f)));
    CHECK(BuildingPassChance(BuildingKind::Never, true, pigW, 12.5f) == 0.0f);
    CHECK(BuildingPassChance(BuildingKind::Shed, false, EmberIgnitionChance(0.12f), 2.0f) <
          BuildingPassChance(BuildingKind::Shed, false, pigT, 4.5f));
    CHECK(BuildingPassChance(BuildingKind::Shed, false, EmberIgnitionChance(0.25f), 2.0f) == 0.0f);
    // Kinds from the islands' names and map types.
    CHECK(BuildingKindOf(0, "data3d\\kulna.p3d") == BuildingKind::Shed);
    CHECK(BuildingKindOf(0, "data3d\\stodola.p3d") == BuildingKind::Barn);
    // Masonry outbuildings keep their walls; timber huts burn away.
    CHECK(BuildingWallsStand("data3d\\statek_kulna.p3d"));
    CHECK(BuildingWallsStand("data3d\\stodola_open.p3d"));
    CHECK_FALSE(BuildingWallsStand("data3d\\bouda2.p3d"));
    CHECK(BuildingKindOf(1, "data3d\\kostel.p3d") == BuildingKind::Church);
    CHECK(BuildingKindOf(3, "data3d\\bunker.p3d") == BuildingKind::Never);
    CHECK(BuildingKindOf(0, "data3d\\panelak.p3d") == BuildingKind::Never);
    CHECK(BuildingKindOf(4, "data3d\\benzina.p3d") == BuildingKind::FuelStation);
    CHECK(BuildingKindOf(0, "data3d\\dum01.p3d") == BuildingKind::StoneHouse);
    CHECK(BuildingKindOf(0, "data3d\\dumruina.p3d") == BuildingKind::Never);
    CHECK(BuildingKindOf(0, "data3d\\garaz_s_tankem.p3d") == BuildingKind::Never);
    CHECK(BuildingKindOf(0, "data3d\\statek_kulna.p3d") == BuildingKind::Shed);
    CHECK(BuildingKindOf(0, "data3d\\stodola_open.p3d") == BuildingKind::Barn);
    CHECK(BuildingKindOf(2, "data3d\\kostelik.p3d") == BuildingKind::Church);
    CHECK(BuildingKindOf(6, "data3d\\newplot2.p3d") == BuildingKind::Fence);
    CHECK(BuildingKindOf(6, "data3d\\ohrada_sama.p3d") == BuildingKind::Fence);
    CHECK(BuildingKindOf(6, "data3d\\plot_green_draty.p3d") == BuildingKind::Never);
    // The delay: a house 5 to 30 min after the front (median about 12), a shed or barn within minutes.
    CHECK(BuildingDelaySec(BuildingKind::StoneHouse, 0.0f) == Approx(300.0f));
    CHECK(BuildingDelaySec(BuildingKind::StoneHouse, 1.0f) == Approx(1800.0f));
    CHECK(BuildingDelaySec(BuildingKind::StoneHouse, 0.5f) > 600.0f);
    CHECK(BuildingDelaySec(BuildingKind::StoneHouse, 0.5f) < 900.0f);
    CHECK(BuildingDelaySec(BuildingKind::Shed, 1.0f) <= 240.0f);
    // A burning neighbour's heat: wood by 6 m for certain and not past 12 m;
    // a house's eaves within 6 m 0.12 typical and 0.3 worst (the village's
    // losses: tools\village_mc.py), none at 12 m.
    CHECK(BuildingHeatChance(BuildingKind::Shed, 5.0f, 0.0f) == Approx(1.0f));
    CHECK(BuildingHeatChance(BuildingKind::Shed, 12.0f, 1.0f) == 0.0f);
    CHECK(BuildingHeatChance(BuildingKind::StoneHouse, 4.0f, 0.0f) == Approx(kHeatEavesTypical));
    CHECK(BuildingHeatChance(BuildingKind::StoneHouse, 4.0f, 1.0f) == Approx(kHeatEavesWorst));
    CHECK(BuildingHeatChance(BuildingKind::StoneHouse, 9.0f, 1.0f) < kHeatEavesWorst);
    CHECK(BuildingHeatChance(BuildingKind::StoneHouse, 12.0f, 1.0f) == 0.0f);
    // The embers at a roof's collapse: 1 to 3 (median 15 m) typical, 3 to 10 (median 40 m) worst.
    CHECK(DayBlend(kCollapseEmbersTypical, kCollapseEmbersWorst, 0.0f) == Approx(2.0f));
    CHECK(DayBlend(kCollapseEmbersTypical, kCollapseEmbersWorst, 1.0f) == Approx(6.5f));
    CHECK(DayBlend(kCollapseEmberMedianTypicalM, kCollapseEmberMedianWorstM, 1.0f) == Approx(40.0f));
}

namespace
{
// A building's timeline from one set of draws, all at u (0..1).
BuildingTimeline TimelineAt(BuildingKind kind, bool roofFirst, float u)
{
    float d[kBuildingTimelineDraws];
    for (float& x : d)
        x = u;
    return BuildingTimelineOf(kind, roofFirst, d);
}
} // namespace

TEST_CASE("a stone house's timeline: smoke, window flames, the roof in, burnt down, a glowing shell", "[firespread]")
{
    for (const float u : {0.0f, 0.5f, 1.0f})
    {
        const BuildingTimeline tl = TimelineAt(BuildingKind::StoneHouse, false, u);
        CAPTURE(u, tl.flashSec, tl.spreadSec, tl.collapseSec, tl.floorSec, tl.downSec, tl.peakM, tl.windowM);
        // window flames 1 to 3 m at 10 to 20 min; the roof in at 30 to 60 min with 3 to 8 m above the walls
        CHECK(tl.flashSec >= 600.0f);
        CHECK(tl.flashSec <= 1200.0f);
        CHECK(tl.collapseSec >= 1800.0f);
        CHECK(tl.collapseSec <= 3600.0f);
        CHECK(tl.peakM >= 3.0f);
        CHECK(tl.peakM <= 8.0f);
        CHECK(tl.windowM >= 1.0f);
        CHECK(tl.windowM <= 3.0f);
        // burnt down in 1.5 to 3 h, then a shell glowing 6 to 12 h, smoking 1 to 2 days
        CHECK(tl.downSec >= 5400.0f);
        CHECK(tl.downSec <= 10800.0f + 1.0f);
        CHECK(tl.glowSec >= 6.0f * 3600.0f);
        CHECK(tl.glowSec <= 12.0f * 3600.0f);
        CHECK(tl.smokeSec >= 86400.0f);
        // Before its first flashover: smoke only, grey; no flame shows.
        const BuildingPhase a = BuildingPhaseAt(tl, 0.5f * tl.flashSec);
        CHECK_FALSE(a.flaming);
        CHECK(a.smoke > 0.0f);
        CHECK(a.black < 0.7f);
        CHECK(BuildingWindowFlameM(tl, 0.0f, 0.5f * tl.flashSec) == 0.0f);
        // The first window, a minute after: 1 to 3 m; the last not yet.
        const float w1 = BuildingWindowFlameM(tl, 0.0f, tl.flashSec + 60.0f);
        CHECK(w1 >= 0.9f * tl.windowM);
        CHECK(w1 <= 3.0f);
        CHECK(BuildingWindowFlameM(tl, 1.0f, tl.flashSec + 60.0f) == 0.0f);
        const BuildingPhase b = BuildingPhaseAt(tl, tl.flashSec + 60.0f);
        CHECK(b.flaming);
        CHECK(b.black > 0.6f);
        CHECK_FALSE(b.collapsed);
        CHECK(b.topM < 0.5f);
        // The roof in: flames 3 to 8 m above the walls within a minute and a half, its heat at the peak.
        const BuildingPhase c = BuildingPhaseAt(tl, tl.collapseSec + 90.0f);
        CHECK(c.collapsed);
        CHECK(c.topM == Approx(tl.peakM).margin(0.05f));
        CHECK(c.heatMW >= 0.99f * tl.peakMW);
        CHECK(c.smoke == Approx(1.0f));
        // Burning down: the flames sink inside the shell, the smoke greys.
        const BuildingPhase d = BuildingPhaseAt(tl, tl.downSec - 60.0f);
        CHECK(d.topM < 0.05f);
        CHECK(d.black < 0.6f);
        // Burnt down: no flame, the rubble glowing, thin smoke; then over.
        const BuildingPhase e = BuildingPhaseAt(tl, tl.downSec + 3600.0f);
        CHECK_FALSE(e.flaming);
        CHECK(e.glow > 0.5f);
        CHECK(e.smoke > 0.0f);
        CHECK(e.smoke < 0.1f);
        CHECK(BuildingPhaseAt(tl, tl.downSec + std::max(tl.glowSec, tl.smokeSec) + 1.0f).over);
    }
    // Roof first: smoke from the tiles, the roof alight at 10 to 15 min, in at 25 to 45 min, the rooms after.
    const BuildingTimeline r = TimelineAt(BuildingKind::StoneHouse, true, 0.5f);
    CHECK(r.flashSec >= 600.0f);
    CHECK(r.flashSec <= 900.0f);
    CHECK(r.collapseSec >= 1500.0f);
    CHECK(r.collapseSec <= 2700.0f);
    CHECK(r.spreadSec > r.collapseSec);
    CHECK(BuildingPhaseAt(r, r.flashSec + 200.0f).topM >= 2.0f);
    CHECK(BuildingPhaseAt(r, r.flashSec + 200.0f).windows == 0.0f);
    CHECK(BuildingWindowFlameM(r, 0.0f, r.collapseSec + 120.0f) > 0.0f);
}

TEST_CASE("a shed, a barn, a church and a fuel station burn to the tenth answer's table", "[firespread]")
{
    for (const float u : {0.0f, 1.0f})
    {
        // a shed: fully alight in 2 to 4 min with 3 to 5 m flames, down by 8 to 15 min
        const BuildingTimeline s = TimelineAt(BuildingKind::Shed, false, u);
        CHECK(s.flashSec >= 120.0f);
        CHECK(s.flashSec <= 240.0f);
        CHECK(BuildingPhaseAt(s, s.flashSec + 1.0f).bodyM >= 3.0f);
        CHECK(BuildingPhaseAt(s, s.flashSec + 1.0f).bodyM <= 5.0f);
        CHECK(s.collapseSec >= 480.0f);
        CHECK(s.collapseSec <= 900.0f);
        CHECK(BuildingPhaseAt(s, s.collapseSec + 1.0f).collapsed);
        CHECK(BuildingPhaseAt(s, s.collapseSec + 1.0f).bodyM < 0.5f * s.peakM);
        // a barn with hay: 10 to 15 m, collapsed in 15 to 30 min, its hay smouldering for days
        const BuildingTimeline b = TimelineAt(BuildingKind::Barn, false, u);
        CHECK(BuildingPhaseAt(b, b.flashSec + 1.0f).bodyM >= 10.0f);
        CHECK(BuildingPhaseAt(b, b.flashSec + 1.0f).bodyM <= 15.0f);
        CHECK(b.collapseSec >= 900.0f);
        CHECK(b.collapseSec <= 1800.0f);
        CHECK(b.smokeSec >= 2.0f * 86400.0f);
        CHECK(BuildingPhaseAt(b, b.downSec + 86400.0f).smoke > 0.0f);
        CHECK(BuildingPhaseAt(b, b.downSec + 86400.0f).black == 0.0f);
        // a church: its roof burns an hour or more with 5 to 12 m above the ridge, in at 1 to 2 h
        const BuildingTimeline c = TimelineAt(BuildingKind::Church, false, u);
        CHECK(c.roofFirst);
        CHECK(c.collapseSec >= 3600.0f);
        CHECK(c.collapseSec <= 7200.0f + 600.0f);
        CHECK(c.collapseSec - c.flashSec >= 1800.0f);
        CHECK(BuildingPhaseAt(c, c.spreadSec + 1.0f).topM >= 5.0f);
        CHECK(BuildingPhaseAt(c, c.spreadSec + 1.0f).topM <= 12.0f);
        // a fuel station: alight within minutes, 3 to 6 m, black smoke
        const BuildingTimeline f = TimelineAt(BuildingKind::FuelStation, false, u);
        CHECK(f.flashSec <= 300.0f);
        CHECK(BuildingPhaseAt(f, f.flashSec + 1.0f).bodyM >= 3.0f);
        CHECK(BuildingPhaseAt(f, f.flashSec + 1.0f).bodyM <= 6.0f);
        CHECK(BuildingPhaseAt(f, f.flashSec + 1.0f).black == Approx(1.0f));
    }
    // a wooden fence: alight along itself within minutes, low, fallen by 10 to 30 min
    for (const float u : {0.0f, 1.0f})
    {
        const BuildingTimeline f = TimelineAt(BuildingKind::Fence, false, u);
        CHECK(f.flashSec <= 180.0f);
        CHECK(BuildingPhaseAt(f, f.flashSec + 1.0f).bodyM >= 0.8f);
        CHECK(BuildingPhaseAt(f, f.flashSec + 1.0f).bodyM <= 1.5f);
        CHECK(f.collapseSec >= 600.0f);
        CHECK(f.collapseSec <= 1800.0f);
    }
    // bunkers, fortresses and concrete never burn
    const BuildingTimeline n = TimelineAt(BuildingKind::Never, false, 0.5f);
    CHECK_FALSE(BuildingPhaseAt(n, 600.0f).flaming);
    CHECK(BuildingPhaseAt(n, 600.0f).smoke == 0.0f);
}

TEST_CASE("a building's smoke is dense and black, grass smoke pale (the plume's colour)", "[firespread]")
{
    // (the plume's black share is the wrecks builder's: PlumeAlbedo's third argument)
    const PlumeRgb grass = PlumeAlbedo(0.0f, 0.0f), black = PlumeAlbedo(1.0f, 0.0f, 1.0f);
    CHECK(grass.r > 0.7f);
    CHECK(black.r == Approx(kPlumeSootRgb.r));
    CHECK(black.r < 0.3f * grass.r);
    // a house's smoke after its first flashover is black (BuildingPhaseAt), a burnt-down shell's grey-white
    const BuildingTimeline tl = TimelineAt(BuildingKind::StoneHouse, false, 0.5f);
    CHECK(BuildingPhaseAt(tl, tl.flashSec + 60.0f).black > 0.9f);
    CHECK(BuildingPhaseAt(tl, tl.downSec + 600.0f).black < 0.2f);
    CHECK(PlumeAlbedo(1.0f, 0.0f, BuildingPhaseAt(tl, tl.flashSec + 60.0f).black).r < 0.3f * grass.r);
}

TEST_CASE("a cell beside a building's bare cell draws its tongues on into the quarter of that square on its side",
          "[firespread]")
{
    // A cell lit all at once (crossSec 0), its band alight everywhere: its
    // tongues stand in its own square, and with an east side that borders a
    // building's bare cell also in that square's west quarter (BuiltRunOn),
    // never beyond it.
    const float cellM = 8.0f;
    BurningView v;
    v.x = 4.0f;
    v.z = 4.0f;
    v.age = 1.0f;
    v.flameMaxM = 1.0f;
    v.flameM = 1.0f;
    v.strength = 1.0f;
    v.residenceSec = 5.0f;
    v.model = 1;
    std::vector<SurfaceTongue> plain, runOn;
    CellSurfaceTongues(v, 5.0f, cellM, 7u, 0.0f, 1.0f, 10.0f, 0.0f, plain);
    v.builtSides = 1u; // +x
    CellSurfaceTongues(v, 5.0f, cellM, 7u, 0.0f, 1.0f, 10.0f, 0.0f, runOn);
    REQUIRE(!plain.empty());
    int past = 0;
    for (const SurfaceTongue& s : plain)
        past += s.ox > 0.5f * cellM ? 1 : 0;
    CHECK(past == 0);
    int into = 0, wrong = 0;
    for (const SurfaceTongue& s : runOn)
    {
        if (s.ox > 0.5f * cellM)
            ++into;
        if (!BuiltRunOn(1u, s.ox, s.oz, cellM))
            ++wrong;
    }
    CHECK(into > 0);
    CHECK(wrong == 0);
    // about a quarter of a square more ground: a quarter more tongues
    CHECK(static_cast<float>(runOn.size()) > 1.1f * static_cast<float>(plain.size()));
    CHECK(static_cast<float>(runOn.size()) < 1.45f * static_cast<float>(plain.size()));
}

namespace
{
// A face of the test house: corners (x, y, z) in order, its outward normal, its texture.
ShellFace Face(std::initializer_list<std::array<float, 3>> corners, std::array<float, 3> n, const char* tex)
{
    ShellFace f;
    f.n = 0;
    for (const auto& c : corners)
    {
        f.p[f.n][0] = c[0];
        f.p[f.n][1] = c[1];
        f.p[f.n][2] = c[2];
        ++f.n;
    }
    const float l = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    for (int a = 0; a < 3; ++a)
        f.nrm[a] = n[a] / l;
    f.tex = tex;
    return f;
}
// A 6 x 8 m house, walls 4 m, gables at x = +-3 up to a 7 m ridge along x,
// a 0.6 m chimney stack standing on the roof by the ridge (6.6 to 8 m), a
// dormer's cheeks on the south slope, an upper floor at 2.5 m, a ground
// floor at 0.2 m and a window pane on the front wall.
std::vector<ShellFace> GuttedTestHouse()
{
    std::vector<ShellFace> f;
    f.push_back(Face({{3, 0, -4}, {3, 0, 4}, {3, 4, 4}, {3, 4, -4}}, {1, 0, 0}, "wall.pac"));                       // 0
    f.push_back(Face({{-3, 0, 4}, {-3, 0, -4}, {-3, 4, -4}, {-3, 4, 4}}, {-1, 0, 0}, "wall.pac"));                  // 1
    f.push_back(Face({{-3, 0, 4}, {3, 0, 4}, {3, 4, 4}, {-3, 4, 4}}, {0, 0, 1}, "wall.pac"));                       // 2
    f.push_back(Face({{3, 0, -4}, {-3, 0, -4}, {-3, 4, -4}, {3, 4, -4}}, {0, 0, -1}, "wall.pac"));                  // 3
    f.push_back(Face({{3, 4, -4}, {3, 4, 4}, {3, 7, 0}}, {1, 0, 0}, "gable.pac"));                                  // 4
    f.push_back(Face({{-3, 4, 4}, {-3, 4, -4}, {-3, 7, 0}}, {-1, 0, 0}, "gable.pac"));                              // 5
    f.push_back(Face({{-3.3f, 4, 4.3f}, {3.3f, 4, 4.3f}, {3.3f, 7, 0}, {-3.3f, 7, 0}}, {0, 4.3f, 3}, "tiles.pac")); // 6
    f.push_back(
        Face({{3.3f, 4, -4.3f}, {-3.3f, 4, -4.3f}, {-3.3f, 7, 0}, {3.3f, 7, 0}}, {0, 4.3f, -3}, "tiles.pac")); // 7
    f.push_back(
        Face({{1.3f, 6.6f, -0.3f}, {1.3f, 6.6f, 0.3f}, {1.3f, 8, 0.3f}, {1.3f, 8, -0.3f}}, {1, 0, 0}, "wall.pac")); // 8
    f.push_back(Face({{0.7f, 6.6f, 0.3f}, {0.7f, 6.6f, -0.3f}, {0.7f, 8, -0.3f}, {0.7f, 8, 0.3f}}, {-1, 0, 0},
                     "wall.pac")); // 9
    f.push_back(
        Face({{0.7f, 6.6f, 0.3f}, {1.3f, 6.6f, 0.3f}, {1.3f, 8, 0.3f}, {0.7f, 8, 0.3f}}, {0, 0, 1}, "wall.pac")); // 10
    f.push_back(Face({{1.3f, 6.6f, -0.3f}, {0.7f, 6.6f, -0.3f}, {0.7f, 8, -0.3f}, {1.3f, 8, -0.3f}}, {0, 0, -1},
                     "wall.pac")); // 11
    f.push_back(
        Face({{0.7f, 8, -0.3f}, {1.3f, 8, -0.3f}, {1.3f, 8, 0.3f}, {0.7f, 8, 0.3f}}, {0, 1, 0}, "cap.pac")); // 12
    f.push_back(Face({{-2, 5, 2}, {-2, 5, 3}, {-2, 6, 3}, {-2, 6, 2}}, {-1, 0, 0}, "dormer.pac"));           // 13
    f.push_back(Face({{-1, 5, 3}, {-1, 5, 2}, {-1, 6, 2}, {-1, 6, 3}}, {1, 0, 0}, "dormer.pac"));            // 14
    f.push_back(Face({{-2.8f, 2.5f, -3.8f}, {2.8f, 2.5f, -3.8f}, {2.8f, 2.5f, 3.8f}, {-2.8f, 2.5f, 3.8f}}, {0, 1, 0},
                     "floor.pac")); // 15
    f.push_back(Face({{-2.8f, 0.2f, -3.8f}, {2.8f, 0.2f, -3.8f}, {2.8f, 0.2f, 3.8f}, {-2.8f, 0.2f, 3.8f}}, {0, 1, 0},
                     "floor.pac"));                                                                                // 16
    f.push_back(Face({{-1, 1, 4.01f}, {0, 1, 4.01f}, {0, 2, 4.01f}, {-1, 2, 4.01f}}, {0, 0, 1}, "dum_okno1.pac")); // 17
    return f;
}
} // namespace

TEST_CASE("a gutted house keeps its walls, gables and chimney (carried to the ground floor), loses its roof, dormer, "
          "upper floor and glass",
          "[firespread]")
{
    // A 6 x 8 m house, walls 4 m, gables at x = +-3 up to a 7 m ridge along x,
    // a 0.6 m chimney stack standing on the roof by the ridge (6.6 to 8 m), a
    // dormer's cheeks on the south slope, an upper floor at 2.5 m, a ground
    // floor at 0.2 m and a window pane on the front wall.
    std::vector<ShellFace> f = GuttedTestHouse();

    const ShellPlan plan = PlanBurntShell(f, false);
    REQUIRE(plan.ok);
    CHECK(plan.eavesY == Approx(4.0f));
    CHECK(plan.ridgeY == Approx(7.0f));
    CHECK(plan.groundY == Approx(0.2f));
    for (int i = 0; i < 4; ++i)
        CHECK(plan.part[static_cast<std::size_t>(i)] == ShellPart::Wall);
    CHECK(plan.part[4] == ShellPart::Carried); // the gables stand over their walls
    CHECK(plan.part[5] == ShellPart::Carried);
    CHECK(plan.part[6] == ShellPart::Roof);
    CHECK(plan.part[7] == ShellPart::Roof);
    for (int i = 8; i <= 12; ++i)
        CHECK(plan.part[static_cast<std::size_t>(i)] == ShellPart::Stack);
    CHECK(plan.part[13] == ShellPart::Loose); // the dormer's cheeks hang off the roof
    CHECK(plan.part[14] == ShellPart::Loose);
    CHECK(plan.part[15] == ShellPart::Floor);
    CHECK(plan.part[16] == ShellPart::Wall); // the ground floor stays
    CHECK(plan.part[17] == ShellPart::Timber);
    // The stack's four sides stepped down 1.4 m at a time, past the ground floor.
    REQUIRE(plan.piers.size() == 1);
    CHECK(plan.piers[0].band.size() == 4);
    CHECK(plan.piers[0].step == Approx(1.4f));
    CHECK(plan.piers[0].copies == 5);
    CHECK(6.6f - plan.piers[0].step * static_cast<float>(plan.piers[0].copies) <= plan.groundY);
    // The walls' and gables' inner sides are added; nothing else's.
    std::vector<int> inner = plan.inner;
    std::sort(inner.begin(), inner.end());
    CHECK(inner == std::vector<int>{0, 1, 2, 3, 4, 5});

    // A church has no chimney: what stands on its roof is a timber turret and goes.
    const ShellPlan church = PlanBurntShell(f, true);
    REQUIRE(church.ok);
    CHECK(church.piers.empty());
    for (int i = 8; i <= 11; ++i)
        CHECK(church.part[static_cast<std::size_t>(i)] == ShellPart::Loose);
    CHECK_FALSE(ShellKeeps(church.part[8]));

    // A wall with a stack built into it: the stack is carried by the wall, no pier.
    std::vector<ShellFace> g = f;
    g.push_back(
        Face({{0.7f, 0.2f, 0.3f}, {1.3f, 0.2f, 0.3f}, {1.3f, 6.6f, 0.3f}, {0.7f, 6.6f, 0.3f}}, {0, 0, 1}, "wall.pac"));
    const ShellPlan built = PlanBurntShell(g, false);
    REQUIRE(built.ok);
    CHECK(built.piers.empty());
    CHECK(built.part[10] == ShellPart::Carried);
    CHECK(built.part[12] == ShellPart::Carried); // its cap with it

    // No roof, no shell (a wall or a ruin keeps the stock rule).
    std::vector<ShellFace> walls(f.begin(), f.begin() + 4);
    CHECK_FALSE(PlanBurntShell(walls, false).ok);
}

TEST_CASE("a gutted house's wall tops and gables break as thick stepped masonry: gables cut to stumps, now and "
          "then one kept tall with purlin pockets (v2, the Columns break)",
          "[firespread]")
{
    const std::vector<ShellFace> f = GuttedTestHouse();
    const ShellPlan plan = PlanBurntShell(f, false);
    REQUIRE(plan.ok);
    const ShellTops tops = PlanBurntTops(f, plan, 1234U, ShellTopsStyle::Columns);
    // The two gable walls (x = +-3) and the two eaves walls (z = +-4).
    CHECK(tops.gables == 2);
    CHECK(tops.walls == 2);
    // Each wall is cut 0.45 m under its lowest top (4 m): the walls across
    // the cut are cut, the gables over it rebuilt whole.
    for (int i = 0; i < 4; ++i)
    {
        bool cut = false;
        for (const auto& c : tops.clip)
            cut = cut || (c.first == i && c.second == Approx(3.55f).margin(0.06));
        CHECK(cut);
    }
    CHECK(std::count(tops.hide.begin(), tops.hide.end(), 4) == 1);
    CHECK(std::count(tops.hide.begin(), tops.hide.end(), 5) == 1);
    REQUIRE(!tops.quads.empty());
    // The masonry stands inside the old outline: nothing above the gable's
    // slope or the eaves, nothing outside the walls' plan, and it is about
    // 0.45 m thick (inner faces 0.4 to 0.5 m in from the wall).
    float maxIn = 0.0f, longestTop = 0.0f;
    for (const ShellQuad& q : tops.quads)
    {
        for (int k = 0; k < 4; ++k)
        {
            const float x = q.p[k][0], y = q.p[k][1], z = q.p[k][2];
            CHECK(std::fabs(x) <= 3.0f + 1e-3f);
            CHECK(std::fabs(z) <= 4.0f + 1e-3f);
            const float slope = 7.0f - 3.0f * std::fabs(z) / 4.0f; // the gable's edge over z
            if (std::fabs(x) > 2.4f && std::fabs(z) < 3.4f)
                CHECK(y <= std::max(slope, 4.0f) + 0.02f);
            else if (std::fabs(x) < 2.4f)
                CHECK(y <= 4.0f + 0.02f);
            maxIn = std::max(maxIn, std::min(3.0f - std::fabs(x), 4.0f - std::fabs(z)));
        }
        if (q.nrm[1] > 0.9f)
        {
            // a top's length along its wall (its other side is the wall's thickness)
            float x0 = 1e9f, x1 = -1e9f, z0 = 1e9f, z1 = -1e9f;
            for (int k = 0; k < 4; ++k)
            {
                x0 = std::min(x0, q.p[k][0]);
                x1 = std::max(x1, q.p[k][0]);
                z0 = std::min(z0, q.p[k][2]);
                z1 = std::max(z1, q.p[k][2]);
            }
            longestTop = std::max(longestTop, std::max(x1 - x0, z1 - z0));
        }
    }
    CHECK(maxIn >= 0.4f);
    CHECK(maxIn <= 0.5f + 1e-3f);
    // A broken edge: every column's top is short (no straight edge of a metre).
    CHECK(longestTop <= 0.65f);
    // Drawn from the building's seed: the same seed breaks it the same way,
    // another seed another way. The owner's choice (option 3): every gable a
    // short ragged stump, no tall triangle left, and now and then (about 0.3
    // of the houses) one gable kept most of its height as a broken stepped
    // piece, the only one with purlin pockets.
    const ShellTops again = PlanBurntTops(f, plan, 1234U, ShellTopsStyle::Columns);
    REQUIRE(again.quads.size() == tops.quads.size());
    CHECK(std::memcmp(again.quads.data(), tops.quads.data(), tops.quads.size() * sizeof(ShellQuad)) == 0);
    int tallHouses = 0, differ = 0;
    for (std::uint32_t seed = 1; seed <= 40; ++seed)
    {
        const ShellTops t = PlanBurntTops(f, plan, seed * 7919U, ShellTopsStyle::Columns);
        CHECK(t.gables == 2);
        CHECK(t.stumps + t.tall == 2);
        CHECK(t.tall <= 1);
        tallHouses += t.tall;
        differ += t.quads.size() != tops.quads.size() ? 1 : 0;
        float highest = 0.0f;
        for (const ShellQuad& q : t.quads)
            for (int k = 0; k < 4; ++k)
                highest = std::max(highest, q.p[k][1]);
        if (t.tall == 0)
        {
            CHECK(t.pockets == 0);
            // a stump keeps at most about a third of the gable's 3 m rise
            CHECK(highest <= 4.0f + 0.35f * 3.0f + 0.7f);
        }
        else
        {
            CHECK(t.pockets >= 1);
            CHECK(highest <= 7.0f - 0.2f + 1e-3f); // its apex broken
            CHECK(highest >= 4.0f + 0.5f * 3.0f);  // most of its height
        }
    }
    CHECK(tallHouses >= 4);
    CHECK(tallHouses <= 22);
    CHECK(differ >= 20);
}

namespace
{
// A stretch of a broken top's face edge along one wall of the test house.
struct TopEdge
{
    float x0, y0, x1, y1; // along the wall (x0 < x1), and the edge's height at each end
};
// The face's top edge of the masonry on the wall plane p[ax] = at (outward
// along +ax when sign > 0, its texture from the test house's face `src`),
// along the other horizontal axis, its straight runs joined.
std::vector<TopEdge> FaceTop(const ShellTops& t, int ax, float at, float sign, int src)
{
    const int al = ax == 0 ? 2 : 0;
    std::vector<TopEdge> e;
    for (const ShellQuad& q : t.quads)
    {
        if (q.src != src || q.nrm[ax] * sign < 0.9f)
            continue;
        bool on = true;
        for (int k = 0; k < q.n; ++k)
            on = on && std::fabs(q.p[k][ax] - at) < 1e-3f;
        if (!on)
            continue;
        float x0 = 1e9f, x1 = -1e9f;
        for (int k = 0; k < q.n; ++k)
        {
            x0 = std::min(x0, q.p[k][al]);
            x1 = std::max(x1, q.p[k][al]);
        }
        float y0 = -1e9f, y1 = -1e9f;
        for (int k = 0; k < q.n; ++k)
        {
            if (std::fabs(q.p[k][al] - x0) < 1e-4f)
                y0 = std::max(y0, q.p[k][1]);
            if (std::fabs(q.p[k][al] - x1) < 1e-4f)
                y1 = std::max(y1, q.p[k][1]);
        }
        if (x1 - x0 > 1e-4f)
            e.push_back({x0, y0, x1, y1});
    }
    std::sort(e.begin(), e.end(), [](const TopEdge& a, const TopEdge& b) { return a.x0 < b.x0; });
    std::vector<TopEdge> runs;
    for (const TopEdge& s : e)
    {
        if (!runs.empty())
        {
            TopEdge& r = runs.back();
            const float sr = (r.y1 - r.y0) / (r.x1 - r.x0), ss = (s.y1 - s.y0) / (s.x1 - s.x0);
            if (std::fabs(s.x0 - r.x1) < 1e-3f && std::fabs(s.y0 - r.y1) < 2e-3f && std::fabs(sr - ss) < 0.02f)
            {
                r.x1 = s.x1;
                r.y1 = s.y1;
                continue;
            }
        }
        runs.push_back(s);
    }
    return runs;
}
// The masonry's ends standing plumb across a wall (faced along it) away
// from its corners: steps of its own (the columns' battlements).
int StepsAlong(const ShellTops& t, int ax, float at, float half, int src)
{
    const int al = ax == 0 ? 2 : 0;
    int steps = 0;
    for (const ShellQuad& q : t.quads)
    {
        if (q.src != src || std::fabs(q.nrm[al]) < 0.995f || std::fabs(q.nrm[1]) > 0.05f)
            continue;
        bool on = true;
        for (int k = 0; k < q.n; ++k)
            on = on && std::fabs(q.p[k][ax]) <= std::fabs(at) + 1e-3f &&
                 std::fabs(q.p[k][ax]) >= std::fabs(at) - 0.6f && std::fabs(q.p[k][al]) < half - 0.05f;
        steps += on ? 1 : 0;
    }
    return steps;
}
// Every corner of the masonry inside the test house's old outline, and its
// depth into the walls at most (it is as thick as the wall).
float MasonryDepthInOutline(const ShellTops& t)
{
    float maxIn = 0.0f;
    for (const ShellQuad& q : t.quads)
        for (int k = 0; k < q.n; ++k)
        {
            const float x = q.p[k][0], y = q.p[k][1], z = q.p[k][2];
            CHECK(std::fabs(x) <= 3.0f + 1e-3f);
            CHECK(std::fabs(z) <= 4.0f + 1e-3f);
            const float slope = 7.0f - 3.0f * std::fabs(z) / 4.0f; // the gable's edge over z
            if (std::fabs(x) > 2.4f && std::fabs(z) < 3.4f)
                CHECK(y <= std::max(slope, 4.0f) + 0.02f);
            else if (std::fabs(x) < 2.4f)
                CHECK(y <= 4.0f + 0.02f);
            maxIn = std::max(maxIn, std::min(3.0f - std::fabs(x), 4.0f - std::fabs(z)));
        }
    return maxIn;
}
} // namespace

TEST_CASE("a gutted house's wall tops break along an irregular sloping line, not in steps (the tops agent: Ragged)",
          "[firespread]")
{
    // The owner on the stepped columns: "it looks like minecraft voxels". A
    // broken masonry top loses its stones in clusters: short slopes and
    // shallow notches at varying lengths, a rare short drop, one or two
    // deeper bites on a long wall, its cap over the thickness a little uneven.
    const std::vector<ShellFace> f = GuttedTestHouse();
    const ShellPlan plan = PlanBurntShell(f, false);
    REQUIRE(plan.ok);
    int tallHouses = 0, differ = 0;
    std::vector<float> lengths;
    const ShellTops first = PlanBurntTops(f, plan, 1234U, ShellTopsStyle::Ragged);
    for (std::uint32_t seed = 1; seed <= 40; ++seed)
    {
        const ShellTops t = PlanBurntTops(f, plan, seed * 7919U, ShellTopsStyle::Ragged);
        REQUIRE(t.gables == 2);
        REQUIRE(t.walls == 2);
        REQUIRE(!t.quads.empty());
        differ += t.quads.size() != first.quads.size() ? 1 : 0;
        // Inside the old outline, as thick as the wall.
        const float maxIn = MasonryDepthInOutline(t);
        CHECK(maxIn >= 0.4f);
        CHECK(maxIn <= 0.5f + 1e-3f);
        // The eaves walls (6 m, z = +-4): a broken line, 0.1 to 0.3 m lost
        // along most of it, one deep bite of 0.5 to 1 m, no plumb steps of
        // its own, no straight edge much over a metre.
        for (const float side : {-1.0f, 1.0f})
        {
            const std::vector<TopEdge> top = FaceTop(t, 2, 4.0f * side, side, side > 0.0f ? 2 : 3);
            REQUIRE(!top.empty());
            CHECK(top.front().x0 == Approx(-3.0f).margin(1e-3));
            CHECK(top.back().x1 == Approx(3.0f).margin(1e-3));
            float lowest = 1e9f, total = 0.0f, shallow = 0.0f, sloped = 0.0f;
            int drops = 0;
            for (const TopEdge& s : top)
            {
                const float dx = s.x1 - s.x0, dy = s.y1 - s.y0;
                const float len = std::sqrt(dx * dx + dy * dy);
                CHECK(len <= 1.05f);
                lengths.push_back(len);
                lowest = std::min({lowest, s.y0, s.y1});
                CHECK(std::max(s.y0, s.y1) <= 4.0f - 0.01f);
                total += dx;
                if (std::min(s.y0, s.y1) >= 4.0f - 0.36f)
                    shallow += dx;
                if (std::fabs(dy) > 0.03f * dx)
                    sloped += dx;
                if (std::fabs(dy) > 0.05f && std::fabs(dy) > 2.5f * dx)
                    ++drops;
            }
            CHECK(total == Approx(6.0f).margin(0.01));
            CHECK(lowest <= 4.0f - 0.45f); // its deep bite
            CHECK(lowest >= 4.0f - 1.05f);
            CHECK(shallow >= 0.5f * total); // (a 6 m wall: its bite is a good share of it)
            CHECK(sloped >= 0.5f * total);
            CHECK(drops <= 3);
            CHECK(StepsAlong(t, 2, 4.0f * side, 3.0f, side > 0.0f ? 2 : 3) == 0);
        }
        // The cap is uneven across the wall's thickness, not a flat slab.
        int caps = 0, tilted = 0;
        for (const ShellQuad& q : t.quads)
            if (q.nrm[1] > 0.3f && std::fabs(q.p[0][2]) > 3.5f && std::fabs(q.p[0][0]) < 2.5f)
            {
                ++caps;
                tilted += std::fabs(q.nrm[2]) > 0.03f ? 1 : 0;
            }
        CHECK(tilted >= caps / 4);
        // The gables: a stump broken across (a third of its rise or so), or
        // the house's one tall piece with its pockets and its apex gone.
        CHECK(t.stumps + t.tall == 2);
        CHECK(t.tall <= 1);
        tallHouses += t.tall;
        float highest = 0.0f;
        for (const ShellQuad& q : t.quads)
            for (int k = 0; k < q.n; ++k)
                highest = std::max(highest, q.p[k][1]);
        if (t.tall == 0)
        {
            CHECK(t.pockets == 0);
            CHECK(highest <= 4.0f + 0.42f * 3.0f + 0.25f);
        }
        else
        {
            CHECK(t.pockets >= 1);
            CHECK(highest <= 7.0f - 0.15f); // its apex broken
            CHECK(highest >= 4.0f + 0.5f * 3.0f);
        }
    }
    CHECK(tallHouses >= 4);
    CHECK(tallHouses <= 22);
    CHECK(differ >= 20);
    // No repeating unit: the straight pieces' lengths vary.
    float mean = 0.0f, var = 0.0f;
    for (const float l : lengths)
        mean += l / static_cast<float>(lengths.size());
    for (const float l : lengths)
        var += (l - mean) * (l - mean) / static_cast<float>(lengths.size());
    CHECK(std::sqrt(var) >= 0.35f * mean);
    // A house's coarser levels: the same break with its smallest bends left
    // out, fewer faces, still inside the outline.
    const ShellTops coarse = PlanBurntTops(f, plan, 1234U, ShellTopsStyle::Ragged, true);
    CHECK(coarse.quads.size() < first.quads.size() * 3 / 4);
    CHECK(MasonryDepthInOutline(coarse) >= 0.4f);
    // The same seed breaks it the same way.
    const ShellTops again = PlanBurntTops(f, plan, 1234U, ShellTopsStyle::Ragged);
    REQUIRE(again.quads.size() == first.quads.size());
    CHECK(std::memcmp(again.quads.data(), first.quads.data(), first.quads.size() * sizeof(ShellQuad)) == 0);
}

TEST_CASE("a gutted house's wall tops cut near-straight a little under the eaves, its gables low on a gentle "
          "diagonal (the tops agent: Restrained)",
          "[firespread]")
{
    const std::vector<ShellFace> f = GuttedTestHouse();
    const ShellPlan plan = PlanBurntShell(f, false);
    REQUIRE(plan.ok);
    for (std::uint32_t seed = 1; seed <= 40; ++seed)
    {
        const ShellTops t = PlanBurntTops(f, plan, seed * 7919U, ShellTopsStyle::Restrained);
        REQUIRE(t.gables == 2);
        REQUIRE(t.walls == 2);
        const float maxIn = MasonryDepthInOutline(t);
        CHECK(maxIn >= 0.4f);
        CHECK(maxIn <= 0.5f + 1e-3f);
        // the walls cut 0.07 to 0.27 m under the eaves, gently: no steps, no bites
        for (const float side : {-1.0f, 1.0f})
        {
            const std::vector<TopEdge> top = FaceTop(t, 2, 4.0f * side, side, side > 0.0f ? 2 : 3);
            REQUIRE(!top.empty());
            for (const TopEdge& s : top)
            {
                CHECK(std::min(s.y0, s.y1) >= 4.0f - 0.27f);
                CHECK(std::max(s.y0, s.y1) <= 4.0f - 0.06f);
                CHECK(std::fabs(s.y1 - s.y0) <= 0.25f * (s.x1 - s.x0) + 1e-3f);
            }
            CHECK(StepsAlong(t, 2, 4.0f * side, 3.0f, side > 0.0f ? 2 : 3) == 0);
        }
        // no tall gable piece, no pockets; the gables low
        CHECK(t.tall == 0);
        CHECK(t.stumps == 2);
        CHECK(t.pockets == 0);
        float highest = 0.0f;
        for (const ShellQuad& q : t.quads)
            for (int k = 0; k < q.n; ++k)
                highest = std::max(highest, q.p[k][1]);
        CHECK(highest <= 4.0f + 0.2f * 3.0f + 0.3f);
        CHECK(highest >= 4.05f);
    }
}

TEST_CASE("the shell tops' break is picked by POSEIDON_XP_FIRESPREAD_SHELLTOPS, the sloped one by default",
          "[firespread]")
{
    // (read once; the unit run leaves it unset)
    CHECK(ShellTopsStyleNow() == ShellTopsStyle::Ragged);
    const std::vector<ShellFace> f = GuttedTestHouse();
    const ShellPlan plan = PlanBurntShell(f, false);
    const ShellTops byDefault = PlanBurntTops(f, plan, 77U);
    const ShellTops ragged = PlanBurntTops(f, plan, 77U, ShellTopsStyle::Ragged);
    REQUIRE(byDefault.quads.size() == ragged.quads.size());
}
