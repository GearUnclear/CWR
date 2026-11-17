// Generated occupier bases (OCCUPIER-FORCES.md 5.7, step 1: zones only).
// The pure scorer on synthetic cell grids (the rejections, spacing, the
// bearing rule, coverage, the track share, the detected-outpost preference,
// the floor and ceiling, landmasses), determinism from the seed, the
// registry's seeding entry, the save round trip (stored rows, never
// re-derived; an old save generates nothing) and the win roster flag.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/BaseGenerate.hpp>
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp> // GGameState (gmGeneratedBases)
#include <Poseidon/Foundation/Modules/Modules.hpp> // InitModules (command registration)
#include <Evaluator/express.hpp>

#include <cmath>
#include <filesystem>
#include <functional>
#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

float D(float ax, float az, float bx, float bz)
{
    return sqrtf((ax - bx) * (ax - bx) + (az - bz) * (az - bz));
}

// A flat, dry, empty 50 m grid with helpers to paint hazards and roads.
struct Grid
{
    BaseGenInputs in;
    int n = 0;
    explicit Grid(int cells, float height = 20.0f)
    {
        n = cells;
        in.cellsX = cells;
        in.cellsZ = cells;
        in.cellSize = 50.0f;
        in.cells.assign((size_t)cells * cells, GenCell());
        for (GenCell& c : in.cells)
        {
            c.height = height;
        }
        in.seed = 7;
    }
    void Paint(float x0, float z0, float x1, float z1, const std::function<void(GenCell&)>& f)
    {
        for (int z = 0; z < n; z++)
        {
            for (int x = 0; x < n; x++)
            {
                const float cx = ((float)x + 0.5f) * 50.0f;
                const float cz = ((float)z + 0.5f) * 50.0f;
                if (cx >= x0 && cx <= x1 && cz >= z0 && cz <= z1)
                {
                    f(in.cells[(size_t)z * n + x]);
                }
            }
        }
    }
    void All(const std::function<void(GenCell&)>& f) { Paint(-1e9f, -1e9f, 1e9f, 1e9f, f); }
    // road points every 25 m, the road bit on their cells
    void Road(float x0, float z0, float x1, float z1, bool track, bool network = true)
    {
        const float len = D(x0, z0, x1, z1);
        const int steps = (int)(len / 25.0f);
        for (int i = 0; i <= steps; i++)
        {
            const float t = steps > 0 ? (float)i / (float)steps : 0.0f;
            GenRoadPoint p;
            p.x = x0 + (x1 - x0) * t;
            p.z = z0 + (z1 - z0) * t;
            p.track = track;
            p.network = network;
            in.roads.push_back(p);
            const int cx = (int)floorf(p.x / 50.0f);
            const int cz = (int)floorf(p.z / 50.0f);
            if (cx >= 0 && cz >= 0 && cx < n && cz < n)
            {
                in.cells[(size_t)cz * n + cx].road = true;
            }
        }
    }
    void Town(const char* name, float x, float z)
    {
        GenPlace p;
        p.name = name;
        p.x = x;
        p.z = z;
        in.towns.push_back(p);
        in.zones.push_back(ScaffoldPoint{x, z});
        in.takenNames.push_back(name);
    }
    void Source(const char* name, float x, float z)
    {
        GenPlace p;
        p.name = name;
        p.x = x;
        p.z = z;
        in.sources.push_back(p);
        in.zones.push_back(ScaffoldPoint{x, z});
        in.takenNames.push_back(name);
    }
    void Keepout(float x, float z)
    {
        in.keepouts.push_back(ScaffoldPoint{x, z});
        in.zones.push_back(ScaffoldPoint{x, z});
    }
    // a hillside: every cell sloped (band 2) but a lattice of single flat
    // cells every step cells, so no 150 m outpost footprint passes and a
    // minor kind's centre cell does (offset 1 puts flat cells 75 m from
    // the scenario's roads, which run on cell edges: a checkpoint's band)
    void Lattice(int step, int offset = 1)
    {
        for (int z = 0; z < n; z++)
        {
            for (int x = 0; x < n; x++)
            {
                in.cells[(size_t)z * n + x].gradient = (x % step == offset && z % step == offset) ? 0 : 2;
            }
        }
    }
    // a map object (the object grid)
    void Object(float x, float z, bool solid, float radius = 3.0f)
    {
        GenObject o;
        o.x = x;
        o.z = z;
        o.radius = radius;
        o.solid = solid;
        in.objects.push_back(o);
    }
};

// Scenario A, 6.4 km square: a trunk road along z 3200, a track north from
// it along x 4800; the base Base0 covers Westford; Southgate and Northend
// are 3.2 and 5.0 km from it.  K 3: floor 1, ceiling 2, so one base can be
// added, and it can cover both open towns only near the track.
Grid ScenarioA()
{
    Grid g(128);
    g.Road(200, 3200, 6200, 3200, false);
    g.Road(4800, 3200, 4800, 6000, true);
    g.Source("Base0", 1200, 3500);
    g.Keepout(3200, 1000); // the Camp
    g.Town("Westford", 1000, 3000);
    g.Town("Southgate", 4400, 3000);
    g.Town("Northend", 5200, 5200);
    return g;
}

