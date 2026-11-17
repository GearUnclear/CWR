// Occupier forces, round 5 (video 6, tmp/of-director-video-6): the band is
// stepped once per transaction (OCCUPIER-FORCES.md 5.6, D15), the escalation
// strike's line names the pool band its rule tested (7.3), and a top-up's
// settle line says what it reinforced and where its bodies went (6.6, 7.1).
// Split out of test_occupier_forces.cpp (over 5000 lines); the helpers below
// are minimal copies of that file's (an anonymous namespace each).
//   * THE FRAME'S ONE BAND STEP.  Video 6 logged "band STRONG -> STRETCHED
//     (pool 23 of 48)" and "STRETCHED -> STRONG (pool 34 of 48)" in one
//     second at 9982 s (the repel's surge deposit, then the column's refunds)
//     and the same at 6414 s (a base revert's deposit, then the adoption's
//     refund), and at 8348 s a refill credit made the band STRONG for zero
//     time and admitted a STRONG-only push-out.  Simulate's StepBandFrame
//     holds the band for the whole frame and steps it once, on the strength
//     the frame left; a wave landing and a forced pool step through it.
//   * THE ESCALATION LINE.  "[op 6] escalation strike: Arudy repelled 2
//     time(s), the pool stays STRONG (pool 16 of 48)": the rule tested the
//     pool band (STRETCHED) and the line printed the strength band.
//   * THE TOP-UP LINE.  "[op 11] Le Port Harbour retaken: 0 stay as its
//     garrison" for a REINFORCE on a base never lost, its 2 bodies gone to
//     the pool above poolStart with no line saying so.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

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

namespace
{

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
};

// four occupier bases of 8 (R = 32) and fourteen administered towns of 4
// (K = 14, G = 56): poolStart 60, refill 37 an hour
std::string AbelLikeConfig()
{
    std::string cfg = "class CfgGuerrillaZones\n{\n    occupierForces = 1;\n    class Zones\n    {\n";
    cfg += "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; position[]={100.0, 100.0, 0.0}; "
           "};\n";
    const char* baseTypes[4] = {"AIRFIELD", "SEAPORT", "OUTPOST", "OUTPOST"};
    for (int i = 0; i < 4; i++)
    {
        cfg += "        class Base" + std::to_string(i) + " { name=\"Base" + std::to_string(i) + "\"; type=\"" +
               baseTypes[i] + "\"; owner=\"OCCUPIER\"; garrison=8; position[]={" + std::to_string(2000 + 1000 * i) +
               ".0, 8000.0, 0.0}; };\n";
    }
    for (int i = 0; i < 14; i++)
    {
        cfg += "        class T" + std::to_string(i) + " { name=\"T" + std::to_string(i) +
               "\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; position[]={" + std::to_string(1000 + 500 * i) +
               ".0, 3000.0, 0.0}; };\n";
    }
    cfg += "    };\n};\n";
    return cfg;
}

