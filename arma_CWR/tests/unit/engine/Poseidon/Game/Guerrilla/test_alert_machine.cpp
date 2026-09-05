#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/Traffic.hpp> // TrafficAmbush / TKPatrol / TKConvoy
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
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

// Two well-separated zones so the nearest-zone break spike is unambiguous.
const char* kAlertConfig = "class CfgGuerrillaZones\n"
                           "{\n"
                           "    class Zones\n"
                           "    {\n"
                           "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                           "heat=0; position[]={1000.0, 1000.0, 50.0}; };\n"
                           "        class Depot   { name=\"Depot\";   type=\"CAMP\";    owner=\"EAST\"; "
                           "heat=0; position[]={5000.0, 5000.0, 50.0}; };\n"
                           "    };\n"
                           "};\n";

const char* kTunedConfig = "class CfgGuerrillaZones\n"
                           "{\n"
                           "    alertInterval = 2;\n"
                           "    alertYellowKnows = 0.25;\n"
                           "    alertRedKnows = 2.5;\n"
                           "    alertWindowSeconds = 40;\n"
                           "    alertHeatYellow = 6;\n"
                           "    alertHeatRed = 20;\n"
                           "    alertHeatBreak = 50;\n"
                           "    undercoverHeatWitness = 11;\n"
                           "    trafficAmbushWindow = 90;\n"
                           "    trafficAmbushRadius = 800;\n"
                           "    trafficAmbushHeat = 7;\n"
                           "    class Zones { class A { name=\"A\"; }; };\n"
                           "};\n";

// Ambush attribution needs an explicitly mixed owner table: a NEUTRAL town
// sitting closer to the wreck than the occupier outpost it belongs to, plus
// a second occupier zone far outside the traffic band (the origin fallback).
const char* kAmbushConfig = "class CfgGuerrillaZones\n"
                            "{\n"
                            "    class Zones\n"
                            "    {\n"
                            "        class Town    { name=\"Town\";    type=\"CITY\";    owner=\"NEUTRAL\"; "
                            "heat=0; position[]={1000.0, 1000.0, 0.0}; };\n"
                            "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                            "heat=0; position[]={2000.0, 1000.0, 0.0}; };\n"
                            "        class Depot   { name=\"Depot\";   type=\"CAMP\";    owner=\"EAST\"; "
                            "heat=0; position[]={9000.0, 9000.0, 0.0}; };\n"
                            "    };\n"
                            "};\n";

// kAmbushConfig with the attribution bound tightened below the test wreck
// distances (the trafficAmbushRadius override seam)
const char* kAmbushNarrowConfig = "class CfgGuerrillaZones\n"
                                  "{\n"
                                  "    trafficAmbushRadius = 200;\n"
                                  "    class Zones\n"
                                  "    {\n"
                                  "        class Town    { name=\"Town\";    type=\"CITY\";    owner=\"NEUTRAL\"; "
                                  "heat=0; position[]={1000.0, 1000.0, 0.0}; };\n"
                                  "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                                  "heat=0; position[]={2000.0, 1000.0, 0.0}; };\n"
                                  "        class Depot   { name=\"Depot\";   type=\"CAMP\";    owner=\"EAST\"; "
                                  "heat=0; position[]={9000.0, 9000.0, 0.0}; };\n"
                                  "    };\n"
                                  "};\n";

// keeps the parsed ParamFile alive for the duration of a test
struct AlertFixture
{
    ParamFile file;
    ZoneRegistry registry;
    AlertMachine machine;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        const ParamEntry* zones = file.FindEntry("CfgGuerrillaZones");
        registry.LoadFromParams(zones, nullptr);
        machine.LoadFromParams(zones);
    }
};

AlertTickInputs MakeInputs(const ZoneRegistry& registry)
{
    AlertTickInputs in;
    in.zones.Resize(registry.NZones());
    for (int i = 0; i < in.zones.Size(); i++)
    {
        in.zones[i] = AlertZoneInputs();
    }
    return in;
}

int CountChanged(const AutoArray<AlertEventRecord>& fired, int zoneIndex, int oldState, int newState)
{
    int count = 0;
    for (int i = 0; i < fired.Size(); i++)
    {
        const AlertEventRecord& ev = fired[i];
        if (ev.type == AEAlertChanged && ev.zoneIndex == zoneIndex && ev.oldState == oldState &&
            ev.newState == newState)
        {
            count++;
        }
    }
    return count;
}

int CountBroken(const AutoArray<AlertEventRecord>& fired)
{
    int count = 0;
    for (int i = 0; i < fired.Size(); i++)
    {
        if (fired[i].type == AEUndercoverBroken)
        {
            count++;
        }
    }
    return count;
}

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// a violent patrol/convoy end as the Traffic drain hands it over: the wreck
// sits at (easting, northing), the origin zone is the spawn-time attribution
// fallback for a wreck out of reach of every occupier zone
TrafficAmbush MakeAmbush(float x, float z, TrafficKind kind, const char* originZone)
{
    TrafficAmbush am;
    am.pos = Vector3(x, 0.0f, z);
    am.kind = kind;
    am.originZone = originZone;
    return am;
}

