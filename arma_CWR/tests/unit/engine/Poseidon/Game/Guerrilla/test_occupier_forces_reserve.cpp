// Occupier forces, round 6 (owner decision 2026-10-17): THE RESERVE FLOOR
// (OCCUPIER-FORCES.md 6.3).  Video 7 (tmp/of-director-video-7) at 2884 s:
// "[op 4] APC (6, BMP) with T72 from Le Port Harbour to Sainte Marie (...
// the pool pays 28, ... short of the required lethality: the most lethal
// affordable package)" took a pool of 32 of 48 to 4, under the 9.6 floor,
// and the occupier answered nothing for most of an hour.  A commander keeps
// a reserve: a director-bought ground operation may not take the pool below
// forcesPoolFloor x poolStart by its own debit.  It waits (one line per
// target and plan, no token, no rest), it is never sent smaller than its
// plan required, and the plan's own cheapest qualifying package may go when
// it keeps the reserve.  Exempt: HQ assaults, wave operations, strikes and
// alarm answers to bases.  Split from test_occupier_forces.cpp (over 5000
// lines); the helpers are minimal copies of that file's.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp> // LogDetail (the line capture)

#include <spdlog/sinks/callback_sink.h>

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

struct Fixture
{
    std::string text;
    ParamFile file;
    ZoneRegistry registry;
    OccupierForces of;

    void Load(const std::string& config)
    {
        text = config;
        QIStream in(text.c_str(), text.size());
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        of.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        of.BindRegistry(&registry);
        REQUIRE(registry.IsActive());
        registry.NoteTickPlayer(true, 99999.0f, 99999.0f);
    }
    ZoneRecord& Z(const char* name)
    {
        const int i = registry.FindZoneIndex(name);
        REQUIRE(i >= 0);
        return *registry.GetZoneMutable(i);
    }
    int I(const char* name) const { return registry.FindZoneIndex(name); }
};

// every Core log payload while it lives
struct CoreLines
{
    spdlog::logger* logger = LogDetail::Get(Poseidon::Foundation::LogCategory::Core);
    std::vector<spdlog::sink_ptr> saved;
    spdlog::level::level_enum savedLevel = spdlog::level::off;
    std::vector<std::string> lines;
    CoreLines()
    {
        saved = logger->sinks();
        savedLevel = logger->level();
        logger->sinks().clear();
        logger->sinks().push_back(std::make_shared<spdlog::sinks::callback_sink_mt>(
            [this](const spdlog::details::log_msg& m) { lines.emplace_back(m.payload.data(), m.payload.size()); }));
        logger->set_level(spdlog::level::trace);
    }
    ~CoreLines()
    {
        logger->sinks() = saved;
        logger->set_level(savedLevel);
    }
    std::vector<std::string> With(std::initializer_list<const char*> needles) const
    {
        std::vector<std::string> out;
        for (const std::string& l : lines)
        {
            bool all = true;
            for (const char* n : needles)
            {
                all = all && l.find(n) != std::string::npos;
            }
            if (all)
            {
                out.push_back(l);
            }
        }
        return out;
    }
    std::string Trail() const
    {
        std::string t;
        for (const std::string& l : lines)
        {
            if (l.find("OccupierForces") != std::string::npos)
            {
                t += l + " | ";
            }
        }
        return t;
    }
};

