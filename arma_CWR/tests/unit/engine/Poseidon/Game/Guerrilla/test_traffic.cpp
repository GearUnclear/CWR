#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp>            // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp> // ASYellow / ASRed
#include <Poseidon/Game/Guerrilla/Traffic.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <math.h>
#include <string.h>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// keeps the parsed ParamFile alive for the duration of a test
struct TrafficFixture
{
    ParamFile file;
    Traffic traffic;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        traffic.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
    }
};

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

TrafficZoneCandidate Zone(int index, float x, float z, bool city, bool occ)
{
    TrafficZoneCandidate c;
    c.index = index;
    c.x = x;
    c.z = z;
    c.isCity = city;
    c.occupierOwned = occ;
    return c;
}

TrafficDecisionInput Input(bool playerValid, int civ, int patrols, int convoys, bool civRoute, bool patrolRoute,
                           bool convoyRoute, float roll, float warLevel = 1.0f)
{
    TrafficDecisionInput in;
    in.enabled = true;
    in.playerValid = playerValid;
    in.liveCiv = civ;
    in.livePatrols = patrols;
    in.liveConvoys = convoys;
    in.hasCivRoute = civRoute;
    in.hasPatrolRoute = patrolRoute;
    in.hasConvoyRoute = convoyRoute;
    in.roll = roll;
    in.warLevel = warLevel;
    return in;
}

CommandeerObs Obs(float carX, float carZ, float carDirX, float carDirZ, float px, float pz, float pdx, float pdz,
                  bool armed)
{
    CommandeerObs o;
    o.carPos = Vector3(carX, 0, carZ);
    o.carDir = Vector3(carDirX, 0, carDirZ);
    o.playerPos = Vector3(px, 0, pz);
    o.playerDir = Vector3(pdx, 0, pdz);
    o.weaponInHands = armed;
    return o;
}

} // namespace

TEST_CASE("Traffic - config parse of the traffic* keys", "[game][guerrilla]")
{
    SECTION("absent config keeps defaults")
    {
        Traffic t;
        t.LoadFromParams(nullptr);
        const TrafficTuning& tu = t.Tuning();
        REQUIRE(tu.enabled);
        REQUIRE(tu.interval == Approx(5.0f));
        REQUIRE(tu.radius == Approx(1500.0f));
        REQUIRE(tu.minSpawnDist == Approx(300.0f));
        REQUIRE(tu.despawnHysteresis == Approx(300.0f));
        REQUIRE(tu.maxCiv == 3);
        REQUIRE(tu.maxPatrols == 1);
        REQUIRE(tu.maxConvoys == 1);
        REQUIRE(tu.civChance == Approx(0.5f));
        REQUIRE(tu.patrolChance == Approx(0.25f));
        REQUIRE(tu.convoyChance == Approx(0.04f));
        REQUIRE(tu.convoyWarScale == Approx(0.5f));
        REQUIRE(tu.civNightScale == Approx(0.1f));
        REQUIRE(tu.dayStart == Approx(6.0f / 24.0f));
        REQUIRE(tu.dayEnd == Approx(21.0f / 24.0f));
        REQUIRE(tu.alertPatrolBoost == Approx(0.5f));
        REQUIRE(tu.curfewWarLevel == Approx(3.0f));
        REQUIRE(tu.curfewPatrolBoost == Approx(2.0f));
        REQUIRE(tu.rainCivFade == Approx(0.6f));
        REQUIRE(tu.stallTimeout == Approx(90.0f));
        REQUIRE(tu.arriveRadius == Approx(60.0f));
        REQUIRE(tu.maxLegs == 3);
        REQUIRE(tu.commandeerRadius == Approx(25.0f));
        REQUIRE(tu.commandeerLaneHalfWidth == Approx(4.0f));
        REQUIRE(tu.commandeerStopDelay == Approx(2.5f));
        REQUIRE(tu.fleeDist == Approx(150.0f));
        REQUIRE(tu.parkChance == Approx(0.6f));
        REQUIRE(tu.parkDwellMin == Approx(60.0f));
        REQUIRE(tu.parkDwellMax == Approx(180.0f));
        REQUIRE(tu.exposeMargin == Approx(150.0f));
        REQUIRE(tu.despawnDeferMax == Approx(90.0f));
        REQUIRE(tu.combatStaleAfter == Approx(120.0f));
        REQUIRE(tu.combatHoldMax == Approx(300.0f));
        REQUIRE(tu.bailCombatWindow == Approx(60.0f));
        REQUIRE_FALSE(tu.scaleCaps);
        REQUIRE(tu.dangerRadius == Approx(200.0f));
        REQUIRE(tu.dangerCloseRadius == Approx(60.0f));
        REQUIRE(tu.dangerCooldown == Approx(45.0f));
        REQUIRE(tu.dangerTtl == Approx(20.0f));
        REQUIRE(tu.maxParked == 2);
        REQUIRE(tu.wreckClearAfter == Approx(1200.0f));
        REQUIRE(t.ConfigWarnings().Size() == 0);
    }

    SECTION("explicit census / recovery keys override, a clean config repairs nothing")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficMaxParked = 5; trafficWreckClearAfter = 600; };\n");
        REQUIRE(f.traffic.Tuning().maxParked == 5);
        REQUIRE(f.traffic.Tuning().wreckClearAfter == Approx(600.0f));
        REQUIRE(f.traffic.ConfigWarnings().Size() == 0);
    }

    SECTION("explicit danger keys override")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDangerRadius = 400; trafficDangerCloseRadius = 100; "
               "trafficDangerCooldown = 10; trafficDangerTtl = 5; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.dangerRadius == Approx(400.0f));
        REQUIRE(tu.dangerCloseRadius == Approx(100.0f));
        REQUIRE(tu.dangerCooldown == Approx(10.0f));
        REQUIRE(tu.dangerTtl == Approx(5.0f));
    }

    SECTION("danger sanity floors and the close-inside-main clamp")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDangerRadius = -10; trafficDangerCloseRadius = -5; "
               "trafficDangerCooldown = -1; trafficDangerTtl = -1; };\n");
        REQUIRE(f.traffic.Tuning().dangerRadius == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().dangerCloseRadius == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().dangerCooldown == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().dangerTtl == Approx(0.0f));
        TrafficFixture g;
        g.Load("class CfgGuerrillaZones { trafficDangerRadius = 100; trafficDangerCloseRadius = 150; };\n");
        REQUIRE(g.traffic.Tuning().dangerCloseRadius == Approx(100.0f)); // pinned to the main radius
    }

    SECTION("keys absent from CfgGuerrillaZones keep defaults")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { tickInterval = 3; cacheInterval = 8; };\n");
        REQUIRE(f.traffic.Tuning().interval == Approx(5.0f));
        REQUIRE(f.traffic.Tuning().maxCiv == 3);
        REQUIRE(f.traffic.Tuning().enabled);
    }

    SECTION("explicit keys override, disabled flag parses")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficEnabled = 0; trafficInterval = 7; trafficRadius = 900; "
               "trafficMaxCiv = 5; trafficMaxPatrols = 2; trafficConvoyChance = 0.1; trafficMaxLegs = 1; "
               "trafficCommandeerRadius = 30; trafficFleeDist = 200; trafficCivNightScale = 0.2; "
               "trafficDayStart = 0.3; trafficDayEnd = 0.8; trafficAlertPatrolBoost = 1.0; "
               "trafficCurfewWarLevel = 2; trafficCurfewPatrolBoost = 3; trafficRainCivFade = 0.9; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE_FALSE(tu.enabled);
        REQUIRE(tu.interval == Approx(7.0f));
        REQUIRE(tu.radius == Approx(900.0f));
        REQUIRE(tu.maxCiv == 5);
        REQUIRE(tu.maxPatrols == 2);
        REQUIRE(tu.convoyChance == Approx(0.1f));
        REQUIRE(tu.maxLegs == 1);
        REQUIRE(tu.commandeerRadius == Approx(30.0f));
        REQUIRE(tu.fleeDist == Approx(200.0f));
        REQUIRE(tu.civNightScale == Approx(0.2f));
        REQUIRE(tu.dayStart == Approx(0.3f));
        REQUIRE(tu.dayEnd == Approx(0.8f));
        REQUIRE(tu.alertPatrolBoost == Approx(1.0f));
        REQUIRE(tu.curfewWarLevel == Approx(2.0f));
        REQUIRE(tu.curfewPatrolBoost == Approx(3.0f));
        REQUIRE(tu.rainCivFade == Approx(0.9f));
        REQUIRE_FALSE(f.traffic.IsActive()); // disabled regardless of the registry
    }

    SECTION("explicit park keys override")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficParkChance = 0.25; trafficParkDwellMin = 10; "
               "trafficParkDwellMax = 20; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.parkChance == Approx(0.25f));
        REQUIRE(tu.parkDwellMin == Approx(10.0f));
        REQUIRE(tu.parkDwellMax == Approx(20.0f));
    }

    SECTION("explicit perception keys override")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficExposeMargin = 50; trafficDespawnDeferMax = 30; "
               "trafficScaleCaps = 1; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.exposeMargin == Approx(50.0f));
        REQUIRE(tu.despawnDeferMax == Approx(30.0f));
        REQUIRE(tu.scaleCaps);
    }

    SECTION("explicit convoy discipline keys override")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficCombatStaleAfter = 45; trafficCombatHoldMax = 90; "
               "trafficBailCombatWindow = 15; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.combatStaleAfter == Approx(45.0f));
        REQUIRE(tu.combatHoldMax == Approx(90.0f));
        REQUIRE(tu.bailCombatWindow == Approx(15.0f));
    }

    SECTION("convoy discipline sanity floors")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficCombatStaleAfter = -30; trafficCombatHoldMax = -1; "
               "trafficBailCombatWindow = -0.5; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        // negatives floor to 0: a 0 hold budget disables the gate, a 0 window
        // disables the bail, neither is allowed to read as "time runs backwards"
        REQUIRE(tu.combatStaleAfter == Approx(0.0f));
        REQUIRE(tu.combatHoldMax == Approx(0.0f));
        REQUIRE(tu.bailCombatWindow == Approx(0.0f));
    }

    SECTION("perception sanity floors")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficExposeMargin = -5; trafficDespawnDeferMax = -1; };\n");
        REQUIRE(f.traffic.Tuning().exposeMargin == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().despawnDeferMax == Approx(0.0f));
    }

    SECTION("sanity floors")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficInterval = 0; trafficMaxCiv = -2; trafficMaxLegs = -1; };\n");
        REQUIRE(f.traffic.Tuning().interval == Approx(0.5f));
        REQUIRE(f.traffic.Tuning().maxCiv == 0);
        REQUIRE(f.traffic.Tuning().maxLegs == 0);
    }

    SECTION("park sanity clamps")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficParkChance = 1.5; trafficParkDwellMin = 120; "
               "trafficParkDwellMax = 30; };\n");
        REQUIRE(f.traffic.Tuning().parkChance == Approx(1.0f));
        REQUIRE(f.traffic.Tuning().parkDwellMin == Approx(120.0f));
        REQUIRE(f.traffic.Tuning().parkDwellMax == Approx(120.0f)); // pinned to the min
        TrafficFixture g;
        g.Load("class CfgGuerrillaZones { trafficParkChance = -0.5; trafficParkDwellMin = -10; };\n");
        REQUIRE(g.traffic.Tuning().parkChance == Approx(0.0f));
        REQUIRE(g.traffic.Tuning().parkDwellMin == Approx(0.0f));
        REQUIRE(g.traffic.Tuning().parkDwellMax == Approx(180.0f));
    }
}