UCCompromise MakeCompromise(float x, float z, const char* reason, bool firstEver)
{
    UCCompromise uc;
    uc.witnessPos = Vector3(x, 0.0f, z);
    uc.reason = reason;
    uc.firstEver = firstEver;
    return uc;
}

} // namespace

TEST_CASE("AlertMachine - defaults match alert.sqs tunables", "[game][guerrilla]")
{
    AlertMachine machine;
    machine.LoadFromParams(nullptr);

    const AlertTuning& t = machine.Tuning();
    REQUIRE(t.alertInterval == Approx(5.0f));       // GM_AL_TICK
    REQUIRE(t.alertYellowKnows == Approx(0.5f));    // YELLOW band floor
    REQUIRE(t.alertRedKnows == Approx(1.5f));       // RED band floor
    REQUIRE(t.alertWindowSeconds == Approx(20.0f)); // GM_AL_WINDOW
    REQUIRE(t.alertHeatYellow == Approx(4.0f));     // GM_AL_HEAT_YELLOW
    REQUIRE(t.alertHeatRed == Approx(15.0f));       // GM_AL_HEAT_RED
    REQUIRE(t.alertHeatBreak == Approx(25.0f));     // GM_AL_HEAT_BREAK
    REQUIRE(t.undercoverHeatWitness == Approx(8.0f));
    REQUIRE(t.trafficAmbushWindow == Approx(120.0f));  // wiped-patrol memory
    REQUIRE(t.trafficAmbushRadius == Approx(1500.0f)); // attribution bound
    REQUIRE(t.trafficAmbushHeat == Approx(4.0f));      // Heat per wreck
}

TEST_CASE("AlertMachine - config keys override the defaults", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kTunedConfig);

    const AlertTuning& t = f.machine.Tuning();
    REQUIRE(t.alertInterval == Approx(2.0f));
    REQUIRE(t.alertYellowKnows == Approx(0.25f));
    REQUIRE(t.alertRedKnows == Approx(2.5f));
    REQUIRE(t.alertWindowSeconds == Approx(40.0f));
    REQUIRE(t.alertHeatYellow == Approx(6.0f));
    REQUIRE(t.alertHeatRed == Approx(20.0f));
    REQUIRE(t.alertHeatBreak == Approx(50.0f));
    REQUIRE(t.undercoverHeatWitness == Approx(11.0f));
    REQUIRE(t.trafficAmbushWindow == Approx(90.0f));
    REQUIRE(t.trafficAmbushRadius == Approx(800.0f));
    REQUIRE(t.trafficAmbushHeat == Approx(7.0f));
}

TEST_CASE("AlertMachine - inactive registry is a no-op", "[game][guerrilla]")
{
    ZoneRegistry registry;
    registry.LoadFromParams(nullptr, nullptr);
    AlertMachine machine;

    AlertTickInputs in;
    in.undercover = true;
    in.breakRequested = true;
    in.breakReason = "fired";

    AutoArray<AlertEventRecord> fired;
    machine.EvaluateAlert(in, 5.0f, registry, fired);

    REQUIRE(fired.Size() == 0);
    REQUIRE(machine.GetZoneState(0) == ASGreen);
    Vector3 pos;
    REQUIRE_FALSE(machine.GetLastKnown(0, pos));
}

TEST_CASE("AlertMachine - escalation thresholds, heat spikes and events", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    SECTION("below the YELLOW band stays GREEN")
    {
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 0.49f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(fired.Size() == 0);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));
    }

    SECTION("GREEN -> YELLOW at the band floor (know >= 0.5)")
    {
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 0.5f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f));
        REQUIRE(f.registry.GetZone(0)->heat == Approx(4.0f)); // GM_AL_HEAT_YELLOW
        REQUIRE(CountChanged(fired, 0, ASGreen, ASYellow) == 1);
        // the other zone is untouched
        REQUIRE(f.machine.GetZoneState(1) == ASGreen);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
    }

    SECTION("YELLOW -> RED at the band floor (know >= 1.5)")
    {
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 1.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        fired.Clear();
        in.zones[0].knows = 1.5f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(4.0f + 15.0f)); // + GM_AL_HEAT_RED
        REQUIRE(CountChanged(fired, 0, ASYellow, ASRed) == 1);
    }

    SECTION("GREEN -> RED directly spikes only the RED amount")
    {
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 4.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f));
        REQUIRE(CountChanged(fired, 0, ASGreen, ASRed) == 1);
    }

    SECTION("steady state fires no events and no heat")
    {
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 4.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(fired.Size() == 0);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f));
    }
}

