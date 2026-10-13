// Occupier counter-attack service (issue #79, CounterAttack.hpp): dispatch on
// a town's flip (the first town a 0.4 roll, every later town answered), the
// war-level force size, reserve debit and write-back, the
// abstract off-screen resolution against the zone tick's retake drain, the
// odds roll, the off switch and the save contract.  Pure: the registry is
// built from inline config, flips run through EvaluateTick, the service is
// driven through OnTownFlipped / Step with no world (ops = null).

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/AI/AI.hpp> // AIGroup (the record's group link)
#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <functional>
#include <math.h>
#include <string.h>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// A Camp, a town 300 m north of it, a second town far away, a post 671 m
// from the town (garrison 6) and a far base (garrison 12).  The town's
// garrison is wiped in the fixture so fighters can raise it.
const char* kCounterConfig =
    "class CfgGuerrillaZones\n"
    "{\n"
    "    class Zones\n"
    "    {\n"
    "        class Camp    { name=\"Camp\";    type=\"CAMP\";    owner=\"RESISTANCE\"; "
    "position[]={1000.0, 1000.0, 0.0}; };\n"
    "        class Town    { name=\"Town\";    type=\"CITY\";    owner=\"NEUTRAL\"; support=20; "
    "position[]={1000.0, 1300.0, 0.0}; };\n"
    "        class Other   { name=\"Other\";   type=\"CITY\";    owner=\"NEUTRAL\"; support=20; "
    "position[]={5000.0, 5000.0, 0.0}; };\n"
    "        class Post    { name=\"Post\";    type=\"OUTPOST\"; owner=\"OCCUPIER\"; garrison=6; "
    "position[]={1600.0, 1600.0, 0.0}; };\n"
    "        class FarBase { name=\"FarBase\"; type=\"AIRFIELD\"; owner=\"OCCUPIER\"; garrison=12; "
    "position[]={9000.0, 9000.0, 0.0}; };\n"
    "    };\n"
    "};\n";

std::string ConfigWith(const char* keys)
{
    std::string cfg = kCounterConfig;
    const std::string anchor = "class CfgGuerrillaZones\n{\n";
    cfg.insert(anchor.size(), std::string("    ") + keys + "\n");
    return cfg;
}

// the first town answered for certain: the resolution cases need a force
std::string Certain(const char* keys = "")
{
    return ConfigWith((std::string("counterAttackFirstTownChance = 1; ") + keys).c_str());
}

struct Fixture
{
    std::string text;
    ParamFile file;
    ZoneRegistry registry;
    CounterAttack ca;
    int camp = -1, town = -1, other = -1, post = -1, farBase = -1;
    static constexpr int GroupSize = 12;

    void Load(const std::string& config)
    {
        text = config;
        QIStream in(text.c_str(), text.size());
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        ca.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        camp = registry.FindZoneIndex("Camp");
        town = registry.FindZoneIndex("Town");
        other = registry.FindZoneIndex("Other");
        post = registry.FindZoneIndex("Post");
        farBase = registry.FindZoneIndex("FarBase");
        REQUIRE(farBase >= 0);
        Z(town).garrison = 0; // the town's defenders are already dead
    }
    ZoneRecord& Z(int i) { return *registry.GetZoneMutable(i); }

    // zone tick inputs: the player at (px, pz), guer fighters in one zone
    ZoneTickInputs Inputs(float px, float pz, int guerZone, int guer, int abstractZone = -1, int abstractBodies = 0)
    {
        ZoneTickInputs in;
        const int n = registry.NZones();
        in.guerCount.Resize(n);
        in.occCount.Resize(n);
        in.abstractOcc.Resize(n);
        for (int i = 0; i < n; i++)
        {
            in.guerCount[i] = 0;
            in.occCount[i] = 0;
            in.abstractOcc[i] = 0;
        }
        in.playerValid = true;
        in.playerX = px;
        in.playerZ = pz;
        if (guerZone >= 0)
        {
            in.guerCount[guerZone] = guer;
        }
        if (abstractZone >= 0)
        {
            in.abstractOcc[abstractZone] = abstractBodies;
        }
        return in;
    }

    // raise the town with fighters (player standing in it); returns the
    // captured events
    AutoArray<ZoneEventRecord> RaiseTown(int zone)
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = Inputs(Z(zone).pos.X(), Z(zone).pos.Z(), zone, 2);
        for (int tick = 0; tick < 20 && stricmp(Z(zone).owner, "GUER") != 0; tick++)
        {
            registry.EvaluateTick(in, fired);
        }
        REQUIRE(Str(Z(zone).owner) == "GUER");
        Z(zone).lastGuer = 0; // the fighters walk out
        return fired;
    }

    static CounterAttackView View(float px, float pz)
    {
        CounterAttackView v;
        v.playerValid = true;
        v.playerX = px;
        v.playerZ = pz;
        return v;
    }

    static std::string Str(const RString& s) { return std::string((const char*)s); }
};

int CountEvents(const AutoArray<ZoneEventRecord>& fired, ZoneEventType type, int zone)
{
    int n = 0;
    for (int i = 0; i < fired.Size(); i++)
    {
        n += fired[i].type == type && fired[i].zoneIndex == zone ? 1 : 0;
    }
    return n;
}

// the player far away from everything (off-screen)
constexpr float FarX = 20000.0f;
constexpr float FarZ = 20000.0f;

} // namespace

TEST_CASE("CounterAttack - config defaults, keys and range checks", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
    const CounterAttackTuning& t = f.ca.Tuning();
    CHECK(t.enabled);
    CHECK(t.size == Approx(0.5f));
    CHECK(t.firstTownChance == Approx(0.4f));
    CHECK(t.chance == Approx(1.0f));
    CHECK(t.warLevelStep == Approx(0.25f));
    CHECK(t.delayMin == Approx(120.0f));
    CHECK(t.delayMax == Approx(300.0f));
    CHECK(t.defenderWeight == Approx(3.0f));
    CHECK(t.maxLive == 2);
    CHECK(f.registry.Tuning().townRevertSupport == Approx(30.0f));
    CHECK(f.ca.TownsTaken() == 0);

    Fixture g;
    g.Load(ConfigWith("counterAttackChance = 3; counterAttackFirstTownChance = -1; counterAttackDelayMin = 50; "
                      "counterAttackDelayMax = 10; counterAttackSize = -1; counterAttackWarLevelStep = 9;"));
    CHECK(g.ca.Tuning().chance == Approx(1.0f));
    CHECK(g.ca.Tuning().firstTownChance == Approx(0.0f));
    CHECK(g.ca.Tuning().delayMin == Approx(50.0f));
    CHECK(g.ca.Tuning().delayMax == Approx(50.0f)); // raised to the minimum
    CHECK(g.ca.Tuning().size == Approx(0.0f));
    CHECK(g.ca.Tuning().warLevelStep == Approx(2.0f));
}