// video 5's Malden as test_occupier_forces.cpp's Video5Config, at the
// shipped Abel's poolStart: R 26 + 2 x 11 towns = 48 (forcesPoolMin 24
// binds nothing), so the floor is 0.2 x 48 = 9.6
std::string Abel48Config()
{
    std::string cfg = "class CfgGuerrillaZones\n{\n    occupierForces = 1;\n    class Zones\n    {\n";
    cfg += "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; position[]={7515.0, 5790.0, 0.0}; "
           "};\n";
    cfg += "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"OCCUPIER\"; garrison=6; "
           "position[]={7600.0, 6300.0, 0.0}; };\n";
    cfg += "        class Harbour { name=\"Le Port Harbour\"; type=\"SEAPORT\"; owner=\"OCCUPIER\"; garrison=8; "
           "position[]={8410.0, 3690.0, 0.0}; };\n";
    cfg += "        class Airfield { name=\"Airfield\"; type=\"AIRFIELD\"; owner=\"OCCUPIER\"; garrison=12; "
           "position[]={8030.0, 9905.0, 0.0}; };\n";
    cfg += "        class Village { name=\"Village\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={7117.0, 6069.0, 0.0}; };\n";
    cfg += "        class LePort { name=\"Le Port\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={8242.0, 3205.0, 0.0}; };\n";
    cfg += "        class Arudy { name=\"Arudy\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={5600.0, 7200.0, 0.0}; };\n";
    for (int i = 0; i < 8; i++)
    {
        cfg += "        class F" + std::to_string(i) + " { name=\"F" + std::to_string(i) +
               "\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; position[]={" + std::to_string(1000 + 400 * i) +
               ".0, 1000.0, 0.0}; };\n";
    }
    cfg += "    };\n};\n";
    return cfg;
}

ForcePackageCatalogue UralOnly()
{
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Ural";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    return cat;
}

// the EAST ladder of Guerrilla.Abel without its air: Ural, BMP, T72
ForcePackageCatalogue EastGround()
{
    ForcePackageCatalogue cat = UralOnly();
    cat.hulls[FPApc].className = "BMP";
    cat.hulls[FPApc].maxSpeedKmh = 65;
    cat.hulls[FPApc].cargo = 8;
    cat.hulls[FPApc].crew = 3;
    cat.hulls[FPTank].className = "T72";
    cat.hulls[FPTank].maxSpeedKmh = 60;
    cat.hulls[FPTank].cargo = 0;
    cat.hulls[FPTank].crew = 3;
    return cat;
}

// CounterAttack::LaunchFunded stand-in: records the orders, a record per target
struct FakeLauncher final : DirectorLauncher
{
    std::vector<CounterAttackOrder> orders;
    std::vector<int> records;
    bool hq = false;
    bool Launch(const CounterAttackOrder& order) override
    {
        orders.push_back(order);
        if (order.kind == CAKHqAssault)
        {
            hq = true;
        }
        else
        {
            records.push_back(order.target);
        }
        return true;
    }
    int BornOps() const override { return 0; }
    bool HasRecord(int zoneIndex) const override
    {
        return std::find(records.begin(), records.end(), zoneIndex) != records.end();
    }
    bool HasHqRecord() const override { return hq; }
    void Resolve(int zoneIndex)
    {
        records.erase(std::remove(records.begin(), records.end(), zoneIndex), records.end());
    }
};

DirectorView ViewOf(const ZoneRegistry& registry)
{
    DirectorView v;
    const int n = registry.NZones();
    v.alert.Resize(n);
    v.spare.Resize(n);
    CounterAttackTuning t;
    for (int i = 0; i < n; i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        v.alert[i] = 0;
        v.spare[i] = 0;
        if (z && stricmp(z->owner, registry.OccupierSide()) == 0 && stricmp(z->type, "CITY") != 0)
        {
            v.spare[i] = CounterAttack::Spare(t, z->garrison, 0, z->garrisonMax, false);
        }
    }
    return v;
}

bool FlipTown(Fixture& f, const char* name, int minute)
{
    ZoneRecord& z = f.Z(name);
    z.owner = f.registry.ResistanceSide();
    z.flipDay = 1;
    z.flipMinute = minute;
    z.heat = z.heat + 40.0f > 100.0f ? 100.0f : z.heat + 40.0f;
    return f.of.NoteTownFlipped(f.registry, f.registry.FindZoneIndex(name));
}

std::vector<CounterAttackOrder> GroundOn(const FakeLauncher& l, int zone)
{
    std::vector<CounterAttackOrder> out;
    for (const CounterAttackOrder& o : l.orders)
    {
        if (o.target == zone && o.kind != CAKStrike)
        {
            out.push_back(o);
        }
    }
    return out;
}