TEST_CASE("Traffic - park decision", "[game][guerrilla]")
{
    TrafficTuning def; // parkChance 0.6
    REQUIRE(Traffic::DecidePark(0.59f, def));
    REQUIRE_FALSE(Traffic::DecidePark(0.6f, def));
    TrafficTuning never;
    never.parkChance = 0.0f;
    REQUIRE_FALSE(Traffic::DecidePark(0.0f, never));
    TrafficTuning always;
    always.parkChance = 1.0f;
    REQUIRE(Traffic::DecidePark(0.999f, always));
}

TEST_CASE("Traffic - loaded park-state policy", "[game][guerrilla][save][load]")
{
    // seated driver: brake wait restarts / degenerate dweller re-parks /
    // departer resumes
    REQUIRE(Traffic::LoadedParkState(TSParking, true) == TSParking);
    REQUIRE(Traffic::LoadedParkState(TSDwelling, true) == TSParking);
    REQUIRE(Traffic::LoadedParkState(TSDeparting, true) == TSDeparting);
    // on-foot driver: everything re-dwells (get-in flags re-issued at expiry)
    REQUIRE(Traffic::LoadedParkState(TSParking, false) == TSDwelling);
    REQUIRE(Traffic::LoadedParkState(TSDwelling, false) == TSDwelling);
    REQUIRE(Traffic::LoadedParkState(TSDeparting, false) == TSDwelling);
    // non-park states pass through untouched
    REQUIRE(Traffic::LoadedParkState(TSDriving, true) == TSDriving);
    REQUIRE(Traffic::LoadedParkState(TSDriving, false) == TSDriving);
    REQUIRE(Traffic::LoadedParkState(TSStalled, true) == TSStalled);
    REQUIRE(Traffic::LoadedParkState(TSStalled, false) == TSStalled);
    // lingering: seated is the state's invariant, an on-foot driver is a
    // degenerate row and restarts driving (the shared guards reconcile it)
    REQUIRE(Traffic::LoadedParkState(TSLingering, true) == TSLingering);
    REQUIRE(Traffic::LoadedParkState(TSLingering, false) == TSDriving);
}

TEST_CASE("Traffic - observed trip endings", "[game][guerrilla]")
{
    SECTION("arrival: unobserved despawns, whatever the legs say")
    {
        REQUIRE(Traffic::ArrivedEndAction(true, 0, 3) == TEndDespawn);
        REQUIRE(Traffic::ArrivedEndAction(true, 3, 3) == TEndDespawn);
    }
    SECTION("arrival observed: re-leg while legs remain, then linger")
    {
        REQUIRE(Traffic::ArrivedEndAction(false, 0, 3) == TEndReLeg);
        REQUIRE(Traffic::ArrivedEndAction(false, 2, 3) == TEndReLeg);
        REQUIRE(Traffic::ArrivedEndAction(false, 3, 3) == TEndLinger);
        // maxLegs 0 = no continuation: straight to the linger ending
        REQUIRE(Traffic::ArrivedEndAction(false, 0, 0) == TEndLinger);
    }
    SECTION("stall: unobserved despawns for every kind")
    {
        REQUIRE(Traffic::StalledEndAction(true, TKCiv) == TEndDespawn);
        REQUIRE(Traffic::StalledEndAction(true, TKPatrol) == TEndDespawn);
        REQUIRE(Traffic::StalledEndAction(true, TKConvoy) == TEndDespawn);
    }
    SECTION("stall observed: a civ driver walks off, crews stay seated")
    {
        REQUIRE(Traffic::StalledEndAction(false, TKCiv) == TEndAbandon);
        REQUIRE(Traffic::StalledEndAction(false, TKPatrol) == TEndLinger);
        REQUIRE(Traffic::StalledEndAction(false, TKConvoy) == TEndLinger);
    }
}

TEST_CASE("Traffic - state enum stays append-only for save compat", "[game][guerrilla][save][load]")
{
    // saved entries store the state as a plain int: appending is the only
    // legal way to grow this enum (911b724 added the park states, #53 adds
    // the lingering ending, the danger response adds the panic cower)
    REQUIRE((int)TSParking == 5);
    REQUIRE((int)TSDwelling == 6);
    REQUIRE((int)TSDeparting == 7);
    REQUIRE((int)TSLingering == 8);
    REQUIRE((int)TSPanicked == 9);
    REQUIRE((int)NTrafficStates == 10);
}

TEST_CASE("Traffic - spawn decision: caps, chances, disabled", "[game][guerrilla]")
{
    TrafficTuning t;

    SECTION("no player / disabled -> nothing")
    {
        REQUIRE(Traffic::DecideSpawn(Input(false, 0, 0, 0, true, true, true, 0.0f), t) == -1);
        TrafficDecisionInput in = Input(true, 0, 0, 0, true, true, true, 0.0f);
        in.enabled = false;
        REQUIRE(Traffic::DecideSpawn(in, t) == -1);
        TrafficTuning off = t;
        off.enabled = false;
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.0f), off) == -1);
    }

    SECTION("rarest first: low roll is a convoy, then the patrol band, then civ")
    {
        // bands at warLevel 1: convoy [0, 0.04), patrol [0.04, 0.29), civ [0.29, 0.79)
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.01f), t) == TKConvoy);
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.10f), t) == TKPatrol);
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.50f), t) == TKCiv);
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.90f), t) == -1);
    }

    SECTION("a full cap or a missing route skips the kind without eating its band")
    {
        // convoy cap reached: the 0.01 roll now falls in the patrol band
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 1, true, true, true, 0.01f), t) == TKPatrol);
        // no patrol route either: civ
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 1, true, false, true, 0.01f), t) == TKCiv);
        // civ cap reached too: nothing
        REQUIRE(Traffic::DecideSpawn(Input(true, 3, 0, 1, true, false, true, 0.01f), t) == -1);
        // no routes at all
        REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, false, false, false, 0.0f), t) == -1);
    }

    SECTION("civ cap boundary")
    {
        REQUIRE(Traffic::DecideSpawn(Input(true, 2, 1, 1, true, false, false, 0.1f), t) == TKCiv);
        REQUIRE(Traffic::DecideSpawn(Input(true, 3, 1, 1, true, false, false, 0.1f), t) == -1);
    }
}

TEST_CASE("Traffic - convoy chance grows with war level and clamps", "[game][guerrilla]")
{
    TrafficTuning t;
    float c1 = Traffic::ConvoyChance(t, 1.0f);
    float c3 = Traffic::ConvoyChance(t, 3.0f);
    float c6 = Traffic::ConvoyChance(t, 6.0f);
    REQUIRE(c1 == Approx(0.04f));
    REQUIRE(c3 == Approx(0.04f * 2.0f));
    REQUIRE(c3 > c1);
    REQUIRE(c6 >= c3);
    // below war level 1 never goes negative / below base
    REQUIRE(Traffic::ConvoyChance(t, 0.0f) == Approx(0.04f));
    // clamp
    TrafficTuning hot;
    hot.convoyChance = 0.2f;
    hot.convoyWarScale = 2.0f;
    REQUIRE(Traffic::ConvoyChance(hot, 10.0f) == Approx(Traffic::ConvoyChanceCap));
    hot.convoyChance = -1.0f;
    REQUIRE(Traffic::ConvoyChance(hot, 1.0f) == Approx(0.0f));
}

TEST_CASE("Traffic - modulation: neutral defaults change nothing", "[game][guerrilla]")
{
    TrafficTuning t;
    float civ = -1, pat = -1;
    Traffic::ModulationFactors(TrafficModulationInput(), t, civ, pat);
    REQUIRE(civ == Approx(1.0f));
    REQUIRE(pat == Approx(1.0f));
    // and neutral scales reproduce the unmodulated bands exactly
    REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.01f), t) == TKConvoy);
    REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.10f), t) == TKPatrol);
    REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.50f), t) == TKCiv);
    REQUIRE(Traffic::DecideSpawn(Input(true, 0, 0, 0, true, true, true, 0.90f), t) == -1);
}

TEST_CASE("Traffic - modulation: time-of-day trapezoid", "[game][guerrilla]")
{
    TrafficTuning t; // day window [6h, 21h], 2 h ramps, night scale 0.1
    float civ, pat;
    TrafficModulationInput in;

    SECTION("deep night and the window edges are the night scale")
    {
        in.dayFraction = 2.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.1f));
        REQUIRE(pat == Approx(1.0f)); // patrols do not follow the clock
        in.dayFraction = 6.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.1f));
        in.dayFraction = 21.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.1f));
        in.dayFraction = 23.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.1f));
    }
    SECTION("ramps rise and fall linearly over 2 h")
    {
        in.dayFraction = 7.0f / 24.0f; // halfway up the morning ramp
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.55f));
        in.dayFraction = 20.0f / 24.0f; // halfway down the evening ramp
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.55f));
    }
    SECTION("plateau between the ramps is 1")
    {
        in.dayFraction = 8.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        in.dayFraction = 12.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        in.dayFraction = 19.0f / 24.0f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
    }
}

TEST_CASE("Traffic - modulation: alert on the civ origin", "[game][guerrilla]")
{
    TrafficTuning t;
    float civ, pat;
    TrafficModulationInput in;

    SECTION("YELLOW thins the civs and boosts the patrols")
    {
        in.originAlertCiv = ASYellow;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(Traffic::AlertYellowCivScale));
        REQUIRE(pat == Approx(1.5f));
    }
    SECTION("RED empties the roads")
    {
        in.originAlertCiv = ASRed;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.0f));
        REQUIRE(pat == Approx(1.5f));
    }
}

TEST_CASE("Traffic - modulation: curfew predicate", "[game][guerrilla]")
{
    TrafficTuning t;
    float civ, pat;
    TrafficModulationInput in; // noon on the wall clock: curfew keys on darkness
    in.warLevel = 3.0f;
    in.nightEffect = 1.0f;
    in.originOccupied = true;

    SECTION("war >= 3 + dark + occupied origin zeroes civ, doubles patrols")
    {
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.0f));
        REQUIRE(pat == Approx(2.0f));
    }
    SECTION("any failed leg leaves the curfew off")
    {
        TrafficModulationInput low = in;
        low.warLevel = 2.9f;
        Traffic::ModulationFactors(low, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        REQUIRE(pat == Approx(1.0f));
        TrafficModulationInput lit = in;
        lit.nightEffect = 0.5f; // the darkness gate is strictly > 0.5
        Traffic::ModulationFactors(lit, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        REQUIRE(pat == Approx(1.0f));
        TrafficModulationInput free = in;
        free.originOccupied = false;
        Traffic::ModulationFactors(free, t, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        REQUIRE(pat == Approx(1.0f));
    }
}

TEST_CASE("Traffic - modulation: rain fades the civs", "[game][guerrilla]")
{
    TrafficTuning t;
    float civ, pat;
    TrafficModulationInput in;
    in.rain = 1.0f;
    Traffic::ModulationFactors(in, t, civ, pat);
    REQUIRE(civ == Approx(0.4f)); // 1 - 0.6
    REQUIRE(pat == Approx(1.0f));
    in.rain = 0.5f;
    Traffic::ModulationFactors(in, t, civ, pat);
    REQUIRE(civ == Approx(0.7f));
}

TEST_CASE("Traffic - modulation: multiplicative composition and clamps", "[game][guerrilla]")
{
    TrafficTuning t;
    float civ, pat;

    SECTION("dusk x YELLOW x rain compose multiplicatively")
    {
        TrafficModulationInput in;
        in.dayFraction = 20.0f / 24.0f; // 0.55 on the evening ramp
        in.originAlertCiv = ASYellow;
        in.rain = 0.5f;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.55f * 0.4f * 0.7f));
        REQUIRE(pat == Approx(1.5f));
    }
    SECTION("curfew stacks on the alert boost")
    {
        TrafficModulationInput in;
        in.originAlertCiv = ASRed;
        in.warLevel = 5.0f;
        in.nightEffect = 1.0f;
        in.originOccupied = true;
        Traffic::ModulationFactors(in, t, civ, pat);
        REQUIRE(civ == Approx(0.0f));
        REQUIRE(pat == Approx(1.5f * 2.0f));
    }
    SECTION("silly tuning never leaves civ outside [0,1] or patrol negative")
    {
        TrafficTuning weird;
        weird.civNightScale = 5.0f;
        TrafficModulationInput night;
        night.dayFraction = 0.0f;
        Traffic::ModulationFactors(night, weird, civ, pat);
        REQUIRE(civ == Approx(1.0f));
        weird.rainCivFade = 3.0f;
        night.rain = 1.0f;
        Traffic::ModulationFactors(night, weird, civ, pat);
        REQUIRE(civ == Approx(0.0f));
        weird.curfewPatrolBoost = -4.0f;
        night.warLevel = 9.0f;
        night.nightEffect = 1.0f;
        night.originOccupied = true;
        Traffic::ModulationFactors(night, weird, civ, pat);
        REQUIRE(pat == Approx(0.0f));
    }
}