TEST_CASE("CounterAttack - pure rules: chance, size, odds, travel, spare", "[game][guerrilla][counterattack]")
{
    CounterAttackTuning t;
    // the first town ever taken: a 0.4 chance; every later town: certain, no
    // decay however many are held
    CHECK(CounterAttack::DispatchChance(t, 0) == Approx(0.4f));
    for (int k = 1; k < 12; k++)
    {
        CHECK(CounterAttack::DispatchChance(t, k) == Approx(1.0f));
    }
    // the force: counterAttackSize * groupSize, scaled UP by the war level
    CHECK(CounterAttack::ForceSize(t, 12, 1.0f) == 6);
    CHECK(CounterAttack::ForceSize(t, 12, 3.0f) == 9);                        // x1.5
    CHECK(CounterAttack::ForceSize(t, 12, 5.0f) == 12);                       // x2
    CHECK(CounterAttack::ForceSize(t, 12, 0.0f) == 6);                        // never scaled down
    CHECK(CounterAttack::ForceSize(t, 12, 10.0f) == CounterAttack::MaxForce); // one group at most
    for (int wl = 1; wl < 10; wl++)
    {
        CHECK(CounterAttack::ForceSize(t, 12, (float)(wl + 1)) >= CounterAttack::ForceSize(t, 12, (float)wl));
    }
    CounterAttackTuning flat;
    flat.warLevelStep = 0;
    CHECK(CounterAttack::ForceSize(flat, 12, 6.0f) == 6);
    CounterAttackTuning tiny;
    tiny.size = 0.01f;
    CHECK(CounterAttack::ForceSize(tiny, 12, 1.0f) == 1); // never below one body
    // switched off: never answered
    CounterAttackTuning off;
    off.enabled = false;
    CHECK(CounterAttack::DispatchChance(off, 0) == Approx(0.0f));
    CHECK(CounterAttack::DispatchChance(off, 3) == Approx(0.0f));
    // the decision seam: go = roll < chance, size only when going
    CounterAttackContext ctx;
    ctx.groupSize = 12;
    ctx.warLevel = 3.0f;
    ctx.seed = CounterAttack::MakeSeed("Town", 1, 600, 1);
    const CounterAttackDecision later = CounterAttack::Decide(t, "Town", 1, ctx);
    CHECK(later.go);
    CHECK(later.size == 9);
    CHECK(later.roll == Approx(CounterAttack::Roll01(ctx.seed, 0)));
    const CounterAttackDecision none = CounterAttack::Decide(off, "Town", 1, ctx);
    CHECK_FALSE(none.go);
    CHECK(none.size == 0);
    // the 60/40 baseline: three fighters against six attackers
    CHECK(CounterAttack::HoldOdds(t, 3, 6) == Approx(0.6f));
    CHECK(CounterAttack::HoldOdds(t, 0, 6) == Approx(0.0f));
    CHECK(CounterAttack::HoldOdds(t, 3, 0) == Approx(1.0f));
    CHECK(CounterAttack::HoldOdds(t, 3, 4) > CounterAttack::HoldOdds(t, 3, 6));
    // travel covers the whole road leg from the base to the town
    CHECK(CounterAttack::TravelTime(t, 2500.0f) == Approx(2500.0f * 1.3f / 4.0f));
    CHECK(CounterAttack::TravelTime(t, 20.0f) == Approx(CounterAttack::MinTravel));
    // a base keeps a quarter of its strength home
    CHECK(CounterAttack::Spare(t, 6.0f, 0, 6.0f, false) == 5);
    CHECK(CounterAttack::Spare(t, 0.0f, 6, 6.0f, true) == 5);
    CHECK(CounterAttack::Spare(t, 0.0f, 6, 6.0f, false) == 0); // a live garrison has no stored reserve
    CHECK(CounterAttack::Spare(t, 1.0f, 0, 6.0f, false) == 0);
    CHECK(CounterAttack::Spare(t, 12.0f, 0, 12.0f, false) == 9);
}

TEST_CASE("CounterAttack - rolls are deterministic and match the 60/40 baseline", "[game][guerrilla][counterattack]")
{
    CHECK(CounterAttack::Roll01(1234u, 2) == CounterAttack::Roll01(1234u, 2));
    CHECK(CounterAttack::Roll01(1234u, 2) != CounterAttack::Roll01(1234u, 3));
    CHECK(CounterAttack::MakeSeed("Town", 1, 600, 0) == CounterAttack::MakeSeed("Town", 1, 600, 0));
    CHECK(CounterAttack::MakeSeed("Town", 1, 600, 0) != CounterAttack::MakeSeed("Town", 1, 601, 0));

    // across many flips (towns, days, minutes) three fighters hold against
    // six attackers about 60% of the time
    CounterAttackTuning t;
    const float hold = CounterAttack::HoldOdds(t, 3, 6);
    const char* towns[] = {"Houdan", "Le Port", "Goisse", "Larche", "Cancon"};
    int held = 0;
    int total = 0;
    for (const char* name : towns)
    {
        for (int day = 1; day <= 8; day++)
        {
            for (int minute = 0; minute < 1440; minute += 15)
            {
                const unsigned int seed = CounterAttack::MakeSeed(name, day, minute, 0);
                const float roll = CounterAttack::Roll01(seed, 2);
                REQUIRE(roll >= 0.0f);
                REQUIRE(roll < 1.0f);
                held += roll < hold ? 1 : 0;
                total++;
            }
        }
    }
    const float share = (float)held / (float)total;
    CHECK(share > 0.57f);
    CHECK(share < 0.63f);
}

TEST_CASE("CounterAttack - the first town is answered about 40% of the time, later towns always",
          "[game][guerrilla][counterattack]")
{
    CounterAttackTuning t;
    const char* towns[] = {"Houdan", "Le Port", "Goisse", "Larche", "Cancon", "Meaux", "Chapoi"};
    int first = 0;
    int later = 0;
    int total = 0;
    for (const char* name : towns)
    {
        for (int day = 1; day <= 6; day++)
        {
            for (int minute = 0; minute < 1440; minute += 20)
            {
                CounterAttackContext ctx;
                ctx.seed = CounterAttack::MakeSeed(name, day, minute, 0);
                first += CounterAttack::Decide(t, name, 0, ctx).go ? 1 : 0;
                for (int k = 1; k <= 4; k++)
                {
                    ctx.seed = CounterAttack::MakeSeed(name, day, minute, k);
                    later += CounterAttack::Decide(t, name, k, ctx).go ? 1 : 0;
                }
                total++;
            }
        }
    }
    const float share = (float)first / (float)total;
    CHECK(share > 0.37f);
    CHECK(share < 0.43f);
    CHECK(later == total * 4);
}