TEST_CASE("AlertMachine - YELLOW disengage window escalates to RED", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.zones[0].knows = 1.0f; // YELLOW band, below RED

    // entering YELLOW starts the 20 s window
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASYellow);
    REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f));

    // three more 5 s ticks bleed the countdown but stay YELLOW
    for (int tick = 0; tick < 3; tick++)
    {
        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);
        REQUIRE(fired.Size() == 0);
    }
    REQUIRE(f.machine.GetZoneTimer(0) == Approx(5.0f));

    // the expiring tick escalates YELLOW -> RED with the RED heat spike
    fired.Clear();
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    REQUIRE(f.machine.GetZoneTimer(0) == Approx(0.0f));
    REQUIRE(CountChanged(fired, 0, ASYellow, ASRed) == 1);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(4.0f + 15.0f));
}

TEST_CASE("AlertMachine - detection is recoverable (downward transitions)", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.zones[0].knows = 4.0f;
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    float heatAfterRed = f.registry.GetZone(0)->heat;

    SECTION("RED -> YELLOW on partial contact restarts the window, no heat")
    {
        fired.Clear();
        in.zones[0].knows = 1.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f)); // window restarted
        REQUIRE(CountChanged(fired, 0, ASRed, ASYellow) == 1);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(heatAfterRed)); // no downward spike

        // and YELLOW -> GREEN once contact is lost entirely
        fired.Clear();
        in.zones[0].knows = 0.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(0.0f));
        REQUIRE(CountChanged(fired, 0, ASYellow, ASGreen) == 1);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(heatAfterRed));
    }

    SECTION("RED -> GREEN directly when contact is fully lost")
    {
        fired.Clear();
        in.zones[0].knows = 0.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(CountChanged(fired, 0, ASRed, ASGreen) == 1);
    }

    SECTION("re-escalating after recovery spikes heat again")
    {
        fired.Clear();
        in.zones[0].knows = 0.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        in.zones[0].knows = 4.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(heatAfterRed + 15.0f));
    }
}

TEST_CASE("AlertMachine - last-known position bookkeeping", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;
    Vector3 pos;

    // nothing recorded before any qualifying contact
    REQUIRE_FALSE(f.machine.GetLastKnown(0, pos));

    AlertTickInputs in = MakeInputs(f.registry);
    in.zones[0].knows = 1.0f;
    in.zones[0].hasLastKnown = true;
    in.zones[0].lastKnown = Vector3(1200.0f, 55.0f, 1300.0f);
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);

    REQUIRE(f.machine.GetLastKnown(0, pos));
    REQUIRE(pos.X() == Approx(1200.0f));
    REQUIRE(pos.Y() == Approx(55.0f));
    REQUIRE(pos.Z() == Approx(1300.0f));
    REQUIRE_FALSE(f.machine.GetLastKnown(1, pos)); // other zone untouched

    // a sub-threshold report does not overwrite the stored position
    in.zones[0].knows = 0.1f;
    in.zones[0].lastKnown = Vector3(9000.0f, 0.0f, 9000.0f);
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASGreen);
    REQUIRE(f.machine.GetLastKnown(0, pos));
    REQUIRE(pos.X() == Approx(1200.0f)); // retained across the decay

    // a new qualifying contact refreshes it
    in.zones[0].knows = 2.0f;
    in.zones[0].lastKnown = Vector3(1500.0f, 60.0f, 1600.0f);
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetLastKnown(0, pos));
    REQUIRE(pos.X() == Approx(1500.0f));
}

TEST_CASE("AlertMachine - a wiped patrol floors the zone into YELLOW", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    // wreck 360 m from Outpost (zone 0), inside the default 1500 m band
    AlertTickInputs in = MakeInputs(f.registry);
    in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost"));

    SECTION("the drain tick escalates, spikes Heat once and points lastKnown at the wreck")
    {
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f)); // disengage window armed
        REQUIRE(CountChanged(fired, 0, ASGreen, ASYellow) == 1);
        // GM_AL_HEAT_YELLOW edge spike + trafficAmbushHeat for the wreck
        REQUIRE(f.registry.GetZone(0)->heat == Approx(4.0f + 4.0f));
        // the QRF drives to the wreck, not to the player
        Vector3 pos;
        REQUIRE(f.machine.GetLastKnown(0, pos));
        REQUIRE(pos.X() == Approx(1200.0f));
        REQUIRE(pos.Z() == Approx(1300.0f));
        // the far zone hears nothing
        REQUIRE(f.machine.GetZoneState(1) == ASGreen);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
        REQUIRE_FALSE(f.machine.GetLastKnown(1, pos));
    }

    SECTION("the held floor stays edge-triggered: one event, one Heat spike")
    {
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        // two more ticks inside the disengage window, nothing new drained
        in.ambushes.Clear();
        for (int tick = 0; tick < 2; tick++)
        {
            fired.Clear();
            f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
            REQUIRE(f.machine.GetZoneState(0) == ASYellow);
            REQUIRE(fired.Size() == 0);
            REQUIRE(f.registry.GetZone(0)->heat == Approx(8.0f)); // no growth
        }
        // a wreck memory is not an engaged contact: the disengage window
        // stays armed instead of bleeding toward RED
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f));
    }
}