void CheckRules(const BaseGenInputs& in, const BaseGenOptions& opt, const BaseGenResult& r)
{
    for (size_t i = 0; i < r.bases.size(); i++)
    {
        const GeneratedBase& b = r.bases[i];
        INFO(S(b.name));
        for (const GenPlace& s : in.sources)
        {
            CHECK(D(b.x, b.z, s.x, s.z) >= opt.baseSpacing);
        }
        for (size_t j = 0; j < i; j++)
        {
            CHECK(D(b.x, b.z, r.bases[j].x, r.bases[j].z) >= opt.baseSpacing);
        }
        for (const ScaffoldPoint& k : in.keepouts)
        {
            CHECK(D(b.x, b.z, k.x, k.z) >= opt.keepoutDist);
        }
        for (const GenPlace& t : in.towns)
        {
            CHECK(D(b.x, b.z, t.x, t.z) >= opt.townMinDist);
        }
        CHECK(b.roadDist >= opt.roadMin);
        CHECK(b.roadDist <= opt.roadMax);
    }
}

} // namespace

TEST_CASE("BaseGenerate - the floor, the ceiling, bearings, the kinds, the track vocabulary", "[guerrilla][basegenerate]")
{
    CHECK(BaseGenFloor(14) == 5);
    CHECK(BaseGenCeiling(14, 10) == 7);
    CHECK(BaseGenFloor(18) == 6);
    CHECK(BaseGenCeiling(18, 10) == 9);
    CHECK(BaseGenCeiling(26, 10) == 10); // never more than 10
    CHECK(BaseGenFloor(0) == 0);
    CHECK(BaseGenBearingDeg(0, 0, 0, 100) == Approx(0.0f).margin(0.01));
    CHECK(BaseGenBearingDeg(0, 0, 100, 0) == Approx(90.0f).margin(0.01));
    CHECK(BaseGenBearingDeg(0, 0, 0, -100) == Approx(180.0f).margin(0.01));
    CHECK(BaseGenBearingDeg(0, 0, -100, 0) == Approx(270.0f).margin(0.01));
    CHECK(BaseGenBearingGap(350, 10) == Approx(20.0f));
    CHECK(BaseGenBearingGap(0, 180) == Approx(180.0f));
    CHECK(RoadModelIsTrack("data3d\\cesta10 25.p3d"));
    CHECK(RoadModelIsTrack("o\\road\\ces10 25.p3d"));
    CHECK_FALSE(RoadModelIsTrack("data3d\\asf10 25.p3d"));
    CHECK_FALSE(RoadModelIsTrack("data3d\\silnice10 25.p3d"));
    CHECK_FALSE(RoadModelIsTrack(""));
    const GenBaseKindInfo& outpost = GenBaseKindOf(GenBaseKind::Outpost);
    CHECK(std::string(outpost.type) == "OUTPOST");
    CHECK(outpost.garrisonMax == 6);
    CHECK(outpost.countsForWin);
    CHECK_FALSE(GenBaseKindOf(GenBaseKind::Checkpoint).countsForWin);
    CHECK_FALSE(GenBaseKindOf(GenBaseKind::Coastal).countsForWin);
    CHECK_FALSE(GenBaseKindOf(GenBaseKind::Helipad).countsForWin);
    GenBaseKind k = GenBaseKind::Outpost;
    CHECK(GenBaseKindFromType("checkpoint", k));
    CHECK(k == GenBaseKind::Checkpoint);
    CHECK_FALSE(GenBaseKindFromType("AIRFIELD", k));
}

TEST_CASE("BaseGenerate - one base closes both gaps on a track, inside every rule", "[guerrilla][basegenerate]")
{
    Grid g = ScenarioA();
    BaseGenOptions opt;
    BaseGenResult r;
    GenerateBases(g.in, opt, r);
    CHECK(r.K == 3);
    CHECK(r.existing == 1);
    CHECK(r.floor == 1);
    CHECK(r.ceiling == 2);
    CHECK(r.uncoveredBefore == 2);
    REQUIRE(r.bases.size() == 1);
    const GeneratedBase& b = r.bases[0];
    CHECK(b.covers == 2);
    CHECK(r.uncoveredAfter == 0);
    CHECK(D(b.x, b.z, 4400, 3000) <= opt.coverDist);
    CHECK(D(b.x, b.z, 5200, 5200) <= opt.coverDist);
    // the third generated base (rounded up) on a track off the trunk road
    CHECK(b.track);
    CHECK(b.trunkDist >= opt.trackTrunkDist);
    CHECK(r.tracks == 1);
    CHECK(r.trackNeed == 1);
    CHECK(b.kind == GenBaseKind::Outpost);
    CHECK_FALSE(b.detected);
    // named after the nearest town
    CHECK((S(b.name) == "Northend Outpost" || S(b.name) == "Southgate Outpost"));
    // the gate faces the road: the track is west or east of the centre
    CHECK((fabsf(b.heading - 270.0f) < 1.0f || fabsf(b.heading - 90.0f) < 1.0f));
    CheckRules(g.in, opt, r);
    // every town now within 2.5 km of a base
    for (const BaseGenTownReport& t : r.towns)
    {
        INFO(S(t.name));
        CHECK(t.nearestDist <= opt.coverDist);
    }
}