// awake with two tokens; the Village, Le Port and Arudy held (heat 40), the
// other two resting for good, `only` alone a candidate
void Abel48Setup(Fixture& f, const ForcePackageCatalogue& cat, const char* only = "Le Port")
{
    f.Load(Abel48Config());
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 48);
    REQUIRE(f.of.ReserveFloorUnits() == Approx(9.6f));
    f.of.SetCatalogue(cat);
    FlipTown(f, "Village", 1);
    REQUIRE(FlipTown(f, "Le Port", 2));
    FlipTown(f, "Arudy", 3);
    for (const char* town : {"Village", "Le Port", "Arudy"})
    {
        if (strcmp(town, only) != 0)
        {
            f.of.ForceCooldown(town, 1.0e7f);
        }
    }
    // urgency kept out: no strike rule joins in (7.3)
    f.of.TuningMutable().urgentHeat = 100;
}

} // namespace

TEST_CASE("OccupierForces - the reserve floor's arithmetic and who it binds", "[game][guerrilla][occupierforces]")
{
    // the largest price that leaves (pool - price) / poolStart >= 0.2
    CHECK(OccupierForces::ReserveRoomOf(32, 48, 0.2f) == 22); // 10 of 48 is 20.8 %, 9 is 18.75 %
    CHECK(OccupierForces::ReserveRoomOf(38, 48, 0.2f) == 28); // video 7's 28 fits from 38
    CHECK(OccupierForces::ReserveRoomOf(37, 48, 0.2f) == 27);
    CHECK(OccupierForces::ReserveRoomOf(48, 48, 0.2f) == 38);
    CHECK(OccupierForces::ReserveRoomOf(8, 48, 0.2f) == 0); // already under the floor
    CHECK(OccupierForces::ReserveRoomOf(15, 50, 0.3f) == 0); // exactly at the floor: nothing to spend
    CHECK(OccupierForces::ReserveRoomOf(16, 50, 0.3f) == 1);
    CHECK(OccupierForces::ReserveRoomOf(20, 48, 0.0f) == 20); // the rule off

    // floored: the director's and push-out's retakes of towns and bases, an alarm answer to a town
    CHECK(OccupierForces::ReserveBindsKind(CAKRetake, CAODirector, false, 0.2f));
    CHECK(OccupierForces::ReserveBindsKind(CAKBase, CAODirector, true, 0.2f));
    CHECK(OccupierForces::ReserveBindsKind(CAKRetake, CAOPushout, false, 0.2f));
    CHECK(OccupierForces::ReserveBindsKind(CAKBase, CAOPushout, true, 0.2f));
    CHECK(OccupierForces::ReserveBindsKind(CAKAlarm, CAOAlarm, false, 0.2f));
    // exempt: the HQ assault (the final stand's push too), wave operations,
    // strikes, an alarm answer to a base, the legacy dispatch, the rule off
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKHqAssault, CAODirector, false, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKRetake, CAOWave, false, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKBase, CAOWave, true, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKReinforce, CAOWave, true, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKStrike, CAODirector, false, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKAlarm, CAOAlarm, true, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKRetake, CAOLegacy, false, 0.2f));
    CHECK_FALSE(OccupierForces::ReserveBindsKind(CAKRetake, CAODirector, false, 0.0f));
}

