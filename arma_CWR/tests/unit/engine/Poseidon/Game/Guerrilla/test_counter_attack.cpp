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
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp> // Entity / EntityType (the column fakes' hulls)
#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // the win latch freeze (7.4)
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the pool seams (5.5)
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
#include <vector>

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

// occupierForces defaults on since phase 7: a case that does not author the
// key is a legacy (key off) case and gets occupierForces = 0 pinned
std::string ConfigWith(const char* keys)
{
    std::string cfg = kCounterConfig;
    const std::string anchor = "class CfgGuerrillaZones\n{\n";
    std::string body = std::string("    ") + keys + "\n";
    if (std::string(keys).find("occupierForces") == std::string::npos)
    {
        body += "    occupierForces = 0;\n";
    }
    cfg.insert(anchor.size(), body);
    return cfg;
}

// the legacy fixture: the zones with the occupier forces layer pinned off
std::string Legacy()
{
    return ConfigWith("");
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
        Z(zone).lastGuerHold = 0;
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
    f.Load(Legacy());
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
    f.Load(Legacy());
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

// hidden within d of c, seen beyond (only the ground round one base is unseen)
ExposureFn HiddenNear(Vector3 c, float d)
{
    return [c, d](Vector3Par p) { return Dist2DTest(p, c) <= d ? (int)CAExposeHidden : (int)CAExposeSeen; };
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

// ===========================================================================
// Occupier forces, phase 3 part B (OCCUPIER-FORCES.md 3, 5.4, 5.5, 6.5, 7.4,
// 14): record fields, LaunchFunded, the listener, the birth chain, real
// speeds, the key-on resolution, pool routing and the freeze.
// ===========================================================================

namespace
{

// the director's ear: every flip and every event, in order
struct FakeListener final : CounterAttackListener
{
    bool take = false;
    int flips = 0;
    int lastK = -1;
    int zoneEvents = 0;
    std::vector<CounterAttackOpEvent> events;
    std::vector<std::string> notes;
    bool OnTownFlipped(int, int k) override
    {
        flips++;
        lastK = k;
        return take;
    }
    void OnCounterAttackOp(const CounterAttackOpEvent& ev) override
    {
        events.push_back(ev);
        notes.push_back(ev.note ? ev.note : "");
    }
    void OnZoneEvents(const AutoArray<ZoneEventRecord>&) override { zoneEvents++; }
    int Count(int type) const
    {
        int n = 0;
        for (const CounterAttackOpEvent& ev : events)
        {
            n += ev.type == type ? 1 : 0;
        }
        return n;
    }
    const CounterAttackOpEvent* Last(int type) const
    {
        for (int i = (int)events.size() - 1; i >= 0; i--)
        {
            if (events[i].type == type)
            {
                return &events[i];
            }
        }
        return nullptr;
    }
};

// installs a listener for one test case (the listener is process-wide)
struct ListenerGuard
{
    explicit ListenerGuard(CounterAttackListener* l) { CounterAttack::SetListener(l); }
    ~ListenerGuard() { CounterAttack::SetListener(nullptr); }
};

// the occupier pool singleton switched on over a fixture's registry for one
// test case (the routing calls go through OccupierForces::Instance())
struct PoolGuard
{
    explicit PoolGuard(Fixture& f)
    {
        OccupierForces& of = OccupierForces::Instance();
        of.Clear();
        const std::string cfg = "class CfgGuerrillaZones { occupierForces = 1; };\n";
        QIStream in(cfg.c_str(), cfg.size());
        file.Parse(in);
        of.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        REQUIRE(of.Activate(f.registry));
        REQUIRE(of.Active());
    }
    ~PoolGuard() { OccupierForces::Instance().Clear(); }
    ParamFile file;
};

CounterAttackOrder FootOrder(int target, int source, int bodies, int opId = 7)
{
    CounterAttackOrder o;
    o.opId = opId;
    o.kind = CAKRetake;
    o.origin = CAODirector;
    o.package = FPFoot;
    o.target = target;
    o.source = source;
    o.bodies = bodies;
    o.funded = true;
    o.abstractSpeed = CounterAttack::FootSpeed;
    return o;
}

CounterAttackOrder TruckOrder(int target, int source, int bodies, int opId = 8)
{
    CounterAttackOrder o = FootOrder(target, source, bodies, opId);
    o.package = FPTruck;
    o.hullClass = "Truck5t";
    o.crew = 1;
    o.priceUnits = 4; // truck 3 + its driver
    o.urgent = true;
    o.abstractSpeed = CounterAttack::RealSpeed(FPTruck, 60.0f);
    return o;
}

// a world that would mount a force: it records the hull it was asked for
// and refuses it (no entities in the unit-test process); the hull state of a
// hull driving home is whatever the case sets
struct HullOps final : CounterAttackOps
{
    int state = CounterAttackOps::HullIntact;
    int crewAlive = 1;
    bool unseen = true;
    int removed = 0;
    int mounts = 0;
    int squads = 0;
    int drives = 0;
    std::string lastHull;
    int lastCrew = 0;
    int Exposure(Vector3Par) const override { return CAExposeHidden; }
    AIGroup* SpawnSquad(int, Vector3Par, Vector3Par) override
    {
        squads++;
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
    AIGroup* SpawnMounted(const RString& hullClass, int crew, int, Vector3Par, Vector3Par, Entity*& hull,
                          AIGroup*& crewGroup) override
    {
        mounts++;
        lastHull = (const char*)hullClass;
        lastCrew = crew;
        hull = nullptr;
        crewGroup = nullptr;
        return nullptr;
    }
    void DriveOn(AIGroup*, Vector3Par) override { drives++; }
    int HullState(Entity*, AIGroup*, int& crew) const override
    {
        crew = crewAlive;
        return state;
    }
    bool HullUnseen(Entity*, AIGroup*) const override { return unseen; }
    void RemoveHull(Entity*, AIGroup*) override { removed++; }
};

// every body the occupier has: pool, reserves and pending deposits (the
// 5.5 ledger with nothing live)
int Ledger(Fixture& f, const CounterAttack& ca)
{
    int sum = OccupierForces::Instance().Pool();
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        sum += (int)floorf(f.Z(i).garrison) + f.Z(i).pending;
    }
    for (int i = 0; i < ca.Count(); i++)
    {
        sum += ca.Get(i)->strength;
    }
    return sum;
}

// a revert by a passer-by standing in the town (live occupiers, the player far)
void PasserByRevert(Fixture& f, int zone, AutoArray<ZoneEventRecord>& fired, int settle = 0)
{
    ZoneTickInputs in = f.Inputs(FarX, FarZ, -1, 0);
    in.occCount[zone] = 3;
    if (settle > 0)
    {
        in.settleOcc.Resize(f.registry.NZones());
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            in.settleOcc[i] = i == zone ? settle : 0;
        }
    }
    for (int tick = 0; tick < 30 && Fixture::Str(f.Z(zone).owner) == "GUER"; tick++)
    {
        f.registry.EvaluateTick(in, fired);
    }
    REQUIRE(Fixture::Str(f.Z(zone).owner) == "NEUTRAL");
}

// three bases for the chain: the Post near the town, a Mid base and the far
// base beyond it
const char* kChainConfig =
    "class CfgGuerrillaZones\n"
    "{\n"
    "    occupierForces = 1;\n"
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
    "        class Mid     { name=\"Mid\";     type=\"OUTPOST\"; owner=\"OCCUPIER\"; garrison=8; "
    "position[]={3000.0, 3000.0, 0.0}; };\n"
    "        class FarBase { name=\"FarBase\"; type=\"AIRFIELD\"; owner=\"OCCUPIER\"; garrison=12; "
    "position[]={9000.0, 9000.0, 0.0}; };\n"
    "    };\n"
    "};\n";

// a legacy record from before phase 3 (the fields a 4400fe9 / 450327e save
// carries), to prove an old save loads as a legacy foot retake
struct OldRecord
{
    RString target;
    RString source;
    int phase = CATravel;
    float clock = 0;
    float travel = 0;
    int strength = 0;
    int size = 0;
    LSError Serialize(ParamArchive& ar)
    {
        PARAM_CHECK(ar.Serialize("target", target, 1, RString()))
        PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
        PARAM_CHECK(ar.Serialize("phase", phase, 1, (int)CAScheduled))
        PARAM_CHECK(ar.Serialize("clock", clock, 1, 0.0f))
        PARAM_CHECK(ar.Serialize("travel", travel, 1, 0.0f))
        PARAM_CHECK(ar.Serialize("strength", strength, 1, 0))
        PARAM_CHECK(ar.Serialize("size", size, 1, 0))
        return LSOK;
    }
};

} // namespace

TEST_CASE("CounterAttack - occupier forces keys and the real speeds (3.2)", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
    CHECK(f.ca.Tuning().forcesOn); // phase 7: the key defaults on
    Fixture off;
    off.Load(Legacy());
    CHECK_FALSE(off.ca.Tuning().forcesOn);
    CHECK(f.ca.Tuning().forcesOpsMax == 2);
    CHECK(f.ca.Tuning().forcesLiveHullsMax == 6);
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1; forcesOpsMax = 9; forcesLiveHullsMax = 3;"));
    CHECK(g.ca.Tuning().forcesOn);
    // forcesOpsMax has one reader, OccupierForces (OpsCap reads its tuning
    // while it is active): this service keeps its own default only
    CHECK(g.ca.Tuning().forcesOpsMax == 2);
    CHECK(g.ca.OpsCap() == 2); // OccupierForces inactive here: the local default
    CHECK(g.ca.Tuning().forcesLiveHullsMax == 3);

    // foot 1.5 m/s; a ground hull 0.6 x and an aircraft 0.8 x maxSpeed (km/h)
    CHECK(CounterAttack::RealSpeed(FPFoot, 0.0f) == Approx(1.5f));
    CHECK(CounterAttack::RealSpeed(FPTruck, 60.0f) == Approx(10.0f));
    CHECK(CounterAttack::RealSpeed(FPTransportHeli, 180.0f) == Approx(40.0f));
    CHECK(CounterAttack::RealSpeed(FPTruck, 0.0f) == Approx(1.5f)); // unknown maxSpeed: on foot

    CounterAttackTuning t;
    CounterAttack::Record legacy;
    // key off: the legacy pacing (counterAttackSpeed 4 over roadFactor 1.3)
    CHECK(CounterAttack::TravelTimeFor(t, legacy, 1000.0f, 0.0f) == Approx(CounterAttack::TravelTime(t, 1000.0f)));
    CHECK(CounterAttack::TravelTimeFor(t, legacy, 1000.0f, 0.0f) == Approx(325.0f));
    // key on: a legacy record walks at the real foot speed; with only the
    // straight distance known the road factor stands in, a real road route
    // is the road
    t.forcesOn = true;
    CHECK(CounterAttack::TravelTimeFor(t, legacy, 1000.0f, 0.0f) == Approx(1300.0f / 1.5f));
    CHECK(CounterAttack::TravelTimeFor(t, legacy, 1000.0f, 1200.0f) == Approx(800.0f));
    // a truck at 10 m/s
    CounterAttack::Record truck;
    truck.abstractSpeed = 10.0f;
    CHECK(CounterAttack::TravelTimeFor(t, truck, 1000.0f, 1200.0f) == Approx(120.0f));
    CHECK(CounterAttack::TravelTimeFor(t, truck, 10.0f, 20.0f) == Approx(CounterAttack::MinTravel));
    // the gmCounterAttackTune "speed" test lever wins over the real speeds
    t.speedLever = true;
    t.speed = 40.0f;
    CHECK(CounterAttack::TravelTimeFor(t, truck, 1000.0f, 1200.0f) == Approx(32.5f));
    // A_lethal and the odds: one body of lethality 1 is the old odds
    CounterAttackTuning d;
    CHECK(CounterAttack::HoldOddsLethal(d, 3, 5.0f) == Approx(CounterAttack::HoldOdds(d, 3, 5)));
    CHECK(CounterAttack::HoldOddsLethal(d, 3, 9.0f) == Approx(0.5f));
    CHECK(CounterAttack::HoldOddsLethal(d, 0, 9.0f) == Approx(0.0f));
}

TEST_CASE("CounterAttack - LaunchFunded: refusals, the spare cap and the abstract real-speed leg",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;

    // refused: a later-phase kind, an administered town, the HQ's town, a
    // resistance source, no bodies, the won latch
    CounterAttackOrder o = FootOrder(f.town, f.post, 12);
    o.kind = CAKBase;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, o, nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.other, f.post, 4), nullptr, fired));
    CounterAttackView hq = far;
    hq.hqZone = f.town;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, hq, FootOrder(f.town, f.post, 4), nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.camp, 4), nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 0), nullptr, fired));
    CounterAttackView won = far;
    won.won = true;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, won, FootOrder(f.town, f.post, 4), nullptr, fired));
    // no free born-operation slot (forcesOpsMax 0)
    f.ca.TuningMutable().forcesOpsMax = 0;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4), nullptr, fired));
    f.ca.TuningMutable().forcesOpsMax = 2;
    CHECK(f.ca.Count() == 0);
    CHECK(ear.events.empty());
    CHECK(f.Z(f.post).garrison == Approx(6.0f));

    // twelve ordered, the Post spares five (it keeps one home): refused
    // whole (M1: a funded order the source cannot grant whole never goes
    // out undersized; the director quotes the same stored spare)
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 12), nullptr, fired));
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == Approx(6.0f));
    CHECK(CounterAttack::FundedSpare(f.ca.Tuning(), f.Z(f.post).garrison, 0, f.Z(f.post).garrisonMax) == 5);
    // five ordered: five leave, and the Launched event says five
    REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 5), nullptr, fired));
    REQUIRE(f.ca.Count() == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(r->strength == 5);
    CHECK(r->opId == 7);
    CHECK(r->kind == CAKRetake);
    CHECK(r->origin == CAODirector);
    CHECK(r->funded);
    CHECK(r->phase == CATravel);
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
    REQUIRE(ear.Count(CAOpLaunched) == 1);
    CHECK(ear.Last(CAOpLaunched)->bodies == 5);
    CHECK(ear.Last(CAOpLaunched)->opId == 7);
    CHECK(ear.Last(CAOpLaunched)->target == f.town);
    CHECK(ear.Last(CAOpLaunched)->source == f.post);
    // the real foot speed over the straight distance x roadFactor (no road
    // net in the test): 670.8 m x 1.3 / 1.5 m/s
    CHECK(r->travel == Approx(670.82f * 1.3f / 1.5f).margin(0.5f));
    CHECK(f.ca.FindByOpId(7) == 0);
    CHECK(f.ca.FindByOpId(99) == -1);
    // an unborn operation does not count against forcesOpsMax
    CHECK(f.ca.BornOps() == 0);
    // one record per town
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.farBase, 3, 9), nullptr, fired));

    // a motorized order always leaves from the stored reserve, at the hull's
    // speed, even with the town in the bubble and the source's garrison live
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    g.RaiseTown(g.town);
    CounterAttackView near = Fixture::View(g.Z(g.town).pos.X(), g.Z(g.town).pos.Z());
    near.garrisonLive.Resize(g.registry.NZones());
    for (int i = 0; i < g.registry.NZones(); i++)
    {
        near.garrisonLive[i] = 0;
    }
    near.garrisonLive[g.post] = 3;
    HullOps hops;
    REQUIRE(g.ca.LaunchFunded(g.registry, near, TruckOrder(g.town, g.post, 5), &hops, fired));
    const CounterAttack::Record* t = g.ca.Get(0);
    CHECK(t->phase == CATravel);
    CHECK(t->strength == 5);
    CHECK(t->wholeRoad);
    CHECK(Fixture::Str(t->hullClass) == "Truck5t");
    CHECK(t->crew == 1);
    CHECK(t->priceUnits == 4);
    CHECK(t->HullPrice() == 3);
    // the test world's road is the straight line, taken as the road: no
    // road factor on top of it
    CHECK(t->travel == Approx(670.82f / 10.0f).margin(0.5f));
}