TEST_CASE("BaseGenerate - the terrain rejections", "[guerrilla][basegenerate]")
{
    struct Hazard
    {
        const char* what;
        std::function<void(GenCell&)> paint;
        bool generates;
    };
    const std::vector<Hazard> hazards = {
        {"none", [](GenCell&) {}, true},
        {"gentle slope (band 1, under 0.10)", [](GenCell& c) { c.gradient = 1; }, true},
        {"water", [](GenCell& c) { c.water = true; }, false},
        {"forest", [](GenCell& c) { c.forestInner = true; }, false},
        {"forest edge", [](GenCell& c) { c.forestOuter = true; }, false},
        {"slope at 0.10", [](GenCell& c) { c.gradient = 2; }, false},
        {"hard objects", [](GenCell& c) { c.hardObjects = 1; }, false},
        {"full cells", [](GenCell& c) { c.full = true; }, false},
        {"low ground", [](GenCell& c) { c.height = 1.5f; }, false},
    };
    for (const Hazard& h : hazards)
    {
        INFO(h.what);
        Grid g = ScenarioA();
        g.All(h.paint);
        BaseGenResult r;
        GenerateBases(g.in, BaseGenOptions(), r);
        CHECK((r.bases.size() == 1) == h.generates);
        if (!h.generates)
        {
            CHECK(r.candidates == 0);
        }
    }
    // a road bit on every cell: always "on a road"
    Grid g = ScenarioA();
    g.All([](GenCell& c) { c.road = true; });
    BaseGenResult r;
    GenerateBases(g.in, BaseGenOptions(), r);
    CHECK(r.bases.empty());
    // a pocket road that does not join the island network serves nothing
    Grid p(128);
    p.Road(200, 3200, 6200, 3200, false, false);
    p.Road(4800, 3200, 4800, 6000, true, false);
    p.in.sources = ScenarioA().in.sources;
    p.in.towns = ScenarioA().in.towns;
    BaseGenResult pr;
    GenerateBases(p.in, BaseGenOptions(), pr);
    CHECK(pr.bases.empty());
    CHECK(pr.uncoveredAfter == 2);
}

TEST_CASE("BaseGenerate - the HQ keepout and spacing hold", "[guerrilla][basegenerate]")
{
    Grid g = ScenarioA();
    g.Keepout(4900, 4300); // an HQ in the lens both towns share
    BaseGenOptions opt;
    BaseGenResult r;
    GenerateBases(g.in, opt, r);
    CheckRules(g.in, opt, r);
    for (const GeneratedBase& b : r.bases)
    {
        CHECK(D(b.x, b.z, 4900, 4300) >= 1600.0f);
    }
}

TEST_CASE("BaseGenerate - the bearing rule picks the side that splits a town's two nearest",
          "[guerrilla][basegenerate]")
{
    // a 10 km map: the town Middle on a north-south trunk road between two
    // bases 2.6 km north and south of it (so it is open); an island in the
    // west carries six towns no site can serve (no road), which makes K 7
    // (ceiling 4) without giving the mainland another source.
    auto build = [](bool northFirst) {
        Grid g(200);
        g.Paint(2000, 0, 2500, 10000, [](GenCell& c) {
            c.deep = true;
            c.water = true;
        });
        g.Road(5000, 500, 5000, 9500, false);
        if (northFirst)
        {
            g.Source("North", 5000, 7600);
            g.Source("South", 5000, 2400);
        }
        else
        {
            g.Source("South", 5000, 2400);
            g.Source("North", 5000, 7600);
        }
        g.Town("Middle", 5000, 5000);
        const float isle[6][2] = {{500, 1000}, {500, 3000}, {500, 5000}, {500, 7000}, {1500, 2000}, {1500, 6000}};
        for (int i = 0; i < 6; i++)
        {
            g.Town(("Isle " + std::to_string(i)).c_str(), isle[i][0], isle[i][1]);
        }
        return g;
    };
    {
        // the nearest two of Middle with a site added are the site and North
        // (the first of the equidistant pair): a northern site would put
        // both north of the town, a southern one splits them
        Grid g = build(true);
        BaseGenResult r;
        GenerateBases(g.in, BaseGenOptions(), r);
        REQUIRE(r.bases.size() == 1);
        CHECK(r.bases[0].z < 5000.0f);
        CHECK(r.towns[0].bearingGap >= 60.0f);
    }
    {
        Grid g = build(false);
        BaseGenResult r;
        GenerateBases(g.in, BaseGenOptions(), r);
        REQUIRE(r.bases.size() == 1);
        CHECK(r.bases[0].z > 5000.0f);
        CHECK(r.towns[0].bearingGap >= 60.0f);
    }
}