TEST_CASE("OccupierForces - video 7 op 4 replayed: the strongest package after massing waits for the reserve",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    Abel48Setup(f, EastGround());
    const int lePort = f.I("Le Port");
    // the video's Sainte Marie: six fighters reported, one repel, so the
    // ladder needs 12 infantry (above every base's spare) and lethality x1.5
    f.of.IngestTip(f.registry, lePort, 6, f.Z("Le Port").pos);
    f.of.ForceRepels("Le Port", 1);
    f.Z("Airfield").garrison = 8; // spares 5; the Harbour 6
    f.of.SetPool(32);
    FakeLauncher launcher;
    const int tokens = f.of.Tokens();
    REQUIRE(tokens >= 1);
    const int massingMax = f.of.Tuning().massingMax;
    for (int tick = 0; tick < massingMax; tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    }
    REQUIRE(launcher.orders.empty());
    REQUIRE(log.With({"massing for Le Port: need 12 infantry"}).size() == (size_t)massingMax);

    // the hold ends: the strongest package one base can bring is the video's
    // APC with a T72 escort (the pool pays 28), which would leave 4 of 48
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    INFO(log.Trail());
    CHECK(launcher.orders.empty()); // no smaller package in its place (truck and T72, 21, would fit)
    CHECK(f.of.Pool() == 32);
    CHECK(f.of.Tokens() == tokens);
    CHECK(f.of.Cooldown("Le Port") <= 0.0f);
    REQUIRE(f.of.ReserveWaitRows() == 1);
    const OccupierForces::ReserveWaitRow* w = f.of.ReserveWait(0);
    CHECK(Str(w->zone) == "Le Port");
    CHECK(w->price == 28);
    CHECK(w->best);
    CHECK(Str(w->plan) == "APC (6, BMP) with T72");
    const std::vector<std::string> lines = log.With({"waits, the pool keeps its reserve"});
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] ==
          "OccupierForces: Le Port: escalation retake after massing (APC (6, BMP) with T72, the pool pays 28, "
          "lethality 25.50 of 32.40, the most lethal package) waits, the pool keeps its reserve (pool 32 of 48, "
          "floor 9.6: the launch would leave 4)");

    // later ticks: the same plan waits, logged once, no new massing hold
    for (int tick = 0; tick < 5; tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    }
    CHECK(launcher.orders.empty());
    CHECK(log.With({"waits, the pool keeps its reserve"}).size() == 1);
    CHECK(log.With({"massing for Le Port: need 12 infantry"}).size() == (size_t)massingMax);
    CHECK(f.of.Tokens() == tokens);

    // 37: 9 would be left, still under 9.6
    f.of.SetPool(37);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(launcher.orders.empty());
    CHECK(log.With({"waits, the pool keeps its reserve"}).size() == 1);

    // 38: the full package goes and leaves 10
    f.of.SetPool(38);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    const std::vector<CounterAttackOrder> ops = GroundOn(launcher, lePort);
    REQUIRE(ops.size() == 1);
    CHECK(ops[0].package == FPApc);
    CHECK(Str(ops[0].hullClass) == "BMP");
    CHECK(Str(ops[0].escortClass) == "T72");
    CHECK(ops[0].bodies == 6);
    CHECK(ops[0].priceUnits == 28);
    CHECK(f.of.Pool() == 10);
    CHECK(f.of.Tokens() == tokens - 1);
    CHECK(f.of.ReserveWaitRows() == 0);
}