TEST_CASE("CounterAttack - the AWAKE hook and the events of a legacy record", "[game][guerrilla][counterattack]")
{
    // the director takes the flip: counted, nothing scheduled
    {
        Fixture f;
        f.Load(Certain());
        f.RaiseTown(f.town);
        FakeListener ear;
        ear.take = true;
        ListenerGuard guard(&ear);
        CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
        CHECK(ear.flips == 1);
        CHECK(ear.lastK == 0);
        CHECK(f.ca.TownsTaken() == 1);
        CHECK(f.ca.Count() == 0);
        // the tick's zone events are forwarded after the service's own handling
        AutoArray<ZoneEventRecord> none;
        f.ca.OnZoneEvents(none);
        CHECK(ear.zoneEvents == 1);
    }
    // DORMANT: the listener declines, the legacy answer reports every state
    // change with origin CAOLegacy and opId 0
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(ear.flips == 1);
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(ear.Count(CAOpLaunched) == 1);
    CHECK(ear.Last(CAOpLaunched)->origin == CAOLegacy);
    CHECK(ear.Last(CAOpLaunched)->opId == 0);
    CHECK(ear.Last(CAOpLaunched)->bodies == 5);
    f.ca.Step(f.registry, far, 5000.0f, nullptr, fired);
    CHECK(ear.Count(CAOpArrived) == 1);
    REQUIRE(f.ca.Get(0)->phase == CAHolding);
    PasserByRevert(f, f.town, fired);
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpSettled) == 1);
    // the record ended with exactly one terminal event
    int terminal = 0;
    for (const CounterAttackOpEvent& ev : ear.events)
    {
        terminal += ev.type == CAOpRepelled || ev.type == CAOpSettled || ev.type == CAOpCancelledUnseen ||
                            ev.type == CAOpAborted
                        ? 1
                        : 0;
    }
    CHECK(terminal == 1);
}

TEST_CASE("CounterAttack - birth chain: launch, backward, the next base back, cancel unseen (D6)",
          "[game][guerrilla][counterattack]")
{
    // at launch: the first unseen point from the base (the base itself here)
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        f.RaiseTown(f.town);
        FakeOps ops;
        const Vector3 town = f.Z(f.town).pos;
        const CounterAttackView near = Fixture::View(town.X(), town.Z());
        AutoArray<ZoneEventRecord> fired;
        REQUIRE(f.ca.LaunchFunded(f.registry, near, FootOrder(f.town, f.post, 5), &ops, fired));
        REQUIRE(f.ca.Get(0)->wholeRoad);
        f.ca.Step(f.registry, near, 1.0f, &ops, fired);
        REQUIRE(ops.spawns == 1);
        CHECK(Dist2DTest(ops.lastFrom, f.Z(f.post).pos) < 1.0f);
        CHECK(f.ca.BornOps() == 0); // the spawn failed: still unborn
    }
    // on the road: backward from the progress point
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        f.RaiseTown(f.town);
        FakeOps ops;
        const CounterAttackView far = Fixture::View(FarX, FarZ);
        AutoArray<ZoneEventRecord> fired;
        REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 5), &ops, fired));
        const float travel = f.ca.Get(0)->travel;
        f.ca.Step(f.registry, far, travel * 0.4f, &ops, fired);
        const Vector3 town = f.Z(f.town).pos;
        f.ca.Step(f.registry, Fixture::View(town.X(), town.Z()), 1.0f, &ops, fired);
        REQUIRE(ops.spawns == 1);
        CHECK(Dist2DTest(ops.lastFrom, f.Z(f.post).pos) == Approx(0.4f * 670.82f).margin(1.0f));
    }
    // the Post's road watched, FarBase's not: the operation moves to the
    // next base back AT ONCE, with no wait (the old one refunded, the new one
    // debited, a base that cannot spare the whole force passed over); once
    // FarBase's road is watched too and no base passes, it waits
    // counterAttackBirthGiveUp and is cancelled unseen; it never counted as
    // born
    Fixture f;
    f.Load(kChainConfig);
    f.RaiseTown(f.town);
    const int mid = f.registry.FindZoneIndex("Mid");
    REQUIRE(mid >= 0);
    f.Z(mid).garrison = 3; // keeps 2 home: spares 1, not the five
    FakeListener ear;
    ListenerGuard guard(&ear);
    FakeOps ops;
    const Vector3 farPos = f.Z(f.farBase).pos;
    ops.exposure = HiddenNear(farPos, 2000.0f); // only round FarBase is unseen
    REQUIRE(ops.exposure(farPos) == CAExposeHidden);
    REQUIRE(ops.exposure(f.Z(f.post).pos) == CAExposeSeen);
    const Vector3 town = f.Z(f.town).pos;
    const CounterAttackView near = Fixture::View(town.X(), town.Z());
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.ca.LaunchFunded(f.registry, near, FootOrder(f.town, f.post, 5), &ops, fired));
    CHECK(f.Z(f.post).garrison == Approx(1.0f));
    f.ca.Step(f.registry, near, 1.0f, &ops, fired); // tried: nothing unseen from the Post, re-sourced at once
    REQUIRE(f.ca.Count() == 1);
    CHECK(Fixture::Str(f.ca.Get(0)->source) == "FarBase");
    CHECK(f.ca.Get(0)->deferTime == Approx(0.0f));
    REQUIRE(ear.Count(CAOpResourced) == 1);
    CHECK(ear.Last(CAOpResourced)->source == f.farBase);
    CHECK(ear.Last(CAOpResourced)->bodies == 5);
    // the Post got its five back (pending: the player stands 671 m from it,
    // inside its deposit bubble); the Mid base was passed over; FarBase paid
    CHECK(f.Z(f.post).garrison + (float)f.Z(f.post).pending == Approx(6.0f));
    CHECK(f.Z(mid).garrison == Approx(3.0f));
    CHECK(f.Z(f.farBase).garrison == Approx(7.0f));
    CHECK(f.ca.BornOps() == 0);
    // the next step births it at FarBase (the test world refuses the squad)
    f.ca.Step(f.registry, near, 1.0f, &ops, fired);
    CHECK(ops.spawns == 1);
    CHECK(Dist2DTest(ops.lastFrom, farPos) < 1.0f);
    // FarBase's road is watched too: no base passes, it waits, then is
    // cancelled unseen
    ops.exposure = AllSeen();
    f.ca.Step(f.registry, near, CounterAttack::RetryInterval + 1.0f, &ops, fired); // the retry runs out
    f.ca.Step(f.registry, near, 1.0f, &ops, fired);                                // nothing passes: waits
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->hiddenLogged);
    CHECK(ear.Count(CAOpResourced) == 1);
    f.ca.Step(f.registry, near, f.ca.Tuning().birthGiveUp + 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpCancelledUnseen) == 1);
    CHECK(ear.Last(CAOpCancelledUnseen)->bodies == 5);
    CHECK(ear.Last(CAOpCancelledUnseen)->opId == 7);
    CHECK(f.Z(f.farBase).garrison == Approx(12.0f));
    CHECK(ops.spawns == 1);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    // nothing visible ever happened: no counterAttack event, no turn-back
    CHECK(CountEvents(fired, ZECounterAttack, f.town) == 0);
    CHECK(ear.Count(CAOpAborted) == 0);
}

TEST_CASE("CounterAttack - birth chain: a nearer base whose road is watched is passed over; none passes, it waits",
          "[game][guerrilla][counterattack]")
{
    // Mid (nearer the town, spares the five) has a watched road; FarBase's
    // is not: the re-source skips Mid for FarBase
    {
        Fixture f;
        f.Load(kChainConfig);
        f.RaiseTown(f.town);
        const int mid = f.registry.FindZoneIndex("Mid");
        REQUIRE(mid >= 0);
        REQUIRE(f.Z(mid).garrison == Approx(8.0f));
        FakeListener ear;
        ListenerGuard guard(&ear);
        FakeOps ops;
        ops.exposure = HiddenNear(f.Z(f.farBase).pos, 2000.0f);
        REQUIRE(ops.exposure(f.Z(mid).pos) == CAExposeSeen);
        const Vector3 town = f.Z(f.town).pos;
        const CounterAttackView near = Fixture::View(town.X(), town.Z());
        AutoArray<ZoneEventRecord> fired;
        REQUIRE(f.ca.LaunchFunded(f.registry, near, FootOrder(f.town, f.post, 5), &ops, fired));
        f.ca.Step(f.registry, near, 1.0f, &ops, fired);
        REQUIRE(f.ca.Count() == 1);
        CHECK(Fixture::Str(f.ca.Get(0)->source) == "FarBase");
        CHECK(f.Z(mid).garrison == Approx(8.0f)); // not debited
        CHECK(f.Z(f.farBase).garrison == Approx(7.0f));
        CHECK(ear.Count(CAOpResourced) == 1);
    }
    // every road watched: no re-source before the give-up wait, then cancel
    {
        Fixture f;
        f.Load(kChainConfig);
        f.RaiseTown(f.town);
        FakeListener ear;
        ListenerGuard guard(&ear);
        FakeOps ops;
        ops.exposure = AllSeen();
        const Vector3 town = f.Z(f.town).pos;
        const CounterAttackView near = Fixture::View(town.X(), town.Z());
        AutoArray<ZoneEventRecord> fired;
        REQUIRE(f.ca.LaunchFunded(f.registry, near, FootOrder(f.town, f.post, 5), &ops, fired));
        for (int i = 0; i < 10; i++)
        {
            f.ca.Step(f.registry, near, 30.0f, &ops, fired); // 300 s, half the give-up
        }
        REQUIRE(f.ca.Count() == 1);
        CHECK(Fixture::Str(f.ca.Get(0)->source) == "Post");
        CHECK(ear.Count(CAOpResourced) == 0);
        CHECK(ear.Count(CAOpCancelledUnseen) == 0);
        for (int i = 0; i < 12 && f.ca.Count() > 0; i++)
        {
            f.ca.Step(f.registry, near, 30.0f, &ops, fired);
        }
        CHECK(f.ca.Count() == 0);
        CHECK(ear.Count(CAOpResourced) == 0);
        CHECK(ear.Count(CAOpCancelledUnseen) == 1);
        CHECK(ear.Count(CAOpAborted) == 0);
        CHECK(ops.spawns == 0);
    }
}

TEST_CASE("CounterAttack - key on, director DORMANT: a legacy record takes the birth chain, never turns back (D6)",
          "[game][guerrilla][counterattack]")
{
    // the legacy Decide path with the key on: the Post's road watched,
    // FarBase's not; the legacy record is re-sourced, not turned back
    {
        Fixture f;
        f.Load(Certain("occupierForces = 1;"));
        f.RaiseTown(f.town);
        REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
        FakeListener ear;
        ListenerGuard guard(&ear);
        FakeOps ops;
        ops.exposure = HiddenNear(f.Z(f.farBase).pos, 2000.0f);
        const Vector3 town = f.Z(f.town).pos;
        const CounterAttackView near = Fixture::View(town.X(), town.Z());
        AutoArray<ZoneEventRecord> fired;
        f.ca.Step(f.registry, near, 300.0f, &ops, fired); // launched with the town in the bubble
        REQUIRE(f.ca.Count() == 1);
        CHECK_FALSE(f.ca.Get(0)->funded);
        CHECK(f.ca.Get(0)->origin == CAOLegacy);
        for (int i = 0; i < 3 && ear.Count(CAOpResourced) == 0; i++)
        {
            f.ca.Step(f.registry, near, 1.0f, &ops, fired);
        }
        REQUIRE(ear.Count(CAOpResourced) == 1);
        CHECK(Fixture::Str(f.ca.Get(0)->source) == "FarBase");
        CHECK(ear.Count(CAOpAborted) == 0);
    }
    // every road watched: after the give-up wait it is cancelled unseen,
    // its bodies back to the Post, never a visible turn-back (Aborted)
    {
        Fixture f;
        f.Load(Certain("occupierForces = 1;"));
        f.RaiseTown(f.town);
        REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
        FakeListener ear;
        ListenerGuard guard(&ear);
        FakeOps ops;
        ops.exposure = AllSeen();
        const Vector3 town = f.Z(f.town).pos;
        const CounterAttackView near = Fixture::View(town.X(), town.Z());
        AutoArray<ZoneEventRecord> fired;
        f.ca.Step(f.registry, near, 300.0f, &ops, fired);
        REQUIRE(f.ca.Count() == 1);
        const int strength = f.ca.Get(0)->strength;
        REQUIRE(strength > 0);
        for (int i = 0; i < 40 && f.ca.Count() > 0; i++)
        {
            f.ca.Step(f.registry, near, 25.0f, &ops, fired);
        }
        CHECK(f.ca.Count() == 0);
        CHECK(ear.Count(CAOpCancelledUnseen) == 1);
        CHECK(ear.Count(CAOpAborted) == 0);
        CHECK(f.Z(f.post).garrison + (float)f.Z(f.post).pending == Approx(6.0f));
        CHECK(CountEvents(fired, ZECounterAttack, f.town) == 0);
    }
}