TEST_CASE("BaseGenerate - a detected outpost is the preferred site and keeps its name", "[guerrilla][basegenerate]")
{
    Grid g = ScenarioA();
    GenDetected d;
    d.name = "Oldfort Post";
    d.x = 4950;
    d.z = 4200;
    d.y = 20;
    g.in.detected.push_back(d);
    // a detected outpost that breaks a rule (inside the town circle) is reported, not used
    GenDetected bad;
    bad.name = "Townfort Post";
    bad.x = 4450;
    bad.z = 3050;
    g.in.detected.push_back(bad);
    BaseGenResult r;
    GenerateBases(g.in, BaseGenOptions(), r);
    REQUIRE(r.bases.size() == 1);
    CHECK(S(r.bases[0].name) == "Oldfort Post");
    CHECK(r.bases[0].detected);
    CHECK(r.bases[0].x == Approx(4950.0f));
    bool reported = false;
    for (const std::string& n : r.notes)
    {
        reported = reported || n.find("Townfort Post") != std::string::npos;
    }
    CHECK(reported);
}

TEST_CASE("BaseGenerate - deterministic from the seed", "[guerrilla][basegenerate]")
{
    Grid a = ScenarioA();
    Grid b = ScenarioA();
    BaseGenResult ra, rb;
    GenerateBases(a.in, BaseGenOptions(), ra);
    GenerateBases(b.in, BaseGenOptions(), rb);
    REQUIRE(ra.bases.size() == rb.bases.size());
    for (size_t i = 0; i < ra.bases.size(); i++)
    {
        CHECK(S(ra.bases[i].name) == S(rb.bases[i].name));
        CHECK(ra.bases[i].x == rb.bases[i].x);
        CHECK(ra.bases[i].z == rb.bases[i].z);
    }
    // another seed still keeps every rule
    Grid c = ScenarioA();
    c.in.seed = 123456789;
    BaseGenResult rc;
    BaseGenOptions opt;
    GenerateBases(c.in, opt, rc);
    REQUIRE(rc.bases.size() == 1);
    CHECK(rc.uncoveredAfter == 0);
    CheckRules(c.in, opt, rc);
}

TEST_CASE("BaseGenerate - redundancy and the floor add bases, the ceiling stops them", "[guerrilla][basegenerate]")
{
    // six towns around one base: all covered, but the landmass has one
    // source of three; K 6 gives ceiling 3, so two are added
    Grid g(200);
    g.Road(1000, 5000, 9000, 5000, false);
    g.Road(5000, 1000, 5000, 9000, false);
    g.Source("Hub", 5200, 5200);
    const float towns[6][2] = {{4000, 5300}, {6400, 5300}, {5300, 4000}, {5300, 6400}, {4200, 4200}, {6200, 6200}};
    for (int i = 0; i < 6; i++)
    {
        g.Town(("Town " + std::to_string(i)).c_str(), towns[i][0], towns[i][1]);
    }
    BaseGenOptions opt;
    BaseGenResult r;
    GenerateBases(g.in, opt, r);
    CHECK(r.uncoveredBefore == 0);
    CHECK(r.ceiling == 3);
    CHECK(r.bases.size() == 2);
    CheckRules(g.in, opt, r);
    // at the ceiling already: nothing
    g.Source("Extra1", 1500, 1500);
    g.Source("Extra2", 8500, 8500);
    BaseGenResult full;
    GenerateBases(g.in, opt, full);
    CHECK(full.bases.empty());
}

TEST_CASE("BaseGenerate - a town on an island is served from its own landmass", "[guerrilla][basegenerate]")
{
    // deep water x 3000..3300 parts the map; Isleton is 1.3 km from Mainbase
    // across the water, so it is open, and only the island may serve it
    Grid g(128);
    g.Paint(3000, 0, 3300, 6400, [](GenCell& c) {
        c.deep = true;
        c.water = true;
    });
    g.Road(200, 3000, 2900, 3000, false);
    g.Road(3400, 3000, 6200, 3000, false);
    g.Source("Mainbase", 2500, 3200);
    g.Town("Westham", 1500, 3300);
    g.Town("Isleton", 3800, 3300);
    g.Town("Farpoint", 5800, 3300);
    BaseGenResult r;
    GenerateBases(g.in, BaseGenOptions(), r);
    CHECK(r.uncoveredBefore == 2);
    REQUIRE(!r.bases.empty());
    for (const GeneratedBase& b : r.bases)
    {
        CHECK(b.x > 3300.0f);
    }
    CHECK(r.uncoveredAfter == 0);
}

// ---------------------------------------------------------------------------
// step 3: object-free footprints, the minor kinds, the open-town diagnostics
// ---------------------------------------------------------------------------

TEST_CASE("BaseGenerate - the kind catalogue: footprints, road bands, what is placed", "[guerrilla][basegenerate]")
{
    const GenBaseKindInfo& outpost = GenBaseKindOf(GenBaseKind::Outpost);
    const GenBaseKindInfo& checkpoint = GenBaseKindOf(GenBaseKind::Checkpoint);
    const GenBaseKindInfo& coastal = GenBaseKindOf(GenBaseKind::Coastal);
    const GenBaseKindInfo& helipad = GenBaseKindOf(GenBaseKind::Helipad);
    CHECK(outpost.footprint == 1);
    CHECK(helipad.footprint == 1);
    CHECK(checkpoint.footprint == 0);
    CHECK(coastal.footprint == 0);
    CHECK(checkpoint.roadMax == Approx(100.0f));
    CHECK(outpost.roadMax == Approx(250.0f));
    CHECK(outpost.placed);
    CHECK(checkpoint.placed);
    CHECK(coastal.placed);
    CHECK_FALSE(helipad.placed); // an air origin first (HeldAirfield), never generated yet
    // every minor kind can source a town retake (spare = garrison - floor(garrison / 4) >= 4)
    for (const GenBaseKindInfo* k : {&checkpoint, &coastal, &helipad})
    {
        INFO(k->type);
        CHECK(k->garrisonMax - k->garrisonMax / 4 >= 4);
        CHECK_FALSE(k->countsForWin);
    }
    CHECK(std::string(coastal.noun) == "Coastal Post");
}