TEST_CASE("Traffic - spawn decision applies the modulation scales", "[game][guerrilla]")
{
    TrafficTuning t;

    SECTION("civScale 0 removes the civ band entirely")
    {
        TrafficDecisionInput in = Input(true, 0, 0, 0, true, false, false, 0.0f);
        in.civScale = 0.0f;
        REQUIRE(Traffic::DecideSpawn(in, t) == -1);
    }
    SECTION("a scaled patrol band shifts the civ band start exactly")
    {
        // bands: convoy [0, 0.04), patrol [0.04, 0.415), civ [0.415, 0.915)
        TrafficDecisionInput in = Input(true, 0, 0, 0, true, true, true, 0.40f);
        in.patrolScale = 1.5f;
        REQUIRE(Traffic::DecideSpawn(in, t) == TKPatrol);
        in.roll = 0.42f;
        REQUIRE(Traffic::DecideSpawn(in, t) == TKCiv);
        in.roll = 0.92f;
        REQUIRE(Traffic::DecideSpawn(in, t) == -1);
    }
    SECTION("a shrunk civ band misses rolls the unscaled band caught")
    {
        TrafficDecisionInput in = Input(true, 0, 0, 0, true, false, false, 0.30f);
        in.civScale = 0.5f; // civ band [0, 0.25)
        REQUIRE(Traffic::DecideSpawn(in, t) == -1);
        in.roll = 0.20f;
        REQUIRE(Traffic::DecideSpawn(in, t) == TKCiv);
    }
    SECTION("a huge patrol boost caps the band at a certainty, not beyond")
    {
        TrafficDecisionInput in = Input(true, 0, 0, 0, false, true, false, 0.999f);
        in.patrolScale = 100.0f;
        REQUIRE(Traffic::DecideSpawn(in, t) == TKPatrol);
    }
}

TEST_CASE("Traffic - far-despawn edge with hysteresis", "[game][guerrilla]")
{
    TrafficTuning t; // 1500 + 300
    float edge = 1800.0f;
    REQUIRE_FALSE(Traffic::ShouldDespawn((edge - 1) * (edge - 1), t));
    REQUIRE_FALSE(Traffic::ShouldDespawn(edge * edge, t));
    REQUIRE(Traffic::ShouldDespawn((edge + 1) * (edge + 1), t));
}

namespace
{
float Sq(float v)
{
    return v * v;
}
} // namespace

TEST_CASE("Traffic - effective band from the live cull", "[game][guerrilla]")
{
    TrafficTuning t; // 300 / 1500 / +300, exposeMargin 150

    SECTION("cull inside the config floor: the config band verbatim")
    {
        // objectsZ 100 + margin 150 = 250 < minSpawnDist 300
        TrafficEffectiveBand b = Traffic::EffectiveBand(t, 100.0f, false, 900.0f);
        REQUIRE(b.minSpawn == Approx(300.0f));
        REQUIRE(b.radius == Approx(1500.0f));
        REQUIRE(b.despawnEdge == Approx(1800.0f));
        REQUIRE(b.closeHold == Approx(300.0f));
        // ... which is exactly ConfigBand, the no-camera fallback
        TrafficEffectiveBand c = Traffic::ConfigBand(t);
        REQUIRE(c.minSpawn == Approx(300.0f));
        REQUIRE(c.radius == Approx(1500.0f));
        REQUIRE(c.despawnEdge == Approx(1800.0f));
        REQUIRE(c.closeHold == Approx(300.0f));
    }

    SECTION("default view distance pushes the floor out; radius and edge stay config (identity)")
    {
        // objectsZ 600 (view 900): floor 750, band narrows to [750, 1500] -
        // the radius and despawn edge are UNCHANGED at the default view
        // distance (the new keys' defaults preserve pre-#53 behaviour)
        TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, false, 900.0f);
        REQUIRE(b.minSpawn == Approx(750.0f));
        REQUIRE(b.radius == Approx(1500.0f));
        REQUIRE(b.despawnEdge == Approx(1800.0f));
        REQUIRE(b.closeHold == Approx(300.0f));
    }

    SECTION("a floor pushed past the config radius WIDENS the band, never empties it")
    {
        // objectsZ 3000 (the cull cap): floor 3150, far beyond radius 1500;
        // the band keeps a road-scan radius of width above the floor
        TrafficEffectiveBand b = Traffic::EffectiveBand(t, 3000.0f, false, 900.0f);
        REQUIRE(b.minSpawn == Approx(3150.0f));
        REQUIRE(b.radius == Approx(3150.0f + Traffic::SpawnScanRadius));
        REQUIRE(b.radius >= b.minSpawn);
        REQUIRE(b.despawnEdge >= b.radius);
        REQUIRE(b.closeHold == Approx(300.0f)); // the audible hold never moves
    }

    SECTION("a floor nearing the config radius keeps a usable width (no thin-annulus regime)")
    {
        // objectsZ 1200: floor 1350, naive band [1350, 1500] is 150 m thin -
        // the radius follows the floor out to keep a road-scan width
        TrafficEffectiveBand b = Traffic::EffectiveBand(t, 1200.0f, false, 900.0f);
        REQUIRE(b.minSpawn == Approx(1350.0f));
        REQUIRE(b.radius == Approx(1350.0f + Traffic::SpawnScanRadius));
        REQUIRE(b.despawnEdge == Approx(b.radius + 300.0f));
    }

    SECTION("night light bound: lights raise the cull to horizontZ + 500")
    {
        // horizontZ 900: light cull 1400 > objectsZ 600 -> floor 1550
        TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, true, 900.0f);
        REQUIRE(b.minSpawn == Approx(1550.0f));
        // ... but never lowers the cull below objectsZ
        TrafficEffectiveBand c = Traffic::EffectiveBand(t, 3000.0f, true, 900.0f);
        REQUIRE(c.minSpawn == Approx(3150.0f));
    }

    SECTION("silly config: radius under the floor clamps the width to zero")
    {
        TrafficTuning w;
        w.minSpawnDist = 800.0f;
        w.radius = 500.0f;
        w.despawnHysteresis = 0.0f;
        TrafficEffectiveBand b = Traffic::EffectiveBand(w, 600.0f, false, 900.0f);
        REQUIRE(b.minSpawn == Approx(800.0f));
        REQUIRE(b.radius == Approx(800.0f));
        REQUIRE(b.despawnEdge == Approx(800.0f));
    }

    SECTION("a negative hysteresis cannot pull the edge inside the safe distance")
    {
        TrafficTuning w;
        w.despawnHysteresis = -1300.0f;
        // objectsZ 600: safe 750, band [750, 1500], raw edge 200 -> floored
        TrafficEffectiveBand b = Traffic::EffectiveBand(w, 600.0f, false, 900.0f);
        REQUIRE(b.despawnEdge == Approx(750.0f));
    }
}

TEST_CASE("Traffic - spawn exposure predicate", "[game][guerrilla]")
{
    TrafficTuning t;
    TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, false, 900.0f); // floor 750

    SECTION("beyond the effective floor is always safe, boundary inclusive")
    {
        REQUIRE(Traffic::CanExposeSpawn(Sq(751.0f), false, true, b));
        REQUIRE(Traffic::CanExposeSpawn(Sq(750.0f), false, true, b));
    }
    SECTION("inside the floor, visible and in view: exposed")
    {
        REQUIRE_FALSE(Traffic::CanExposeSpawn(Sq(500.0f), false, true, b));
    }
    SECTION("terrain hides a close point")
    {
        REQUIRE(Traffic::CanExposeSpawn(Sq(500.0f), true, true, b));
    }
    SECTION("outside the view cone hides a close point")
    {
        REQUIRE(Traffic::CanExposeSpawn(Sq(500.0f), false, false, b));
    }
}

TEST_CASE("Traffic - despawn exposure predicate", "[game][guerrilla]")
{
    TrafficTuning t;
    // floor 750 (the imperceptibility bound), edge 1800, closeHold 300
    TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, false, 900.0f);

    SECTION("in the view cone is never safe, whatever the distance or terrain")
    {
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(3000.0f), false, true, b));
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(100.0f), true, true, b));
    }
    SECTION("out of the cone and beyond the imperceptibility bound: safe, boundary inclusive")
    {
        // the distance leg is the cull bound (band.minSpawn), NOT the far
        // edge - an in-band ending on open terrain must be able to tear
        // down once the car is beyond draw range (caps must not pin)
        REQUIRE(Traffic::CanExposeDespawn(Sq(751.0f), false, false, b));
        REQUIRE(Traffic::CanExposeDespawn(Sq(750.0f), false, false, b));
        REQUIRE(Traffic::CanExposeDespawn(Sq(2251.0f), false, false, b));
    }
    SECTION("out of the cone and terrain-hidden inside the bound: safe")
    {
        REQUIRE(Traffic::CanExposeDespawn(Sq(400.0f), true, false, b));
    }
    SECTION("out of the cone but clear and inside the bound: not safe")
    {
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(400.0f), false, false, b));
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(749.0f), false, false, b));
    }
    SECTION("inside the close hold nothing despawns, even terrain-hidden out of the cone")
    {
        // an idling engine at 200 m is audible where the mesh is not
        // visible: the pre-#53 close-range hold survives the gate
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(200.0f), true, false, b));
        REQUIRE_FALSE(Traffic::CanExposeDespawn(Sq(299.0f), true, false, b));
        REQUIRE(Traffic::CanExposeDespawn(Sq(300.0f), true, false, b)); // hold boundary
    }
    SECTION("the band overload of ShouldDespawn tracks the effective edge")
    {
        REQUIRE(Traffic::ShouldDespawn(Sq(1801.0f), b));
        REQUIRE_FALSE(Traffic::ShouldDespawn(Sq(1800.0f), b));
    }
}