TEST_CASE("CounterAttack - a town's flip schedules exactly one counter-attack", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    AutoArray<ZoneEventRecord> fired = f.RaiseTown(f.town);
    REQUIRE(CountEvents(fired, ZECaptured, f.town) == 1);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    REQUIRE(f.ca.Count() == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(Fixture::Str(r->target) == "Town");
    CHECK(r->phase == CAScheduled);
    CHECK(r->size == 6);
    CHECK(r->townsBefore == 0);
    CHECK(f.ca.TownsTaken() == 1);
    CHECK(r->clock >= 120.0f);
    CHECK(r->clock <= 300.0f);
    // the same flip seen again does not add a second force
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Count() == 1);
    // not a flip: an administered town, a base, the Camp
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize));
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.post, Fixture::GroupSize));
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.camp, Fixture::GroupSize));
    // the start-town HQ is never a target and does not count as a town taken:
    // the next town is still the first (k = 0)
    Fixture hq;
    hq.Load(Certain());
    hq.registry.ExemptTownGarrison(hq.town);
    hq.RaiseTown(hq.town);
    CHECK_FALSE(hq.ca.OnTownFlipped(hq.registry, hq.town, Fixture::GroupSize));
    CHECK(hq.ca.TownsTaken() == 0);
    hq.Z(hq.other).garrison = 0;
    hq.RaiseTown(hq.other);
    REQUIRE(hq.ca.OnTownFlipped(hq.registry, hq.other, Fixture::GroupSize));
    CHECK(hq.ca.Get(0)->townsBefore == 0);
}

TEST_CASE("CounterAttack - counterAttack = 0 restores the old behaviour", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("counterAttack = 0; counterAttackFirstTownChance = 1;"));
    CHECK_FALSE(f.ca.Tuning().enabled);
    f.RaiseTown(f.town);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Count() == 0);
    CHECK(f.ca.TownsTaken() == 0);
    // a second town: still nothing
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize));
    CHECK(f.ca.Count() == 0);
}

TEST_CASE("CounterAttack - support gains are the plain supportRate whatever is held (no momentum)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
    f.RaiseTown(f.town); // one town held
    f.Z(f.other).garrison = 0;
    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs in = f.Inputs(f.Z(f.other).pos.X(), f.Z(f.other).pos.Z(), f.other, 1);
    f.registry.EvaluateTick(in, fired);
    CHECK(f.Z(f.other).support == Approx(25.0f));
}

TEST_CASE("CounterAttack - from the second town on every flip is answered", "[game][guerrilla][counterattack]")
{
    // the first town is never answered here; the second always is, with the
    // full force, and so is the first town taken again after it fell
    Fixture f;
    f.Load(ConfigWith("counterAttackFirstTownChance = 0;"));
    f.RaiseTown(f.town);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.TownsTaken() == 1);
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize));
    CHECK(f.ca.Get(0)->townsBefore == 1);
    CHECK(f.ca.Get(0)->size == 6); // no decay
    CHECK(f.ca.TownsTaken() == 2);
    // Town falls and is taken again: k counts towns taken, not held, so it
    // is no longer a first town
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.registry.RevertTown(f.town, fired));
    f.Z(f.town).garrison = 0;
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Get(1)->townsBefore == 2);
    CHECK(f.ca.Count() == 2); // both towns are under attack

    // towns held at the start count: a mission that opens with a town in
    // resistance hands has no "first town"
    Fixture g;
    g.Load(ConfigWith("counterAttackFirstTownChance = 0;"));
    g.Z(g.other).owner = "GUER";
    g.RaiseTown(g.town);
    REQUIRE(g.ca.OnTownFlipped(g.registry, g.town, Fixture::GroupSize));
    CHECK(g.ca.Get(0)->townsBefore == 1);

    // later towns are always answered, whatever the flip stamp
    for (int minute = 0; minute < 120; minute++)
    {
        Fixture h;
        h.Load(ConfigWith("counterAttackFirstTownChance = 0;"));
        h.ca.SetTownsTaken(1 + minute % 5);
        h.RaiseTown(h.town);
        h.Z(h.town).flipDay = 2;
        h.Z(h.town).flipMinute = minute;
        CHECK(h.ca.OnTownFlipped(h.registry, h.town, Fixture::GroupSize));
    }
}

TEST_CASE("CounterAttack - the war level scales the force up, capped at one group", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize, 3.0f));
    CHECK(f.ca.Get(0)->size == 9);

    Fixture g;
    g.Load(Certain());
    g.RaiseTown(g.town);
    REQUIRE(g.ca.OnTownFlipped(g.registry, g.town, Fixture::GroupSize, 6.0f));
    CHECK(g.ca.Get(0)->size == CounterAttack::MaxForce);

    // the base's spare still caps what leaves: Post spares 5 of its 6
    AutoArray<ZoneEventRecord> fired;
    g.ca.Step(g.registry, Fixture::View(FarX, FarZ), 300.0f, nullptr, fired);
    REQUIRE(g.ca.Get(0)->phase == CATravel);
    CHECK(Fixture::Str(g.ca.Get(0)->source) == "Post");
    CHECK(g.ca.Get(0)->strength == 5);
}

TEST_CASE("CounterAttack - the launch seam sends a chosen force from a chosen base", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("counterAttackFirstTownChance = 0;"));
    f.RaiseTown(f.town);
    REQUIRE_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    // the far base, not the nearest, with three men
    REQUIRE(f.ca.LaunchFrom(f.registry, far, f.town, f.farBase, 3, nullptr, fired));
    REQUIRE(f.ca.Count() == 1);
    CHECK(Fixture::Str(f.ca.Get(0)->source) == "FarBase");
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->strength == 3);
    CHECK(f.Z(f.farBase).garrison == Approx(9.0f));
    // one record per town; never against an administered town, from a
    // resistance zone, or more than the base can spare
    CHECK_FALSE(f.ca.LaunchFrom(f.registry, far, f.town, f.post, 2, nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFrom(f.registry, far, f.other, f.post, 2, nullptr, fired));
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    CHECK_FALSE(f.ca.LaunchFrom(f.registry, far, f.other, f.camp, 2, nullptr, fired));
    f.Z(f.post).garrison = 1; // keeps its one man home
    CHECK_FALSE(f.ca.LaunchFrom(f.registry, far, f.other, f.post, 2, nullptr, fired));
    f.Z(f.post).garrison = 6;
    REQUIRE(f.ca.LaunchFrom(f.registry, far, f.other, f.post, 12, nullptr, fired));
    CHECK(f.ca.Get(1)->strength == 5);
    // the force resolves on the ordinary path: it arrives and holds
    f.ca.Step(f.registry, far, 5000.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->phase == CAHolding);
    CHECK(CountEvents(fired, ZECounterAttack, f.town) == 1);
}