TEST_CASE("BaseGenerate - the scorer prefers an object-free footprint, never at the cost of a base",
          "[guerrilla][basegenerate]")
{
    BaseGenOptions opt;
    // the clear scenario's pick
    Grid g0 = ScenarioA();
    BaseGenResult r0;
    GenerateBases(g0.in, opt, r0);
    REQUIRE(r0.bases.size() == 1);
    const float x0 = r0.bases[0].x;
    const float z0 = r0.bases[0].z;
    CHECK(r0.bases[0].solids == 0);
    CHECK(r0.bases[0].trees == 0);
    // an orchard over that pick: the base moves to clear ground, still one
    // base covering both open towns
    Grid g1 = ScenarioA();
    for (int i = 0; i < 40; i++)
    {
        g1.Object(x0 - 50.0f + (float)(i % 8) * 14.0f, z0 - 40.0f + (float)(i / 8) * 20.0f, false);
    }
    BaseGenResult r1;
    GenerateBases(g1.in, opt, r1);
    REQUIRE(r1.bases.size() == 1);
    CHECK(D(r1.bases[0].x, r1.bases[0].z, x0, z0) > 50.0f);
    CHECK(r1.bases[0].trees == 0);
    CHECK(r1.uncoveredAfter == 0);
    CheckRules(g1.in, opt, r1);
    // one wall beside the new pick moves it again: a solid weighs nearly a town
    Grid g2 = g1;
    g2.Object(r1.bases[0].x + 30.0f, r1.bases[0].z, true, 6.0f);
    BaseGenResult r2;
    GenerateBases(g2.in, opt, r2);
    REQUIRE(r2.bases.size() == 1);
    CHECK(r2.bases[0].solids == 0);
    CHECK(D(r2.bases[0].x, r2.bases[0].z, r1.bases[0].x + 30.0f, r1.bases[0].z) >= opt.layoutReach);
    CHECK(r2.uncoveredAfter == 0);
    // clutter everywhere: still exactly one base (it never costs a base or
    // a covered town), and it reports what stands there
    Grid g3 = ScenarioA();
    for (float x = 100; x < 6400; x += 40)
    {
        for (float z = 100; z < 6400; z += 40)
        {
            g3.Object(x, z, false, 2.0f);
        }
    }
    BaseGenResult r3;
    GenerateBases(g3.in, opt, r3);
    REQUIRE(r3.bases.size() == 1);
    CHECK(r3.bases[0].trees > 0);
    CHECK(r3.uncoveredAfter == 0);
    // a detected outpost's own props are no clutter: it stays preferred
    Grid g4 = ScenarioA();
    GenDetected d;
    d.name = "Oldfort Post";
    d.x = 4950;
    d.z = 4200;
    d.y = 20;
    g4.in.detected.push_back(d);
    for (int i = 0; i < 6; i++)
    {
        g4.Object(4950.0f + (float)i * 8.0f, 4200.0f, true);
    }
    BaseGenResult r4;
    GenerateBases(g4.in, opt, r4);
    REQUIRE(r4.bases.size() == 1);
    CHECK(S(r4.bases[0].name) == "Oldfort Post");
}