TEST_CASE("AlertMachine - a wiped convoy floors the zone straight into RED", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKConvoy, "Outpost"));
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);

    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    REQUIRE(CountChanged(fired, 0, ASGreen, ASRed) == 1);
    // GM_AL_HEAT_RED edge spike (no YELLOW spike) + trafficAmbushHeat
    REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f + 4.0f));
    Vector3 pos;
    REQUIRE(f.machine.GetLastKnown(0, pos));
    REQUIRE(pos.X() == Approx(1200.0f));
    REQUIRE(pos.Z() == Approx(1300.0f));
}

TEST_CASE("AlertMachine - the ambush floor outlives the disengage window", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost"));
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASYellow);
    in.ambushes.Clear(); // drained: later ticks carry no new stimulus

    SECTION("the held floor holds YELLOW steady and never bleeds into RED")
    {
        // the floor (120 s) far outlives alertWindowSeconds (20 s), but a
        // wreck memory is not an engaged contact: the disengage window only
        // bleeds under LIVE gathered knowledge, so a single wipe must not
        // oscillate YELLOW-RED and re-dispatch the QRF every window
        for (int tick = 0; tick < 8; tick++)
        {
            fired.Clear();
            f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
            REQUIRE(f.machine.GetZoneState(0) == ASYellow);
            REQUIRE(fired.Size() == 0);
        }
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f));  // still armed
        REQUIRE(f.registry.GetZone(0)->heat == Approx(8.0f)); // no RED spike
    }

    SECTION("a live contact under the floor resumes the countdown into RED")
    {
        // the garrison actually engages: gathered YELLOW-band knowledge
        // bleeds the window exactly as it did before the floor existed
        in.zones[0].knows = 1.0f;
        for (int tick = 0; tick < 3; tick++)
        {
            fired.Clear();
            f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
            REQUIRE(f.machine.GetZoneState(0) == ASYellow);
        }
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(5.0f));

        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
        REQUIRE(CountChanged(fired, 0, ASYellow, ASRed) == 1);
        // YELLOW edge + wreck heat from the drain tick, then the RED edge
        REQUIRE(f.registry.GetZone(0)->heat == Approx(4.0f + 4.0f + 15.0f));
    }

    SECTION("the zone calms once the window has fully decayed")
    {
        // use-then-decrement: the floor covers exactly window/dt ticks; the
        // drain tick was the first, so floorTicks - 1 more still run on it
        const int floorTicks = (int)(f.machine.Tuning().trafficAmbushWindow / 5.0f);
        for (int tick = 0; tick < floorTicks - 1; tick++)
        {
            fired.Clear();
            f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        }
        REQUIRE(f.machine.GetZoneState(0) != ASGreen); // floor still holding

        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(f.machine.GetZoneTimer(0) == Approx(0.0f));

        // and it stays calm with nothing left to act on
        for (int tick = 0; tick < 3; tick++)
        {
            fired.Clear();
            f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
            REQUIRE(f.machine.GetZoneState(0) == ASGreen);
            REQUIRE(fired.Size() == 0);
        }
        // the wreck fix is a memory, not an alert: it survives the calm
        Vector3 pos;
        REQUIRE(f.machine.GetLastKnown(0, pos));
        REQUIRE(pos.X() == Approx(1200.0f));
    }
}

TEST_CASE("AlertMachine - gathered knowledge composes with the ambush floor", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;
    Vector3 pos;

    AlertTickInputs in = MakeInputs(f.registry);

    SECTION("a live contact above the floor dominates it")
    {
        in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost")); // YELLOW floor
        in.zones[0].knows = 2.0f;                                           // RED band
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
        REQUIRE(CountChanged(fired, 0, ASGreen, ASRed) == 1);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f + 4.0f));
    }

    SECTION("a qualifying live fix overwrites the wreck position")
    {
        in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost"));
        in.zones[0].knows = 2.0f;
        in.zones[0].hasLastKnown = true;
        in.zones[0].lastKnown = Vector3(1500.0f, 60.0f, 1600.0f);
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetLastKnown(0, pos));
        REQUIRE(pos.X() == Approx(1500.0f)); // the man beats the wreck
        REQUIRE(pos.Z() == Approx(1600.0f));
    }

    SECTION("a sub-band live fix cannot overwrite the wreck while the floor holds")
    {
        in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost"));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);

        // a garrison holding the zone in band by the FLOOR must not promote
        // its own sub-band guess over the wreck the QRF is driving to
        fired.Clear();
        in.ambushes.Clear();
        in.zones[0].knows = 0.1f;
        in.zones[0].hasLastKnown = true;
        in.zones[0].lastKnown = Vector3(9000.0f, 0.0f, 9000.0f);
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow); // still floored
        REQUIRE(f.machine.GetLastKnown(0, pos));
        REQUIRE(pos.X() == Approx(1200.0f));
        REQUIRE(pos.Z() == Approx(1300.0f));
    }
}