TEST_CASE("Traffic - perception-aware spawn point selection", "[game][guerrilla]")
{
    TrafficTuning t;                                                           // hard floor 300
    TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, false, 900.0f); // floor 750, cap 1500
    Vector3 player(0, 0, 0);

    SECTION("distance-only (null obs): farthest point past the exposure floor")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(200, 0, 0));  // under the hard floor - never eligible
        pts.Add(Vector3(500, 0, 0));  // inside the exposure floor
        pts.Add(Vector3(900, 0, 0));  // past the exposure floor
        pts.Add(Vector3(2500, 0, 0)); // beyond the cap
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, nullptr) == 2);
    }

    SECTION("a widened band accepts points past the config radius")
    {
        // objectsZ 3000: floor 3150, cap 3150 + SpawnScanRadius
        TrafficEffectiveBand wide = Traffic::EffectiveBand(t, 3000.0f, false, 900.0f);
        AutoArray<Vector3> pts;
        pts.Add(Vector3(1400, 0, 0)); // inside the config band but under the pushed floor
        pts.Add(Vector3(3500, 0, 0)); // past the config radius, inside the widened cap
        pts.Add(Vector3(4500, 0, 0)); // beyond the widened cap
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, wide, nullptr) == 1);
    }

    SECTION("the exposure distance leg follows the camera when the obs carry it")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(500, 0, 0)); // inside the floor of the PLAYER...
        AutoArray<TrafficExposeObs> obs;
        obs.Resize(1);
        obs[0].camDist2 = Sq(2000.0f); // ... but far from the scripted camera
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, &obs) == 0);
        // and the converse: far from the player, right in front of the camera
        AutoArray<Vector3> far;
        far.Add(Vector3(1400, 0, 0));
        AutoArray<TrafficExposeObs> nearCam;
        nearCam.Resize(1);
        nearCam[0].camDist2 = Sq(100.0f);
        REQUIRE(Traffic::SelectSpawnPoint(far, player, t, b, &nearCam) == -1);
    }

    SECTION("no eligible point without observations, LOS or the cone legalize close ones")
    {
        AutoArray<Vector3> close;
        close.Add(Vector3(400, 0, 0));
        close.Add(Vector3(600, 0, 0));
        REQUIRE(Traffic::SelectSpawnPoint(close, player, t, b, nullptr) == -1);

        AutoArray<TrafficExposeObs> hidden; // terrain hides only the near one
        hidden.Resize(2);
        hidden[0].losBlocked = true;
        REQUIRE(Traffic::SelectSpawnPoint(close, player, t, b, &hidden) == 0);

        AutoArray<TrafficExposeObs> cone; // both eligible, but tiers differ:
        cone.Resize(2);                   // terrain-hidden survives a turn-around,
        cone[0].losBlocked = true;        // cone-only does not - the closer hidden
        cone[1].inFrustum = false;        // point wins over the farther cone-only one
        REQUIRE(Traffic::SelectSpawnPoint(close, player, t, b, &cone) == 0);
    }

    SECTION("the config minSpawnDist stays a hard floor even for hidden points")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(200, 0, 0));
        AutoArray<TrafficExposeObs> obs;
        obs.Resize(1);
        obs[0].losBlocked = true;
        obs[0].inFrustum = false;
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, &obs) == -1);
    }

    SECTION("config-band identity: the perception variant with defaults matches the legacy pick")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(100, 0, 0));
        pts.Add(Vector3(400, 0, 0));
        pts.Add(Vector3(0, 0, 900));
        pts.Add(Vector3(2000, 0, 0));
        TrafficEffectiveBand cfg = Traffic::ConfigBand(t);
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, cfg, nullptr) == Traffic::SelectSpawnPoint(pts, player, t));
    }
}

TEST_CASE("Traffic - spawn point tier scoring (alibi preferences)", "[game][guerrilla]")
{
    TrafficTuning t;                                                           // hard floor 300
    TrafficEffectiveBand b = Traffic::EffectiveBand(t, 600.0f, false, 900.0f); // floor 750, cap 1500
    Vector3 player(0, 0, 0);

    SECTION("terrain-hidden beats cone-only whatever the distances")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(400, 0, 0)); // losBlocked: survives a turn-around
        pts.Add(Vector3(740, 0, 0)); // cone-only: one head turn from exposure
        AutoArray<TrafficExposeObs> obs;
        obs.Resize(2);
        obs[0].losBlocked = true;
        obs[1].inFrustum = false;
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, &obs) == 0);
    }

    SECTION("a distance pass is turn-around-proof too: same tier as terrain-hidden, farthest wins")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(500, 0, 0));  // losBlocked
        pts.Add(Vector3(1400, 0, 0)); // beyond the floor
        AutoArray<TrafficExposeObs> obs;
        obs.Resize(2);
        obs[0].losBlocked = true;
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, &obs) == 1);
    }

    SECTION("preferOrigin: an in-zone point beats a farther out-of-zone one")
    {
        Vector3 origin(0, 0, 900);
        AutoArray<Vector3> pts;
        pts.Add(Vector3(0, 0, 1000)); // 100 m from the origin centre: in-zone
        pts.Add(Vector3(1400, 0, 0)); // far out on the open road
        // both pass by distance (1000 and 1400 >= floor 750)
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, nullptr, &origin, true) == 0);
        // ... but only for the kinds that ask for it
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, nullptr, &origin, false) == 1);
        // ... and a null origin means no preference
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, nullptr, nullptr, true) == 1);
    }

    SECTION("preferOrigin never overrides the exposure gate")
    {
        Vector3 origin(500, 0, 0);
        AutoArray<Vector3> pts;
        pts.Add(Vector3(500, 0, 0));  // in-zone but visible inside the floor: gated out
        pts.Add(Vector3(1400, 0, 0)); // out-of-zone distance pass
        REQUIRE(Traffic::SelectSpawnPoint(pts, player, t, b, nullptr, &origin, true) == 1);
    }
}

TEST_CASE("Traffic - alibi spawn point selection", "[game][guerrilla]")
{
    TrafficTuning t; // minSpawnDist 300 stays the audible hard floor
    Vector3 player(0, 0, 0);
    Vector3 town(600, 0, 0);

    SECTION("only points inside AlibiOriginRadius of the town qualify; farthest from the player wins")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(400, 0, 0));  // in-town, 400 m out
        pts.Add(Vector3(700, 0, 0));  // in-town, 700 m out
        pts.Add(Vector3(850, 0, 0));  // in-town, 850 m out
        pts.Add(Vector3(1200, 0, 0)); // 600 m past the centre: outside the town
        REQUIRE(Traffic::SelectAlibiPoint(pts, player, town, t, nullptr) == 2);
    }

    SECTION("hidden curbs beat visible ones even when closer")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(400, 0, 0));
        pts.Add(Vector3(850, 0, 0));
        AutoArray<TrafficExposeObs> obs;
        obs.Resize(2);
        obs[0].losBlocked = true; // a building-shadowed curb
        REQUIRE(Traffic::SelectAlibiPoint(pts, player, town, t, &obs) == 0);
        // out-of-cone counts as hidden too
        obs[0].losBlocked = false;
        obs[0].inFrustum = false;
        REQUIRE(Traffic::SelectAlibiPoint(pts, player, town, t, &obs) == 0);
    }

    SECTION("a visible curb is still a legal last resort (a pull-out is a plausible birth)")
    {
        AutoArray<Vector3> pts;
        pts.Add(Vector3(500, 0, 0));
        REQUIRE(Traffic::SelectAlibiPoint(pts, player, town, t, nullptr) == 0);
    }

    SECTION("the audible hard floor still applies inside the town")
    {
        Vector3 nearTown(400, 0, 0);
        AutoArray<Vector3> pts;
        pts.Add(Vector3(200, 0, 0)); // in-town but 200 m from the player
        REQUIRE(Traffic::SelectAlibiPoint(pts, player, nearTown, t, nullptr) == -1);
    }

    SECTION("no eligible point: -1")
    {
        AutoArray<Vector3> none;
        REQUIRE(Traffic::SelectAlibiPoint(none, player, town, t, nullptr) == -1);
    }
}

TEST_CASE("Traffic - cap scaling with the widened band", "[game][guerrilla]")
{
    SECTION("identity at (or below) the config band")
    {
        REQUIRE(Traffic::ScaleCap(3, 1500.0f, 1500.0f) == 3);
        REQUIRE(Traffic::ScaleCap(3, 1000.0f, 1500.0f) == 3); // never below the config
        REQUIRE(Traffic::ScaleCap(1, 1500.0f, 1500.0f) == 1);
    }
    SECTION("linear with the band radius, rounded")
    {
        REQUIRE(Traffic::ScaleCap(3, 3000.0f, 1500.0f) == 6);
        REQUIRE(Traffic::ScaleCap(1, 3000.0f, 1500.0f) == 2);
        REQUIRE(Traffic::ScaleCap(3, 2000.0f, 1500.0f) == 4); // 4.0
        REQUIRE(Traffic::ScaleCap(3, 1750.0f, 1500.0f) == 4); // 3.5 rounds up
        REQUIRE(Traffic::ScaleCap(3, 1600.0f, 1500.0f) == 3); // 3.2 rounds down
    }
    SECTION("degenerate inputs stay put")
    {
        REQUIRE(Traffic::ScaleCap(0, 3000.0f, 1500.0f) == 0); // 0 = never spawns, stays never
        REQUIRE(Traffic::ScaleCap(3, 3000.0f, 0.0f) == 3);    // silly config radius: identity
        REQUIRE(Traffic::ScaleCap(-1, 3000.0f, 1500.0f) == -1);
    }
}

TEST_CASE("Traffic - route picking honours the effective band's origin gate", "[game][guerrilla]")
{
    TrafficTuning t; // config radius 1500
    AutoArray<TrafficZoneCandidate> zones;
    zones.Add(Zone(0, 3000, 1000, true, false)); // CITY 2000 m from the player
    zones.Add(Zone(1, 4500, 1000, true, false)); // CITY dest, 1500 m from the origin
    int o = -1, d = -1;

    // config radius: no origin in reach
    REQUIRE_FALSE(Traffic::PickRoute(TKCiv, zones, 1000, 1000, t, 0.0f, o, d));
    // widened band: the pushed-out origin is reachable again
    TrafficEffectiveBand b = Traffic::ConfigBand(t);
    b.radius = 2500.0f;
    REQUIRE(Traffic::PickRoute(TKCiv, zones, 1000, 1000, t, 0.0f, o, d, -1, &b));
    REQUIRE(o == 0);
    REQUIRE(d == 1);
    // null band = the config radius, unchanged behaviour
    REQUIRE_FALSE(Traffic::PickRoute(TKCiv, zones, 1000, 1000, t, 0.0f, o, d, -1, nullptr));
}