TEST_CASE("BaseGenerate - a minor kind closes a gap no outpost footprint fits", "[guerrilla][basegenerate]")
{
    BaseGenOptions opt;
    {
        // a hillside: no 150 m square is under 0.10, single flat cells are
        Grid g = ScenarioA();
        g.Lattice(2);
        BaseGenResult r;
        GenerateBases(g.in, opt, r);
        CHECK(r.candidates > 0);
        CHECK(r.minorCandidates == r.candidates); // nothing fits an outpost
        REQUIRE(!r.bases.empty());
        CHECK(r.minor == (int)r.bases.size());
        for (const GeneratedBase& b : r.bases)
        {
            INFO(S(b.name));
            CHECK(b.kind == GenBaseKind::Checkpoint);
            CHECK(S(b.name).find("Checkpoint") != std::string::npos);
            // the checkpoint's road band: at the shoulder, 60 to 100 m out
            CHECK(b.roadDist >= opt.roadMin);
            CHECK(b.roadDist <= GenBaseKindOf(GenBaseKind::Checkpoint).roadMax);
        }
        CHECK(r.uncoveredAfter < r.uncoveredBefore);
        CheckRules(g.in, opt, r);
        bool noted = false;
        for (const std::string& n : r.notes)
        {
            noted = noted || n.find("no outpost site serves the open need") != std::string::npos;
        }
        CHECK(noted);
    }
    {
        // near the sea the minor kind is a coastal post
        Grid g = ScenarioA();
        g.Lattice(2);
        g.Paint(6250, 0, 6400, 6400, [](GenCell& c) {
            c.deep = true;
            c.water = true;
        });
        BaseGenOptions wide = opt;
        wide.coastDist = 3000.0f; // every site of the scenario is "near the sea"
        BaseGenResult r;
        GenerateBases(g.in, wide, r);
        REQUIRE(!r.bases.empty());
        CHECK(r.bases[0].kind == GenBaseKind::Coastal);
        CHECK(S(r.bases[0].name).find("Coastal Post") != std::string::npos);
    }
    {
        // an outpost site and minor sites both serving: the outpost wins
        Grid g = ScenarioA();
        g.Lattice(2);
        g.Paint(4850, 3900, 5300, 4500, [](GenCell& c) { c.gradient = 0; }); // one flat field by the track
        BaseGenResult r;
        GenerateBases(g.in, opt, r);
        REQUIRE(!r.bases.empty());
        CHECK(r.bases[0].kind == GenBaseKind::Outpost);
        CHECK(r.minor == 0);
    }
    {
        // the minor kinds off: the town stays open, and the log names the
        // near misses (cells one rule short of a site) with the nearest one
        Grid g = ScenarioA();
        g.Lattice(2);
        BaseGenOptions off = opt;
        off.minorKinds = false;
        BaseGenResult r;
        GenerateBases(g.in, off, r);
        CHECK(r.bases.empty());
        CHECK(r.uncoveredAfter == 2);
        bool open = false, near = false;
        for (const std::string& n : r.notes)
        {
            open = open || n.find("Northend is open") != std::string::npos;
            near = near || (n.find("Northend near misses") != std::string::npos &&
                            n.find("slope") != std::string::npos && n.find("nearest [") != std::string::npos);
        }
        CHECK(open);
        CHECK(near);
    }
}

// ---------------------------------------------------------------------------
// the registry: the seeding entry, the save round trip, gmGeneratedBases
// ---------------------------------------------------------------------------

namespace
{

// scenario A as a world
struct FakeSites final : BaseSiteProbe
{
    bool enabled = true;
    bool shifted = false; // a different world: its track lies 1 km further east
    mutable int calls = 0;
    bool Gather(BaseGenInputs& in) const override
    {
        calls++;
        if (!enabled)
        {
            return false;
        }
        Grid g(128);
        g.Road(200, 3200, 6200, 3200, false);
        g.Road(shifted ? 5800.0f : 4800.0f, 3200, shifted ? 5800.0f : 4800.0f, 6000, true);
        in.cellsX = g.in.cellsX;
        in.cellsZ = g.in.cellsZ;
        in.cellSize = g.in.cellSize;
        in.cells = g.in.cells;
        in.roads = g.in.roads;
        return true;
    }
};

const char* kGenConfig = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    generateBases = 1;\n"
                         "    generateBasesSeed = 7;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Camp  { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; "
                         "position[]={3200.0, 1000.0, 20.0}; };\n"
                         "        class Base0 { name=\"Base0\"; type=\"OUTPOST\"; owner=\"OCCUPIER\"; garrison=6; "
                         "position[]={1200.0, 3500.0, 20.0}; };\n"
                         "        class Westford  { name=\"Westford\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={1000.0, 3000.0, 20.0}; };\n"
                         "        class Southgate { name=\"Southgate\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={4400.0, 3000.0, 20.0}; };\n"
                         "        class Northend  { name=\"Northend\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={5200.0, 5200.0, 20.0}; };\n"
                         "    };\n"
                         "};\n"
                         "class CfgGuerrillaFactions\n"
                         "{\n"
                         "    class East { side=\"EAST\"; tiers[]={\"SoldierEB\"}; };\n"
                         "    class Guer { side=\"GUER\"; holdClass=\"SoldierGB\"; };\n"
                         "};\n";

std::string Replace(const std::string& s, const char* from, const char* to)
{
    std::string out(s);
    out.replace(out.find(from), strlen(from), to);
    return out;
}

struct GenFixture
{
    ParamFile file;
    ZoneRegistry registry;
    FakeSites sites;
    void Load(const std::string& config)
    {
        QIStream in(config.c_str(), config.size());
        file.Parse(in);
        registry.SetSiteProbeOverride(&sites);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"));
    }
    const ZoneRecord* Generated() const
    {
        for (int i = 0; i < registry.NZones(); i++)
        {
            if (registry.GetZone(i)->generated)
            {
                return registry.GetZone(i);
            }
        }
        return nullptr;
    }
};

} // namespace