TEST_CASE("CounterAttack - no base with a reserve: no counter-attack", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.Z(f.post).garrison = 1;    // keeps its one man home (25% of 6)
    f.Z(f.farBase).garrison = 0; // wiped
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, Fixture::View(FarX, FarZ), 400.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(fired.Size() == 0);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
}

TEST_CASE("CounterAttack - off-screen: an empty town is drained and reverted after the travel time",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;

    // the delay runs out: the nearest base (Post) sends 5 of its 6, keeping 1
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(f.ca.Count() == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(r->phase == CATravel);
    CHECK(Fixture::Str(r->source) == "Post");
    CHECK(r->strength == 5);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
    const float travel = CounterAttack::TravelTime(f.ca.Tuning(), 670.82f);
    CHECK(r->clock == Approx(travel).margin(0.5f));

    // still on the road: nothing reaches the town
    f.ca.Step(f.registry, far, travel - 5.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.AbstractPresence(f.registry, f.town) == 0);

    // arrival in an empty town: the force holds it
    f.ca.Step(f.registry, far, 6.0f, nullptr, fired);
    CHECK(CountEvents(fired, ZECounterAttack, f.town) == 1);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.AbstractPresence(f.registry, f.town) == 5);

    // the zone tick drains it off-screen; the town holds until support
    // falls below townRevertSupport (30), not supportFlip (60)
    f.Z(f.town).support = 60.0f;
    ZoneTickInputs in = f.Inputs(FarX, FarZ, -1, 0, f.town, f.ca.AbstractPresence(f.registry, f.town));
    for (int tick = 0; tick < 6; tick++)
    {
        f.registry.EvaluateTick(in, fired);
    }
    CHECK(f.Z(f.town).lastRemote);
    CHECK(f.Z(f.town).support == Approx(30.0f));
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    f.registry.EvaluateTick(in, fired);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "NEUTRAL");
    CHECK(CountEvents(fired, ZETownLost, f.town) == 1);
    CHECK(f.Z(f.town).garrison == Approx(4.0f)); // the force is the new garrison

    // the record finishes on the next step; nothing goes back to the post
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
}

TEST_CASE("CounterAttack - off-screen: fighters in the town face one odds roll", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    const unsigned int seed = f.ca.Get(0)->seed;
    const int attackers = f.ca.Get(0)->strength;
    REQUIRE(attackers == 5);

    // three fighters were left in the town (the last zone tick saw them)
    f.Z(f.town).lastGuer = 3;
    f.ca.Step(f.registry, far, 1000.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(CountEvents(fired, ZECounterAttack, f.town) == 1);
    const bool repelled = CounterAttack::Roll01(seed, 2) < CounterAttack::HoldOdds(f.ca.Tuning(), 3, attackers);
    if (repelled)
    {
        // half the force (rounded up) is lost; the survivors go home
        CHECK(CountEvents(fired, ZECounterRepelled, f.town) == 1);
        CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
        CHECK(f.Z(f.post).garrison == Approx(1.0f + 2.0f));
    }
    else
    {
        CHECK(CountEvents(fired, ZETownLost, f.town) == 1);
        CHECK(Fixture::Str(f.Z(f.town).owner) == "NEUTRAL");
        CHECK(f.Z(f.post).garrison == Approx(1.0f));
    }
}

TEST_CASE("CounterAttack - a repelled force writes its survivors back to its base", "[game][guerrilla][counterattack]")
{
    // sweep flip minutes until both outcomes are seen; check the write-back
    // on every repelled case
    bool sawRepel = false;
    bool sawRetake = false;
    for (int minute = 0; minute < 200 && !(sawRepel && sawRetake); minute++)
    {
        Fixture f;
        f.Load(Certain());
        f.RaiseTown(f.town);
        f.Z(f.town).flipDay = 1;
        f.Z(f.town).flipMinute = minute;
        REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
        AutoArray<ZoneEventRecord> fired;
        const CounterAttackView far = Fixture::View(FarX, FarZ);
        f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
        f.Z(f.town).lastGuer = 3;
        f.ca.Step(f.registry, far, 1000.0f, nullptr, fired);
        if (CountEvents(fired, ZECounterRepelled, f.town) == 1)
        {
            sawRepel = true;
            CHECK(f.Z(f.post).garrison == Approx(3.0f));
        }
        else
        {
            sawRetake = true;
            CHECK(CountEvents(fired, ZETownLost, f.town) == 1);
        }
    }
    CHECK(sawRepel);
    CHECK(sawRetake);
}

TEST_CASE("CounterAttack - a force on the road turns back when the town falls", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(f.Z(f.post).garrison == Approx(1.0f));
    REQUIRE(f.registry.RevertTown(f.town, fired)); // a patrol drained it meanwhile
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == Approx(6.0f)); // all five home again
}

TEST_CASE("CounterAttack - inside the bubble the live squad waits, it is never skipped",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    // the player stands in the town; no world (ops null): the live spawn
    // cannot happen, so the force waits on the road with its clock stopped
    const CounterAttackView near = Fixture::View(f.Z(f.town).pos.X(), f.Z(f.town).pos.Z());
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, near, 300.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    const float travel = f.ca.Get(0)->clock;
    CHECK(f.ca.Get(0)->travel == Approx(travel));
    f.ca.Step(f.registry, near, 1000.0f, nullptr, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(travel));
    CHECK(fired.Size() == 0);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    // a dead player freezes the clocks
    CounterAttackView dead = near;
    dead.playerValid = false;
    f.ca.Step(f.registry, dead, 1000.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->clock == Approx(travel));
}

