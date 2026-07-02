#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

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
                           "    class Zones { class A { name=\"A\"; }; };\n"
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
    REQUIRE_FALSE(machine.BreakLatched());
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

TEST_CASE("AlertMachine - undercover break conditions", "[game][guerrilla]")
{
    AlertFixture f;
    f.Load(kAlertConfig);
    AutoArray<AlertEventRecord> fired;

    AlertTickInputs in = MakeInputs(f.registry);
    in.playerValid = true;
    // player closest to Depot (zone 1)
    in.playerX = 4800.0f;
    in.playerZ = 4900.0f;

    SECTION("no break while not undercover")
    {
        in.playerInVehicle = true;
        in.breakRequested = true;
        in.breakReason = "fired";
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE_FALSE(f.machine.BreakLatched());
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
    }

    SECTION("mounting a vehicle breaks cover and spikes the nearest zone")
    {
        in.undercover = true;
        in.playerInVehicle = true;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(Str(fired[fired.Size() - 1].reason) == "vehicle");
        REQUIRE(f.machine.BreakLatched());
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f)); // GM_AL_HEAT_BREAK
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));  // not the nearest

        // fires once: no repeat while the script has not dropped gmUndercover
        fired.Clear();
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 0);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(25.0f));
    }

    SECTION("a script break request (fired EH) carries its reason")
    {
        in.undercover = true;
        in.breakRequested = true;
        in.breakReason = "fired";
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(Str(fired[fired.Size() - 1].reason) == "fired");
    }

    SECTION("the latch re-arms when cover drops and is re-established")
    {
        in.undercover = true;
        in.playerInVehicle = true;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(f.machine.BreakLatched());

        // script reacted: gmUndercover = false
        in.undercover = false;
        in.playerInVehicle = false;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE_FALSE(f.machine.BreakLatched());

        // cover re-established, new mount breaks again
        fired.Clear();
        in.undercover = true;
        in.playerInVehicle = true;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(f.registry.GetZone(1)->heat == Approx(50.0f));
    }

    SECTION("no heat spike without a valid player, event still fires")
    {
        in.playerValid = false;
        in.undercover = true;
        in.playerInVehicle = true;
        f.machine.EvaluateAlert(in, 5.0f, f.registry, fired);
        REQUIRE(CountBroken(fired) == 1);
        REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));
        REQUIRE(f.registry.GetZone(1)->heat == Approx(0.0f));
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