// video 5's Malden, as test_occupier_forces.cpp's Video5Config: the Harbour
// by Le Port, the Outpost by Arudy, the Airfield north; poolStart 60
std::string Video5Config()
{
    std::string cfg = "class CfgGuerrillaZones\n{\n    occupierForces = 1;\n    forcesPoolMin = 60;\n"
                      "    class Zones\n    {\n";
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

ForcePackageCatalogue TruckCatalogue()
{
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Truck5t";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    return cat;
}

// the EAST ladder of Guerrilla.Abel: Ural, BMP, T72, Mi17, Su25
ForcePackageCatalogue EastLadder()
{
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Ural";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    cat.hulls[FPTransportHeli].className = "Mi17";
    cat.hulls[FPTransportHeli].maxSpeedKmh = 240;
    cat.hulls[FPTransportHeli].cargo = 12;
    cat.hulls[FPTransportHeli].crew = 2;
    cat.hulls[FPApc].className = "BMP";
    cat.hulls[FPApc].maxSpeedKmh = 65;
    cat.hulls[FPApc].cargo = 8;
    cat.hulls[FPApc].crew = 3;
    cat.hulls[FPTank].className = "T72";
    cat.hulls[FPTank].maxSpeedKmh = 60;
    cat.hulls[FPTank].cargo = 0;
    cat.hulls[FPTank].crew = 3;
    cat.hulls[FPJet].className = "Su25";
    cat.hulls[FPJet].maxSpeedKmh = 1000;
    cat.hulls[FPJet].cargo = 0;
    cat.hulls[FPJet].crew = 1;
    return cat;
}

// CounterAttack::LaunchFunded stand-in: records the orders, a record per target
struct FakeLauncher final : DirectorLauncher
{
    std::vector<CounterAttackOrder> orders;
    std::vector<int> records;
    bool Launch(const CounterAttackOrder& order) override
    {
        orders.push_back(order);
        records.push_back(order.target);
        return true;
    }
    int BornOps() const override { return 0; }
    bool HasRecord(int zoneIndex) const override
    {
        return std::find(records.begin(), records.end(), zoneIndex) != records.end();
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

const CounterAttackOrder* FirstKind(const FakeLauncher& l, int kind)
{
    for (const CounterAttackOrder& o : l.orders)
    {
        if (o.kind == kind)
        {
            return &o;
        }
    }
    return nullptr;
}

size_t BandLines(const CoreLines& log)
{
    return log.With({"OccupierForces: band "}).size();
}

} // namespace

TEST_CASE("OccupierForces - a booking that debits and credits within one frame steps and logs the band at most once",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 60);
    f.of.SetPool(40);
    REQUIRE(f.of.Band() == OFBandStrong); // STRONG is left only under 36

    SECTION("the bare ledger (no frame hold) steps at each seam: the video's false pair")
    {
        const size_t before = BandLines(log);
        REQUIRE_FALSE(f.of.FrameHoldOpen());
        CHECK(f.of.Deposit(11, "base retaken") == 11);
        f.of.Refund(f.registry, 11, "adopted squad");
        CHECK(f.of.Pool() == 40);
        CHECK(BandLines(log) == before + 2); // STRONG -> STRETCHED -> STRONG
    }

    SECTION("in a frame the intermediate states neither log nor gate")
    {
        f.of.StepBandFrame();
        REQUIRE(f.of.FrameHoldOpen());
        const size_t before = BandLines(log);
        // 6414 s: the revert's pool-funded deposit, then the adoption refund
        CHECK(f.of.Deposit(11, "base retaken") == 11);
        CHECK(f.of.Pool() == 29);
        CHECK(f.of.Band() == OFBandStrong);       // every gate this frame reads one band
        CHECK(f.of.BandNow() == OFBandStretched); // the step the frame would make now
        f.of.Refund(f.registry, 11, "adopted squad");
        // 9982 s: the repel's surge deposit, then the column home in pieces
        CHECK(f.of.Deposit(12, "surge") == 12);
        CHECK(f.of.Band() == OFBandStrong);
        f.of.Refund(f.registry, 9, "withdrawing soldiers home");
        f.of.Refund(f.registry, 3, "hull home intact");
        CHECK(f.of.Pool() == 40);
        f.of.StepBandFrame(); // the frame's one step: the strength is back
        CHECK(f.of.Band() == OFBandStrong);
        CHECK(BandLines(log) == before);

        // a genuine net movement inside one frame: one line, on the final
        // strength, never the intermediate pool
        REQUIRE(f.of.Spend(15, "operation hull and crew"));
        f.of.Refund(f.registry, 3, "escort refused");
        CHECK(f.of.Band() == OFBandStrong);
        f.of.StepBandFrame();
        CHECK(f.of.Band() == OFBandStretched);
        const std::vector<std::string> lines = log.With({"OccupierForces: band "});
        REQUIRE(lines.size() == before + 1);
        CHECK(lines.back().find("band STRONG -> STRETCHED (pool 28 of 60; strength 28") != std::string::npos);
    }
}

TEST_CASE("OccupierForces - the director tick is one transaction: a refill credit and a launch debit never gate or log",
          "[game][guerrilla][occupierforces]")
{
    // video 6 at 8348 s: the tick's refill took the pool from 31 to 32 of 48
    // (STRONG enters over 66 %), the push-out it admitted took it to 28, and
    // the band was STRONG for zero time.  Here: 39 of 60 STRETCHED, a refill
    // unit makes 40 (over 39.6), a launch debit takes it to 35
    CoreLines log;
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(30);
    f.of.SetPool(39);
    REQUIRE(f.of.Band() == OFBandStretched);
    f.of.StepBandFrame();
    const size_t before = BandLines(log);
    f.of.Tick(f.registry, 100.0f); // 37 an hour: one unit
    REQUIRE(f.of.Pool() == 40);
    CHECK(f.of.Band() == OFBandStretched); // the tick reads the frame's band
    CHECK(f.of.OpsMax() == OccupierForces::OpsMaxFor(f.of.Tuning(), OFBandStretched));
    CHECK(f.of.BandNow() == OFBandStrong);
    REQUIRE(f.of.Spend(5, "operation hull and crew"));
    f.of.StepBandFrame();
    CHECK(f.of.Band() == OFBandStretched);
    CHECK(BandLines(log) == before);

    // the refill alone: the next frame steps it, once
    f.of.Tick(f.registry, 600.0f); // 6 units: 41
    CHECK(f.of.Band() == OFBandStretched);
    f.of.StepBandFrame();
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(BandLines(log) == before + 1);
}

TEST_CASE("OccupierForces - a wave landing and a forced pool step through the frame hold, not through a BandHold",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(0);
    REQUIRE(f.of.Band() == OFBandSpent);
    f.of.StepBandFrame();
    // the allocation after a landing spends the landed bodies in this tick:
    // a SPENT band would refuse the hub top-up and every wave strike
    f.of.LandWave(30);
    CHECK(f.of.Band() == OFBandStretched);
    CHECK(f.of.Deposit(2, "hub top-up") == 2);
    {
        // an explicit BandHold still defers even a forced pool
        OccupierForces::BandHold hold(f.of);
        f.of.SetPool(50);
        CHECK(f.of.Band() == OFBandStretched);
    }
    CHECK(f.of.Band() == OFBandStretched); // the frame hold is still open
    f.of.StepBandFrame();
    CHECK(f.of.Band() == OFBandStrong);
    // a forced pool outside any explicit hold is read at once
    f.of.SetPool(10);
    CHECK(f.of.Band() == OFBandThin);
}

TEST_CASE("OccupierForces - the escalation strike's line names the pool band its rule tested",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    f.Load(Video5Config());
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 60);
    f.of.SetCatalogue(EastLadder());
    FlipTown(f, "Village", 1);
    REQUIRE(FlipTown(f, "Le Port", 2));
    FlipTown(f, "Arudy", 3);
    f.of.ForceCooldown("Village", 1.0e7f);
    f.of.ForceCooldown("Arudy", 1.0e7f);
    f.of.IngestTip(f.registry, f.registry.FindZoneIndex("Le Port"), 4, f.Z("Le Port").pos);
    // urgency kept out: the escalation strike alone decides (7.3)
    f.of.TuningMutable().urgentHeat = 100;
    f.of.ForceRepels("Le Port", 2);
    // a pool where the airframe leaves the POOL STRETCHED while the strength
    // (the airframe committed, 5.6) stays STRONG: the video's shape
    f.of.SetPool(66); // the APC with its BMP escort pays 22, the Su25 23: 21 left
    REQUIRE(f.of.Band() == OFBandStrong);
    FakeLauncher launcher;
    for (int tick = 0; tick < 6 && !FirstKind(launcher, CAKStrike); tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f); // massing may hold a few ticks
    }
    const CounterAttackOrder* strike = FirstKind(launcher, CAKStrike);
    std::string trail;
    for (const std::string& l : log.lines)
    {
        if (l.find("OccupierForces") != std::string::npos)
        {
            trail += l + " | ";
        }
    }
    INFO(trail);
    REQUIRE(strike != nullptr);
    const int pool = f.of.Pool();
    INFO("pool " << pool << ", strength " << f.of.BandStrength());
    // the rule's quantity: the pool after both debits, STRETCHED
    REQUIRE(OccupierForces::StepBand(OFBandStrong, pool, 60) == OFBandStretched);
    // the band's own input still reads STRONG (36 of 60 or more)
    CHECK(f.of.BandStrength() >= 36);
    const std::string expect = "the pool stays STRETCHED (pool " + std::to_string(pool) + " of 60)";
    CHECK(log.With({"escalation strike: Le Port repelled 2 time(s)", expect.c_str()}).size() == 1);
    CHECK(log.With({"escalation strike:", "the pool stays STRONG"}).empty());
}