TEST_CASE("CounterAttack - key on: an arrival against defenders goes live, waits over budget, never rolls",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain("occupierForces = 1;"));
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    FakeListener ear;
    ListenerGuard guard(&ear);
    FakeOps ops;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, &ops, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    // the legacy record walks at the real foot speed with the key on, over
    // the road the test world gives (the straight line, taken as the road)
    CHECK(f.ca.Get(0)->travel == Approx(670.82f / 1.5f).margin(0.5f));
    // three fighters stand in the town: no odds roll, a live birth at the
    // 250 m bound (off-screen the gate passes), which the test world refuses
    f.Z(f.town).lastGuer = 3;
    f.ca.Step(f.registry, far, 5000.0f, &ops, fired);
    CHECK(CountEvents(fired, ZECounterAttack, f.town) == 1);
    REQUIRE(ops.spawns == 1);
    CHECK(Dist2DTest(ops.lastFrom, f.Z(f.town).pos) >= 250.0f);
    CHECK(Dist2DTest(ops.lastFrom, f.Z(f.town).pos) < 320.0f);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(0.0f));
    CHECK(CountEvents(fired, ZECounterRepelled, f.town) == 0);
    CHECK(CountEvents(fired, ZETownLost, f.town) == 0);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    CHECK(ear.Count(CAOpArrived) == 1);
    // over budget it waits at the town edge doing nothing
    f.ca.TuningMutable().maxLive = 0;
    for (int i = 0; i < 5; i++)
    {
        f.ca.Step(f.registry, far, CounterAttack::RetryInterval + 1.0f, &ops, fired);
    }
    CHECK(ops.spawns == 1);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    CHECK(ear.Count(CAOpArrived) == 1);
    // the defenders leave: an empty town is held and drained
    f.Z(f.town).lastGuer = 0;
    f.ca.Step(f.registry, far, 1.0f, &ops, fired);
    CHECK(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.AbstractPresence(f.registry, f.town) == 5);
    // they come back while it holds: it goes back to the edge to be born
    f.ca.TuningMutable().maxLive = 2;
    f.Z(f.town).lastGuerHold = 1;
    f.ca.Step(f.registry, far, 1.0f, &ops, fired);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(ops.spawns == 2);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
}

TEST_CASE("CounterAttack - key off: the legacy Fight keeps A_lethal for the odds and A_bodies for the losses",
          "[game][guerrilla][counterattack]")
{
    bool sawRepel = false;
    bool sawRetake = false;
    for (int opId = 1; opId < 200 && !(sawRepel && sawRetake); opId++)
    {
        Fixture f;
        f.Load(Legacy());
        f.RaiseTown(f.town);
        CounterAttackOrder o = FootOrder(f.town, f.post, 5, opId);
        o.funded = false;
        o.abstractSpeed = 0;
        o.lethality = 2.0f; // A_lethal 10 for five bodies
        const CounterAttackView far = Fixture::View(FarX, FarZ);
        AutoArray<ZoneEventRecord> fired;
        REQUIRE(f.ca.LaunchFunded(f.registry, far, o, nullptr, fired));
        const unsigned int seed = f.ca.Get(0)->seed;
        CHECK(f.ca.Get(0)->lethality == Approx(2.0f));
        f.Z(f.town).lastGuer = 3;
        f.ca.Step(f.registry, far, 5000.0f, nullptr, fired);
        CHECK(f.ca.Count() == 0);
        const bool repelled = CounterAttack::Roll01(seed, 2) < CounterAttack::HoldOddsLethal(f.ca.Tuning(), 3, 10.0f);
        if (repelled)
        {
            sawRepel = true;
            // the losses count bodies: ceil(5 x 0.5) = 3 lost, 2 home
            CHECK(CountEvents(fired, ZECounterRepelled, f.town) == 1);
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

TEST_CASE("CounterAttack - record fields survive save/load; an old save loads as a legacy foot retake",
          "[game][guerrilla][counterattack][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "counter-attack-forces.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    f.RaiseTown(f.town);
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    CounterAttackOrder o = TruckOrder(f.town, f.post, 5, 41);
    o.lethality = 1.25f;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, o, nullptr, fired));
    const_cast<CounterAttack::Record*>(f.ca.Get(0))->tried = "Mid";
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
    CHECK(r->opId == 41);
    CHECK(r->kind == CAKRetake);
    CHECK(r->origin == CAODirector);
    CHECK(r->package == FPTruck);
    CHECK(r->priceUnits == 4);
    CHECK(Fixture::Str(r->hullClass) == "Truck5t");
    CHECK(r->crew == 1);
    CHECK(r->urgent);
    CHECK(r->funded);
    CHECK(r->abstractSpeed == Approx(saved.abstractSpeed));
    CHECK(r->lethality == Approx(1.25f));
    CHECK(Fixture::Str(r->tried) == "Mid");
    CHECK_FALSE(r->hullDone);
    CHECK(r->clock == Approx(saved.clock));
    CHECK(loaded.Tuning().forcesOn); // reread from the mission config

    // a save written before phase 3: the defaults
    {
        AutoArray<OldRecord> rows;
        OldRecord old;
        old.target = "Town";
        old.source = "Post";
        old.clock = 50.0f;
        old.travel = 100.0f;
        old.strength = 4;
        old.size = 6;
        rows.Add(old);
        int towns = 1;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.Serialize("Attacks", rows, 1) == LSOK);
        REQUIRE(ar.Serialize("townsTaken", towns, 1, 0) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    CounterAttack legacy;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(legacy.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(legacy.Serialize(ar) == LSOK);
    }
    REQUIRE(legacy.Count() == 1);
    const CounterAttack::Record* l = legacy.Get(0);
    CHECK(l->opId == 0);
    CHECK(l->kind == CAKRetake);
    CHECK(l->origin == CAOLegacy);
    CHECK(l->package == FPFoot);
    CHECK(l->hullClass.GetLength() == 0);
    CHECK_FALSE(l->funded);
    CHECK(l->abstractSpeed == Approx(0.0f));
    CHECK(l->lethality == Approx(1.0f));
    CHECK(l->strength == 4);
    CHECK(legacy.HomeHullCount() == 0);

    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

TEST_CASE("CounterAttack - pool routing: a passer-by revert is pool-funded, SPENT leaves it hollow (5.3, 5.5)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    OccupierForces& of = OccupierForces::Instance();
    const int before = of.Pool();
    const int ledger = Ledger(f, f.ca);
    AutoArray<ZoneEventRecord> fired;
    PasserByRevert(f, f.town, fired);
    CHECK(f.Z(f.town).garrison == Approx(4.0f));
    CHECK(f.Z(f.town).revertRefill == 4);
    CHECK(of.Pool() == before - 4);
    CHECK(Ledger(f, f.ca) == ledger);

    // an empty pool (SPENT): the next revert is hollow
    of.SetPool(0);
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    PasserByRevert(f, f.other, fired);
    CHECK(f.Z(f.other).garrison == Approx(0.0f));
    CHECK(f.Z(f.other).revertRefill == 0);
    CHECK(of.Pool() == 0);
}

TEST_CASE("CounterAttack - pool routing: a holding force settles as the town's garrison, its surplus to the pool",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    OccupierForces& of = OccupierForces::Instance();
    FakeListener ear;
    ListenerGuard guard(&ear);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    f.ca.Step(f.registry, far, 5000.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.SettlePresence(f.registry, f.town) == 5);
    const int before = of.Pool();
    const int ledger = Ledger(f, f.ca);
    // the drain reverts the town: the five are its garrison (four fit), the
    // fifth goes to the pool; nothing is drawn for the refill
    PasserByRevert(f, f.town, fired, f.ca.SettlePresence(f.registry, f.town));
    CHECK(f.Z(f.town).garrison == Approx(4.0f));
    CHECK(f.Z(f.town).revertRefill == 0);
    CHECK(of.Pool() == before + 1);
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(ear.Count(CAOpSettled) == 1);
    // pool + reserves + pending + operation strength is unchanged
    CHECK(Ledger(f, f.ca) == ledger);
    // with the key off a holding force settles nothing (the legacy refill)
    Fixture g;
    g.Load(Certain());
    CHECK(g.ca.SettlePresence(g.registry, g.town) == 0);
}

TEST_CASE("CounterAttack - a travelling force whose town reverts under it tops it up or goes home (5.4)",
          "[game][guerrilla][counterattack]")
{
    // the pool is empty, so the passer-by revert is hollow: the force goes
    // on as a top-up column and fills the garrison; one survivor goes home
    {
        Fixture f;
        f.Load(Certain("occupierForces = 1;"));
        f.RaiseTown(f.town);
        PoolGuard pool(f);
        FakeListener ear;
        ListenerGuard guard(&ear);
        REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
        const CounterAttackView far = Fixture::View(FarX, FarZ);
        AutoArray<ZoneEventRecord> fired;
        f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
        REQUIRE(f.ca.Get(0)->phase == CATravel);
        OccupierForces::Instance().SetPool(0);
        PasserByRevert(f, f.town, fired);
        REQUIRE(f.Z(f.town).garrison == Approx(0.0f));
        f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
        REQUIRE(f.ca.Count() == 1);
        CHECK(f.ca.Get(0)->kind == CAKReinforce);
        CHECK(f.ca.Get(0)->phase == CATravel);
        f.ca.Step(f.registry, far, 5000.0f, nullptr, fired);
        CHECK(f.ca.Count() == 0);
        CHECK(f.Z(f.town).garrison == Approx(4.0f));
        CHECK(f.Z(f.post).garrison == Approx(2.0f)); // 1 kept home + the one who did not fit
        CHECK(ear.Count(CAOpSettled) == 1);
        CHECK(ear.Last(CAOpSettled)->bodies == 4);
    }
    // the pool paid a full refill: the garrison is whole, the force goes home
    Fixture f;
    f.Load(Certain("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    PasserByRevert(f, f.town, fired);
    REQUIRE(f.Z(f.town).garrison == Approx(4.0f));
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.post).garrison == Approx(6.0f));
    CHECK(ear.Count(CAOpReturned) == 1);
    CHECK(ear.Last(CAOpReturned)->bodies == 5);
    CHECK(ear.Count(CAOpAborted) == 1);
}

TEST_CASE("CounterAttack - hulls: refund at home, loss, re-creation, an abstract hull refunded at the end",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;

    // a hull driving home intact: deleted where nobody sees it, its price
    // back, its driver home as a body
    CounterAttack::Record r;
    r.opId = 12;
    r.target = "Town";
    r.source = "Post";
    r.hullClass = "Truck5t";
    r.crew = 1;
    r.priceUnits = 4;
    f.Z(f.post).garrison = 3;
    HullOps hops;
    f.ca.SendHullHome(r, nullptr, nullptr, 1, &hops, f.registry);
    CHECK(f.ca.HomeHullCount() == 1);
    CHECK(hops.drives == 1);
    hops.unseen = false;
    f.ca.Step(f.registry, far, 1.0f, &hops, fired);
    CHECK(f.ca.HomeHullCount() == 1); // seen: it keeps driving
    hops.unseen = true;
    f.ca.Step(f.registry, far, CounterAttack::RetryInterval + 1.0f, &hops, fired);
    CHECK(f.ca.HomeHullCount() == 0);
    CHECK(hops.removed == 1);
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 3); // the hull's price, not its crew
    CHECK(ear.Last(CAOpHullRefund)->bodies == 1);
    CHECK(ear.Last(CAOpHullRefund)->opId == 12);
    CHECK(f.Z(f.post).garrison == Approx(4.0f));

    // destroyed on the way home: lost
    f.ca.SendHullHome(r, nullptr, nullptr, 1, &hops, f.registry);
    hops.state = CounterAttackOps::HullDestroyed;
    f.ca.Step(f.registry, far, 1.0f, &hops, fired);
    CHECK(f.ca.HomeHullCount() == 0);
    REQUIRE(ear.Count(CAOpHullLost) == 1);
    CHECK(ear.Last(CAOpHullLost)->units == 3);

    // a return to abstract travel keeps the hull's class and price, so the
    // next birth re-creates it
    CounterAttack::Record m = r;
    m.phase = CALive;
    m.strength = 5;
    CounterAttack::ReturnToRoad(m, 0.5f, 5, 100.0f);
    CHECK(Fixture::Str(m.hullClass) == "Truck5t");
    CHECK(m.priceUnits == 4);
    CHECK(m.crew == 1);
    CHECK_FALSE(m.hullDone);

    // the birth of a motorized force asks the world for its hull; a hull
    // that cannot be created sends the force on foot and the whole price back
    HullOps born;
    const CounterAttackView near = Fixture::View(f.Z(f.town).pos.X(), f.Z(f.town).pos.Z());
    REQUIRE(f.ca.LaunchFunded(f.registry, near, TruckOrder(f.town, f.post, 3), &born, fired));
    f.ca.Step(f.registry, near, 1.0f, &born, fired);
    CHECK(born.mounts == 1);
    CHECK(born.lastHull == "Truck5t");
    CHECK(born.lastCrew == 1);
    CHECK(born.squads == 1); // then on foot
    REQUIRE(ear.Count(CAOpHullRefund) == 2);
    CHECK(ear.Last(CAOpHullRefund)->units == 4);
    CHECK(f.ca.Get(0)->hullDone);

    // a motorized force that never needed its hull (arrived abstractly in an
    // empty town, settled): the hull drives home unseen at once
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    g.RaiseTown(g.town);
    FakeListener ear2;
    ListenerGuard guard2(&ear2);
    REQUIRE(g.ca.LaunchFunded(g.registry, far, TruckOrder(g.town, g.post, 5), nullptr, fired));
    REQUIRE(g.Z(g.post).garrison == Approx(1.0f));
    g.ca.Step(g.registry, far, 5000.0f, nullptr, fired);
    REQUIRE(g.ca.Get(0)->phase == CAHolding);
    PasserByRevert(g, g.town, fired, g.ca.SettlePresence(g.registry, g.town));
    g.ca.Step(g.registry, far, 1.0f, nullptr, fired);
    CHECK(g.ca.Count() == 0);
    REQUIRE(ear2.Count(CAOpSettled) == 1);
    REQUIRE(ear2.Count(CAOpHullRefund) == 1);
    CHECK(ear2.Last(CAOpHullRefund)->units == 3);
    CHECK(ear2.Last(CAOpHullRefund)->bodies == 1);
    CHECK(g.Z(g.post).garrison == Approx(2.0f)); // its driver home
}

TEST_CASE("CounterAttack - the win latch freezes launches, steps and reverts (7.4)", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(Certain());
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, far, 300.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    const float clock = f.ca.Get(0)->clock;

    struct EndGuard
    {
        ~EndGuard() { CampaignEnd::Instance().Clear(); } // never leak the latch into another case
    } endGuard;
    CampaignEnd& end = CampaignEnd::Instance();
    end.Clear();
    end.LoadFromParams(nullptr);
    f.Z(f.post).owner = "GUER";
    f.Z(f.farBase).owner = "GUER";
    REQUIRE(end.Evaluate(f.registry));
    REQUIRE(end.Won());
    CounterAttackView won = far;
    won.won = true;
    // nothing in flight steps
    f.ca.Step(f.registry, won, 5000.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->clock == Approx(clock));
    // nothing launches
    f.Z(f.other).garrison = 0;
    f.RaiseTown(f.other);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize));
    CHECK_FALSE(f.ca.LaunchFrom(f.registry, won, f.other, f.farBase, 3, nullptr, fired));
    // nothing reverts
    CHECK_FALSE(f.registry.RevertTown(f.town, fired));
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    end.Clear();
    CHECK(f.registry.RevertTown(f.town, fired));
}