TEST_CASE("BaseGenerate - the registry seeds a generated OUTPOST behind generateBases", "[guerrilla][basegenerate]")
{
    {
        GenFixture f;
        f.Load(kGenConfig);
        CHECK(f.registry.Tuning().generateBases);
        REQUIRE(f.registry.NGeneratedBases() == 1);
        const ZoneRecord* z = f.Generated();
        REQUIRE(z != nullptr);
        CHECK(S(z->name) == S(f.registry.GeneratedBaseAt(0)->name));
        CHECK(S(z->type) == "OUTPOST");
        CHECK(S(z->ownerConfig) == "OCCUPIER");
        CHECK(S(z->owner) == S(f.registry.OccupierSide()));
        CHECK(z->seeded);
        CHECK(z->countsForWin);
        CHECK(z->garrison == Approx(6.0f));
        CHECK(z->garrisonMax == Approx(6.0f));
        CHECK(S(z->marker) == "gmZoneGen_0");
        CHECK(f.registry.IsGarrisonable(*z));
        // the conservation stand-in: not authored, so the pool's R skips it
        CHECK_FALSE(CampaignEnd::IsAuthoredOccupier(*z, f.registry));
        CHECK(CampaignEnd::IsGeneratedOccupier(*z, f.registry));
        // the win roster takes it
        CampaignEnd end;
        end.TakeRoster(f.registry);
        CHECK(end.RosterSize() == 2);
        CHECK(f.registry.GeneratedBaseAt(0)->layoutSeed != 0);
        // a second load in the same session re-injects, never re-derives
        f.sites.shifted = true;
        const float x = z->pos.X();
        f.registry.LoadFromParams(f.file.FindEntry("CfgGuerrillaZones"), f.file.FindEntry("CfgGuerrillaFactions"));
        REQUIRE(f.Generated() != nullptr);
        CHECK(f.Generated()->pos.X() == x);
    }
    {
        GenFixture f;
        f.Load(Replace(kGenConfig, "generateBases = 1;", "generateBases = 0;"));
        CHECK_FALSE(f.registry.Tuning().generateBases);
        CHECK(f.registry.NGeneratedBases() == 0);
        CHECK(f.Generated() == nullptr);
        CHECK(f.sites.calls == 0);
    }
    {
        // absent = off
        GenFixture f;
        f.Load(Replace(kGenConfig, "generateBases = 1;", ""));
        CHECK(f.registry.NGeneratedBases() == 0);
    }
    {
        // no world: nothing
        GenFixture f;
        f.sites.enabled = false;
        f.Load(kGenConfig);
        CHECK(f.registry.NGeneratedBases() == 0);
    }
}

namespace
{

// a flat dry world with a four-tower compound at [4950, 4200]: the base
// detector reports it as an OUTPOST candidate (status offline)
struct FlatHeight final : HeightField
{
    float Height(float, float) const override { return 20.0f; }
};
struct TowerWorld final : BaseWorldProbe
{
    FlatHeight height;
    bool Gather(BaseDetectInputs& in) const override
    {
        in.worldClass = "Testland";
        in.worldDisplay = "Testland";
        in.worldSize = 6400.0f;
        in.height = &height;
        const float pts[4][2] = {{4920, 4170}, {4980, 4170}, {4920, 4230}, {4980, 4230}};
        for (const auto& p : pts)
        {
            DetectObject o;
            o.x = p[0];
            o.z = p[1];
            o.model = "data3d\\hlaska.p3d";
            o.radius = 4.0f;
            in.objects.push_back(o);
        }
        return true;
    }
};

} // namespace

TEST_CASE("BaseGenerate - the detector's OUTPOST candidate becomes the generated base", "[guerrilla][basegenerate][bases]")
{
    ParamFile file;
    {
        QIStream in(kGenConfig, strlen(kGenConfig));
        file.Parse(in);
    }
    FakeSites sites;
    TowerWorld world;
    ZoneRegistry registry;
    registry.SetSiteProbeOverride(&sites);
    registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"), nullptr,
                            nullptr, nullptr, nullptr, nullptr, &world);
    REQUIRE(registry.BaseDetectionRan());
    REQUIRE(registry.DetectedOutposts().size() == 1);
    const BaseCandidate& c = registry.DetectedOutposts()[0];
    CHECK(c.status == BaseStatus::Offline);
    CHECK(registry.NSeededBases() == 0); // seedBases never seeds an OUTPOST
    REQUIRE(registry.NGeneratedBases() == 1);
    const GeneratedBaseRow* row = registry.GeneratedBaseAt(0);
    CHECK(S(row->name) == S(c.name));
    CHECK(row->detected);
    CHECK(row->x == Approx(c.x));
    CHECK(row->z == Approx(c.z));
    const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(c.name));
    REQUIRE(z != nullptr);
    CHECK(z->generated);
    CHECK(S(z->type) == "OUTPOST");
}

TEST_CASE("BaseGenerate - minor posts stay out of the win roster", "[guerrilla][basegenerate][campaignend]")
{
    ZoneRecord z;
    z.type = "OUTPOST";
    CHECK(CampaignEnd::CountsForWin(z));
    z.countsForWin = false;
    CHECK_FALSE(CampaignEnd::CountsForWin(z));
    z.countsForWin = true;
    z.type = "CHECKPOINT";
    CHECK_FALSE(CampaignEnd::CountsForWin(z));
    CHECK(CampaignEnd::IsMinorPostType("helipad"));
    CHECK(CampaignEnd::IsMinorPostType("COASTAL"));
    CHECK(CampaignEnd::IsMinorPostType("RADAR"));
    CHECK_FALSE(CampaignEnd::IsMinorPostType("SEAPORT"));
    // an authored checkpoint: military, the occupier's, but not in the roster
    GenFixture f;
    f.Load(Replace(Replace(kGenConfig, "generateBases = 1;", "generateBases = 0;"), "        class Westford",
                   "        class Gate { name=\"Gate\"; type=\"CHECKPOINT\"; owner=\"OCCUPIER\"; garrison=5; "
                   "position[]={3000.0, 3000.0, 20.0}; };\n        class Westford"));
    REQUIRE(f.registry.FindZoneIndex("Gate") >= 0);
    CampaignEnd end;
    end.TakeRoster(f.registry);
    CHECK(end.RosterSize() == 1);
    CHECK(S(end.RosterName(0)) == "Base0");
}