TEST_CASE("OccupierForces - a REINFORCE settle says what it reinforced and where its bodies went, never retaken",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    REQUIRE(FlipTown(f, "T13", 1) == false);
    REQUIRE(FlipTown(f, "T12", 2)); // AWAKE
    f.Z("Base3").owner = RString("GUER");
    f.Z("Base3").heat = 100;
    f.Z("Base2").garrison = 3; // bled: the wave's reinforce column goes there
    f.Z("Base0").garrison = 5;
    f.of.TuningMutable().waveLead = 0;
    DirectorView v = ViewOf(f.registry);
    f.of.RequestWave(f.registry, v, 1);
    f.of.Direct(f.registry, v, launcher, 60.0f); // lands now
    const CounterAttackOrder* column = FirstKind(launcher, CAKReinforce);
    REQUIRE(column != nullptr);
    const int base2 = f.registry.FindZoneIndex("Base2");
    const int base0 = f.registry.FindZoneIndex("Base0");
    REQUIRE(column->target == base2);
    const std::string id = "[op " + std::to_string(column->opId) + "]";

    // CounterAttack's abstract top-up (CounterAttack.cpp, the reinforce
    // arrival): Base2 has no room, the column's return finds none at the hub
    // either, and the surplus goes to the pool (5.5), then the settle
    const int bodies = column->bodies;
    REQUIRE(bodies > 0);
    f.of.Refund(f.registry, bodies, "surplus", base0);
    CounterAttackOpEvent ev;
    ev.opId = column->opId;
    ev.kind = CAKReinforce;
    ev.origin = CAOWave;
    ev.target = base2;
    ev.source = base0;
    ev.type = CAOpReturned;
    ev.bodies = 0;
    ev.units = bodies;
    ev.note = "home";
    f.of.NoteOpEvent(f.registry, ev);
    ev.type = CAOpSettled;
    ev.bodies = 0;
    ev.units = 0;
    ev.note = "topped up";
    f.of.NoteOpEvent(f.registry, ev);

    CHECK(log.With({id.c_str(), "retaken"}).empty());
    const std::string what = id + " Base2 reinforced: 0 joined its garrison; 0 back at Base0, " +
                             std::to_string(bodies) + " to the pool (pool " + std::to_string(f.of.Pool()) + " of 60";
    const std::vector<std::string> lines = log.With({what.c_str()});
    REQUIRE(lines.size() == 1);
    // above poolStart the pool is the wave headroom (5.5): the line says so
    if (f.of.Pool() > 60)
    {
        CHECK(lines[0].find("over poolStart in the wave headroom") != std::string::npos);
    }

    // a live top-up (no return event): the joined bodies alone
    CounterAttackOpEvent live = ev;
    live.opId = column->opId;
    // a second REINFORCE row is not needed: the line is per event, the row
    // was finished and pruned only once nothing is outstanding
    if (f.of.FindOp(column->opId) >= 0)
    {
        live.bodies = 3;
        f.of.NoteOpEvent(f.registry, live);
        CHECK(log.With({id.c_str(), "Base2 reinforced: 3 joined its garrison (pool"}).size() == 1);
    }
}