TEST_CASE("CounterAttack - pending attacks survive save/load", "[game][guerrilla][counterattack][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "counter-attack.bin";
    const std::string config = Certain("counterAttackDelayMin = 200; counterAttackDelayMax = 200;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }

    Fixture f;
    f.Load(config);
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    f.ca.Step(f.registry, far, 200.0f, nullptr, fired); // on the road
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    const CounterAttack::Record saved = *f.ca.Get(0);
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.ca.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }

    CounterAttack loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    REQUIRE(loaded.Count() == 1);
    const CounterAttack::Record* r = loaded.Get(0);
    CHECK(Fixture::Str(r->target) == "Town");
    CHECK(Fixture::Str(r->source) == "Post");
    CHECK(r->phase == CATravel);
    CHECK(r->clock == Approx(saved.clock));
    CHECK(r->travel == Approx(saved.travel));
    CHECK(r->strength == 5);
    CHECK(r->size == 6);
    CHECK(r->seed == saved.seed);
    CHECK(r->salt == saved.salt);
    CHECK(loaded.TownsTaken() == 1);                   // k of the next flip survives the reload
    CHECK(loaded.Tuning().delayMin == Approx(200.0f)); // reread from the mission config
    CHECK(loaded.Tuning().firstTownChance == Approx(1.0f));

    // the loaded record resolves exactly like the original: arrival in the
    // empty town holds it
    loaded.Step(f.registry, far, saved.clock + 1.0f, nullptr, fired);
    CHECK(loaded.Get(0)->phase == CAHolding);

    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

namespace
{

float Dist2DTest(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

typedef std::function<int(Vector3Par)> ExposureFn;

ExposureFn AllHidden()
{
    return [](Vector3Par) { return (int)CAExposeHidden; };
}

ExposureFn AllSeen()
{
    return [](Vector3Par) { return (int)CAExposeSeen; };
}

// hidden at least d from c, seen inside
ExposureFn HiddenFrom(Vector3 c, float d)
{
    return [c, d](Vector3Par p) { return Dist2DTest(p, c) >= d ? (int)CAExposeHidden : (int)CAExposeSeen; };
}

// a perception predicate for the birth rule
struct FakeGate final : CounterAttackGate
{
    ExposureFn exposure;
    explicit FakeGate(ExposureFn e) : exposure(std::move(e)) {}
    int Exposure(Vector3Par pos) const override { return exposure(pos); }
};

// a world with no groups: it records where a squad was asked to be born and
// refuses to create it (no AIGroup exists in the unit-test process), so the
// record stays where the rules left it
struct FakeOps final : CounterAttackOps
{
    ExposureFn exposure = AllHidden();
    int spawns = 0;
    Vector3 lastFrom = VZero;
    Vector3 lastLeg = VZero;
    int Exposure(Vector3Par pos) const override { return exposure(pos); }
    AIGroup* SpawnSquad(int, Vector3Par from, Vector3Par firstLeg) override
    {
        spawns++;
        lastFrom = from;
        lastLeg = firstLeg;
        return nullptr;
    }
    AIGroup* DetachFromGarrison(int, int, Vector3Par, int& taken) override
    {
        taken = 0;
        return nullptr;
    }
    int Alive(AIGroup*, Vector3&) override { return 0; }
    void MoveOn(AIGroup*, Vector3Par) override {}
    void Assault(AIGroup*, Vector3Par) override {}
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        route.Add(from);
        route.Add(to);
    }
    void Remove(AIGroup*) override {}
    void Adopt(int, AIGroup*) override {}
    bool SquadUnseen(AIGroup*) const override { return false; }
};

// a straight 1000 m road along X from the base (0) to the town (1000),
// RouteStep apart
AutoArray<Vector3> StraightRoad()
{
    AutoArray<Vector3> raw;
    raw.Add(Vector3(0, 0, 0));
    raw.Add(Vector3(1000, 0, 0));
    AutoArray<Vector3> route;
    CounterAttack::ResampleRoute(raw, CounterAttack::RouteStep, route);
    return route;
}

// launch: from the base toward the town
CounterAttackSpawnPick Launch(const AutoArray<Vector3>& road, ExposureFn e)
{
    const FakeGate gate(std::move(e));
    return CounterAttack::PickSpawnPoint(road, 0.0f, true, 250.0f, gate);
}

// a force the player walked up to: back from its progress point
CounterAttackSpawnPick Rejoin(const AutoArray<Vector3>& road, float progress, ExposureFn e)
{
    const FakeGate gate(std::move(e));
    return CounterAttack::PickSpawnPoint(road, progress, false, 250.0f, gate);
}

ExposureFn HiddenX(float lo, float hi, int tier = CAExposeHidden)
{
    return [lo, hi, tier](Vector3Par p) { return p.X() >= lo && p.X() <= hi ? tier : (int)CAExposeSeen; };
}

} // namespace

TEST_CASE("CounterAttack - route geometry: length, points, resampling, progress", "[game][guerrilla][counterattack]")
{
    AutoArray<Vector3> raw;
    raw.Add(Vector3(0, 0, 0));
    raw.Add(Vector3(300, 0, 0));
    raw.Add(Vector3(300, 0, 400));
    CHECK(CounterAttack::RouteLength(raw) == Approx(700.0f));
    const Vector3 mid = CounterAttack::RoutePoint(raw, 0.5f);
    CHECK(mid.X() == Approx(300.0f));
    CHECK(mid.Z() == Approx(50.0f));
    CHECK(CounterAttack::RoutePoint(raw, 0.0f).X() == Approx(0.0f));
    CHECK(CounterAttack::RoutePoint(raw, 1.0f).Z() == Approx(400.0f));
    AutoArray<Vector3> route;
    CounterAttack::ResampleRoute(raw, 50.0f, route);
    CHECK(route.Size() == 15); // 6 legs of 50 m, then 8
    CHECK(CounterAttack::RouteLength(route) == Approx(700.0f));
    CHECK(route[6].X() == Approx(300.0f)); // the corner is kept
    CHECK(route[6].Z() == Approx(0.0f));
    CounterAttack::Record r;
    r.travel = 100.0f;
    r.clock = 25.0f;
    CHECK(CounterAttack::Progress(r) == Approx(0.75f));
    r.clock = 150.0f;
    CHECK(CounterAttack::Progress(r) == Approx(0.0f));
    r.travel = 0;
    CHECK(CounterAttack::Progress(r) == Approx(0.0f));
    // a road that winds far round is not walked (Abel: Le Port Harbour to
    // Houdan 3064 m of road for 2707 m straight is; the Outpost's 1816 m for
    // 535 m is not)
    CHECK(CounterAttack::RoadIsReasonable(3064.0f, 2707.0f));
    CHECK_FALSE(CounterAttack::RoadIsReasonable(1816.0f, 535.0f));
    // the march allowance covers the road still ahead
    const AutoArray<Vector3> road = StraightRoad();
    CHECK(CounterAttack::MarchAllowance(road, 0.5f) ==
          Approx(500.0f / CounterAttack::MarchSpeed + CounterAttack::MarchSlack));
}

TEST_CASE("CounterAttack - birth point: the first unseen point from the base, never near the town",
          "[game][guerrilla][counterattack]")
{
    const AutoArray<Vector3> road = StraightRoad();
    REQUIRE(road.Size() == 21);

    // launch: nobody sees the base, born at the base
    CounterAttackSpawnPick pick = Launch(road, AllHidden());
    CHECK(pick.reason == CASpawnBase);
    CHECK(pick.pos.X() == Approx(0.0f));
    CHECK(pick.fraction == Approx(0.0f));

    // the base is watched (everything short of 400 m is seen): the first
    // route point from the base that nobody sees
    pick = Launch(road, HiddenX(399.0f, 2000.0f));
    CHECK(pick.reason == CASpawnRoute);
    CHECK(pick.pos.X() == Approx(400.0f));
    CHECK(pick.fraction == Approx(0.4f));

    // the player sees the whole road: deferred
    CHECK(Launch(road, AllSeen()).reason == CASpawnDeferred);
    CHECK(Launch(AutoArray<Vector3>(), AllHidden()).reason == CASpawnDeferred);

    // only the last 250 m before the town are unseen: never born there
    CHECK(Launch(road, HiddenX(760.0f, 2000.0f)).reason == CASpawnDeferred);
    pick = Launch(road, HiddenX(749.0f, 2000.0f));
    CHECK(pick.reason == CASpawnRoute);
    CHECK(pick.pos.X() == Approx(750.0f));

    // cone-only points rank last and count only within 300 m of the base
    pick = Launch(road, HiddenX(-1.0f, 2000.0f, CAExposeConeOnly));
    CHECK(pick.reason == CASpawnBase);
    pick = Launch(road, HiddenX(199.0f, 2000.0f, CAExposeConeOnly));
    CHECK(pick.reason == CASpawnRoute);
    CHECK(pick.pos.X() == Approx(200.0f));
    CHECK(Launch(road, HiddenX(349.0f, 2000.0f, CAExposeConeOnly)).reason == CASpawnDeferred);
    pick = Launch(road, [](Vector3Par p)
                  { return p.X() >= 599.0f && p.X() <= 601.0f ? (int)CAExposeHidden : (int)CAExposeConeOnly; });
    CHECK(pick.pos.X() == Approx(600.0f)); // a hidden point beats a cone-only base

    // a force the player walked up to searches BACKWARD from its progress
    // point: a column 70% of the way is born there, not at its base
    pick = Rejoin(road, 0.7f, AllHidden());
    CHECK(pick.reason == CASpawnRoute);
    CHECK(pick.pos.X() == Approx(700.0f));
    // its progress point watched: the first unseen point behind it
    pick = Rejoin(road, 0.7f, HiddenX(-1.0f, 451.0f));
    CHECK(pick.pos.X() == Approx(450.0f));
    pick = Rejoin(road, 0.37f, HiddenX(-1.0f, 2000.0f));
    CHECK(pick.pos.X() == Approx(370.0f)); // the progress point itself, between two road points
    // never ahead of it
    CHECK(Rejoin(road, 0.5f, HiddenX(549.0f, 2000.0f)).reason == CASpawnDeferred);
    // a cone-only point behind it still counts only near the base
    CHECK(Rejoin(road, 0.7f, HiddenX(-1.0f, 2000.0f, CAExposeConeOnly)).pos.X() == Approx(300.0f));
    // its progress point already inside the bound: it holds at the town edge
    CHECK(Rejoin(road, 0.9f, AllHidden()).reason == CASpawnHold);
    CHECK(Rejoin(road, 0.8f, AllHidden()).reason == CASpawnHold);
}

TEST_CASE("CounterAttack - a travelling force is born on its road behind its progress point, or waits",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    FakeOps ops;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    // off-screen launch from the Post's stored reserve: abstract travel
    f.ca.Step(f.registry, far, 300.0f, &ops, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    REQUIRE(Fixture::Str(f.ca.Get(0)->source) == "Post");
    CHECK_FALSE(f.ca.Get(0)->wholeRoad);
    const float travel = f.ca.Get(0)->travel;
    REQUIRE(travel > 100.0f);
    // 40% of the road covered with nobody watching
    f.ca.Step(f.registry, far, travel * 0.4f, &ops, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    CHECK(CounterAttack::Progress(*f.ca.Get(0)) == Approx(0.4f).margin(0.001f));
    CHECK(ops.spawns == 0);

    // the player walks into the town: the force is born at its progress
    // point (268 m from the Post) when he cannot see it there
    const Vector3 post = f.Z(f.post).pos;
    const Vector3 town = f.Z(f.town).pos;
    const CounterAttackView near = Fixture::View(town.X(), town.Z());
    const float clock = f.ca.Get(0)->clock;
    ops.exposure = AllHidden();
    f.ca.Step(f.registry, near, 1.0f, &ops, fired);
    REQUIRE(ops.spawns == 1);
    CHECK(Dist2DTest(ops.lastFrom, post) == Approx(0.4f * 670.82f).margin(1.0f));
    // the first leg runs down the same road, toward the town
    CHECK(Dist2DTest(ops.lastLeg, town) < Dist2DTest(ops.lastFrom, town));
    // the spawn failed (no world): it waits, the arrival clock stopped
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(clock));

    // its progress point is watched, a point behind it is not: born there,
    // the first such point back toward the base
    ops.exposure = [post](Vector3Par p)
    { return Dist2DTest(p, post) <= 150.0f ? (int)CAExposeHidden : (int)CAExposeSeen; };
    f.ca.Step(f.registry, near, CounterAttack::RetryInterval + 1.0f, &ops, fired);
    f.ca.Step(f.registry, near, 1.0f, &ops, fired);
    REQUIRE(ops.spawns == 2);
    CHECK(Dist2DTest(ops.lastFrom, post) <= 150.0f);
    CHECK(Dist2DTest(ops.lastFrom, post) > 100.0f);
    CHECK(f.ca.Get(0)->clock == Approx(clock));

    // nothing between the base and the progress point is unseen: deferred,
    // retried every RetryInterval, the clock stopped, nothing done to the town
    ops.exposure = HiddenFrom(post, 300.0f);
    f.ca.Step(f.registry, near, CounterAttack::RetryInterval + 1.0f, &ops, fired); // the retry runs out
    f.ca.Step(f.registry, near, 1.0f, &ops, fired);                                // tried: nothing unseen
    CHECK(ops.spawns == 2);
    for (int i = 0; i < 10; i++)
    {
        f.ca.Step(f.registry, near, 25.0f, &ops, fired);
    }
    CHECK(ops.spawns == 2);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(clock));
    CHECK(fired.Size() == 0);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));

    // ... and after counterAttackBirthGiveUp it turns back: the reserve is
    // refunded, nothing reached the town.  The player in the town is 671 m
    // from the Post, inside its deposit bubble: the five come home as a
    // PENDING deposit (S5), not bodies appearing in his world
    for (int i = 0; i < 30 && f.ca.Count() > 0; i++)
    {
        f.ca.Step(f.registry, near, 25.0f, &ops, fired);
    }
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
    CHECK(f.Z(f.post).pending == 5);
    CHECK(f.Z(f.post).pendingOrigin == f.post);
    CHECK(fired.Size() == 0);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
}