TEST_CASE("BaseGenerate - stored rows survive save/load unchanged; an old save generates nothing",
          "[guerrilla][basegenerate][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "zone-registry-generated.bin";
    {
        QIStream in(kGenConfig, strlen(kGenConfig));
        ExtParsMission.Parse(in);
    }
    RString savedName;
    float savedX = 0, savedZ = 0;
    {
        GenFixture f;
        f.Load(kGenConfig);
        REQUIRE(f.registry.NGeneratedBases() == 1);
        const int gi = f.registry.FindZoneIndex(f.Generated()->name);
        ZoneRecord& z = *f.registry.GetZoneMutable(gi);
        savedName = z.name;
        savedX = z.pos.X();
        savedZ = z.pos.Z();
        z.owner = f.registry.ResistanceSide(); // liberated
        z.garrison = 2;
        z.lostCount = 1;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.registry.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }
    SECTION("a different world, even none at all, reloads the stored base")
    {
        for (int variant = 0; variant < 2; variant++)
        {
            FakeSites other;
            other.shifted = variant == 0;
            other.enabled = variant == 0;
            ZoneRegistry loaded;
            loaded.SetSiteProbeOverride(&other);
            ParamArchiveLoad ar;
            REQUIRE(ar.LoadBin(archivePath.string().c_str()));
            ar.FirstPass();
            REQUIRE(loaded.Serialize(ar) == LSOK);
            ar.SecondPass();
            REQUIRE(loaded.Serialize(ar) == LSOK);
            CHECK(other.calls == 0); // never re-derived
            REQUIRE(loaded.NGeneratedBases() == 1);
            CHECK(S(loaded.GeneratedBaseAt(0)->name) == S(savedName));
            const ZoneRecord* z = loaded.GetZone(loaded.FindZoneIndex(savedName));
            REQUIRE(z != nullptr);
            CHECK(z->generated);
            CHECK(z->pos.X() == savedX);
            CHECK(z->pos.Z() == savedZ);
            CHECK(z->garrisonMax == Approx(6.0f));
            // the dynamic state overlays it like any zone row
            CHECK(S(z->owner) == S(loaded.ResistanceSide()));
            CHECK(z->garrison == Approx(2.0f));
            CHECK(z->lostCount == 1);
            CHECK(loaded.IsHeldBase(*z));
            CHECK(loaded.NZones() == 6);
        }
    }
    SECTION("a save from before the feature generates nothing")
    {
        const std::filesystem::path oldPath = dir / "zone-registry-pregenerated.bin";
        {
            ParamArchiveSave ar(WorldSerializeVersion);
            int v = 1;
            REQUIRE(ar.Serialize("guerrillaSaveVersion", v, 1, 0) == LSOK);
            REQUIRE(ar.SaveBin(oldPath.string().c_str()));
        }
        FakeSites world;
        ZoneRegistry loaded;
        loaded.SetSiteProbeOverride(&world);
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(oldPath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        CHECK(loaded.Tuning().generateBases);
        CHECK(loaded.NGeneratedBases() == 0);
        CHECK(world.calls == 0);
        CHECK(loaded.NZones() == 5);
        std::filesystem::remove(oldPath);
    }
    ExtParsMission.Clear();
    std::filesystem::remove(archivePath);
}

TEST_CASE("BaseGenerate - gmGeneratedBases lists the rows", "[guerrilla][basegenerate]")
{
    GGameState.Reset();
    Poseidon::Foundation::InitModules();
    ParamFile file;
    {
        QIStream in(kGenConfig, strlen(kGenConfig));
        file.Parse(in);
    }
    FakeSites sites;
    ZoneRegistry& registry = ZoneRegistry::Instance();
    registry.Clear();
    {
        GameValue none = GGameState.Evaluate("gmGeneratedBases");
        REQUIRE(none.GetType() == GameArray);
        CHECK(((const GameArrayType&)none).Size() == 0);
    }
    registry.SetSiteProbeOverride(&sites);
    registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"));
    GameValue rows = GGameState.Evaluate("gmGeneratedBases");
    REQUIRE(rows.GetType() == GameArray);
    const GameArrayType& array = rows;
    REQUIRE(array.Size() == 1);
    const GameArrayType& row = array[0];
    REQUIRE(row.Size() == 8);
    CHECK(S((RString)(GameStringType)row[0]) == S(registry.GeneratedBaseAt(0)->name));
    CHECK(S((RString)(GameStringType)row[1]) == "OUTPOST");
    const GameArrayType& pos = row[2];
    REQUIRE(pos.Size() == 3);
    CHECK((float)pos[0] == Approx(registry.GeneratedBaseAt(0)->x));
    CHECK((float)pos[1] == Approx(registry.GeneratedBaseAt(0)->z));
    CHECK((float)row[3] == Approx(6.0f));
    CHECK((bool)row[4]);
    registry.Clear();
}