// ===========================================================================
// Occupier forces, phase 4 part A (OCCUPIER-FORCES.md 7.1, 7.2, 9.1): BASE,
// HQ_ASSAULT and REINFORCE launches, HqAssaultPresence (D14), the HQ's fall,
// the moved HQ, a BASE force retaking an empty base, the wave cap, the
// record fields across a save and the lose freeze.
// ===========================================================================

namespace
{

// the HQ at the Camp, the player far away
CounterAttackView HqFar(Fixture& f)
{
    CounterAttackView v = Fixture::View(FarX, FarZ);
    v.hqEstablished = true;
    v.hqPos = f.Z(f.camp).pos;
    v.hqZone = f.camp;
    return v;
}

CounterAttackOrder KindOrder(int kind, int target, int source, int bodies, int opId)
{
    CounterAttackOrder o = FootOrder(target, source, bodies, opId);
    o.kind = kind;
    return o;
}

} // namespace

TEST_CASE("CounterAttack - phase 4 launches: BASE, HQ_ASSAULT and REINFORCE validation",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    f.Z(f.farBase).owner = "GUER"; // a held base
    const CounterAttackView far = HqFar(f);
    AutoArray<ZoneEventRecord> fired;

    // BASE: only on a held military zone
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKBase, f.town, f.post, 4, 1), nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKBase, f.camp, f.post, 4, 1), nullptr, fired));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKBase, f.farBase, f.post, 4, 1), nullptr, fired));
    CHECK(f.ca.Get(0)->kind == CAKBase);
    CHECK(f.ca.FindByTarget("FarBase") == 0);
    // one record per target
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKBase, f.farBase, f.post, 1, 2), nullptr, fired));

    // HQ_ASSAULT: needs an established HQ; target -1 allowed; destination HqPos
    CounterAttackView noHq = far;
    noHq.hqEstablished = false;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, noHq, KindOrder(CAKHqAssault, -1, f.post, 1, 3), nullptr, fired));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, -1, f.post, 1, 3), nullptr, fired));
    const int hq = f.ca.FindHqAssault();
    REQUIRE(hq >= 0);
    const CounterAttack::Record* r = f.ca.Get(hq);
    CHECK(r->kind == CAKHqAssault);
    CHECK(Fixture::Str(r->target).empty());
    CHECK(r->destPos.X() == Approx(far.hqPos.X()));
    CHECK(r->destPos.Z() == Approx(far.hqPos.Z()));
    CHECK(CounterAttack::DestOf(f.registry, *r).X() == Approx(far.hqPos.X()));
    // one assault at a time; an HQ record never takes a zone's slot
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, f.camp, f.post, 1, 4), nullptr, fired));
    CHECK(f.ca.FindByTarget("") < 0);

    // REINFORCE: an occupier-held post other than the source
    f.Z(f.farBase).owner = "EAST";
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.town, f.post, 1, 5), nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.post, f.post, 1, 5), nullptr, fired));
    // alarms and strikes are later phases
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKAlarm, f.town, f.post, 1, 6), nullptr, fired));
    // the lose latch refuses everything
    CounterAttackView lost = far;
    lost.lost = true;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, lost, FootOrder(f.town, f.post, 1, 7), nullptr, fired));
}

TEST_CASE("CounterAttack - HqAssaultPresence counts only HQ_ASSAULT forces at the HQ (D14)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town); // the town 300 m from the Camp: a passer-by's target
    FakeListener ear;
    ListenerGuard guard(&ear);
    const CounterAttackView far = HqFar(f);
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4, 1), nullptr, fired));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, f.camp, f.farBase, 6, 2), nullptr, fired));
    f.ca.Step(f.registry, far, 20000.0f, nullptr, fired);
    // both arrived: the retake holds the empty town, the assault holds the
    // undefended HQ; only the assault counts at the HQ
    const int hq = f.ca.FindHqAssault();
    REQUIRE(hq >= 0);
    REQUIRE(f.ca.Get(hq)->phase == CAHolding);
    REQUIRE(f.ca.Get(f.ca.FindByTarget("Town"))->phase == CAHolding);
    const Vector3 hqPos = far.hqPos;
    CHECK(f.ca.HqAssaultPresence(hqPos, 150.0f, nullptr) == 6);
    CHECK(f.ca.HqAssaultPresence(f.Z(f.town).pos, 150.0f, nullptr) == 0); // the town is 300 m off
    // the assault holding at a Camp HQ never settles as a zone garrison
    CHECK(f.ca.SettlePresence(f.registry, f.camp) == 0);
    // the player near the HQ suspends the holding force: it counts no more
    CounterAttackView near = far;
    near.playerX = hqPos.X();
    near.playerZ = hqPos.Z();
    f.ca.Step(f.registry, near, 1.0f, nullptr, fired);
    CHECK(f.ca.HqAssaultPresence(hqPos, 150.0f, nullptr) == 0);
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.HqAssaultPresence(hqPos, 150.0f, nullptr) == 6);
    // defenders at the HQ: the holding force goes live (it waits at the edge
    // here: no world), and an abstract force at the edge counts nothing
    CounterAttackView defended = far;
    defended.hqDefenders = 2;
    f.ca.Step(f.registry, defended, 1.0f, nullptr, fired);
    CHECK(f.ca.Get(f.ca.FindHqAssault())->phase == CATravel);
    CHECK(f.ca.HqAssaultPresence(hqPos, 150.0f, nullptr) == 0);
}

TEST_CASE("CounterAttack - the HQ falls: the assault settles, its bodies to the pool; a moved HQ sends it home",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    OccupierForces& of = OccupierForces::Instance();
    FakeListener ear;
    ListenerGuard guard(&ear);
    const CounterAttackView far = HqFar(f);
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, f.camp, f.farBase, 6, 2), nullptr, fired));
    f.ca.Step(f.registry, far, 20000.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CAHolding);
    const int before = of.Pool();
    // GuerrillaBase::Destroy un-establishes the HQ (and queues hqLost)
    CounterAttackView fallen = far;
    fallen.hqEstablished = false;
    CHECK(f.ca.SettleHqAssaults(f.registry, fallen, nullptr) == 1);
    CHECK(f.ca.Count() == 0);
    CHECK(of.Pool() == before + 6);
    REQUIRE(ear.Last(CAOpSettled) != nullptr);
    CHECK(ear.Last(CAOpSettled)->kind == CAKHqAssault);
    CHECK(ear.Last(CAOpSettled)->bodies == 6);

    // the same through the step (the HQ no longer established)
    f.Z(f.farBase).garrison = 12;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, f.camp, f.farBase, 6, 3), nullptr, fired));
    f.ca.Step(f.registry, fallen, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);

    // a moved HQ: the force goes home to its base
    f.Z(f.farBase).garrison = 12;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, f.camp, f.farBase, 6, 4), nullptr, fired));
    const float reserve = f.Z(f.farBase).garrison; // 6 left after the launch
    CounterAttackView moved = far;
    moved.hqPos = f.Z(f.other).pos;
    f.ca.Step(f.registry, moved, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.farBase).garrison == Approx(reserve + 6));
    CHECK(ear.Last(CAOpAborted) != nullptr);
}

TEST_CASE("CounterAttack - a BASE force holds an empty base and retakes it through occCapture",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.registry.Tuning().baseRetake);
    FakeListener ear;
    ListenerGuard guard(&ear);
    f.Z(f.farBase).owner = "GUER"; // a held base, empty
    f.Z(f.farBase).garrison = 0;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKBase, f.farBase, f.post, 5, 9), nullptr, fired));
    f.ca.Step(f.registry, far, 50000.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CAHolding);
    CHECK(f.ca.AbstractPresence(f.registry, f.farBase) == 5);
    CHECK(f.ca.SettlePresence(f.registry, f.farBase) == 5);
    // the zone tick runs the base meter off-screen on the abstract presence
    ZoneTickInputs in = f.Inputs(FarX, FarZ, -1, 0, f.farBase, 5);
    in.settleOcc.Resize(f.registry.NZones());
    for (int i = 0; i < in.settleOcc.Size(); i++)
    {
        in.settleOcc[i] = 0;
    }
    in.settleOcc[f.farBase] = 5;
    AutoArray<ZoneEventRecord> zf;
    for (int tick = 0; tick < 10 && stricmp(f.Z(f.farBase).owner, "GUER") == 0; tick++)
    {
        f.registry.EvaluateTick(in, zf);
    }
    CHECK(CountEvents(zf, ZEBaseLost, f.farBase) == 1);
    CHECK(Fixture::Str(f.Z(f.farBase).owner) == "EAST");
    // the holding bodies are the base's new garrison; the record settles
    CHECK(f.Z(f.farBase).garrison >= 5.0f);
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Last(CAOpSettled) != nullptr);
    CHECK(ear.Last(CAOpSettled)->kind == CAKBase);
}

TEST_CASE("CounterAttack - wave operations come on top of forcesOpsMax (OpsCap)", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    AutoArray<ZoneEventRecord> fired;
    f.ca.TuningMutable().forcesOpsMax = 0;
    CHECK(f.ca.OpsCap() == 0);
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4, 1), nullptr, fired));
    CounterAttackOrder wave = FootOrder(f.town, f.post, 4, 2);
    wave.origin = CAOWave;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, wave, nullptr, fired));
    CHECK(f.ca.WaveRecords() == 1);
    CHECK(f.ca.OpsCap() == 1);
    CHECK(f.ca.BornOps(false) == 0);
}

TEST_CASE("CounterAttack - destPos and hqDone survive save/load; the lose latch freezes the step",
          "[game][guerrilla][counterattack][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "counter-attack-phase4.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    const CounterAttackView far = HqFar(f);
    AutoArray<ZoneEventRecord> fired;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKHqAssault, -1, f.farBase, 6, 12), nullptr, fired));
    const_cast<CounterAttack::Record*>(f.ca.Get(0))->hqDone = true;
    const float clock = f.ca.Get(0)->clock;
    // lost: nothing steps
    CounterAttackView lost = far;
    lost.lost = true;
    f.ca.Step(f.registry, lost, 500.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->clock == Approx(clock));
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
    CHECK(loaded.Get(0)->kind == CAKHqAssault);
    CHECK(loaded.Get(0)->destPos.X() == Approx(far.hqPos.X()));
    CHECK(loaded.Get(0)->destPos.Z() == Approx(far.hqPos.Z()));
    CHECK(loaded.Get(0)->hqDone);
    CHECK(loaded.FindHqAssault() == 0);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

// ===========================================================================
// Phase 5 columns (OCCUPIER-FORCES.md 6.5) and the review fixes M2, M3, M5,
// L2, L3: a fake world that hands out real (inert) groups and hulls, so the
// birth, the escort, the town edge, losses, refunds and the leftover cleanup
// run through the record's links.
// ===========================================================================

