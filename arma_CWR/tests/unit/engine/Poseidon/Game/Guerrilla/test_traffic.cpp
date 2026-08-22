#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
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
        REQUIRE(tu.stallTimeout == Approx(90.0f));
        REQUIRE(tu.arriveRadius == Approx(60.0f));
        REQUIRE(tu.maxLegs == 3);
        REQUIRE(tu.commandeerRadius == Approx(25.0f));
        REQUIRE(tu.commandeerLaneHalfWidth == Approx(4.0f));
        REQUIRE(tu.commandeerStopDelay == Approx(2.5f));
        REQUIRE(tu.fleeDist == Approx(150.0f));
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
               "trafficCommandeerRadius = 30; trafficFleeDist = 200; };\n");
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
        REQUIRE_FALSE(f.traffic.IsActive()); // disabled regardless of the registry
    }

    SECTION("sanity floors")
    {
        TrafficFixture f;
        f.Load("class CfgGuerrillaZones { trafficInterval = 0; trafficMaxCiv = -2; trafficMaxLegs = -1; };\n");
        REQUIRE(f.traffic.Tuning().interval == Approx(0.5f));
        REQUIRE(f.traffic.Tuning().maxCiv == 0);
        REQUIRE(f.traffic.Tuning().maxLegs == 0);
    }
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

TEST_CASE("Traffic - far-despawn edge with hysteresis", "[game][guerrilla]")
{
    TrafficTuning t; // 1500 + 300
    float edge = 1800.0f;
    REQUIRE_FALSE(Traffic::ShouldDespawn((edge - 1) * (edge - 1), t));
    REQUIRE_FALSE(Traffic::ShouldDespawn(edge * edge, t));
    REQUIRE(Traffic::ShouldDespawn((edge + 1) * (edge + 1), t));
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

TEST_CASE("Traffic - event/kind name mapping and handler bookkeeping", "[game][guerrilla]")
{
    REQUIRE(Traffic::EventTypeFromName("spawned") == TESpawned);
    REQUIRE(Traffic::EventTypeFromName("Despawned") == TEDespawned);
    REQUIRE(Traffic::EventTypeFromName("commandeered") == TECommandeered);
    REQUIRE(Traffic::EventTypeFromName("arrived") == TEArrived);
    REQUIRE(Traffic::EventTypeFromName("driverKilled") == TEDriverKilled);
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
    }

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}