TEST_CASE("CounterAttack - a force that has nearly arrived stays abstract at the town edge in the bubble",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    FakeOps ops;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, &ops, fired);
    const float travel = f.ca.Get(0)->travel;
    f.ca.Step(f.registry, far, travel * 0.7f, &ops, fired); // 201 m from the town
    const float clock = f.ca.Get(0)->clock;
    const Vector3 town = f.Z(f.town).pos;
    const CounterAttackView near = Fixture::View(town.X(), town.Z());
    for (int i = 0; i < 40; i++)
    {
        f.ca.Step(f.registry, near, 25.0f, &ops, fired); // 1000 s, past the give-up
    }
    CHECK(ops.spawns == 0);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(clock));
    CHECK(fired.Size() == 0);
    // the player leaves: it arrives off-screen
    f.ca.Step(f.registry, far, clock + 1.0f, &ops, fired);
    CHECK(f.ca.Get(0)->phase == CAHolding);
}

TEST_CASE("CounterAttack - a town inside the bubble at launch: born at the base when unseen, never at a watched base",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    FakeOps ops;
    const Vector3 town = f.Z(f.town).pos;
    const Vector3 post = f.Z(f.post).pos;
    // the player in the town; the gate passes anything 300 m from him (the
    // Post is 671 m away)
    ops.exposure = HiddenFrom(town, 300.0f);
    const CounterAttackView near = Fixture::View(town.X(), town.Z());
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, near, 300.0f, &ops, fired); // launch: stored reserve, abstract
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->wholeRoad);
    f.ca.Step(f.registry, near, 1.0f, &ops, fired); // birth
    REQUIRE(ops.spawns == 1);
    CHECK(ops.lastFrom.X() == Approx(post.X()).margin(0.5f));
    CHECK(ops.lastFrom.Z() == Approx(post.Z()).margin(0.5f));

    // the player standing at the Post: never there, farther down the road
    // (a force launched into the bubble may be born anywhere on its road)
    Fixture g;
    g.Load(Certain());
    g.RaiseTown(g.town);
    REQUIRE(g.ca.OnTownFlipped(g.registry, g.town, Fixture::GroupSize));
    FakeOps watched;
    const Vector3 gpost = g.Z(g.post).pos;
    watched.exposure = HiddenFrom(gpost, 300.0f);
    const CounterAttackView atBase = Fixture::View(gpost.X(), gpost.Z());
    g.ca.Step(g.registry, atBase, 300.0f, &watched, fired);
    REQUIRE(g.ca.Get(0)->phase == CATravel);
    g.ca.Step(g.registry, atBase, 1.0f, &watched, fired);
    REQUIRE(watched.spawns == 1);
    CHECK(Dist2DTest(watched.lastFrom, gpost) >= 300.0f);
    CHECK(Dist2DTest(watched.lastFrom, gpost) < 350.0f);
}