namespace
{

struct ColumnOps final : CounterAttackOps
{
    Ref<EntityType> type = new EntityType(nullptr);
    Ref<Entity> carrier = new Entity(nullptr, type, -1);
    Ref<Entity> escort = new Entity(nullptr, type, -1);
    Ref<AIGroup> inf = new AIGroup();
    Ref<AIGroup> carrierCrew = new AIGroup();
    Ref<AIGroup> escortCrew = new AIGroup();
    // the fake world's state
    int infAlive = 4;
    Vector3 infAt = VZero;
    int carrierCrewAlive = 1, carrierSeated = 1, carrierState = HullIntact;
    int escortCrewAlive = 2, escortSeated = 2, escortState = HullIntact;
    Vector3 carrierCrewAt = VZero, escortCrewAt = VZero;
    int exposure = CAExposeHidden;
    std::function<int(Vector3Par)> exposureFn; // set: wins over exposure
    bool squadUnseen = false, hullUnseen = false, bodiesUnseen = false;
    // the top-up cap (5.5): the surplus group SplitOff hands out
    Ref<AIGroup> split = new AIGroup();
    int splitAlive = 0;
    Vector3 splitAt = VZero;
    int adopts = 0, adoptedBodies = 0;
    Vector3 lastCarrierDrive = VZero;
    // what the service asked for
    int mounts = 0, escorts = 0, squads = 0, dismounts = 0, moves = 0;
    int carrierSupports = 0, escortSupports = 0, escortDrives = 0;
    std::vector<std::string> removals; // "inf", "carrierCrew", "escortCrew", "hull:carrier", "hull:escort"
    Vector3 lastMountFrom = VZero, lastEscortFrom = VZero;

    std::string Name(AIGroup* g) const
    {
        if (g == inf.GetRef())
        {
            return "inf";
        }
        if (g == split.GetRef())
        {
            return "split";
        }
        return g == carrierCrew.GetRef() ? "carrierCrew" : (g ? "escortCrew" : "null");
    }
    int Exposure(Vector3Par p) const override { return exposureFn ? exposureFn(p) : exposure; }
    AIGroup* SpawnSquad(int, Vector3Par, Vector3Par) override
    {
        squads++;
        return inf;
    }
    AIGroup* DetachFromGarrison(int, int, Vector3Par, int& taken) override
    {
        taken = 0;
        return nullptr;
    }
    int Alive(AIGroup* g, Vector3& at) override
    {
        if (g == inf.GetRef())
        {
            at = infAt;
            return infAlive;
        }
        if (g == carrierCrew.GetRef())
        {
            at = carrierCrewAt;
            return carrierCrewAlive;
        }
        if (g == escortCrew.GetRef())
        {
            at = escortCrewAt;
            return escortCrewAlive;
        }
        if (g == split.GetRef())
        {
            at = splitAt;
            return splitAlive;
        }
        return 0;
    }
    void MoveOn(AIGroup*, Vector3Par) override { moves++; }
    void Assault(AIGroup*, Vector3Par) override {}
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        route.Add(from);
        route.Add(to);
    }
    void Remove(AIGroup* g) override { removals.push_back(Name(g)); }
    void Adopt(int, AIGroup* g) override
    {
        adopts++;
        adoptedBodies += g == inf.GetRef() ? infAlive : 0;
    }
    AIGroup* SplitOff(AIGroup* g, int n, Vector3Par, int& taken) override
    {
        // the leader always stays
        taken = g == inf.GetRef() ? (n < infAlive - 1 ? n : infAlive - 1) : 0;
        if (taken <= 0)
        {
            taken = 0;
            return nullptr;
        }
        infAlive -= taken;
        splitAlive = taken;
        splitAt = infAt;
        return split;
    }
    bool SquadUnseen(AIGroup*) const override { return squadUnseen; }
    bool BodiesUnseen(AIGroup*) const override { return bodiesUnseen; }
    AIGroup* SpawnMounted(const RString&, int, int, Vector3Par from, Vector3Par, Entity*& hull,
                          AIGroup*& crewGroup) override
    {
        mounts++;
        lastMountFrom = from;
        hull = carrier;
        crewGroup = carrierCrew;
        return inf;
    }
    Entity* SpawnEscort(const RString&, int, Vector3Par from, Vector3Par, AIGroup*& crewGroup) override
    {
        escorts++;
        lastEscortFrom = from;
        crewGroup = escortCrew;
        return escort;
    }
    void DriveOn(AIGroup* g, Vector3Par to) override
    {
        escortDrives += g == escortCrew.GetRef() ? 1 : 0;
        if (g == carrierCrew.GetRef())
        {
            lastCarrierDrive = to;
        }
    }
    void Dismount(AIGroup*, Entity*) override { dismounts++; }
    void Support(AIGroup* g, Vector3Par) override
    {
        carrierSupports += g == carrierCrew.GetRef() ? 1 : 0;
        escortSupports += g == escortCrew.GetRef() ? 1 : 0;
    }
    int HullState(Entity* hull, AIGroup*, int& crew) const override
    {
        crew = 0;
        if (hull && hull == carrier.GetRef())
        {
            crew = carrierSeated;
            return carrierState;
        }
        if (hull && hull == escort.GetRef())
        {
            crew = escortSeated;
            return escortState;
        }
        return HullGone;
    }
    float HullSpeed(Entity*) const override { return 0.0f; }
    bool HullUnseen(Entity*, AIGroup*) const override { return hullUnseen; }
    void RemoveHull(Entity* hull, AIGroup* crewGroup) override
    {
        if (crewGroup)
        {
            removals.push_back(Name(crewGroup)); // the crew FIRST
        }
        removals.push_back(hull == carrier.GetRef() ? "hull:carrier" : "hull:escort");
    }
};

int Removed(const ColumnOps& ops, const char* what)
{
    int n = 0;
    for (const std::string& s : ops.removals)
    {
        n += s == what ? 1 : 0;
    }
    return n;
}

// a truck carrier with an APC escort: 4 bodies, the truck and its driver
// (4), the M113 and its two crew (10)
CounterAttackOrder ColumnOrder(int target, int source, int opId = 21)
{
    CounterAttackOrder o = TruckOrder(target, source, 4, opId);
    o.escortClass = "M113";
    o.escortCrew = 2;
    o.escortPriceUnits = 10;
    o.priceUnits = 14;
    return o;
}

// the player standing in the town (the force must be live)
CounterAttackView AtTown(Fixture& f)
{
    return Fixture::View(f.Z(f.town).pos.X(), f.Z(f.town).pos.Z());
}

// a point d metres from the town toward the Post
Vector3 TowardPost(Fixture& f, float d)
{
    Vector3 dir = f.Z(f.post).pos - f.Z(f.town).pos;
    dir[1] = 0;
    return f.Z(f.town).pos + dir.Normalized() * d;
}

// a column launched and born at the Post with the player in the town
void BornColumn(Fixture& f, ColumnOps& ops, AutoArray<ZoneEventRecord>& fired)
{
    ops.infAt = f.Z(f.post).pos;
    ops.carrierCrewAt = f.Z(f.post).pos;
    ops.escortCrewAt = f.Z(f.post).pos;
    REQUIRE(f.ca.LaunchFunded(f.registry, AtTown(f), ColumnOrder(f.town, f.post), &ops, fired));
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    REQUIRE(f.ca.Get(0)->phase == CALive);
}

} // namespace

TEST_CASE("CounterAttack - a column: the escort is born unseen behind the carrier, drives with it and supports",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    BornColumn(f, ops, fired);
    const CounterAttack::Record* r = f.ca.Get(0);
    // both hulls born, each hull's share of the price kept apart
    CHECK(ops.mounts == 1);
    CHECK(ops.escorts == 1);
    CHECK(f.ca.LiveHulls() == 2);
    CHECK(CounterAttack::LiveEscortOf(*r) == 1);
    CHECK(r->priceUnits == 4);
    CHECK(r->escortPriceUnits == 10);
    CHECK(r->EscortHullPrice() == 8);
    CHECK(r->OwedUnits() == 14);
    // EscortGap behind the carrier, on the side away from the town
    CHECK(Dist2DTest(ops.lastEscortFrom, ops.lastMountFrom) == Approx(CounterAttack::EscortGap).margin(0.1f));
    CHECK(Dist2DTest(ops.lastEscortFrom, f.Z(f.town).pos) > Dist2DTest(ops.lastMountFrom, f.Z(f.town).pos));

    // a return to abstract travel (everyone out of sight, the player far):
    // the cargo, then each hull crew first; the prices stay for the rebirth
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    ops.squadUnseen = true;
    ops.bodiesUnseen = true; // every member, the dead too (the vanish checks corpses)
    ops.hullUnseen = true;
    f.ca.Step(f.registry, far, CounterAttack::VanishHold + 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(r->phase == CATravel);
    CHECK(f.ca.LiveHulls() == 0);
    const std::vector<std::string> crewFirst = {"inf", "carrierCrew", "hull:carrier", "escortCrew", "hull:escort"};
    CHECK(ops.removals == crewFirst);
    CHECK(r->priceUnits == 4);
    CHECK(r->escortPriceUnits == 10);
    CHECK(r->OwedUnits() == 14);
    CHECK(ear.Count(CAOpHullLost) == 2);
    CHECK(ear.Last(CAOpHullLost)->units == 0);

    // re-born together when the player comes back
    ops.removals.clear();
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(r->phase == CALive);
    CHECK(ops.mounts == 2);
    CHECK(ops.escorts == 2);
    CHECK(f.ca.LiveHulls() == 2);

    // the escort drives the carrier's legs; at the 250 m mark the truck stops,
    // the infantry gets out, the truck holds back and the escort supports
    ops.infAt = TowardPost(f, 600.0f);
    ops.squadUnseen = false;
    ops.bodiesUnseen = false;
    ops.hullUnseen = false;
    f.ca.Step(f.registry, AtTown(f), CounterAttack::ResendInterval + 1.0f, &ops, fired);
    CHECK(ops.escortDrives > 0);
    CHECK_FALSE(r->escortWait);
    // the column keeps together: the carrier waits for a lagging escort
    ops.infAt = TowardPost(f, 400.0f); // the escort still at the Post, 271 m back
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(r->escortWait);
    ops.escortCrewAt = TowardPost(f, 440.0f);
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK_FALSE(r->escortWait);
    ops.infAt = TowardPost(f, 200.0f);
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(ops.escortSupports >= 1); // it closes up as the carrier brakes
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(ops.dismounts == 1);
    CHECK(r->dismounted);
    CHECK(ops.carrierSupports == 0); // a truck holds back
    CHECK(ops.escortSupports >= 2);
    CHECK(ear.Count(CAOpArrived) == 1);

    // the infantry is wiped out: both intact hulls drive home and are
    // deleted unseen, each refunding its own hull price, the crews home
    // (the player stands within the deposit bubble of the Post: the crews
    // come home as a pending deposit, S5)
    const float postBefore = f.Z(f.post).garrison + (float)f.Z(f.post).pending;
    ops.infAlive = 0;
    ops.hullUnseen = true;
    ops.bodiesUnseen = true;
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.ca.HomeHullCount() == 0);
    REQUIRE(ear.Count(CAOpHullRefund) == 2);
    int refunded = 0, crewHome = 0;
    for (const CounterAttackOpEvent& ev : ear.events)
    {
        if (ev.type == CAOpHullRefund)
        {
            refunded += ev.units;
            crewHome += ev.bodies;
        }
    }
    CHECK(refunded == 3 + 8);
    CHECK(crewHome == 1 + 2);
    CHECK(f.Z(f.post).garrison + (float)f.Z(f.post).pending == Approx(postBefore + 3.0f));
    // the wiped squad's dead group left unseen (L7)
    CHECK(f.ca.LeftoverCount() == 0);
    CHECK(Removed(ops, "inf") == 1);
}

TEST_CASE("CounterAttack - lost hulls leave a cleanup: wrecks stay, abandoned hulls and crews go unseen (M3)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    BornColumn(f, ops, fired);
    const CounterAttack::Record* r = f.ca.Get(0);
    const int lostBefore = OccupierForces::Instance().Lost();

    // the escort is destroyed; one of its two crew bailed out alive next to
    // the player
    ops.escortState = CounterAttackOps::HullDestroyed;
    ops.escortSeated = 0;
    ops.escortCrewAlive = 1;
    ops.escortCrewAt = f.Z(f.town).pos;
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(r->escortDone);
    CHECK(CounterAttack::LiveEscortOf(*r) == 0);
    REQUIRE(ear.Count(CAOpHullLost) == 1);
    CHECK(ear.Last(CAOpHullLost)->units == 8);
    CHECK(OccupierForces::Instance().Lost() == lostBefore + 1); // only the dead crewman (L4)
    REQUIRE(f.ca.LeftoverCount() == 1);
    CHECK(f.ca.GetLeftover(0)->hull.GetLink() == nullptr); // the wreck stays in the world
    CHECK(f.ca.GetLeftover(0)->living == 1);
    // the column carries on with its carrier
    CHECK(r->phase == CALive);
    CHECK(f.ca.LiveHulls() == 1);

    // next to the player the straggler stays, even unseen
    ops.bodiesUnseen = true;
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.LeftoverCount() == 1);
    // away from him and unseen: deleted, home as a body
    const float postBefore = f.Z(f.post).garrison + (float)f.Z(f.post).pending;
    ops.escortCrewAt = Vector3(FarX, 0, FarZ);
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.LeftoverCount() == 0);
    CHECK(Removed(ops, "escortCrew") == 1);
    CHECK(f.Z(f.post).garrison + (float)f.Z(f.post).pending == Approx(postBefore + 1.0f));

    // the truck's driver dies in his seat: the intact truck is abandoned; it
    // and the dead crew's group leave only where nobody sees them
    ops.removals.clear();
    ops.carrierSeated = 0;
    ops.carrierCrewAlive = 0;
    ops.hullUnseen = false;
    ops.bodiesUnseen = false;
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    CHECK(r->hullDone);
    CHECK(r->OwedUnits() == 0);
    REQUIRE(f.ca.LeftoverCount() == 1);
    CHECK(f.ca.GetLeftover(0)->hull.GetLink() == ops.carrier.GetRef());
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.LeftoverCount() == 1); // seen: kept
    ops.hullUnseen = true;
    ops.bodiesUnseen = true;
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.LeftoverCount() == 0);
    CHECK(Removed(ops, "hull:carrier") == 1);
    CHECK(Removed(ops, "carrierCrew") == 1);
}