TEST_CASE("OccupierForces - a qualifying plan over the floor waits rather than go undersized",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    Abel48Setup(f, EastGround());
    const int lePort = f.I("Le Port");
    // four fighters and one repel: lethality 21.6 and 8 infantry from the
    // Airfield (it spares 9).  The arrival rule picks the BMP column (the
    // pool pays 22); the truck with a T72 (21) also meets the rung
    f.of.IngestTip(f.registry, lePort, 4, f.Z("Le Port").pos);
    f.of.ForceRepels("Le Port", 1);
    FakeLauncher launcher;
    const DirectorAction plan = f.of.PlanFor(f.registry, ViewOf(f.registry), lePort, CAODirector);
    REQUIRE(plan.go);
    INFO(ForcePackages::PackageName(plan.choice.quote.package)
         << " " << Str(plan.choice.quote.hullClass) << " with " << Str(plan.choice.quote.escortClass) << " bodies "
         << plan.choice.quote.bodies << " lethality " << plan.choice.quote.lethality << " of " << plan.choice.lReq);
    REQUIRE_FALSE(plan.choice.shortOfLethality);
    REQUIRE(plan.choice.quote.poolPrice == 22);

    // a pool that pays it but would fall to 7: everything the room of 19
    // buys is short of 21.6, so nothing goes
    f.of.SetPool(29);
    const int tokens = f.of.Tokens();
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    INFO(log.Trail());
    CHECK(launcher.orders.empty());
    CHECK(f.of.Tokens() == tokens);
    CHECK(f.of.Cooldown("Le Port") <= 0.0f);
    REQUIRE(log.With({"OccupierForces: Le Port: escalation retake (APC (8, BMP) with BMP, the pool pays 22, lethality "
                      "22.00 of 21.60) waits, the pool keeps its reserve (pool 29 of 48, floor 9.6: the launch "
                      "would leave 7)"})
                .size() == 1);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(log.With({"waits, the pool keeps its reserve"}).size() == 1);

    // a pool short of the price (15): the plan does not shrink to the pool
    f.of.SetPool(15);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(launcher.orders.empty());
    CHECK(log.With({"waits, the pool keeps its reserve"}).size() == 1); // the same plan
    REQUIRE(f.of.ReserveWaitRows() == 1);
    CHECK(f.of.ReserveWait(0)->price == 22);

    // 31: the BMP column would leave 9, the truck and T72 (21), which meets
    // the same rung, leaves 10 and goes
    f.of.SetPool(31);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(GroundOn(launcher, lePort).size() == 1);
    const CounterAttackOrder o = GroundOn(launcher, lePort)[0];
    CHECK(o.priceUnits == 21);
    CHECK(o.package == FPTruck);
    CHECK(Str(o.escortClass) == "T72");
    CHECK(o.bodies >= 8);
    CHECK(f.of.Pool() == 10);
    CHECK(f.of.ReserveWaitRows() == 0);
    CHECK(log.With({"the reserve floor: APC (8, BMP) with BMP (the pool pays 22) would leave the pool under its "
                    "reserve (pool 31 of 48, floor 9.6); TRUCK (",
                    "with T72, which meets the plan, goes (the pool pays 21)"})
              .size() == 1);
}

TEST_CASE("OccupierForces - under the floor the plan's own cheaper qualifying package goes, with one line",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    // Arudy alone, nobody reported: the arrival rule rides (a walk from the
    // Airfield is over forcesSlowEta), the truck's pool price 4
    Abel48Setup(f, UralOnly(), "Arudy");
    const int arudy = f.I("Arudy");
    FakeLauncher launcher;
    const DirectorAction plan = f.of.PlanFor(f.registry, ViewOf(f.registry), arudy, CAODirector);
    REQUIRE(plan.go);
    REQUIRE(plan.choice.quote.package == FPTruck);
    REQUIRE(plan.choice.quote.poolPrice == 4);
    // pool 12 (THIN, above the 20 % operations floor): the truck would leave
    // 8, the foot squad that meets the same plan leaves 12
    f.of.SetPool(12);
    REQUIRE(f.of.CanOperate());
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    INFO(log.Trail());
    const std::vector<CounterAttackOrder> ops = GroundOn(launcher, arudy);
    REQUIRE(ops.size() == 1);
    CHECK(ops[0].package == FPFoot);
    CHECK(ops[0].bodies >= plan.minUsed);
    CHECK(f.of.Pool() == 12);
    CHECK(log.With({"the reserve floor: TRUCK (", "(the pool pays 4) would leave the pool under its reserve (pool 12 of "
                    "48, floor 9.6); FOOT (",
                    "which meets the plan, goes (the pool pays 0)"})
              .size() == 1);
    CHECK(log.With({"waits, the pool keeps its reserve"}).empty());
}

