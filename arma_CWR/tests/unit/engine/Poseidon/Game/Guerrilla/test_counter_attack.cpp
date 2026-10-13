// Occupier counter-attack service (issue #79, CounterAttack.hpp): dispatch on
// a town's flip, the momentum decay, reserve debit and write-back, the
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
    CHECK(t.chance == Approx(1.0f));
    CHECK(t.delayMin == Approx(120.0f));
    CHECK(t.delayMax == Approx(300.0f));
    CHECK(t.defenderWeight == Approx(3.0f));
    CHECK(f.registry.Tuning().townRevertSupport == Approx(30.0f));
    CHECK(f.registry.Tuning().momentumSupportStep == Approx(0.1f));
    CHECK(f.registry.Tuning().momentumSupportCap == Approx(2.0f));

    Fixture g;
    g.Load(ConfigWith("counterAttackChance = 3; counterAttackDelayMin = 50; counterAttackDelayMax = 10; "
                      "counterAttackSize = -1; momentumSupportCap = 0.5;"));
    CHECK(g.ca.Tuning().chance == Approx(1.0f));
    CHECK(g.ca.Tuning().delayMin == Approx(50.0f));
    CHECK(g.ca.Tuning().delayMax == Approx(50.0f)); // raised to the minimum
    CHECK(g.ca.Tuning().size == Approx(0.0f));
    CHECK(g.registry.Tuning().momentumSupportCap == Approx(1.0f));
}

TEST_CASE("CounterAttack - pure rules: chance, size, odds, travel, spare", "[game][guerrilla][counterattack]")
{
    CounterAttackTuning t;
    // the first town: the full force, always answered
    CHECK(CounterAttack::DispatchChance(t, 0) == Approx(1.0f));
    CHECK(CounterAttack::ForceSize(t, 12, 0) == 6);
    // momentum: every town already held shrinks both
    CHECK(CounterAttack::DispatchChance(t, 1) == Approx(0.85f));
    CHECK(CounterAttack::DispatchChance(t, 3) == Approx(0.614125f));
    CHECK(CounterAttack::ForceSize(t, 12, 1) == 5);
    CHECK(CounterAttack::ForceSize(t, 12, 3) == 4);
    CHECK(CounterAttack::ForceSize(t, 12, 20) == 1); // never below one body
    for (int k = 0; k < 6; k++)
    {
        CHECK(CounterAttack::DispatchChance(t, k + 1) < CounterAttack::DispatchChance(t, k));
        CHECK(CounterAttack::ForceSize(t, 12, k + 1) <= CounterAttack::ForceSize(t, 12, k));
    }
    // the 60/40 baseline: three fighters against six attackers
    CHECK(CounterAttack::HoldOdds(t, 3, 6) == Approx(0.6f));
    CHECK(CounterAttack::HoldOdds(t, 0, 6) == Approx(0.0f));
    CHECK(CounterAttack::HoldOdds(t, 3, 0) == Approx(1.0f));
    CHECK(CounterAttack::HoldOdds(t, 3, 4) > CounterAttack::HoldOdds(t, 3, 6));
    // travel covers the road leg short of the staging point
    CHECK(CounterAttack::TravelTime(t, 2500.0f) == Approx(2000.0f * 1.3f / 4.0f));
    CHECK(CounterAttack::TravelTime(t, 300.0f) == Approx(CounterAttack::MinTravel));
    // a base keeps a quarter of its strength home
    CHECK(CounterAttack::Spare(t, 6.0f, 0, 6.0f, false) == 5);
    CHECK(CounterAttack::Spare(t, 0.0f, 6, 6.0f, true) == 5);
    CHECK(CounterAttack::Spare(t, 0.0f, 6, 6.0f, false) == 0); // a live garrison has no stored reserve
    CHECK(CounterAttack::Spare(t, 1.0f, 0, 6.0f, false) == 0);
    CHECK(CounterAttack::Spare(t, 12.0f, 0, 12.0f, false) == 9);
    // the momentum multiplier on support gains
    ZoneTuning zt;
    CHECK(ZoneRegistry::SupportMomentum(zt, 0) == Approx(1.0f));
    CHECK(ZoneRegistry::SupportMomentum(zt, 3) == Approx(1.3f));
    CHECK(ZoneRegistry::SupportMomentum(zt, 40) == Approx(2.0f)); // capped
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

TEST_CASE("CounterAttack - a town's flip schedules exactly one counter-attack", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
    AutoArray<ZoneEventRecord> fired = f.RaiseTown(f.town);
    REQUIRE(CountEvents(fired, ZECaptured, f.town) == 1);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    REQUIRE(f.ca.Count() == 1);
    const CounterAttack::Record* r = f.ca.Get(0);
    CHECK(Fixture::Str(r->target) == "Town");
    CHECK(r->phase == CAScheduled);
    CHECK(r->size == 6);
    CHECK(r->townsBefore == 0);
    CHECK(r->clock >= 120.0f);
    CHECK(r->clock <= 300.0f);
    // the same flip seen again does not add a second force
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Count() == 1);
    // not a flip: an administered town, a base, the Camp
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.other, Fixture::GroupSize));
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.post, Fixture::GroupSize));
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.camp, Fixture::GroupSize));
    // the start-town HQ is never a target
    Fixture hq;
    hq.Load(kCounterConfig);
    hq.registry.ExemptTownGarrison(hq.town);
    hq.RaiseTown(hq.town);
    CHECK_FALSE(hq.ca.OnTownFlipped(hq.registry, hq.town, Fixture::GroupSize));
}