TEST_CASE("Traffic - route picking", "[game][guerrilla]")
{
    TrafficTuning t;
    AutoArray<TrafficZoneCandidate> zones;
    zones.Add(Zone(0, 1000, 1000, true, false)); // CITY A (near the player)
    zones.Add(Zone(1, 2500, 1000, true, false)); // CITY B, 1500 m from A
    zones.Add(Zone(2, 9000, 9000, true, false)); // CITY C, far from everything
    zones.Add(Zone(3, 1200, 1400, false, true)); // occupier OUTPOST near the player
    zones.Add(Zone(4, 4000, 4000, false, true)); // occupier CAMP far away
    zones.Add(Zone(5, 1300, 900, false, false)); // resistance CAMP near the player
    int o = -1, d = -1;

    SECTION("civ: CITY within radius -> a different CITY, 800..5000 m preferred")
    {
        REQUIRE(Traffic::PickRoute(TKCiv, zones, 1000, 1000, t, 0.0f, o, d));
        REQUIRE(o == 0);
        REQUIRE(d == 1);
        REQUIRE(o != d);
    }

    SECTION("civ: fallback to any other CITY when none sits in the band")
    {
        AutoArray<TrafficZoneCandidate> two;
        two.Add(Zone(0, 1000, 1000, true, false));
        two.Add(Zone(1, 1100, 1000, true, false)); // 100 m: too close for the preferred band
        REQUIRE(Traffic::PickRoute(TKCiv, two, 1000, 1000, t, 0.0f, o, d));
        REQUIRE(o == 0);
        REQUIRE(d == 1);
    }

    SECTION("civ: nothing when no CITY is within radius of the player")
    {
        REQUIRE_FALSE(Traffic::PickRoute(TKCiv, zones, 20000, 20000, t, 0.0f, o, d));
        REQUIRE(o == -1);
        REQUIRE(d == -1);
    }

    SECTION("civ: a lone CITY has no destination")
    {
        AutoArray<TrafficZoneCandidate> one;
        one.Add(Zone(0, 1000, 1000, true, false));
        REQUIRE_FALSE(Traffic::PickRoute(TKCiv, one, 1000, 1000, t, 0.0f, o, d));
    }

    SECTION("patrol: occupier pair, never a CITY / resistance zone")
    {
        REQUIRE(Traffic::PickRoute(TKPatrol, zones, 1000, 1000, t, 0.0f, o, d));
        REQUIRE(o == 3);
        REQUIRE(d == 4);
    }

    SECTION("convoy: occupier non-CITY origin")
    {
        REQUIRE(Traffic::PickRoute(TKConvoy, zones, 1000, 1000, t, 0.5f, o, d));
        REQUIRE(o == 3);
        REQUIRE(d == 4);
        // the far CAMP is the only occupier zone in range -> its dest is the outpost
        REQUIRE(Traffic::PickRoute(TKConvoy, zones, 4000, 4000, t, 0.5f, o, d));
        REQUIRE(o == 4);
        REQUIRE(d == 3);
    }

    SECTION("patrol: a single occupier zone has no route")
    {
        AutoArray<TrafficZoneCandidate> one;
        one.Add(Zone(3, 1200, 1400, false, true));
        one.Add(Zone(0, 1000, 1000, true, false));
        REQUIRE_FALSE(Traffic::PickRoute(TKPatrol, one, 1000, 1000, t, 0.0f, o, d));
    }

    SECTION("pinned origin (force spawn) skips the player-range gate")
    {
        REQUIRE(Traffic::PickRoute(TKCiv, zones, 20000, 20000, t, 0.0f, o, d, 2));
        REQUIRE(o == 2);
        REQUIRE((d == 0 || d == 1)); // fallback pool: neither is in the 800..5000 band
        // a pinned origin of the wrong kind fails
        REQUIRE_FALSE(Traffic::PickRoute(TKCiv, zones, 1000, 1000, t, 0.0f, o, d, 3));
        REQUIRE_FALSE(Traffic::PickRoute(TKConvoy, zones, 1000, 1000, t, 0.0f, o, d, 0));
    }

    SECTION("roll edge values stay in range")
    {
        REQUIRE(Traffic::PickRoute(TKCiv, zones, 1500, 1000, t, 0.999f, o, d));
        REQUIRE(Traffic::PickRoute(TKCiv, zones, 1500, 1000, t, 1.0f, o, d));
        REQUIRE(Traffic::PickRoute(TKCiv, zones, 1500, 1000, t, -1.0f, o, d));
    }
}

TEST_CASE("Traffic - spawn point inside the player band, farthest wins", "[game][guerrilla]")
{
    TrafficTuning t; // band [300, 1500]
    Vector3 player(0, 0, 0);
    AutoArray<Vector3> pts;
    pts.Add(Vector3(100, 0, 0));  // too close
    pts.Add(Vector3(400, 0, 0));  // in band
    pts.Add(Vector3(0, 0, 900));  // in band, farthest
    pts.Add(Vector3(2000, 0, 0)); // beyond radius
    REQUIRE(Traffic::SelectSpawnPoint(pts, player, t) == 2);

    AutoArray<Vector3> none;
    none.Add(Vector3(10, 0, 0));
    none.Add(Vector3(5000, 0, 0));
    REQUIRE(Traffic::SelectSpawnPoint(none, player, t) == -1);
    AutoArray<Vector3> empty;
    REQUIRE(Traffic::SelectSpawnPoint(empty, player, t) == -1);
}

TEST_CASE("Traffic - commandeer trigger matrix", "[game][guerrilla]")
{
    TrafficTuning t; // radius 25, lane half-width 4
    // car at origin heading +Z (north)
    SECTION("player ahead in the lane, unarmed -> trigger")
    {
        REQUIRE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 1, 15, 0, -1, false), t));
    }
    SECTION("player ahead but outside the lane -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 8, 15, 0, -1, false), t));
    }
    SECTION("player behind the car -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 0, -15, 0, 1, false), t));
    }
    SECTION("player beside the car -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, -1, 0, false), t));
    }
    SECTION("armed player beside the car aiming at it -> trigger")
    {
        REQUIRE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, -1, 0, true), t));
    }
    SECTION("armed player beside the car aiming elsewhere -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, 0, 1, true), t));
    }
    SECTION("unarmed player beside the car looking at it -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, -1, 0, false), t));
    }
    SECTION("out of radius, even in the lane and aiming -> no trigger")
    {
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 0, 40, 0, -1, true), t));
    }
    SECTION("aim cone edge: 14 deg in, 16 deg out")
    {
        float in14 = 14.0f * 3.14159265f / 180.0f;
        float out16 = 16.0f * 3.14159265f / 180.0f;
        // player east of the car, aiming west with a small tilt
        REQUIRE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, -cosf(in14), sinf(in14), true), t));
        REQUIRE_FALSE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 1, 10, 0, -cosf(out16), sinf(out16), true), t));
    }
    SECTION("car heading is normalized, not assumed unit")
    {
        REQUIRE(Traffic::CommandeerTriggered(Obs(0, 0, 0, 3, 1, 15, 0, -1, false), t));
    }
}

TEST_CASE("Traffic - stall expiry", "[game][guerrilla]")
{
    TrafficTuning t;
    REQUIRE_FALSE(Traffic::StallExpired(0.0f, t));
    REQUIRE_FALSE(Traffic::StallExpired(89.0f, t));
    REQUIRE(Traffic::StallExpired(90.0f, t));
    t.stallTimeout = 0; // disabled
    REQUIRE_FALSE(Traffic::StallExpired(1000.0f, t));
}

TEST_CASE("Traffic - blocked recovery staging inside the stall window", "[game][guerrilla]")
{
    TrafficTuning t; // stallTimeout 90: retry threshold 30, U-turn threshold 60

    SECTION("staged thresholds: retry the leg first, then U-turn")
    {
        REQUIRE(Traffic::DecideBlocked(29.0f, 0, true, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(30.0f, 0, true, t) == TBlockRetryLeg);
        // the first retry is spent; the same clock reading is below the next stage
        REQUIRE(Traffic::DecideBlocked(30.0f, 1, true, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(59.0f, 1, true, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(60.0f, 1, true, t) == TBlockUTurn);
    }
    SECTION("retries cap: a car that defeated both is left to the expiry ladder")
    {
        REQUIRE(Traffic::DecideBlocked(89.0f, 2, true, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(1000.0f, 2, true, t) == TBlockNone);
    }
    SECTION("speed guard: slow-but-moving cars are not blocked")
    {
        REQUIRE(Traffic::DecideBlocked(60.0f, 0, false, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(60.0f, 1, false, t) == TBlockNone);
    }
    SECTION("stallTimeout 0 disables the tier along with the ladder")
    {
        t.stallTimeout = 0;
        REQUIRE(Traffic::DecideBlocked(1000.0f, 0, true, t) == TBlockNone);
    }
    SECTION("the staging tracks a tuned timeout")
    {
        t.stallTimeout = 30.0f;
        REQUIRE(Traffic::DecideBlocked(9.0f, 0, true, t) == TBlockNone);
        REQUIRE(Traffic::DecideBlocked(10.0f, 0, true, t) == TBlockRetryLeg);
        REQUIRE(Traffic::DecideBlocked(20.0f, 1, true, t) == TBlockUTurn);
    }
}

TEST_CASE("Traffic - combat gate on the stall and trip-end ladder", "[game][guerrilla]")
{
    TrafficTuning t; // combatStaleAfter 120, combatHoldMax 300

    SECTION("a quiet convoy that never saw a shot is clear")
    {
        // "never disclosed" arrives as a huge sinceDisclosed: stale by construction,
        // so the !inCombat leg clears without any special-case sentinel
        REQUIRE(Traffic::CombatGateAction(false, 1e9f, 0.0f, t) == TCGClear);
    }

    SECTION("a hot convoy holds the ladder")
    {
        // inCombat wins over a stale disclosure: the crew is shooting right now
        REQUIRE(Traffic::CombatGateAction(true, 1e9f, 0.0f, t) == TCGHold);
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 0.0f, t) == TCGHold);
        // combat ended but the contact is fresh (10 s < 120 s stale window)
        REQUIRE(Traffic::CombatGateAction(false, 10.0f, 0.0f, t) == TCGHold);
        // one tick short of the stale window still holds (119.9 < 120)
        REQUIRE(Traffic::CombatGateAction(false, 119.9f, 0.0f, t) == TCGHold);
    }

    SECTION("the stale boundary is inclusive: exactly combatStaleAfter clears")
    {
        REQUIRE(Traffic::CombatGateAction(false, 120.0f, 0.0f, t) == TCGClear); // 120 >= 120
        REQUIRE(Traffic::CombatGateAction(false, 120.1f, 0.0f, t) == TCGClear);
    }

    SECTION("an ended episode clears even with the hold budget spent")
    {
        // the clear leg outranks the exhausted leg, so the caller gets its
        // budget-reset signal instead of a permanent TCGExhausted
        REQUIRE(Traffic::CombatGateAction(false, 200.0f, 400.0f, t) == TCGClear); // 200 >= 120 first
    }

    SECTION("the hold budget is bounded: exactly combatHoldMax is exhausted")
    {
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 299.9f, t) == TCGHold);      // 299.9 < 300
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 300.0f, t) == TCGExhausted); // 300 >= 300
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 900.0f, t) == TCGExhausted);
        // still-fresh contact without live combat exhausts on the same budget
        REQUIRE(Traffic::CombatGateAction(false, 5.0f, 300.0f, t) == TCGExhausted);
    }

    SECTION("no re-hold: an exhausted gate never falls back to holding")
    {
        // the budget is NOT reset on exhaustion, so a second still-hot pass at
        // the same heldTime keeps the ladder running instead of re-freezing it
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 305.0f, t) == TCGExhausted);
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 305.0f, t) == TCGExhausted);
        REQUIRE(Traffic::CombatGateAction(false, 1.0f, 305.0f, t) == TCGExhausted);
    }

    SECTION("a zero hold budget disables the gate outright")
    {
        TrafficTuning off = t;
        off.combatHoldMax = 0.0f;
        // disabled outranks every other leg: hot, fresh and unspent still clears
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 0.0f, off) == TCGClear);
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 500.0f, off) == TCGClear);
        off.combatHoldMax = -10.0f; // a hand-edited config cannot revive it either
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 0.0f, off) == TCGClear);
    }

    SECTION("a zero stale window ends the episode the instant fire stops")
    {
        TrafficTuning snappy = t;
        snappy.combatStaleAfter = 0.0f;
        REQUIRE(Traffic::CombatGateAction(false, 0.0f, 0.0f, snappy) == TCGClear); // 0 >= 0
        REQUIRE(Traffic::CombatGateAction(true, 0.0f, 0.0f, snappy) == TCGHold);   // still shooting
    }

    SECTION("a negative sinceDisclosed is stale, never recent")
    {
        // regression: a never-disclosed group's raw Time subtraction
        // underflows to ~-2.1e6 (TIME_MIN is Time(-INT_MAX)); the world
        // layer maps it to huge-positive, and the pure gate must ALSO
        // treat a negative that slips through as stale - the broken
        // reading froze every quiet patrol from spawn
        REQUIRE(Traffic::CombatGateAction(false, -2.1e6f, 0.0f, t) == TCGClear);
        REQUIRE(Traffic::CombatGateAction(false, -0.1f, 0.0f, t) == TCGClear);
        // live combat still holds regardless of the garbage timestamp
        REQUIRE(Traffic::CombatGateAction(true, -2.1e6f, 0.0f, t) == TCGHold);
    }
}