TEST_CASE("AlertMachine - ambush attribution chain", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAmbushConfig); // Town NEUTRAL, Outpost EAST, Depot EAST (far)
    AutoArray<AlertEventRecord> fired;
    Vector3 pos;

    AlertTickInputs in = MakeInputs(f.registry);

    SECTION("the nearest OCCUPIER zone wins over a nearer neutral one")
    {
        // 100 m from Town, 900 m from Outpost: the town owns no garrison to
        // alert, so the outpost gets the call
        in.ambushes.Add(MakeAmbush(1100.0f, 1000.0f, TKPatrol, "Outpost"));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(1) == ASYellow);
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(f.machine.GetZoneState(2) == ASGreen);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(8.0f));
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));
        REQUIRE(f.machine.GetLastKnown(1, pos));
        REQUIRE(pos.X() == Approx(1100.0f));
        REQUIRE_FALSE(f.machine.GetLastKnown(0, pos));
    }

    SECTION("a wreck beyond the band falls back to the spawn origin by name")
    {
        // 5000 m from Outpost, 5657 m from Depot: nothing inside 1500 m
        in.ambushes.Add(MakeAmbush(5000.0f, 5000.0f, TKConvoy, "Depot"));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(2) == ASRed);
        REQUIRE(CountChanged(fired, 2, ASGreen, ASRed) == 1);
        REQUIRE(f.registry.GetZone(2)->heat == Approx(15.0f + 4.0f));
        REQUIRE(f.machine.GetZoneState(0) == ASGreen);
        REQUIRE(f.machine.GetZoneState(1) == ASGreen);
        REQUIRE(f.machine.GetLastKnown(2, pos));
        REQUIRE(pos.X() == Approx(5000.0f));
        REQUIRE(pos.Z() == Approx(5000.0f));
    }

    SECTION("out of band with no resolvable origin is dropped silently")
    {
        in.ambushes.Add(MakeAmbush(5000.0f, 5000.0f, TKPatrol, "Nowhere"));
        in.ambushes.Add(MakeAmbush(5000.0f, 5000.0f, TKConvoy, ""));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(fired.Size() == 0);
        for (int i = 0; i < 3; i++)
        {
            REQUIRE(f.machine.GetZoneState(i) == ASGreen);
            REQUIRE(f.registry.GetZone(i)->heat == Approx(0.0f));
            REQUIRE_FALSE(f.machine.GetLastKnown(i, pos));
        }
    }

    SECTION("the origin fallback drops a zone the occupier no longer holds")
    {
        // 900 m from Town but 1900 m from Outpost: the occupier scan finds
        // nothing, and the spawn-time origin has since flipped to NEUTRAL.
        // Alerting it would QRF a town the occupier does not hold (the
        // queue survives saves, zones flip mid-route), so the stimulus is
        // dropped instead
        in.ambushes.Add(MakeAmbush(100.0f, 1000.0f, TKPatrol, "Town"));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(fired.Size() == 0);
        for (int i = 0; i < 3; i++)
        {
            REQUIRE(f.machine.GetZoneState(i) == ASGreen);
            REQUIRE(f.registry.GetZone(i)->heat == Approx(0.0f));
        }
    }

    SECTION("a tightened trafficAmbushRadius pushes a near wreck to the fallback")
    {
        AlertFixture nf;
        nf.Load(kAmbushNarrowConfig); // the same zone table, radius 200 m
        AutoArray<AlertEventRecord> nfired;
        AlertTickInputs nin = MakeInputs(nf.registry);
        // 360 m from Outpost: inside the default 1500 m bound, outside the
        // tightened 200 m one - the scan misses and the (occupier-held)
        // Depot origin gets the call instead
        nin.ambushes.Add(MakeAmbush(2360.0f, 1000.0f, TKPatrol, "Depot"));
        nf.machine.EvaluateAlert(nin, 5.0f, nf.registry, nfired);
        REQUIRE(nf.machine.GetZoneState(1) == ASGreen);  // Outpost missed
        REQUIRE(nf.machine.GetZoneState(2) == ASYellow); // Depot floored
        REQUIRE(CountChanged(nfired, 2, ASGreen, ASYellow) == 1);
    }
}