TEST_CASE("OccupierForces - the reserve floor's exemptions: the HQ assault and an alarm answer to a base",
          "[game][guerrilla][occupierforces]")
{
    SECTION("the final push on the HQ may dig into the reserve")
    {
        CoreLines log;
        Fixture f;
        Abel48Setup(f, EastGround());
        // only the HQ is left: the towns fall back
        f.Z("Village").owner = RString("NEUTRAL");
        f.Z("Le Port").owner = RString("NEUTRAL");
        f.Z("Arudy").owner = RString("NEUTRAL");
        DirectorView v = ViewOf(f.registry);
        v.hqEstablished = true;
        v.hqZone = f.I("Camp");
        v.hqPos = f.Z("Camp").pos;
        f.of.IngestTip(f.registry, v.hqZone, 6, v.hqPos);
        f.of.SetPool(30);
        FakeLauncher launcher;
        f.of.Direct(f.registry, v, launcher, 60.0f);
        INFO(log.Trail());
        REQUIRE(launcher.orders.size() >= 1);
        CHECK(launcher.orders[0].kind == CAKHqAssault);
        CHECK(launcher.orders[0].priceUnits > OccupierForces::ReserveRoomOf(30, 48, 0.2f));
        CHECK(f.of.Pool() < 10); // under the floor: exempt
        CHECK(log.With({"waits, the pool keeps its reserve"}).empty());
    }
    SECTION("an alarm at a town waits for the reserve, one at a base behind it is answered")
    {
        CoreLines log;
        Fixture f;
        Abel48Setup(f, UralOnly());
        ZoneRecord& town = f.Z("F0");
        town.owner = f.registry.OccupierSide();
        const int f0 = f.I("F0");
        const int outpost = f.I("Outpost");
        f.of.IngestTip(f.registry, f0, 5, town.pos); // the town first (more fighters)
        f.of.IngestTip(f.registry, outpost, 4, f.Z("Outpost").pos);
        DirectorView v = ViewOf(f.registry);
        v.alert[f0] = ASRed;
        v.alert[outpost] = ASRed;
        const DirectorAction townPlan = f.of.PlanAlarm(f.registry, v, f0);
        REQUIRE(townPlan.go);
        const int price = townPlan.choice.quote.poolPrice;
        REQUIRE(price > 0);
        // a pool the answer would take to 9 of 48
        f.of.SetPool(price + 9);
        REQUIRE_FALSE(f.of.KeepsReserve(price));
        FakeLauncher launcher;
        f.of.AnswerAlarms(f.registry, v, launcher);
        INFO(log.Trail());
        REQUIRE(launcher.orders.size() == 1);
        CHECK(launcher.orders[0].kind == CAKAlarm);
        CHECK(launcher.orders[0].target == outpost); // exempt: may dig into the reserve
        CHECK(log.With({"OccupierForces: F0: alarm answer (", "waits, the pool keeps its reserve"}).size() == 1);
        CHECK(f.of.Cooldown("F0") <= 0.0f);
        // the base's record lives; the town is weighed again and still waits, logged once
        v.alert[outpost] = 0;
        launcher.Resolve(outpost);
        f.of.AnswerAlarms(f.registry, v, launcher);
        CHECK(launcher.orders.size() == 1);
        CHECK(log.With({"waits, the pool keeps its reserve"}).size() == 1);
        // the pool refills past price + floor: the town is answered
        f.of.SetPool(price + 10);
        f.of.AnswerAlarms(f.registry, v, launcher);
        REQUIRE(launcher.orders.size() == 2);
        CHECK(launcher.orders[1].target == f0);
        CHECK(f.of.ReserveWaitRows() == 0);
    }
    SECTION("forcesPoolFloor 0 turns the rule off")
    {
        Fixture f;
        Abel48Setup(f, EastGround());
        f.of.TuningMutable().poolFloor = 0.0f;
        f.of.IngestTip(f.registry, f.I("Le Port"), 4, f.Z("Le Port").pos);
        f.of.ForceRepels("Le Port", 1);
        f.of.SetPool(28);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(GroundOn(launcher, f.I("Le Port")).size() == 1);
        CHECK(GroundOn(launcher, f.I("Le Port"))[0].priceUnits == 22); // the plan as before the rule
        CHECK(f.of.Pool() == 6);
    }
}