TEST_CASE("Traffic - convoy bail trigger matrix", "[game][guerrilla]")
{
    TrafficTuning t; // bailCombatWindow 60

    SECTION("a dead escort in a recent contact makes the convoy bail")
    {
        REQUIRE(Traffic::ConvoyBailTriggered(true, true, 0.0f, t));
        REQUIRE(Traffic::ConvoyBailTriggered(true, true, 30.0f, t)); // 30 <= 60
    }

    SECTION("the window boundary is inclusive")
    {
        REQUIRE(Traffic::ConvoyBailTriggered(true, true, 60.0f, t)); // 60 <= 60
    }

    SECTION("a dead escort with a stale contact does not bail")
    {
        // just past the window: the escort died long ago, the trip carries on
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, true, 60.1f, t));
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, true, 1e9f, t)); // never disclosed
    }

    SECTION("a convoy that never had an escort does not bail")
    {
        // no escort to lose: escortDead is meaningless without escortExisted
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(false, true, 0.0f, t));
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(false, false, 0.0f, t));
    }

    SECTION("a living escort does not bail")
    {
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, false, 0.0f, t));
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, false, 1e9f, t));
    }

    SECTION("a zero window disables the bail entirely")
    {
        TrafficTuning off = t;
        off.bailCombatWindow = 0.0f;
        // even the freshest possible contact: 0 is the "off" value, not "instant"
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, true, 0.0f, off));
    }

    SECTION("a negative sinceDisclosed is not recent")
    {
        // regression: a never-disclosed group's raw Time subtraction
        // underflows negative (TIME_MIN), and -2.1e6 <= 60 would read as
        // "recent" - a convoy that lost its escort to a crash with no
        // combat ever must NOT bail
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, true, -2.1e6f, t));
        REQUIRE_FALSE(Traffic::ConvoyBailTriggered(true, true, -0.1f, t));
    }
}

TEST_CASE("Traffic - crew disposal when a bailed vehicle is cleaned up", "[game][guerrilla]")
{
    // dead: the seat does not matter, the person is a body either way
    REQUIRE(Traffic::CrewDisposal(true, true) == TCDBody);
    REQUIRE(Traffic::CrewDisposal(true, false) == TCDBody);
    // alive and still seated: the pre-fix bug filed this person as a corpse and
    // left him riding a stopped hull, so he must be unseated before he flees
    REQUIRE(Traffic::CrewDisposal(false, true) == TCDDismountFlee);
    // alive and already on foot: nothing to unseat, just run
    REQUIRE(Traffic::CrewDisposal(false, false) == TCDFlee);
}

TEST_CASE("Traffic - danger ring buffer insert/coalesce/expiry", "[game][guerrilla]")
{
    AutoArray<TrafficDangerEvent> buf;

    SECTION("insert and coalesce")
    {
        Traffic::AddDangerEvent(buf, Vector3(100, 0, 100), 1.0f, false);
        REQUIRE(buf.Size() == 1);
        // a burst lands within the coalesce radius: one episode, refreshed
        Traffic::AddDangerEvent(buf, Vector3(110, 0, 100), 2.0f, true);
        REQUIRE(buf.Size() == 1);
        REQUIRE(buf[0].severity == Approx(2.0f)); // max wins
        REQUIRE(buf[0].playerCaused);             // OR-ed
        // a weaker constituent never lowers the episode severity or clears
        // its player attribution
        Traffic::AddDangerEvent(buf, Vector3(100, 0, 100), 0.5f, false);
        REQUIRE(buf[0].severity == Approx(2.0f));
        REQUIRE(buf[0].playerCaused);
        // beyond the coalesce radius: its own episode
        Traffic::AddDangerEvent(buf, Vector3(200, 0, 100), 1.0f, false);
        REQUIRE(buf.Size() == 2);
        REQUIRE_FALSE(buf[1].playerCaused);
    }

    SECTION("coalescing refreshes the age")
    {
        Traffic::AddDangerEvent(buf, Vector3(0, 0, 0), 1.0f, false);
        Traffic::AgeDangerEvents(buf, 15.0f, 20.0f);
        REQUIRE(buf[0].age == Approx(15.0f));
        Traffic::AddDangerEvent(buf, Vector3(0, 0, 0), 1.0f, false);
        REQUIRE(buf[0].age == Approx(0.0f));
    }

    SECTION("expiry at the ttl")
    {
        Traffic::AddDangerEvent(buf, Vector3(0, 0, 0), 1.0f, false);
        Traffic::AddDangerEvent(buf, Vector3(500, 0, 0), 1.0f, false);
        Traffic::AgeDangerEvents(buf, 10.0f, 20.0f);
        REQUIRE(buf.Size() == 2);
        Traffic::AddDangerEvent(buf, Vector3(500, 0, 0), 1.0f, false); // refreshed mid-life
        Traffic::AgeDangerEvents(buf, 10.0f, 20.0f);
        REQUIRE(buf.Size() == 1); // the stale episode aged out, the refreshed one lives
        Traffic::AgeDangerEvents(buf, 10.0f, 20.0f);
        REQUIRE(buf.Size() == 0);
    }

    SECTION("ttl 0 empties the ring")
    {
        Traffic::AddDangerEvent(buf, Vector3(0, 0, 0), 1.0f, false);
        Traffic::AgeDangerEvents(buf, 5.0f, 0.0f);
        REQUIRE(buf.Size() == 0);
    }

    SECTION("a full ring evicts the oldest episode")
    {
        for (int i = 0; i < Traffic::MaxDangerEvents; i++)
        {
            Traffic::AddDangerEvent(buf, Vector3((float)(i * 200), 0, 0), 1.0f, false);
            Traffic::AgeDangerEvents(buf, 1.0f, 100.0f); // stagger: the first insert ends up oldest
        }
        REQUIRE(buf.Size() == Traffic::MaxDangerEvents);
        Traffic::AddDangerEvent(buf, Vector3(0, 0, 5000), 3.0f, true);
        REQUIRE(buf.Size() == Traffic::MaxDangerEvents);
        bool foundNew = false;
        bool foundOldest = false;
        for (int i = 0; i < buf.Size(); i++)
        {
            if (buf[i].pos.Z() > 4000.0f)
            {
                foundNew = true;
            }
            if (buf[i].pos.X() < 1.0f && buf[i].pos.Z() < 1.0f)
            {
                foundOldest = true;
            }
        }
        REQUIRE(foundNew);
        REQUIRE_FALSE(foundOldest);
    }

    SECTION("a full ring still coalesces instead of evicting")
    {
        for (int i = 0; i < Traffic::MaxDangerEvents; i++)
        {
            Traffic::AddDangerEvent(buf, Vector3((float)(i * 200), 0, 0), 1.0f, false);
            Traffic::AgeDangerEvents(buf, 1.0f, 100.0f);
        }
        REQUIRE(buf.Size() == Traffic::MaxDangerEvents);
        // a burst near episode 3 merges into it: no eviction, episode 0 kept
        Traffic::AddDangerEvent(buf, Vector3(610, 0, 0), 2.0f, true);
        REQUIRE(buf.Size() == Traffic::MaxDangerEvents);
        bool foundFirst = false;
        bool foundMerged = false;
        for (int i = 0; i < buf.Size(); i++)
        {
            if (buf[i].pos.X() < 1.0f)
            {
                foundFirst = true;
            }
            if (buf[i].pos.X() > 599.0f && buf[i].pos.X() < 601.0f)
            {
                foundMerged = buf[i].severity > 1.5f && buf[i].playerCaused && buf[i].age < 0.5f;
            }
        }
        REQUIRE(foundFirst);
        REQUIRE(foundMerged);
    }

    SECTION("NearestDanger picks the closest episode")
    {
        float d = -1;
        REQUIRE(Traffic::NearestDanger(buf, Vector3(0, 0, 0), d) == -1);
        REQUIRE(d == Approx(-1.0f));
        Traffic::AddDangerEvent(buf, Vector3(300, 0, 0), 1.0f, false);
        Traffic::AddDangerEvent(buf, Vector3(100, 0, 0), 1.0f, false);
        REQUIRE(Traffic::NearestDanger(buf, Vector3(0, 0, 0), d) == 1);
        REQUIRE(d == Approx(100.0f));
    }
}

TEST_CASE("Traffic - danger reaction decision", "[game][guerrilla]")
{
    TrafficTuning t; // dangerRadius 200, close 60; severity 1 leaves both as-is

    SECTION("guards: kind, cooldown latch, severity, disabled radius")
    {
        REQUIRE(Traffic::DecideDangerReaction(50, 1, TKPatrol, TSDriving, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(50, 1, TKConvoy, TSDriving, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(50, 1, TKCiv, TSDriving, 10, 0, t) == TDRNone); // latch holds
        REQUIRE(Traffic::DecideDangerReaction(50, 0, TKCiv, TSDriving, 0, 0, t) == TDRNone);
        t.dangerRadius = 0; // the feature-off case: no distance reacts
        REQUIRE(Traffic::DecideDangerReaction(1, 5, TKCiv, TSDriving, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(1, 5, TKCiv, TSStalled, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(1, 5, TKCiv, TSParking, 0, 0, t) == TDRNone);
    }

    SECTION("commandeer-owned and already-panicked states never react")
    {
        REQUIRE(Traffic::DecideDangerReaction(10, 1, TKCiv, TSStopping, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(10, 1, TKCiv, TSExiting, 0, 0, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(10, 1, TKCiv, TSPanicked, 0, 0, t) == TDRNone);
    }

    SECTION("out of band -> none; severity scales the band, clamped")
    {
        REQUIRE(Traffic::DecideDangerReaction(250, 1, TKCiv, TSDriving, 0, 0.6f, t) == TDRNone);
        // severity 4 caps the scale at DangerScaleMax 1.5: band 300, and
        // 250 falls in the far band (close band 90) -> rush at roll 0.6
        REQUIRE(Traffic::DecideDangerReaction(250, 4, TKCiv, TSDriving, 0, 0.6f, t) == TDRRush);
        REQUIRE(Traffic::DecideDangerReaction(310, 4, TKCiv, TSDriving, 0, 0.6f, t) == TDRNone);
        // a whisper floors at DangerScaleMin 0.5: band 100, close band 30,
        // so 90 is a far-band rush at roll 0.6
        REQUIRE(Traffic::DecideDangerReaction(110, 0.01f, TKCiv, TSDriving, 0, 0.6f, t) == TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(90, 0.01f, TKCiv, TSDriving, 0, 0.6f, t) == TDRRush);
    }

    SECTION("band edges are inclusive at severity 1")
    {
        // the reaction band edge: exactly 200 reacts, past it does not
        REQUIRE(Traffic::DecideDangerReaction(200, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRUTurn);
        REQUIRE(Traffic::DecideDangerReaction(200.5f, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRNone);
        // the close-band edge: exactly 60 is close (cower at roll 0.2),
        // just past it is far (U-turn at the same roll)
        REQUIRE(Traffic::DecideDangerReaction(60, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRCower);
        REQUIRE(Traffic::DecideDangerReaction(60.5f, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRUTurn);
    }

    SECTION("roll band edges")
    {
        // close: [0, 0.5) cower, [0.5, 0.8) bail, [0.8, 1) U-turn
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.5f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.8f, t) == TDRUTurn);
        // far: [0, 0.5) U-turn, [0.5, 0.75) rush, [0.75, 1) cower
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDriving, 0, 0.5f, t) == TDRRush);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDriving, 0, 0.75f, t) == TDRCower);
    }

    SECTION("close band rolls: cower / bail / U-turn")
    {
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRCower);
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.49f, t) == TDRCower);
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.6f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSDriving, 0, 0.9f, t) == TDRUTurn);
    }

    SECTION("far band rolls: U-turn / rush / cower")
    {
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDriving, 0, 0.2f, t) == TDRUTurn);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDriving, 0, 0.6f, t) == TDRRush);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDriving, 0, 0.9f, t) == TDRCower);
    }

    SECTION("TSArrived reacts like driving, minus the meaningless rush")
    {
        REQUIRE(Traffic::DecideDangerReaction(30, 1, TKCiv, TSArrived, 0, 0.2f, t) == TDRCower);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSArrived, 0, 0.2f, t) == TDRUTurn);
        // the rush slot maps to cower at the destination: the arrival
        // ladder would override a same-leg re-issue anyway
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSArrived, 0, 0.6f, t) == TDRCower);
    }

    SECTION("severity mappings")
    {
        REQUIRE(Traffic::DangerSeverityFromAudible(Traffic::DangerRifleAudibleFire) == Approx(1.0f));
        REQUIRE(Traffic::DangerSeverityFromAudible(2.0f * Traffic::DangerRifleAudibleFire) == Approx(2.0f));
        REQUIRE(Traffic::DangerSeverityFromAudible(0.0f) == Approx(0.0f));
        // non-explosive impacts (every landed bullet reaches the blast
        // hook) map to no episode, whatever their indirect damage
        REQUIRE(Traffic::DangerSeverityFromBlast(false, 100.0f, 10.0f) == Approx(0.0f));
        // an explosive below the power floor is a firecracker, not a blast
        REQUIRE(Traffic::DangerSeverityFromBlast(true, 1.0f, 0.5f) == Approx(0.0f));
        // a hand grenade (indirectHit ~9, range ~5) maxes the band
        REQUIRE(Traffic::DangerSeverityFromBlast(true, 9.0f, 5.0f) == Approx(Traffic::DangerExplosionSeverity));
        // the power floor is inclusive
        REQUIRE(Traffic::DangerSeverityFromBlast(true, 2.0f, 2.0f) == Approx(Traffic::DangerExplosionSeverity));
    }

    SECTION("wreck danger-source cutoff")
    {
        // default ttl 20 x scale 3 = 60 s of radiating, then set dressing
        REQUIRE(Traffic::WreckDangerLive(0.0f, t));
        REQUIRE(Traffic::WreckDangerLive(59.0f, t));
        REQUIRE_FALSE(Traffic::WreckDangerLive(60.0f, t));
        TrafficTuning off;
        off.dangerTtl = 0;
        REQUIRE_FALSE(Traffic::WreckDangerLive(0.0f, off));
    }

    SECTION("ended and parked states bail regardless of the roll")
    {
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSStalled, 0, 0.1f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSLingering, 0, 0.9f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSParking, 0, 0.5f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDwelling, 0, 0.5f, t) == TDRBail);
        REQUIRE(Traffic::DecideDangerReaction(150, 1, TKCiv, TSDeparting, 0, 0.5f, t) == TDRBail);
        // ... but never out of band
        REQUIRE(Traffic::DecideDangerReaction(250, 1, TKCiv, TSStalled, 0, 0.1f, t) == TDRNone);
    }

    SECTION("reaction names")
    {
        REQUIRE(std::string(Traffic::DangerReactionName(TDRCower)) == "cower");
        REQUIRE(std::string(Traffic::DangerReactionName(TDRUTurn)) == "uturn");
        REQUIRE(std::string(Traffic::DangerReactionName(TDRRush)) == "rush");
        REQUIRE(std::string(Traffic::DangerReactionName(TDRBail)) == "bail");
        REQUIRE(std::string(Traffic::DangerReactionName(TDRNone)) == "none");
        REQUIRE(std::string(Traffic::DangerReactionName(-7)) == "none");
    }
}