TEST_CASE("AlertMachine - repeat ambushes keep costing Heat", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;
    Vector3 pos;

    // tick 1: a convoy wipe pins RED (RED edge spike + wreck heat)
    AlertTickInputs in = MakeInputs(f.registry);
    in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKConvoy, "Outpost"));
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f + 4.0f));

    // tick 2: a patrol wiped in the same zone: no FSM edge left to spike,
    // but the wreck still costs trafficAmbushHeat - patrol farming inside
    // a held window is not free
    fired.Clear();
    in.ambushes.Clear();
    in.ambushes.Add(MakeAmbush(1400.0f, 1500.0f, TKPatrol, "Outpost"));
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(fired.Size() == 0);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(15.0f + 4.0f + 4.0f));
    // the milder patrol floor neither softens the RED floor nor steals the
    // QRF fix from the severer convoy wreck ...
    REQUIRE(f.machine.GetLastKnown(0, pos));
    REQUIRE(pos.X() == Approx(1200.0f));

    // ... but it re-arms the decay window in full: the floor now expires
    // 120 s after tick 2, not after tick 1.  23 contact-free ticks (115 s)
    // keep RED; the 24th calms - one tick later than the tick-1 window
    in.ambushes.Clear();
    for (int tick = 0; tick < 23; tick++)
    {
        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    }
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    fired.Clear();
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASGreen);
}

TEST_CASE("AlertMachine - convoy floor expiry under live partial contact", "[game][guerrilla]")
{
    // when the RED floor expires while the garrison still holds a live
    // YELLOW-band contact, the zone takes the pre-existing partial-contact
    // path: RED de-escalates to YELLOW with a re-armed window and can
    // escalate back after it (the accepted RED-YELLOW oscillation edge
    // documented in STATUS.md, not a feature-specific behaviour)
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKConvoy, "Outpost"));
    in.zones[0].knows = 1.0f; // live partial contact throughout
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);
    in.ambushes.Clear();

    // 23 more floored ticks: RED holds (the floor dominates gathered 1.0)
    for (int tick = 0; tick < 23; tick++)
    {
        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
    }

    // expiry tick: know falls to the gathered 1.0 - partial contact,
    // RED -> YELLOW with a fresh window, no heat on de-escalation
    fired.Clear();
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASYellow);
    REQUIRE(CountChanged(fired, 0, ASRed, ASYellow) == 1);
    REQUIRE(f.machine.GetZoneTimer(0) == Approx(20.0f));
}

TEST_CASE("AlertMachine - undercover break gate (no campaign latch)", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);

    SECTION("no marking while not undercover")
    {
        in.breakRequested = true;
        in.breakReason = "fired";
        REQUIRE_FALSE(f.machine.BreakShouldMark(in));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
    }

    SECTION("a break request with no witnesses stays silent")
    {
        // event and Heat flow only from compromise notifications - a shot
        // nobody witnessed is heard, not identified
        in.undercover = true;
        in.breakRequested = true;
        in.breakReason = "fired";
        REQUIRE(f.machine.BreakShouldMark(in));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
    }

    SECTION("a second break with cover held reaches a group that learned the face in between")
    {
        // issue #19 regression: the old campaign latch armed on the first
        // in-cover request and re-armed only when gmUndercover dropped;
        // under the keep-cover lifecycle it never drops, so every
        // gmBreakUndercover after the campaign's first was ignored.

        // first break: one witness group, the campaign-first compromise
        in.undercover = true;
        in.breakRequested = true;
        in.breakReason = "fired";
        REQUIRE(f.machine.BreakShouldMark(in));
        in.compromises.Add(MakeCompromise(4800.0f, 4900.0f, "fired", true));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f)); // GM_AL_HEAT_BREAK

        // a second group gains a knowledge record between the breaks; the
        // second request must still pass the mark gate (the old gate
        // consulted the latch the tick above had set, and returned false)
        fired.Clear();
        in.compromises.Clear();
        in.breakRequested = true;
        in.breakReason = "fired";
        REQUIRE(f.machine.BreakShouldMark(in));
        // ... so MarkAllWitnessesCompromised runs; per-group idempotence
        // means only the NEW group turns compromised, and its (non-first)
        // notification drains through this tick
        in.compromises.Add(MakeCompromise(1100.0f, 900.0f, "fired", false));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);                      // event stays edge-triggered
        REQUIRE(f.registry.GetZone(0)->heat == Approx(8.0f));  // undercoverHeatWitness
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f)); // first witness zone unchanged
    }
}