TEST_CASE("CounterAttack - a force held at the town edge by the player gives up: born there or cancelled unseen (M2)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    ops.exposure = CAExposeSeen; // every road point is in the player's view
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4, 31), &ops, fired));
    const CounterAttack::Record* r = f.ca.Get(0);
    // almost there, off-screen
    f.ca.Step(f.registry, far, r->travel - 5.0f, &ops, fired);
    REQUIRE(r->phase == CATravel);
    // the player walks into his town: the force holds at the edge
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(r->edgeHold);
    f.ca.Step(f.registry, AtTown(f), 60.0f, &ops, fired);
    CHECK(f.ca.Count() == 1); // not yet
    // the give-up: no unseen point at the town edge either, so it is called
    // off unseen; its bodies go home and the town is free for other operations
    f.ca.Step(f.registry, AtTown(f), f.ca.Tuning().birthGiveUp, &ops, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.ca.FindByTarget("Town") < 0);
    REQUIRE(ear.Count(CAOpCancelledUnseen) == 1);
    CHECK(ear.Last(CAOpCancelledUnseen)->units == 0); // foot: no hull price owed
    // home as a pending deposit (the Post is inside the player's bubble, S5)
    CHECK(f.Z(f.post).garrison + (float)f.Z(f.post).pending == Approx(6.0f));
    CHECK(ops.squads == 0);
    f.Z(f.post).garrison = 6; // the deposit released
    f.Z(f.post).pending = 0;
    f.Z(f.post).pendingOrigin = -1;

    // with an unseen point at the edge the held force is born there instead
    REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4, 32), &ops, fired));
    r = f.ca.Get(0);
    f.ca.Step(f.registry, far, r->travel - 5.0f, &ops, fired);
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(r->edgeHold);
    ops.exposure = CAExposeHidden;
    f.ca.Step(f.registry, AtTown(f), f.ca.Tuning().birthGiveUp, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(r->phase == CALive);
    CHECK(r->arrivalLive);
    CHECK(ops.squads == 1);
}

TEST_CASE("CounterAttack - a live top-up squad whose town is lost walks home and leaves unseen (M5)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    ops.infAt = f.Z(f.post).pos;
    // the Town is the occupier's (NEUTRAL, administered): a top-up column
    REQUIRE(f.ca.LaunchFunded(f.registry, AtTown(f), KindOrder(CAKReinforce, f.town, f.post, 4, 41), &ops, fired));
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    const CounterAttack::Record* r = f.ca.Get(0);
    REQUIRE(r->phase == CALive);
    CHECK(f.Z(f.post).garrison == Approx(2.0f));
    // the player takes the town while the squad walks in
    f.RaiseTown(f.town);
    const int movesBefore = ops.moves;
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(r->withdrawing);
    CHECK(ops.removals.empty()); // never deleted in view
    CHECK(ops.moves > movesBefore);
    // out of his sight and away: it leaves, its bodies home
    ops.squadUnseen = true;
    ops.bodiesUnseen = true; // every member, the dead too (the vanish checks corpses)
    ops.infAt = f.Z(f.post).pos;
    f.ca.Step(f.registry, Fixture::View(FarX, FarZ), 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(Removed(ops, "inf") == 1);
    REQUIRE(ear.Count(CAOpAborted) == 1);
    CHECK(f.Z(f.post).garrison == Approx(6.0f));
}

TEST_CASE("CounterAttack - REINFORCE columns top up a post through the room cap, refuse and return (phase 5, 5.5)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = Fixture::View(FarX, FarZ);
    // a bled base (4 of 12): four bodies from the Post travel and join it
    f.Z(f.farBase).garrison = 4;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.farBase, f.post, 4, 51), nullptr, fired));
    CHECK(f.Z(f.post).garrison == Approx(2.0f));
    f.ca.Step(f.registry, far, 100000.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(f.Z(f.farBase).garrison == Approx(8.0f));
    REQUIRE(ear.Count(CAOpSettled) == 1);
    CHECK(ear.Last(CAOpSettled)->bodies == 4);

    // only two fit: two join, two go home to the Post (its room)
    f.Z(f.farBase).garrison = 10;
    f.Z(f.post).garrison = 6;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.farBase, f.post, 4, 52), nullptr, fired));
    f.ca.Step(f.registry, far, 100000.0f, nullptr, fired);
    CHECK(f.Z(f.farBase).garrison == Approx(12.0f));
    CHECK(f.Z(f.post).garrison == Approx(4.0f));
    CHECK(ear.Last(CAOpSettled)->bodies == 2);

    // a post the resistance holds is refused, and one lost on the way sends
    // the column home
    f.Z(f.farBase).garrison = 4;
    f.Z(f.post).garrison = 6;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.farBase, f.post, 4, 53), nullptr, fired));
    f.Z(f.farBase).owner = "GUER";
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, KindOrder(CAKReinforce, f.farBase, f.post, 1, 54), nullptr, fired));
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(f.ca.Count() == 0);
    CHECK(ear.Count(CAOpAborted) == 1);
    CHECK(f.Z(f.post).garrison == Approx(6.0f));
}

// a view of the player at (px, pz) with the town's live garrison
CounterAttackView ViewWithLive(Fixture& f, float px, float pz, int zone, int live)
{
    CounterAttackView v = Fixture::View(px, pz);
    v.garrisonLive.Resize(f.registry.NZones());
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        v.garrisonLive[i] = 0;
    }
    v.garrisonLive[zone] = live;
    return v;
}

TEST_CASE("CounterAttack - a live top-up squad joins up to the room; the surplus walks home (5.5, garrisonMax)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    ops.infAt = f.Z(f.post).pos;
    const float max = floorf(f.Z(f.town).garrisonMax);
    REQUIRE(max == Approx(4.0f));
    // a top-up of 4 from the Post (6 -> 2); the town regrows while it walks
    REQUIRE(f.ca.LaunchFunded(f.registry, AtTown(f), KindOrder(CAKReinforce, f.town, f.post, 4, 43), &ops, fired));
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    const CounterAttack::Record* r = f.ca.Get(0);
    REQUIRE(r->phase == CALive);
    // at the town: 1 live defender, 2 in reserve; room 1 for 4
    f.Z(f.town).garrison = 2;
    const int ledger1 = Ledger(f, f.ca); // the squad's 4 count as its record's strength
    const int pool0 = OccupierForces::Instance().Pool();
    const CounterAttackView v = ViewWithLive(f, f.Z(f.town).pos.X(), f.Z(f.town).pos.Z(), f.town, 1);
    ops.infAt = f.Z(f.town).pos;
    f.ca.Step(f.registry, v, 1.0f, &ops, fired); // the town edge
    f.ca.Step(f.registry, v, 1.0f, &ops, fired); // the adoption
    REQUIRE(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpSettled) == 1);
    // the stored 2 gave way to the pool, 1 more split off and walks home:
    // 3 join; live 1 + 3 = 4 = garrisonMax, nothing over it
    CHECK(ear.Last(CAOpSettled)->bodies == 3);
    CHECK(ops.adopts == 1);
    CHECK(ops.adoptedBodies == 3);
    CHECK(f.Z(f.town).garrison == Approx(0.0f));
    CHECK(1 + ops.adoptedBodies + (int)f.Z(f.town).garrison + f.Z(f.town).pending <= (int)max);
    CHECK(OccupierForces::Instance().Pool() == pool0 + 2);
    REQUIRE(f.ca.LeftoverCount() == 1);
    CHECK(ops.splitAlive == 1);
    // the straggler leaves unseen, away from the player: home to the Post
    const float postBefore = f.Z(f.post).garrison;
    ops.splitAt = f.Z(f.post).pos;
    ops.bodiesUnseen = true;
    f.ca.Step(f.registry, Fixture::View(FarX, FarZ), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.LeftoverCount() == 0);
    CHECK(Removed(ops, "split") == 1);
    CHECK(f.Z(f.post).garrison == Approx(postBefore + 1.0f));
    // the 5.5 ledger: pool + reserves + pending + the bodies in the field
    // (the 3 adopted live ones) is unchanged
    CHECK(Ledger(f, f.ca) + ops.adoptedBodies == ledger1);

    // a town already full: the whole squad walks home, nothing joins
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    ColumnOps gops;
    gops.infAt = g.Z(g.post).pos;
    REQUIRE(g.ca.LaunchFunded(g.registry, AtTown(g), KindOrder(CAKReinforce, g.town, g.post, 4, 44), &gops, fired));
    g.ca.Step(g.registry, AtTown(g), 1.0f, &gops, fired);
    REQUIRE(g.ca.Get(0)->phase == CALive);
    const CounterAttackView gv = ViewWithLive(g, g.Z(g.town).pos.X(), g.Z(g.town).pos.Z(), g.town, 4);
    gops.infAt = g.Z(g.town).pos;
    g.ca.Step(g.registry, gv, 1.0f, &gops, fired);
    g.ca.Step(g.registry, gv, 1.0f, &gops, fired);
    CHECK(g.ca.Count() == 0);
    CHECK(gops.adopts == 0);
    CHECK(g.ca.LeftoverCount() == 1);
}

TEST_CASE("CounterAttack - a hull driving home never drives into a base the resistance took (L13)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    f.RaiseTown(f.town);
    ColumnOps ops;
    ops.infAt = f.Z(f.post).pos;
    ops.carrierCrewAt = f.Z(f.post).pos;
    REQUIRE(f.ca.LaunchFunded(f.registry, AtTown(f), TruckOrder(f.town, f.post, 4, 45), &ops, fired));
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(f.ca.Get(0)->phase == CALive);
    REQUIRE(ops.mounts == 1);
    // the squad is wiped; the truck drives home in the player's sight
    ops.infAlive = 0;
    ops.bodiesUnseen = true;
    ops.hullUnseen = false;
    f.ca.Step(f.registry, AtTown(f), 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 0);
    REQUIRE(f.ca.HomeHullCount() == 1);
    // the Post falls behind it: it turns for FarBase instead
    f.Z(f.post).owner = "GUER";
    ops.carrierCrewAt = TowardPost(f, 300.0f);
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(ops.lastCarrierDrive.X() == Approx(f.Z(f.farBase).pos.X()));
    CHECK(ops.lastCarrierDrive.Z() == Approx(f.Z(f.farBase).pos.Z()));
    // FarBase falls too: no base is left, it stops where it is
    f.Z(f.farBase).owner = "GUER";
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(ops.lastCarrierDrive.X() == Approx(ops.carrierCrewAt.X()));
    CHECK(ops.lastCarrierDrive.Z() == Approx(ops.carrierCrewAt.Z()));
    // unseen it is deleted; its driver goes to the pool (no base)
    const int pool0 = OccupierForces::Instance().Pool();
    ops.hullUnseen = true;
    f.ca.Step(f.registry, AtTown(f), CounterAttack::RetryInterval + 1.0f, &ops, fired);
    CHECK(f.ca.HomeHullCount() == 0);
    CHECK(Removed(ops, "hull:carrier") == 1);
    CHECK(OccupierForces::Instance().Pool() == pool0 + 1);
}

TEST_CASE("CounterAttack - a live force whose road ahead passes the player never returns to abstract travel (L13)",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    ColumnOps ops;
    // the player watches the road FarBase -> Town from its midpoint: every
    // point within 1200 m of him is seen
    const Vector3 a = f.Z(f.farBase).pos;
    const Vector3 b = f.Z(f.town).pos;
    const Vector3 mid = (a + b) * 0.5f;
    ops.exposureFn = [mid](Vector3Par p)
    {
        const float dx = p.X() - mid.X();
        const float dz = p.Z() - mid.Z();
        return dx * dx + dz * dz <= 1200.0f * 1200.0f ? (int)CAExposeSeen : (int)CAExposeHidden;
    };
    f.RaiseTown(f.town);
    const CounterAttackView onRoad = Fixture::View(mid.X(), mid.Z());
    REQUIRE(f.ca.LaunchFunded(f.registry, onRoad, TruckOrder(f.town, f.farBase, 4, 46), &ops, fired));
    const CounterAttack::Record* r = f.ca.Get(0);
    for (int i = 0; i < 4000 && r->phase != CALive; i++)
    {
        f.ca.Step(f.registry, onRoad, 5.0f, &ops, fired);
    }
    REQUIRE(r->phase == CALive);
    REQUIRE(ops.mounts == 1);
    // born out of his sight, more than the despawn edge back up the road
    const Vector3 born = ops.lastMountFrom;
    REQUIRE(sqrtf((born.X() - mid.X()) * (born.X() - mid.X()) + (born.Z() - mid.Z()) * (born.Z() - mid.Z())) >
            f.registry.Tuning().cacheRadius + 50.0f);
    ops.infAt = born;
    ops.carrierCrewAt = born;
    ops.squadUnseen = true;
    ops.bodiesUnseen = true;
    ops.hullUnseen = true;
    // unseen and away, but its road runs on through the player's bubble:
    // it stays live (abstract, it would be re-born behind him at once)
    for (int i = 0; i < 3; i++)
    {
        f.ca.Step(f.registry, onRoad, CounterAttack::VanishHold, &ops, fired);
    }
    CHECK(r->phase == CALive);
    CHECK(ops.mounts == 1);
    CHECK(Removed(ops, "hull:carrier") == 0);
    // the player steps 1500 m off the road: now it returns to the road
    Vector3 side = b - a;
    side[1] = 0;
    side = side.Normalized();
    const Vector3 off(mid.X() - side.Z() * 1500.0f, 0, mid.Z() + side.X() * 1500.0f);
    const CounterAttackView offRoad = Fixture::View(off.X(), off.Z());
    for (int i = 0; i < 3 && r->phase == CALive; i++)
    {
        f.ca.Step(f.registry, offRoad, CounterAttack::VanishHold, &ops, fired);
    }
    CHECK(r->phase == CATravel);
}