TEST_CASE("CounterAttack - a holding force does nothing to a town in the player's bubble",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    FakeOps ops;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, &ops, fired);
    f.ca.Step(f.registry, far, 5000.0f, &ops, fired);
    REQUIRE(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.AbstractPresence(f.registry, f.town) == 5);
    // the player walks in: never born at the town centre, no drain, no roll
    const Vector3 town = f.Z(f.town).pos;
    const CounterAttackView near = Fixture::View(town.X() + 100.0f, town.Z());
    f.Z(f.town).lastGuer = 3;
    fired.Clear();
    for (int i = 0; i < 10; i++)
    {
        f.ca.Step(f.registry, near, 30.0f, &ops, fired);
    }
    CHECK(ops.spawns == 0);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.AbstractPresence(f.registry, f.town) == 0);
    CHECK(fired.Size() == 0);
    // he leaves: the force works the town again (here: the odds roll)
    f.ca.Step(f.registry, far, 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(fired.Size() == 1);
}

TEST_CASE("CounterAttack - a live squad counts as occupier presence in whatever town it stands in (D5)",
          "[game][guerrilla][counterattack]")
{
    // the occupier census (ZoneRegistry::GatherInputs) has no ownership or
    // target filter (OCCUPIER-FORCES.md D5, D24, reverting 450327e's
    // own-target rule): a column bound for Town that marches through Other,
    // a town the resistance also holds, drains Other like any patrol
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    REQUIRE(Fixture::Str(f.ca.Get(0)->target) == "Town");
    f.Z(f.other).support = 60.0f;
    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs in = f.Inputs(FarX, FarZ, -1, 0);
    in.occCount[f.other] = 5; // the passing column, counted where it stands
    f.registry.EvaluateTick(in, fired);
    CHECK(f.Z(f.other).support == Approx(55.0f));
    for (int tick = 0; tick < 6; tick++)
    {
        f.registry.EvaluateTick(in, fired);
    }
    CHECK(Fixture::Str(f.Z(f.other).owner) == "NEUTRAL");
    CHECK(CountEvents(fired, ZETownLost, f.other) == 1);
    // its own target is untouched (it is not there yet)
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
}

TEST_CASE("CounterAttack - U2: an adopted squad comes off the revert refill, never over the reserve",
          "[game][guerrilla][counterattack]")
{
    // the player in the town at the revert: the refill (4) is pending
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs in = f.Inputs(f.Z(f.town).pos.X(), f.Z(f.town).pos.Z(), -1, 0);
    in.occCount[f.town] = 5; // the live squad standing in the street
    for (int tick = 0; tick < 20 && Fixture::Str(f.Z(f.town).owner) == "GUER"; tick++)
    {
        f.registry.EvaluateTick(in, fired);
    }
    REQUIRE(Fixture::Str(f.Z(f.town).owner) == "NEUTRAL");
    REQUIRE(f.Z(f.town).pending == 4);
    REQUIRE(f.Z(f.town).revertRefill == 4);
    // a squad of 5 adopted: the whole refill comes off, nothing below 0
    CHECK(CounterAttack::SubtractAdopted(f.Z(f.town), 5) == 4);
    CHECK(f.Z(f.town).pending == 0);
    CHECK(f.Z(f.town).garrison == Approx(0.0f));
    CHECK(f.Z(f.town).revertRefill == 0);

    // off-screen revert (direct refill) and a squad of 3: reserve 4 - 3 = 1
    Fixture g;
    g.Load(Certain());
    g.RaiseTown(g.town);
    ZoneRecord& z = g.Z(g.town);
    ZoneTickInputs farTick = g.Inputs(FarX, FarZ, -1, 0);
    farTick.occCount[g.town] = 3;
    for (int tick = 0; tick < 20 && Fixture::Str(g.Z(g.town).owner) == "GUER"; tick++)
    {
        g.registry.EvaluateTick(farTick, fired);
    }
    REQUIRE(Fixture::Str(z.owner) == "NEUTRAL");
    REQUIRE(z.garrison == Approx(4.0f));
    REQUIRE(z.revertRefill == 4);
    CHECK(CounterAttack::SubtractAdopted(z, 3) == 3);
    CHECK(z.garrison == Approx(1.0f)); // reserve + adopted = garrisonMax

    // a reserve the town already held beyond the refill is kept: 2 stored
    // before the revert, a refill of 2, a squad of 3 takes only the refill
    Fixture h;
    h.Load(Certain());
    h.RaiseTown(h.town);
    h.Z(h.town).garrison = 2; // e.g. survivors written back onto the held record
    for (int tick = 0; tick < 20 && Fixture::Str(h.Z(h.town).owner) == "GUER"; tick++)
    {
        h.registry.EvaluateTick(farTick, fired);
    }
    REQUIRE(Fixture::Str(h.Z(h.town).owner) == "NEUTRAL");
    REQUIRE(h.Z(h.town).garrison == Approx(4.0f));
    REQUIRE(h.Z(h.town).revertRefill == 2);
    CHECK(CounterAttack::SubtractAdopted(h.Z(h.town), 3) == 2);
    CHECK(h.Z(h.town).garrison == Approx(2.0f));
    // no refill to cut (no revert this tick): adoption takes nothing
    CHECK(CounterAttack::SubtractAdopted(h.Z(h.town), 3) == 0);
    CHECK(h.Z(h.town).garrison == Approx(2.0f));
}