TEST_CASE("Traffic - event/kind name mapping and handler bookkeeping", "[game][guerrilla]")
{
    REQUIRE(Traffic::EventTypeFromName("spawned") == TESpawned);
    REQUIRE(Traffic::EventTypeFromName("Despawned") == TEDespawned);
    REQUIRE(Traffic::EventTypeFromName("commandeered") == TECommandeered);
    REQUIRE(Traffic::EventTypeFromName("arrived") == TEArrived);
    REQUIRE(Traffic::EventTypeFromName("driverKilled") == TEDriverKilled);
    REQUIRE(Traffic::EventTypeFromName("parked") == TEParked);
    REQUIRE(Traffic::EventTypeFromName("Departed") == TEDeparted);
    REQUIRE(Traffic::EventTypeFromName("bailed") == TEBailed);
    REQUIRE(Traffic::EventTypeFromName("BAILED") == TEBailed); // case-insensitive like the others
    REQUIRE(Traffic::EventTypeFromName("Panicked") == TEPanicked);
    REQUIRE(Traffic::EventTypeFromName("bogus") == -1);
    REQUIRE(Traffic::EventTypeFromName(nullptr) == -1);

    REQUIRE(Traffic::KindFromName("civ") == TKCiv);
    REQUIRE(Traffic::KindFromName("PATROL") == TKPatrol);
    REQUIRE(Traffic::KindFromName("convoy") == TKConvoy);
    REQUIRE(Traffic::KindFromName("all") == -1);
    REQUIRE(std::string(Traffic::KindName(TKCiv)) == "civ");
    REQUIRE(std::string(Traffic::KindName(TKPatrol)) == "patrol");
    REQUIRE(std::string(Traffic::KindName(TKConvoy)) == "convoy");
    REQUIRE(std::string(Traffic::KindName(-1)) == "all");
    // events append ahead of the count sentinel only (handlers serialize by
    // name, so the order is a convention, not a save contract): bailed from
    // the convoy-discipline landing, then panicked from the danger landing
    REQUIRE((int)TEBailed == (int)NTrafficEventTypes - 2);
    REQUIRE((int)TEPanicked == (int)NTrafficEventTypes - 1);

    Traffic t;
    t.SetEventHandler(TESpawned, "hSpawned");
    t.SetEventHandler(TEDriverKilled, "hKilled");
    REQUIRE(Str(t.GetEventHandler(TESpawned)) == "hSpawned");
    REQUIRE(Str(t.GetEventHandler(TEDriverKilled)) == "hKilled");
    REQUIRE(Str(t.GetEventHandler(TEArrived)).empty());
    t.SetEventHandler((TrafficEventType)99, "x"); // out of range: ignored
    REQUIRE(Str(t.GetEventHandler((TrafficEventType)99)).empty());
    t.Clear();
    REQUIRE(Str(t.GetEventHandler(TESpawned)).empty());
}

TEST_CASE("Traffic - queries are safe on an empty service", "[game][guerrilla]")
{
    Traffic t;
    REQUIRE(t.Count(-1) == 0);
    REQUIRE(t.Count(TKCiv) == 0);
    REQUIRE(t.NEntries() == 0);
    REQUIRE(t.EntryVehicle(0) == nullptr);
    REQUIRE(t.EntryVehicle(-1) == nullptr);
    TrafficKind k;
    int o, d;
    TrafficState s;
    REQUIRE_FALSE(t.FindEntry(nullptr, k, o, d, s));
    REQUIRE_FALSE(t.IsTrafficGroup(nullptr));
    REQUIRE_FALSE(t.Release(nullptr));
    // inactive without a registry: the per-frame hook is a no-op
    ZoneRegistry::Instance().Clear();
    REQUIRE_FALSE(t.IsActive());
    t.Simulate(10.0f);
    REQUIRE(t.NEntries() == 0);
}

TEST_CASE("Traffic - ambush queue drain", "[game][guerrilla]")
{
    Traffic t;
    t.QueueAmbushForTest(Vector3(1200.0f, 0.0f, 1300.0f), TKPatrol, "Outpost");
    t.QueueAmbushForTest(Vector3(4200.0f, 12.0f, 900.0f), TKConvoy, "Depot");

    AutoArray<TrafficAmbush> out;
    out.Add(TrafficAmbush()); // stale row from an earlier tick: must be dropped
    t.ConsumeAmbushes(out);
    REQUIRE(out.Size() == 2);
    // queue order is wipe order - the alert tick attributes them one by one
    REQUIRE(out[0].pos.X() == Approx(1200.0f));
    REQUIRE(out[0].pos.Z() == Approx(1300.0f));
    REQUIRE(out[0].kind == TKPatrol);
    REQUIRE(Str(out[0].originZone) == "Outpost");
    REQUIRE(out[1].pos.X() == Approx(4200.0f));
    REQUIRE(out[1].pos.Y() == Approx(12.0f));
    REQUIRE(out[1].pos.Z() == Approx(900.0f));
    REQUIRE(out[1].kind == TKConvoy);
    REQUIRE(Str(out[1].originZone) == "Depot");

    // the drain empties the queue: a second tick sees nothing
    t.ConsumeAmbushes(out);
    REQUIRE(out.Size() == 0);

    // and Clear() drops anything queued since
    t.QueueAmbushForTest(Vector3(500.0f, 0.0f, 500.0f), TKPatrol, "Outpost");
    t.Clear();
    t.ConsumeAmbushes(out);
    REQUIRE(out.Size() == 0);
}