TEST_CASE("CounterAttack - forcesOpsMax has one reader: OccupierForces' tuning while it is active",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    CHECK(f.ca.OpsCap() == 2);
    f.ca.TuningMutable().forcesOpsMax = 0;
    CHECK(f.ca.OpsCap() == 0); // inactive: the local default
    {
        PoolGuard pool(f);
        OccupierForces::Instance().TuningMutable().opsMax = 5;
        CHECK(f.ca.OpsCap() == 5);
    }
    CHECK(f.ca.OpsCap() == 0);
}

// ---------------------------------------------------------------------------
// phase 6A: air packages, strikes and alarm answers (OCCUPIER-FORCES.md 3.1,
// 3.2, 6.5, 8.2, D3, D4, D9, D17, D20)
// ---------------------------------------------------------------------------

namespace
{

// a world with air: one helicopter (or jet) entity, its crew and its cargo
// groups; the airframe's status is whatever the case sets
struct AirOps final : CounterAttackOps
{
    Ref<EntityType> type = new EntityType(nullptr);
    Ref<Entity> heli = new Entity(nullptr, type, -1);
    Ref<AIGroup> cargo = new AIGroup();
    Ref<AIGroup> crew = new AIGroup();
    int exposure = CAExposeHidden;
    bool lzOk = true;
    bool known = true;
    CounterAttackAirStatus status;
    int cargoAlive = 0;
    Vector3 cargoAt = VZero;
    int inserts = 0, sorties = 0, reveals = 0, removed = 0;
    int nextId = 1;
    Vector3 lastBirth = VZero, lastLz = VZero, lastTarget = VZero;
    float lastAlt = 0;
    int lastN = 0;
    std::string lastClass;

    int Exposure(Vector3Par) const override { return exposure; }
    AIGroup* SpawnSquad(int, Vector3Par, Vector3Par) override { return nullptr; }
    AIGroup* DetachFromGarrison(int, int, Vector3Par, int& taken) override
    {
        taken = 0;
        return nullptr;
    }
    int Alive(AIGroup* g, Vector3& at) override
    {
        if (g && g == cargo.GetRef())
        {
            at = cargoAt;
            return cargoAlive;
        }
        return 0;
    }
    void MoveOn(AIGroup*, Vector3Par) override {}
    void Assault(AIGroup*, Vector3Par) override {}
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        route.Add(from);
        route.Add(to);
    }
    void Remove(AIGroup*) override { removed++; }
    void Adopt(int, AIGroup*) override {}
    bool SquadUnseen(AIGroup*) const override { return false; }
    bool AirPickLz(Vector3Par target, const Vector3*, Vector3& lz) override
    {
        lz = target + Vector3(400.0f, 0, 0);
        return lzOk;
    }
    int AirInsert(const RString& hullClass, int n, Vector3Par birth, float altAgl, Vector3Par lz, Vector3Par target,
                  AIGroup*& cargoOut, Entity*& hull, AIGroup*& crewGroup) override
    {
        inserts++;
        lastClass = (const char*)hullClass;
        lastN = n;
        lastBirth = birth;
        lastAlt = altAgl;
        lastLz = lz;
        lastTarget = target;
        cargoOut = cargo;
        hull = heli;
        crewGroup = crew;
        status.cargoBorn = n;
        return nextId++;
    }
    int AirSortie(const RString& hullClass, Vector3Par birth, float altAgl, Vector3Par target, float,
                  Entity*& hull, AIGroup*& crewGroup) override
    {
        sorties++;
        lastClass = (const char*)hullClass;
        lastBirth = birth;
        lastAlt = altAgl;
        lastTarget = target;
        hull = heli;
        crewGroup = crew;
        return nextId++;
    }
    bool AirStatus(int, CounterAttackAirStatus& out) const override
    {
        out = status;
        return known;
    }
    int AirRevealNear(int, Vector3Par, float) override { return ++reveals; }
};

CounterAttackOrder StrikeOrder(int zone, int opId = 61)
{
    CounterAttackOrder o;
    o.opId = opId;
    o.kind = CAKStrike;
    o.origin = CAODirector;
    o.package = FPJet;
    o.hullClass = "Su25";
    o.crew = 1;
    o.priceUnits = 23; // jet 22 + its pilot
    o.funded = true;
    o.strikeZone = zone;
    o.source = -1;
    o.abstractSpeed = 200.0f;
    return o;
}

CounterAttackOrder HeliOrder(int target, int source, int airfield, int opId = 62)
{
    CounterAttackOrder o = FootOrder(target, source, 4, opId);
    o.package = FPTransportHeli;
    o.hullClass = "Mi17";
    o.crew = 2;
    o.priceUnits = 12; // transport heli 10 + two crew
    o.airOrigin = airfield;
    o.abstractSpeed = CounterAttack::RealSpeed(FPTransportHeli, 240.0f);
    return o;
}

CounterAttackView WithMap(CounterAttackView v)
{
    v.mapSize = 10000.0f;
    return v;
}

} // namespace

TEST_CASE("CounterAttack - phase 6A: the off-map edge and the birth altitudes", "[game][guerrilla][counterattack]")
{
    const Vector3 t(1000, 0, 1300);
    // nearest edge: x = 1000 is the closest (west)
    Vector3 p = CounterAttack::OffMapEdgePoint(t, 10000.0f, -1, 1500.0f);
    CHECK(p.X() == Approx(-1500.0f));
    CHECK(p.Z() == Approx(1300.0f));
    p = CounterAttack::OffMapEdgePoint(t, 10000.0f, CAEdgeNorth, 1500.0f);
    CHECK(p.X() == Approx(1000.0f));
    CHECK(p.Z() == Approx(11500.0f));
    p = CounterAttack::OffMapEdgePoint(t, 10000.0f, CAEdgeEast, 500.0f);
    CHECK(p.X() == Approx(10500.0f));
    p = CounterAttack::OffMapEdgePoint(t, 10000.0f, CAEdgeSouth, 500.0f);
    CHECK(p.Z() == Approx(-500.0f));
    CHECK(CounterAttack::OffMapEdgePoint(t, 0.0f, -1, 1500.0f).X() == 0.0f); // no map: no edge
    CHECK(CounterAttack::AirBirthAlt(FPJet) == Approx(250.0f));
    CHECK(CounterAttack::AirBirthAlt(FPTransportHeli) == Approx(100.0f));
    CHECK(CounterAttack::AirBirthAlt(FPAttackHeli) == Approx(100.0f));
}

TEST_CASE("CounterAttack - phase 6A: forcesSortieTime and forcesOffMapEdge are read with the 16 defaults",
          "[game][guerrilla][counterattack]")
{
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        CHECK(f.ca.Tuning().sortieTime == Approx(300.0f));
        CHECK(f.ca.Tuning().offMapEdge == -1);
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1; forcesSortieTime = 9999; forcesOffMapEdge = \"E\";"));
        CHECK(f.ca.Tuning().sortieTime == Approx(1800.0f));
        CHECK(f.ca.Tuning().offMapEdge == CAEdgeEast);
    }
}

TEST_CASE("CounterAttack - phase 6A: strike and alarm validation in LaunchFunded", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));

    // strikes: a jet or an attack helicopter, an airframe, a target
    CounterAttackOrder bad = StrikeOrder(f.town);
    bad.package = FPTruck;
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, bad, nullptr, fired));
    bad = StrikeOrder(f.town);
    bad.hullClass = RString();
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, bad, nullptr, fired));
    bad = StrikeOrder(-1);
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, bad, nullptr, fired)); // no town, no reported position
    bad = StrikeOrder(f.town);
    bad.airOrigin = f.post; // an OUTPOST is no airfield
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, bad, nullptr, fired));
    CHECK(f.ca.Count() == 0);
    // a valid strike needs no source and no bodies; a second one on the same town is refused
    REQUIRE(f.ca.LaunchFunded(f.registry, far, StrikeOrder(f.town), nullptr, fired));
    CHECK_FALSE(f.ca.LaunchFunded(f.registry, far, StrikeOrder(f.town, 63), nullptr, fired));
    // a reported position works without a town
    CounterAttackOrder reported = StrikeOrder(-1, 64);
    reported.strikePos = Vector3(3000, 0, 3000);
    reported.package = FPAttackHeli;
    reported.hullClass = "Mi24";
    REQUIRE(f.ca.LaunchFunded(f.registry, far, reported, nullptr, fired));
    CHECK(f.ca.Get(1)->strikeZone == -1);
    CHECK(f.ca.Get(1)->strikePos.X() == Approx(3000.0f));
    // a strike does not take the town's one-record slot: a retake still goes
    REQUIRE(f.ca.LaunchFunded(f.registry, far, FootOrder(f.town, f.post, 4, 65), nullptr, fired));
    CHECK(f.ca.Count() == 3);

    // alarm answers (8.2): an occupier-held zone, never its own source, one at a time
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    g.RaiseTown(g.town);
    CounterAttackOrder alarm = KindOrder(CAKAlarm, -1, g.farBase, 4, 71);
    alarm.alarmZone = g.town; // a resistance-held town is no occupier zone
    CHECK_FALSE(g.ca.LaunchFunded(g.registry, far, alarm, nullptr, fired));
    alarm.alarmZone = g.farBase; // the zone under attack never answers itself
    CHECK_FALSE(g.ca.LaunchFunded(g.registry, far, alarm, nullptr, fired));
    alarm.alarmZone = g.post;
    alarm.alarmPos = g.Z(g.post).pos + Vector3(50, 0, 0);
    REQUIRE(g.ca.LaunchFunded(g.registry, far, alarm, nullptr, fired));
    const CounterAttack::Record* a = g.ca.Get(0);
    CHECK(a->kind == CAKAlarm);
    CHECK(a->alarmZone == g.post);
    CHECK(Fixture::Str(a->target) == "Post");
    CHECK(a->destPos.X() == Approx(g.Z(g.post).pos.X() + 50.0f)); // it travels to the reported position
    CHECK(g.ca.FindAlarm() == 0);
    alarm.opId = 72;
    CHECK_FALSE(g.ca.LaunchFunded(g.registry, far, alarm, nullptr, fired)); // one at a time
}

TEST_CASE("CounterAttack - phase 6A: an abstract strike reaches its town once, then flies home",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, StrikeOrder(f.town), nullptr, fired));
    const CounterAttack::Record* r = f.ca.Get(0);
    // off-map: the west edge 1500 m out, 2500 m from the town at 200 m/s
    CHECK(r->travel == Approx(12.5f));
    CHECK(r->airFrom.X() == Approx(-1500.0f));
    CHECK(ear.Count(CAOpLaunched) == 1);
    f.ca.Step(f.registry, far, 5.0f, nullptr, fired);
    CHECK(ear.Count(CAOpStruck) == 0);
    f.ca.Step(f.registry, far, 10.0f, nullptr, fired);
    REQUIRE(ear.Count(CAOpStruck) == 1);
    const CounterAttackOpEvent* s = ear.Last(CAOpStruck);
    CHECK(s->zone == f.town);
    CHECK(s->pos.X() == Approx(f.Z(f.town).pos.X()));
    CHECK(s->pos.Z() == Approx(f.Z(f.town).pos.Z()));
    CHECK(s->kind == CAKStrike);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 22); // the jet; its pilot home as a body
    CHECK(ear.Last(CAOpHullRefund)->bodies == 1);
}

TEST_CASE("CounterAttack - phase 6A: a live strike is born unseen at altitude and strikes once over its target",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    AirOps ops;
    const CounterAttackView atTown = WithMap(AtTown(f));
    REQUIRE(f.ca.LaunchFunded(f.registry, atTown, StrikeOrder(f.town), &ops, fired));
    // everything in view: it stays abstract and waits
    ops.exposure = CAExposeSeen;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(ops.sorties == 0);
    // hidden at altitude: born airborne, 250 m up for a jet
    ops.exposure = CAExposeHidden;
    f.ca.Step(f.registry, atTown, CounterAttack::RetryInterval + 1.0f, &ops, fired);
    REQUIRE(ops.sorties == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(r->phase == CALive);
    CHECK(ops.lastAlt == Approx(250.0f));
    CHECK(Dist2DTest(ops.lastBirth, f.Z(f.town).pos) >= 250.0f);
    CHECK(ear.Count(CAOpBorn) == 1);
    REQUIRE(ear.Count(CAOpAirborne) == 1);
    CHECK(ear.Last(CAOpAirborne)->zone == -1); // off-map
    // inbound: nothing struck yet
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(ear.Count(CAOpStruck) == 0);
    // over the target: struck once, reveals only fresh sightings (none here)
    ops.status.sortie = true;
    ops.status.loiter = true;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ear.Count(CAOpStruck) == 1);
    CHECK(ops.reveals == 0);
    // removed crew first by OccupierAir: the price back, the pilot home
    ops.status.gone = true;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 22);
    CHECK(ear.Count(CAOpStruck) == 1);
}