TEST_CASE("AlertMachine - undercover compromise drain", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.undercover = true;

    SECTION("the campaign-first compromise fires the event and spikes the witness zone")
    {
        // witness closest to Depot (zone 1)
        in.compromises.Add(MakeCompromise(4800.0f, 4900.0f, "weapon", true));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(Str(fired[fired.Size() - 1].reason) == "weapon");
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f)); // GM_AL_HEAT_BREAK
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));  // not the nearest
    }

    SECTION("a later witness raises quiet heat, no event")
    {
        // witness closest to Outpost (zone 0)
        in.compromises.Add(MakeCompromise(1100.0f, 900.0f, "slung", false));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(8.0f)); // undercoverHeatWitness
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
    }

    SECTION("a mixed batch in one tick heats each witness's own zone")
    {
        in.compromises.Add(MakeCompromise(4800.0f, 4900.0f, "vehicle", true));
        in.compromises.Add(MakeCompromise(1100.0f, 900.0f, "vehicle", false));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(Str(fired[fired.Size() - 1].reason) == "vehicle");
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f));
        REQUIRE(f.registry.GetZone(0)->heat == Approx(8.0f));
    }

    SECTION("ticks without compromises add nothing")
    {
        in.compromises.Add(MakeCompromise(4800.0f, 4900.0f, "weapon", true));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        fired.Clear();
        in.compromises.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f));
    }

    SECTION("compromises drain in the same tick as a break request")
    {
        // a pending gmBreakUndercover and a drained compromise in the same
        // tick: one event
        in.breakRequested = true;
        in.breakReason = "fired";
        in.compromises.Add(MakeCompromise(4800.0f, 4900.0f, "fired", true));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f));
    }
}

TEST_CASE("AlertMachine - event handler bookkeeping and Clear", "[game][guerrilla]")
{
    REQUIRE(AlertMachine::EventTypeFromName("alertChanged") == AEAlertChanged);
    REQUIRE(AlertMachine::EventTypeFromName("UNDERCOVERBROKEN") == AEUndercoverBroken);
    REQUIRE(AlertMachine::EventTypeFromName("noSuchEvent") == -1);
    REQUIRE(AlertMachine::EventTypeFromName(nullptr) == -1);

    AlertFixture f;
    f.Load(kAlertConfig);

    f.machine.SetEventHandler(AEAlertChanged, "hint \"alert\"");
    REQUIRE(Str(f.machine.GetEventHandler(AEAlertChanged)) == "hint \"alert\"");

    // drive some state in before clearing
    AutoArray<AlertEventRecord> fired;
    AlertTickInputs in = MakeInputs(f.registry);
    in.zones[0].knows = 4.0f;
    in.zones[0].hasLastKnown = true;
    in.zones[0].lastKnown = Vector3(1.0f, 2.0f, 3.0f);
    f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
    REQUIRE(f.machine.GetZoneState(0) == ASRed);

    f.machine.Clear();
    REQUIRE(Str(f.machine.GetEventHandler(AEAlertChanged)).empty());
    REQUIRE(f.machine.GetZoneState(0) == ASGreen);
    Vector3 pos;
    REQUIRE_FALSE(f.machine.GetLastKnown(0, pos));

    // out-of-range queries are safe and default to GREEN / none
    REQUIRE(f.machine.GetZoneState(-1) == ASGreen);
    REQUIRE(f.machine.GetZoneState(99) == ASGreen);
    REQUIRE_FALSE(f.machine.GetLastKnown(99, pos));
    REQUIRE(f.machine.GetZoneTimer(99) == Approx(0.0f));
}

TEST_CASE("AlertMachine - FSM state, handlers and break flags survive save/load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "alert-machine-roundtrip.bin";

    {
        AlertFixture f;
        f.Load(kAlertConfig);
        AutoArray<AlertEventRecord> fired;

        // tick 1: Outpost straight to RED with a last-known fix; Depot into
        // YELLOW (window = 20 s)
        AlertTickInputs in = MakeInputs(f.registry);
        in.playerValid = true;
        in.playerX = 1000.0f;
        in.playerZ = 1000.0f;
        in.undercover = true;
        in.breakRequested = true;
        in.breakReason = "fired";
        in.zones[0].knows = 4.0f;
        in.zones[0].hasLastKnown = true;
        in.zones[0].lastKnown = Vector3(1200.0f, 55.0f, 1300.0f);
        in.zones[1].knows = 1.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);

        // tick 2: hold contact so Depot's window bleeds 20 -> 15
        in.breakRequested = false;
        in.breakReason = RString();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASRed);
        REQUIRE(f.machine.GetZoneState(1) == ASYellow);
        REQUIRE(f.machine.GetZoneTimer(1) == Approx(15.0f));

        f.machine.SetEventHandler(AEAlertChanged, "gmEvtAlert = gmEvtAlert + [_this]");
        f.machine.SetEventHandler(AEUndercoverBroken, "hBroken");
        // a gmBreakUndercover still waiting for its consuming tick
        f.machine.RequestBreak("fired");

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.machine.Serialize(ar, f.registry) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    // fresh machine; the registry stands in for the config-rebuilt zone
    // table the second load pass matches the saved rows against by name
    AlertFixture f2;
    f2.Load(kAlertConfig);
    AlertMachine loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
    }

    CHECK(loaded.GetZoneState(0) == ASRed);
    CHECK(loaded.GetZoneState(1) == ASYellow);
    CHECK(loaded.GetZoneTimer(1) == Approx(15.0f)); // disengage window mid-bleed

    Vector3 pos;
    REQUIRE(loaded.GetLastKnown(0, pos));
    CHECK(pos.X() == Approx(1200.0f));
    CHECK(pos.Y() == Approx(55.0f));
    CHECK(pos.Z() == Approx(1300.0f));
    CHECK_FALSE(loaded.GetLastKnown(1, pos));

    CHECK(Str(loaded.GetEventHandler(AEAlertChanged)) == "gmEvtAlert = gmEvtAlert + [_this]");
    CHECK(Str(loaded.GetEventHandler(AEUndercoverBroken)) == "hBroken");

    // the not-yet-consumed break request survives
    CHECK(loaded.BreakPending());
    CHECK(Str(loaded.BreakReason()) == "fired");

    std::filesystem::remove(archivePath);
}