TEST_CASE("CounterAttack - counterAttack = 0 restores the old behaviour", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("counterAttack = 0;"));
    CHECK_FALSE(f.ca.Tuning().enabled);
    CHECK(f.registry.Tuning().momentumSupportStep == Approx(0.0f));
    f.RaiseTown(f.town);
    CHECK_FALSE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Count() == 0);
    // and no momentum: Other rises at the plain supportRate with a town held
    AutoArray<ZoneEventRecord> fired;
    f.Z(f.other).garrison = 0;
    ZoneTickInputs in = f.Inputs(f.Z(f.other).pos.X(), f.Z(f.other).pos.Z(), f.other, 1);
    f.registry.EvaluateTick(in, fired);
    CHECK(f.Z(f.other).support == Approx(25.0f));
}

TEST_CASE("CounterAttack - momentum speeds support gains elsewhere", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
    f.RaiseTown(f.town); // one town held
    f.Z(f.other).garrison = 0;
    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs in = f.Inputs(f.Z(f.other).pos.X(), f.Z(f.other).pos.Z(), f.other, 1);
    f.registry.EvaluateTick(in, fired);
    CHECK(f.Z(f.other).support == Approx(20.0f + 5.0f * 1.1f));
}

TEST_CASE("CounterAttack - the second town gets a smaller, less certain answer", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(ConfigWith("counterAttackChanceDecay = 0;")); // keep the dispatch certain, test the size
    f.Z(f.other).owner = "GUER";                         // a town already held
    f.RaiseTown(f.town);
    REQUIRE(f.registry.LiberatedTownCount() == 2);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    CHECK(f.ca.Get(0)->townsBefore == 1);
    CHECK(f.ca.Get(0)->size == 5);

    // with the chance decaying to nothing the occupier stops answering
    Fixture g;
    g.Load(ConfigWith("counterAttackChanceDecay = 1;"));
    g.Z(g.other).owner = "GUER";
    g.RaiseTown(g.town);
    CHECK_FALSE(g.ca.OnTownFlipped(g.registry, g.town, Fixture::GroupSize));
}

TEST_CASE("CounterAttack - no base with a reserve: no counter-attack", "[game][guerrilla][counterattack]")
{
    Fixture f;
    f.Load(kCounterConfig);
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
    f.Load(kCounterConfig);
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
    f.Load(kCounterConfig);
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
        f.Load(kCounterConfig);
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
    f.Load(kCounterConfig);
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
    f.Load(kCounterConfig);
    f.RaiseTown(f.town);
    REQUIRE(f.ca.OnTownFlipped(f.registry, f.town, Fixture::GroupSize));
    // the player stands in the town; no world (ops null): the live spawn
    // cannot happen, so the force stays on the road and retries
    const CounterAttackView near = Fixture::View(f.Z(f.town).pos.X(), f.Z(f.town).pos.Z());
    AutoArray<ZoneEventRecord> fired;
    f.ca.Step(f.registry, near, 300.0f, nullptr, fired);
    REQUIRE(f.ca.Get(0)->phase == CATravel);
    f.ca.Step(f.registry, near, 1000.0f, nullptr, fired);
    REQUIRE(f.ca.Count() == 1);
    CHECK(f.ca.Get(0)->phase == CATravel);
    CHECK(f.ca.Get(0)->clock == Approx(CounterAttack::RetryInterval));
    CHECK(fired.Size() == 0);
    CHECK(Fixture::Str(f.Z(f.town).owner) == "GUER");
    // a dead player freezes the clocks
    CounterAttackView dead = near;
    dead.playerValid = false;
    f.ca.Step(f.registry, dead, 1000.0f, nullptr, fired);
    CHECK(f.ca.Get(0)->clock == Approx(CounterAttack::RetryInterval));
}

TEST_CASE("CounterAttack - pending attacks survive save/load", "[game][guerrilla][counterattack][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "counter-attack.bin";
    const std::string config = ConfigWith("counterAttackDelayMin = 200; counterAttackDelayMax = 200;");
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
    CHECK(r->strength == 5);
    CHECK(r->size == 6);
    CHECK(r->seed == saved.seed);
    CHECK(r->salt == saved.salt);
    CHECK(loaded.Tuning().delayMin == Approx(200.0f)); // reread from the mission config

    // the loaded record resolves exactly like the original: arrival in the
    // empty town holds it
    loaded.Step(f.registry, far, saved.clock + 1.0f, nullptr, fired);
    CHECK(loaded.Get(0)->phase == CAHolding);

    ExtParsMission.Clear();
    std::filesystem::remove(path);
}