TEST_CASE("CounterAttack - phase 6A: a helicopter insertion is born airborne, lands, unloads and flies home",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    AirOps ops;
    const CounterAttackView atTown = WithMap(AtTown(f));
    REQUIRE(f.ca.LaunchFunded(f.registry, atTown, HeliOrder(f.town, f.post, f.farBase), &ops, fired));
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(r->IsAir());
    CHECK(r->airOrigin == f.farBase);
    CHECK(r->airFrom.X() == Approx(9000.0f)); // its flight line starts at the airfield
    CHECK(r->phase == CATravel);
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ops.inserts == 1);
    CHECK(r->phase == CALive);
    CHECK(ops.lastClass == "Mi17");
    CHECK(ops.lastN == 4);
    CHECK(ops.lastAlt == Approx(100.0f));
    CHECK(Dist2DTest(ops.lastBirth, f.Z(f.town).pos) >= 250.0f);
    CHECK(Dist2DTest(ops.lastLz, f.Z(f.town).pos) == Approx(400.0f));
    CHECK(r->airLz.X() == Approx(ops.lastLz.X()));
    CHECK(ear.Count(CAOpBorn) == 1);
    REQUIRE(ear.Count(CAOpAirborne) == 1);
    CHECK(ear.Last(CAOpAirborne)->zone == f.farBase);
    CHECK(f.ca.LiveHulls() == 1);
    // flying in: the record waits on OccupierAir
    ops.cargoAlive = 4;
    ops.cargoAt = ops.lastLz;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK_FALSE(r->dismounted);
    // down 5 m from the LZ and unloaded: the infantry is the live squad
    ops.status.landed = true;
    ops.status.landedOffset = 5.0f;
    ops.status.landedAt = ops.lastLz;
    ops.status.state = 3; // AISDeparting
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ear.Count(CAOpLanded) == 1);
    CHECK(ear.Last(CAOpLanded)->zone == f.town);
    CHECK(r->dismounted);
    CHECK(r->hullDone);
    CHECK(f.ca.AirHomes() == 1);
    CHECK(f.ca.LiveHulls() == 1); // the airframe flying home still counts
    CHECK(ear.Count(CAOpHullRefund) == 0);
    // removed crew first where unseen: the price back, the crew home as bodies
    ops.status.gone = true;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(f.ca.AirHomes() == 0);
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 10);
    CHECK(ear.Last(CAOpHullRefund)->bodies == 2);
}

TEST_CASE("CounterAttack - phase 6A: a landing off the LZ is a failed insertion, re-planned once, then cancelled",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    AirOps ops;
    const CounterAttackView atTown = WithMap(AtTown(f));
    REQUIRE(f.ca.LaunchFunded(f.registry, atTown, HeliOrder(f.town, f.post, f.farBase), &ops, fired));
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ops.inserts == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    // down 150 m off: failed (the airframe flies home with its cargo)
    ops.status.landed = true;
    ops.status.landedOffset = 150.0f;
    ops.status.failReason = 1; // AFROffLz
    ops.status.state = 5;      // AISFailed
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ear.Count(CAOpLanded) == 1);
    CHECK(ear.notes.back() == "off the LZ");
    CHECK(r->phase == CALive);
    // removed unseen with its cargo aboard: back to abstract flight, re-planned once
    ops.status.gone = true;
    ops.cargoAlive = 0;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(r->phase == CATravel);
    CHECK(r->airReplans == 1);
    CHECK(r->strength == 4);
    CHECK(ear.Count(CAOpCancelledUnseen) == 0);
    // the second insertion is born again
    ops.status = CounterAttackAirStatus();
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    REQUIRE(ops.inserts == 2);
    CHECK(r->phase == CALive);
    // and fails again: cancelled unseen, the bodies home, the airframe's price owed back
    ops.status.landed = true;
    ops.status.landedOffset = 300.0f;
    ops.status.failReason = 1;
    ops.status.state = 5;
    ops.status.gone = true;
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpCancelledUnseen) == 1);
    CHECK(ear.Last(CAOpCancelledUnseen)->units == 12);
    CHECK(ear.Last(CAOpCancelledUnseen)->bodies == 4);
}

TEST_CASE("CounterAttack - phase 6A: no LZ or no airframe sends the insertion on foot, its price back",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    AirOps ops;
    ops.lzOk = false;
    const CounterAttackView atTown = WithMap(AtTown(f));
    REQUIRE(f.ca.LaunchFunded(f.registry, atTown, HeliOrder(f.town, f.post, f.farBase), &ops, fired));
    f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
    CHECK(ops.inserts == 0);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(r->package == FPFoot);
    CHECK_FALSE(r->IsAir());
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 12);
}

TEST_CASE("CounterAttack - phase 6A: the air re-plan of the birth chain buys a helicopter from the pool",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.RaiseTown(f.town);
    PoolGuard pool(f);
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, TruckOrder(f.town, f.post, 4), nullptr, fired));
    CounterAttack::Record& r = const_cast<CounterAttack::Record&>(*f.ca.Get(0));
    REQUIRE(r.phase == CATravel);
    ForcePackageCatalogue none;
    CHECK_FALSE(f.ca.AirReplan(f.registry, far, r, none)); // no helicopter authored: cancel as before
    ForcePackageCatalogue cat;
    cat.hulls[FPTransportHeli].className = "Mi17";
    cat.hulls[FPTransportHeli].maxSpeedKmh = 240.0f;
    cat.hulls[FPTransportHeli].cargo = 14;
    cat.hulls[FPTransportHeli].crew = 2;
    const int before = OccupierForces::Instance().Pool();
    REQUIRE(f.ca.AirReplan(f.registry, far, r, cat));
    CHECK(r.package == FPTransportHeli);
    CHECK(Fixture::Str(r.hullClass) == "Mi17");
    CHECK(r.crew == 2);
    CHECK(r.airReplanned);
    CHECK(r.airOrigin == f.farBase); // the occupier-held airfield: the airfield price
    CHECK(r.priceUnits == 12);
    CHECK(OccupierForces::Instance().Pool() == before - 12);
    REQUIRE(ear.Count(CAOpHullRefund) == 1);
    CHECK(ear.Last(CAOpHullRefund)->units == 4); // the truck goes back through the ledger
    CHECK(r.airFrom.X() == Approx(9000.0f));
    CHECK(r.clock == Approx(r.travel));
    // re-planned air is not re-planned again
    CHECK_FALSE(f.ca.AirReplan(f.registry, far, r, cat));
    // cancelled while abstract: the directly spent price comes back directly
    const int mid = OccupierForces::Instance().Pool();
    f.Z(f.town).owner = "OCCUPIER"; // the target reverts: the record goes home
    f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
    CHECK(OccupierForces::Instance().Pool() >= mid);
}

TEST_CASE("CounterAttack - phase 6A: the air re-plan's airframe settles through the operation's ledger row (5.5)",
          "[game][guerrilla][counterattack][occupierforces][air]")
{
    // three endings of a re-planned insertion: it lands and the airframe
    // flies home, it is refunded while still abstract, it is shot down.  The
    // ledger row holds the airframe like a ground hull, so pool + outstanding
    // is conserved by the re-plan and by every refund; a loss moves exactly
    // the hull price from the row to the losses.
    for (int ending = 0; ending < 3; ending++)
    {
        INFO("ending " << ending << " (0 lands, 1 refunded abstract, 2 shot down)");
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        f.RaiseTown(f.town);
        PoolGuard pool(f);
        OccupierForces& of = OccupierForces::Instance();
        ListenerGuard guard(&of); // the director's own ledger hears the events
        AutoArray<ZoneEventRecord> fired;
        const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));
        const CounterAttackView atTown = WithMap(AtTown(f));
        // the director's side of a truck launch: the pool pays, a row holds it
        const CounterAttackOrder order = TruckOrder(f.town, f.post, 4);
        REQUIRE(of.Spend(order.priceUnits, "test launch"));
        REQUIRE(of.BookLeverOp(order.opId, "Town", order.kind, order.package, order.priceUnits));
        REQUIRE(f.ca.LaunchFunded(f.registry, far, order, nullptr, fired));
        f.Z(f.post).garrisonMax = 100; // room: bodies and crew home never spill into the pool
        CounterAttack::Record& r = const_cast<CounterAttack::Record&>(*f.ca.Get(0));
        REQUIRE(r.phase == CATravel);
        auto held = [&]() { return of.Pool() + of.OpOutstanding(of.FindOp(order.opId)); };
        const int held0 = held();
        const int lost0 = of.Lost();

        ForcePackageCatalogue cat;
        cat.hulls[FPTransportHeli].className = "Mi17";
        cat.hulls[FPTransportHeli].maxSpeedKmh = 240.0f;
        cat.hulls[FPTransportHeli].cargo = 14;
        cat.hulls[FPTransportHeli].crew = 2;
        REQUIRE(f.ca.AirReplan(f.registry, far, r, cat));
        // the row took the airframe: the truck's 4 back, the airframe's 12 on
        CHECK_FALSE(r.airReplanned);
        CHECK(r.package == FPTransportHeli); // gmOccupierOps column 3
        CHECK(Fixture::Str(r.hullClass) == "Mi17"); // column 11
        REQUIRE(of.FindOp(order.opId) >= 0);
        CHECK(of.OpPackage(of.FindOp(order.opId)) == FPTransportHeli);
        CHECK(of.OpOutstanding(of.FindOp(order.opId)) == 12);
        CHECK(held() == held0);

        if (ending == 1)
        {
            // the target reverts while it is abstract: the hull price back
            // through the row, the crew home as bodies
            f.Z(f.town).owner = "OCCUPIER";
            f.ca.Step(f.registry, far, 1.0f, nullptr, fired);
            CHECK(f.ca.Count() == 0);
            CHECK(held() == held0);
            CHECK(of.OpOutstanding(of.FindOp(order.opId)) == 2); // the crew settle as bodies (5.5)
            CHECK(of.Lost() == lost0);
            continue;
        }
        // born airborne near the town
        AirOps ops;
        for (int i = 0; i < 5 && ops.inserts == 0; i++)
        {
            f.ca.Step(f.registry, atTown, CounterAttack::RetryInterval + 1.0f, &ops, fired);
        }
        REQUIRE(ops.inserts == 1);
        CHECK(r.phase == CALive);
        CHECK(CounterAttack::LiveHullOf(r) == 1); // gmOccupierOps column 8
        CHECK(held() == held0);
        ops.cargoAlive = 4;
        ops.cargoAt = ops.lastLz;
        if (ending == 0)
        {
            // down on the LZ and unloaded; the airframe flies home and is
            // removed unseen: its price back through the row
            ops.status.landed = true;
            ops.status.landedOffset = 5.0f;
            ops.status.landedAt = ops.lastLz;
            ops.status.state = 3; // AISDeparting
            f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
            REQUIRE(f.ca.AirHomes() == 1);
            ops.status.gone = true;
            f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
            CHECK(f.ca.AirHomes() == 0);
            CHECK(held() == held0);
            CHECK(of.OpOutstanding(of.FindOp(order.opId)) == 2);
            CHECK(of.Lost() == lost0);
            continue;
        }
        // shot down with its cargo out: the hull price moves from the row to
        // the losses, the crew are lost as bodies
        ops.status.hullLost = true;
        f.ca.Step(f.registry, atTown, 1.0f, &ops, fired);
        CHECK(held() == held0 - 10);
        CHECK(of.OpOutstanding(of.FindOp(order.opId)) == 2);
        CHECK(of.Lost() == lost0 + 10 + 2);
    }
}

TEST_CASE("CounterAttack - phase 6A: an abstract alarm answer reaches the zone, answers once and settles",
          "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    FakeListener ear;
    ListenerGuard guard(&ear);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));
    f.Z(f.post).garrison = 2; // room for the answer
    CounterAttackOrder alarm = KindOrder(CAKAlarm, -1, f.farBase, 4, 81);
    alarm.alarmZone = f.post;
    REQUIRE(f.ca.LaunchFunded(f.registry, far, alarm, nullptr, fired));
    for (int i = 0; i < 100 && f.ca.Count() > 0; i++)
    {
        f.ca.Step(f.registry, far, 100.0f, nullptr, fired);
    }
    CHECK(f.ca.Count() == 0);
    REQUIRE(ear.Count(CAOpAlarmAnswered) == 1);
    CHECK(ear.Last(CAOpAlarmAnswered)->zone == f.post);
    CHECK(ear.Last(CAOpAlarmAnswered)->kind == CAKAlarm);
    CHECK(ear.Count(CAOpSettled) == 1);
}

TEST_CASE("CounterAttack - phase 6A: air fields and strikes survive save/load",
          "[game][guerrilla][counterattack][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "counter-attack-air.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    f.RaiseTown(f.town);
    AutoArray<ZoneEventRecord> fired;
    const CounterAttackView far = WithMap(Fixture::View(FarX, FarZ));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, StrikeOrder(f.town), nullptr, fired));
    REQUIRE(f.ca.LaunchFunded(f.registry, far, HeliOrder(f.town, f.post, f.farBase), nullptr, fired));
    f.ca.Step(f.registry, far, 2.0f, nullptr, fired);
    REQUIRE(f.ca.Count() == 2);
    const CounterAttack::Record strike = *f.ca.Get(0);
    const CounterAttack::Record heli = *f.ca.Get(1);
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
    REQUIRE(loaded.Count() == 2);
    const CounterAttack::Record* s = loaded.Get(0);
    CHECK(s->kind == CAKStrike);
    CHECK(s->package == FPJet);
    CHECK(s->strikeZone == f.town);
    CHECK(s->strikePos.X() == Approx(strike.strikePos.X()));
    CHECK(s->strikePos.Z() == Approx(strike.strikePos.Z()));
    CHECK(s->airFrom.X() == Approx(strike.airFrom.X()));
    CHECK(s->clock == Approx(strike.clock));
    CHECK_FALSE(s->struck);
    const CounterAttack::Record* h = loaded.Get(1);
    CHECK(h->package == FPTransportHeli);
    CHECK(h->airOrigin == f.farBase);
    CHECK(h->airFrom.X() == Approx(heli.airFrom.X()));
    CHECK(h->airFrom.Z() == Approx(heli.airFrom.Z()));
    CHECK(h->clock == Approx(heli.clock));
    CHECK(h->priceUnits == 12);
    CHECK(h->crew == 2);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}