TEST_CASE("AlertMachine - the ambush stimulus floor survives save/load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "alert-machine-ambush-roundtrip.bin";

    {
        AlertFixture f;
        f.Load(kAlertConfig);
        AutoArray<AlertEventRecord> fired;

        // one drain tick: Outpost floored into YELLOW, 115 s of window left
        AlertTickInputs in = MakeInputs(f.registry);
        in.ambushes.Add(MakeAmbush(1200.0f, 1300.0f, TKPatrol, "Outpost"));
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.machine.Serialize(ar, f.registry) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    AlertFixture f2;
    f2.Load(kAlertConfig);
    AlertMachine loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
    }

    CHECK(loaded.GetZoneState(0) == ASYellow);
    CHECK(loaded.GetZoneTimer(0) == Approx(20.0f));
    Vector3 pos;
    REQUIRE(loaded.GetLastKnown(0, pos));
    CHECK(pos.X() == Approx(1200.0f));
    CHECK(pos.Z() == Approx(1300.0f));

    // the floor itself is only observable through behaviour: a tick with no
    // gathered knowledge at all must NOT calm the zone
    AutoArray<AlertEventRecord> fired;
    AlertTickInputs in = MakeInputs(f2.registry);
    loaded.EvaluateAlert(in, 5.0f, f2.registry, fired);
    CHECK(loaded.GetZoneState(0) == ASYellow);
    // floor-only YELLOW: the disengage window stays armed, no bleed
    CHECK(loaded.GetZoneTimer(0) == Approx(20.0f));
    CHECK(fired.Size() == 0);

    // ... and it expires on the saved countdown, not on a fresh window: 115 s
    // left means 23 floored ticks in all, the 24th calms the zone
    for (int tick = 0; tick < 22; tick++)
    {
        fired.Clear();
        loaded.EvaluateAlert(in, 5.0f, f2.registry, fired);
    }
    CHECK(loaded.GetZoneState(0) != ASGreen);
    fired.Clear();
    loaded.EvaluateAlert(in, 5.0f, f2.registry, fired);
    CHECK(loaded.GetZoneState(0) == ASGreen);

    std::filesystem::remove(archivePath);
}

TEST_CASE("AlertMachine - a pre-traffic-alert save loads with no floor", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "alert-machine-old-save.bin";

    {
        // a zone in YELLOW from an ordinary garrison contact: both stimulus
        // fields sit at their defaults, and the defaulted Serialize overloads
        // omit default-valued entries entirely - the archive written here has
        // the exact shape of a save from before the traffic-alert feature
        AlertFixture f;
        f.Load(kAlertConfig);
        AutoArray<AlertEventRecord> fired;
        AlertTickInputs in = MakeInputs(f.registry);
        in.zones[0].knows = 1.0f;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.GetZoneState(0) == ASYellow);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.machine.Serialize(ar, f.registry) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    AlertFixture f2;
    f2.Load(kAlertConfig);
    AlertMachine loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar, f2.registry) == LSOK);
    }

    CHECK(loaded.GetZoneState(0) == ASYellow);
    CHECK(loaded.GetZoneTimer(0) == Approx(20.0f));

    // no floor was read back: the very next contact-free tick calms the zone
    AutoArray<AlertEventRecord> fired;
    AlertTickInputs in = MakeInputs(f2.registry);
    loaded.EvaluateAlert(in, 5.0f, f2.registry, fired);
    CHECK(loaded.GetZoneState(0) == ASGreen);
    CHECK(CountChanged(fired, 0, ASYellow, ASGreen) == 1);

    std::filesystem::remove(archivePath);
}