TEST_CASE("CounterAttack - U3 and S5: survivors go home as whole bodies, pending inside the bubble",
          "[game][guerrilla][counterattack]")
{
    // a fractional room (garrisonMax 6.5 set by hand) never stores a fraction
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(f.Z(f.post).garrison == Approx(1.0f));
    f.Z(f.post).garrisonMax = 6.5f;
    REQUIRE(f.registry.RevertTown(f.town, fired)); // the town fell under the force
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == 6.0f); // 1 + floor(6.5 - 1) = 6, exactly
    CHECK(f.Z(f.post).pending == 0);     // the player is far: a direct write

    // the same turn-back with the player standing at the Post: pending
    Fixture g;
    g.Load(Certain());
    g.RaiseTown(g.town);
    REQUIRE(g.ca.OnTownFlipped(g.registry, g.town, Fixture::GroupSize));
    g.ca.Step(g.registry, far, 300.0f, nullptr, fired);
    REQUIRE(g.registry.RevertTown(g.town, fired));
    const CounterAttackView atPost = Fixture::View(g.Z(g.post).pos.X(), g.Z(g.post).pos.Z());
    g.ca.Step(g.registry, atPost, 1.0f, nullptr, fired);
    CHECK(g.ca.Count() == 0);
    CHECK(g.Z(g.post).garrison == Approx(1.0f));
    CHECK(g.Z(g.post).pending == 5);
    // reserve + pending is the base's whole strength again
    CHECK(g.Z(g.post).garrison + (float)g.Z(g.post).pending == Approx(6.0f));
}

TEST_CASE("CounterAttack - the HQ's town is never a target (N4a)", "[game][guerrilla][counterattack]")
{
    // OnTownFlipped: the flip counts as a town taken, nothing is scheduled
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize, 1.0f, f.town));
    CHECK(f.ca.Count() == 0);
    CHECK(f.ca.TownsTaken() == 1);
    // the next town is answered as a second town (k = 1)
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize, 1.0f, f.town));
    CHECK(f.ca.Get(0)->townsBefore == 1);

    // LaunchFrom refuses the HQ's town
    Fixture g;
    g.Load(ConfigWith("counterAttackFirstTownChance = 0;"));
    g.RaiseTown(g.town);
    CounterAttackView far = Fixture::View(FarX, FarZ);
    far.hqZone = g.town;
    AutoArray<ZoneEventRecord> fired;
    CHECK_FALSE(g.ca.LaunchFrom(g.registry, far, g.town, g.post, 3, nullptr, fired));
    far.hqZone = -1;
    CHECK(g.ca.LaunchFrom(g.registry, far, g.town, g.post, 3, nullptr, fired));

    // a scheduled force whose town became the HQ's meanwhile is cancelled
    Fixture h;
    h.Load(Certain());
    h.RaiseTown(h.town);
    REQUIRE(h.ca.OnTownFlipped(h.registry, h.town, Fixture::GroupSize));
    CounterAttackView hq = Fixture::View(FarX, FarZ);
    hq.hqZone = h.town;
    h.ca.Step(h.registry, hq, 300.0f, nullptr, fired);
    CHECK(h.ca.Count() == 0);
    CHECK(h.Z(h.post).garrison == Approx(6.0f)); // nothing left the Post
}

TEST_CASE("CounterAttack - a squad that leaves the player's world returns to the abstract road seamlessly",
          "[game][guerrilla][counterattack]")
{
    // where along the road the column is
    AutoArray<Vector3> road = StraightRoad();
    CHECK(CounterAttack::RouteFractionNear(road, Vector3(300, 0, 20)) == Approx(0.3f));
    CHECK(CounterAttack::RouteFractionNear(road, Vector3(-50, 0, 0)) == Approx(0.0f));
    CHECK(CounterAttack::RouteFractionNear(road, Vector3(1200, 0, 0)) == Approx(1.0f));

    // a force born from the abstract road at 40%, marched to 70%: back on
    // the abstract road at 70%, its four survivors the whole strength, the
    // arrival clock where an unbroken abstract march would have it
    CounterAttack::Record r;
    r.phase = CALive;
    r.strength = 6;
    r.travel = 200.0f;
    r.clock = -300.0f; // the live march allowance
    r.leg = CounterAttack::LegRoad;
    r.wholeRoad = true;
    CounterAttack::ReturnToRoad(r, 0.7f, 4, 999.0f);
    CHECK(r.phase == CATravel);
    CHECK(r.strength == 4);
    CHECK(r.travel == Approx(200.0f));
    CHECK(r.clock == Approx(60.0f));
    CHECK(CounterAttack::Progress(r) == Approx(0.7f));
    CHECK_FALSE(r.wholeRoad); // re-born later from its progress point, backward
    CHECK(r.leg == CounterAttack::LegUnknown);
    // a garrison that marched out never travelled: its leg is the base distance
    CounterAttack::Record g;
    g.phase = CALive;
    g.strength = 5;
    CounterAttack::ReturnToRoad(g, 0.25f, 5, 400.0f);
    CHECK(g.travel == Approx(400.0f));
    CHECK(g.clock == Approx(300.0f));
    CHECK(CounterAttack::Progress(g) == Approx(0.25f));

    // re-born by the birth rule from where it got to, not from its base
    const FakeGate gate(AllHidden());
    const CounterAttackSpawnPick pick =
        CounterAttack::PickSpawnPoint(road, CounterAttack::Progress(r), r.wholeRoad, 250.0f, gate);
    CHECK(pick.pos.X() == Approx(700.0f));
}