TEST_CASE("Traffic - handlers and rows survive save/load, dead links are pruned", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "traffic-roundtrip.bin";

    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    trafficMaxCiv = 2;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class TownA { name=\"TownA\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={1000.0, 1000.0, 50.0}; };\n"
                         "        class TownB { name=\"TownB\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={3000.0, 1000.0, 50.0}; };\n"
                         "    };\n"
                         "};\n";
    {
        QIStream in(config, strlen(config));
        ExtParsMission.Parse(in);
    }
    ZoneRegistry::Instance().InitMission();
    REQUIRE(ZoneRegistry::Instance().FindZoneIndex("TownA") >= 0);

    {
        Traffic t;
        t.LoadFromConfig();
        REQUIRE(t.Tuning().maxCiv == 2);
        t.SetEventHandler(TESpawned, "gmEvtTrSpawned = gmEvtTrSpawned + [_this]");
        t.SetEventHandler(TEDriverKilled, "_this call GM_fnCivKilledEH");
        t.SetEventHandler(TEParked, "gmEvtTrParked = gmEvtTrParked + [_this]");
        t.SetEventHandler(TEDeparted, "gmEvtTrDeparted = gmEvtTrDeparted + [_this]");
        t.SetEventHandler(TEBailed, "gmEvtTrBailed = gmEvtTrBailed + [_this]");
        // a live row whose hull link is null (no live world in a unit test):
        // must be pruned on load rather than resurrected as a ghost entry
        t.MarkEntryForTest(TKCiv, "TownA", "TownB", 2);
        REQUIRE(t.NEntries() == 1);
        REQUIRE(t.Count(TKCiv) == 1);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(t.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    Traffic loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    CHECK(Str(loaded.GetEventHandler(TESpawned)) == "gmEvtTrSpawned = gmEvtTrSpawned + [_this]");
    CHECK(Str(loaded.GetEventHandler(TEDriverKilled)) == "_this call GM_fnCivKilledEH");
    CHECK(Str(loaded.GetEventHandler(TEParked)) == "gmEvtTrParked = gmEvtTrParked + [_this]");
    CHECK(Str(loaded.GetEventHandler(TEDeparted)) == "gmEvtTrDeparted = gmEvtTrDeparted + [_this]");
    CHECK(Str(loaded.GetEventHandler(TEBailed)) == "gmEvtTrBailed = gmEvtTrBailed + [_this]");
    CHECK(Str(loaded.GetEventHandler(TEArrived)).empty());
    // the config was re-read on the second pass
    CHECK(loaded.Tuning().maxCiv == 2);
    // null-hull row pruned
    CHECK(loaded.NEntries() == 0);
    CHECK(loaded.Count(-1) == 0);

    // an empty archive (pre-traffic save) loads clean
    {
        Traffic fresh;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        ParamArchiveLoad lar;
        REQUIRE(lar.LoadBin(archivePath.string().c_str()));
        lar.FirstPass();
        REQUIRE(fresh.Serialize(lar) == LSOK);
        lar.SecondPass();
        REQUIRE(fresh.Serialize(lar) == LSOK);
        CHECK(fresh.NEntries() == 0);
        CHECK(Str(fresh.GetEventHandler(TESpawned)).empty());
        // a pre-park save carries no onParked/onDeparted keys: both load empty
        CHECK(Str(fresh.GetEventHandler(TEParked)).empty());
        CHECK(Str(fresh.GetEventHandler(TEDeparted)).empty());
        // a pre-bail save carries no onBailed key either: it loads empty, not garbage
        CHECK(Str(fresh.GetEventHandler(TEBailed)).empty());
    }

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}

TEST_CASE("Traffic - the ambush queue survives save/load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "traffic-ambush-roundtrip.bin";

    {
        // a patrol wiped inside the tick-interval window before the save:
        // the stimulus must reach the AlertMachine after the load, not be
        // forgotten with the session
        Traffic t;
        t.QueueAmbushForTest(Vector3(1200.0f, 0.0f, 1300.0f), TKPatrol, "Outpost");
        t.QueueAmbushForTest(Vector3(4200.0f, 12.0f, 900.0f), TKConvoy, "Depot");

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(t.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    Traffic loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    AutoArray<TrafficAmbush> out;
    loaded.ConsumeAmbushes(out);
    REQUIRE(out.Size() == 2);
    CHECK(out[0].pos.X() == Approx(1200.0f));
    CHECK(out[0].pos.Z() == Approx(1300.0f));
    CHECK(out[0].kind == TKPatrol);
    CHECK(Str(out[0].originZone) == "Outpost");
    CHECK(out[1].pos.X() == Approx(4200.0f));
    CHECK(out[1].pos.Y() == Approx(12.0f));
    CHECK(out[1].kind == TKConvoy);
    CHECK(Str(out[1].originZone) == "Depot");

    // an archive with no Ambushes subclass (a pre-traffic-alert save) loads
    // an empty queue rather than failing
    {
        Traffic fresh;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        ParamArchiveLoad lar;
        REQUIRE(lar.LoadBin(archivePath.string().c_str()));
        lar.FirstPass();
        REQUIRE(fresh.Serialize(lar) == LSOK);
        lar.SecondPass();
        REQUIRE(fresh.Serialize(lar) == LSOK);
        AutoArray<TrafficAmbush> none;
        fresh.ConsumeAmbushes(none);
        CHECK(none.Size() == 0);
    }

    // scrub the process-wide state other tests expect untouched (the load's
    // second pass runs LoadFromConfig/ApplyPendingLoad against the global
    // registry singleton)
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}

// ---------------------------------------------------------------------------
// issue #55 (the complaints book): the ordinances are read before they are
// obeyed, the parked census, roadside recovery, the coroner's names
// ---------------------------------------------------------------------------

namespace
{
bool AnyWarningMentions(const AutoArray<RString>& warnings, const char* key)
{
    for (int i = 0; i < warnings.Size(); i++)
    {
        if (strstr((const char*)warnings[i], key))
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST_CASE("Traffic - config repairs: the transposed band", "[game][guerrilla]")
{
    SECTION("radius under the spawn floor is an empty band: repaired to floor + default width, and said so")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficMinSpawnDist = 1500; trafficRadius = 300; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.minSpawnDist == Approx(1500.0f));
        REQUIRE(tu.radius == Approx(1500.0f + 1200.0f));
        REQUIRE(tu.radius > tu.minSpawnDist);
        REQUIRE(f.traffic.ConfigWarnings().Size() == 1);
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficRadius"));
        // the band the spawn logic derives from it is usable again
        TrafficEffectiveBand b = Traffic::ConfigBand(tu);
        REQUIRE(b.radius - b.minSpawn == Approx(1200.0f));
    }

    SECTION("radius equal to the floor is just as empty")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficMinSpawnDist = 800; trafficRadius = 800; };\n");
        REQUIRE(f.traffic.Tuning().radius == Approx(2000.0f));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficRadius"));
    }

    SECTION("a negative hysteresis (edge inside the band) and a negative floor are floored")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDespawnHysteresis = -200; trafficMinSpawnDist = -50; };\n");
        REQUIRE(f.traffic.Tuning().despawnHysteresis == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().minSpawnDist == Approx(0.0f));
        REQUIRE(f.traffic.Tuning().radius == Approx(1500.0f)); // still above the (repaired) floor: untouched
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficDespawnHysteresis"));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficMinSpawnDist"));
        REQUIRE_FALSE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficRadius"));
    }
}

TEST_CASE("Traffic - config repairs: the day window written in hours", "[game][guerrilla]")
{
    SECTION("hours past 1 are read as hours, not as a day fraction")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDayStart = 6; trafficDayEnd = 21; };\n");
        REQUIRE(f.traffic.Tuning().dayStart == Approx(6.0f / 24.0f));
        REQUIRE(f.traffic.Tuning().dayEnd == Approx(21.0f / 24.0f));
        REQUIRE(f.traffic.ConfigWarnings().Size() == 2);
        // and the trapezoid the keys feed reads noon as full day, not night
        TrafficModulationInput noon;
        float civ, patrol;
        Traffic::ModulationFactors(noon, f.traffic.Tuning(), civ, patrol);
        REQUIRE(civ == Approx(1.0f));
    }

    SECTION("a window that closes before it opens falls back to the defaults")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDayStart = 0.9; trafficDayEnd = 0.2; };\n");
        REQUIRE(f.traffic.Tuning().dayStart == Approx(6.0f / 24.0f));
        REQUIRE(f.traffic.Tuning().dayEnd == Approx(21.0f / 24.0f));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficDayStart"));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficDayEnd"));
    }

    SECTION("a legal fractional window repairs nothing")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficDayStart = 0.3; trafficDayEnd = 0.8; };\n");
        REQUIRE(f.traffic.Tuning().dayStart == Approx(0.3f));
        REQUIRE(f.traffic.Tuning().dayEnd == Approx(0.8f));
        REQUIRE(f.traffic.ConfigWarnings().Size() == 0);
    }
}

TEST_CASE("Traffic - config repairs: probabilities, the arrival radius, the caps", "[game][guerrilla]")
{
    SECTION("chances are clamped to [0,1] both ways")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficCivChance = 1.5; trafficPatrolChance = -0.2; "
               "trafficConvoyChance = 2; trafficRainCivFade = 3; trafficCivNightScale = -1; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.civChance == Approx(1.0f));
        REQUIRE(tu.patrolChance == Approx(0.0f));
        REQUIRE(tu.convoyChance == Approx(1.0f));
        REQUIRE(tu.rainCivFade == Approx(1.0f));
        REQUIRE(tu.civNightScale == Approx(0.0f));
        REQUIRE(f.traffic.ConfigWarnings().Size() == 5);
    }

    SECTION("a non-positive arrival radius means no car ever arrives: back to the default")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficArriveRadius = 0; };\n");
        REQUIRE(f.traffic.Tuning().arriveRadius == Approx(60.0f));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficArriveRadius"));
    }

    SECTION("negative caps and durations are floored at zero, each with its own line")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficMaxParked = -1; trafficWreckClearAfter = -5; "
               "trafficStallTimeout = -1; trafficFleeDist = -10; };\n");
        const TrafficTuning& tu = f.traffic.Tuning();
        REQUIRE(tu.maxParked == 0);
        REQUIRE(tu.wreckClearAfter == Approx(0.0f));
        REQUIRE(tu.stallTimeout == Approx(0.0f));
        REQUIRE(tu.fleeDist == Approx(0.0f));
        REQUIRE(f.traffic.ConfigWarnings().Size() == 4);
    }

    SECTION("the pre-#55 silent clamps now speak: interval floor, danger close-inside-main")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficInterval = 0; trafficDangerCloseRadius = 500; };\n");
        REQUIRE(f.traffic.Tuning().interval == Approx(0.5f));
        REQUIRE(f.traffic.Tuning().dangerCloseRadius == Approx(200.0f));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficInterval"));
        REQUIRE(AnyWarningMentions(f.traffic.ConfigWarnings(), "trafficDangerCloseRadius"));
    }

    SECTION("a reload clears the previous warnings")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficCivChance = 5; };\n");
        REQUIRE(f.traffic.ConfigWarnings().Size() == 1);
        f.traffic.LoadFromParams(nullptr);
        REQUIRE(f.traffic.ConfigWarnings().Size() == 0);
    }
}

TEST_CASE("Traffic - the parked census", "[game][guerrilla]")
{
    TrafficTuning t; // parkChance 0.6, maxParked 2

    SECTION("room in the census: the plain roll decides")
    {
        REQUIRE(Traffic::DecidePark(0.59f, 0, t));
        REQUIRE(Traffic::DecidePark(0.59f, 1, t));
        REQUIRE_FALSE(Traffic::DecidePark(0.6f, 1, t));
    }

    SECTION("at the cap the roll always loses: the road keeps flowing")
    {
        REQUIRE_FALSE(Traffic::DecidePark(0.0f, 2, t));
        REQUIRE_FALSE(Traffic::DecidePark(0.0f, 7, t));
    }

    SECTION("maxParked 0 never parks; the census-free form is parked = 0")
    {
        TrafficTuning none = t;
        none.maxParked = 0;
        REQUIRE_FALSE(Traffic::DecidePark(0.0f, 0, none));
        REQUIRE(Traffic::DecidePark(0.1f, t) == Traffic::DecidePark(0.1f, 0, t));
        REQUIRE(Traffic::DecidePark(0.9f, t) == Traffic::DecidePark(0.9f, 0, t));
    }

    SECTION("the census counts only civ park-state rows")
    {
        Traffic tr;
        REQUIRE(tr.CountParked() == 0);
        tr.MarkEntryForTest(TKCiv, "A", "B", 1); // TSDriving
        tr.MarkEntryForTest(TKPatrol, "A", "B", 1);
        REQUIRE(tr.Count(-1) == 2);
        REQUIRE(tr.CountParked() == 0);
    }
}

TEST_CASE("Traffic - roadside recovery clock", "[game][guerrilla]")
{
    TrafficTuning t; // wreckClearAfter 1200
    REQUIRE_FALSE(Traffic::ReleasedStale(0.0f, t));
    REQUIRE_FALSE(Traffic::ReleasedStale(1199.9f, t));
    REQUIRE(Traffic::ReleasedStale(1200.0f, t)); // boundary inclusive
    REQUIRE(Traffic::ReleasedStale(5000.0f, t));
    TrafficTuning never = t;
    never.wreckClearAfter = 0;
    REQUIRE_FALSE(Traffic::ReleasedStale(1e9f, never)); // 0 = the distance rules alone
}

TEST_CASE("Traffic - the coroner's names and the diag block", "[game][guerrilla]")
{
    SECTION("every verdict has a distinct name, none reads unknown")
    {
        for (int i = 0; i < NTrafficSpawnFailures; i++)
        {
            std::string name = Traffic::SpawnFailureName(i);
            REQUIRE(name != "unknown");
            for (int j = 0; j < i; j++)
            {
                REQUIRE(name != Traffic::SpawnFailureName(j));
            }
        }
        REQUIRE(Str(Traffic::SpawnFailureName(TSFNone)) == "none");
        REQUIRE(Str(Traffic::SpawnFailureName(TSFNoRoute)) == "noRoute");
        REQUIRE(Str(Traffic::SpawnFailureName(TSFGroupBudget)) == "groupBudget");
        REQUIRE(Str(Traffic::SpawnFailureName(NTrafficSpawnFailures)) == "unknown");
        REQUIRE(Str(Traffic::SpawnFailureName(-1)) == "unknown");
    }

    SECTION("a fresh service has an empty report")
    {
        Traffic t;
        const TrafficDiag& d = t.Diag();
        REQUIRE(d.passes == 0);
        REQUIRE(d.spawned == 0);
        REQUIRE(d.failed == 0);
        REQUIRE(d.lastFailKind == -1);
        REQUIRE(d.lastFail == TSFNone);
        REQUIRE_FALSE(d.hasCivRoute);
        REQUIRE(t.NReleased() == 0);
        REQUIRE(t.NFleeing() == 0);
    }
}
