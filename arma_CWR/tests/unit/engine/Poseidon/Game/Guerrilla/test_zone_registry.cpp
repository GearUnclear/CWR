#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

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

// Mirrors the Phase-1 GM_ZONES seed (guerrilla-mode/.../init.sqs) plus a far
// zone for reveal-radius coverage.  position[] is authored in getPos order
// [easting, northing, elevation].
const char* kDemoConfig = "class CfgGuerrillaZones\n"
                          "{\n"
                          "    tickInterval = 3; zoneArea = 150; revealRadius = 1500; cacheRadius = 800;\n"
                          "    supportRate = 5; supportFlip = 60; heatCapSpike = 40; defaultIncome = 25;\n"
                          "    holdCount = 3;\n"
                          "    class Zones\n"
                          "    {\n"
                          "        class Camp    { name=\"Camp\";    type=\"CAMP\";    owner=\"GUER\";    "
                          "garrison=0; support=100; income=0;  heat=0; marker=\"gmZoneMarker_0\"; "
                          "position[]={6519.04, 6473.68, 149.66}; };\n"
                          "        class Village { name=\"Village\"; type=\"CITY\";    owner=\"NEUTRAL\"; "
                          "garrison=0; support=20;  income=0;  heat=0; marker=\"gmZoneMarker_1\"; "
                          "position[]={6300.0, 6600.0, 150.0}; };\n"
                          "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"EAST\";    "
                          "garrison=8; support=0;   income=0;  heat=0; marker=\"gmZoneMarker_2\"; "
                          "position[]={6750.0, 6300.0, 150.0}; };\n"
                          "        class FarPost { name=\"FarPost\"; type=\"OUTPOST\"; owner=\"EAST\";    "
                          "garrison=4; support=0;   income=25; heat=0; marker=\"gmZoneMarker_3\"; "
                          "position[]={20000.0, 20000.0, 0.0}; };\n"
                          "    };\n"
                          "};\n"
                          "class CfgGuerrillaFactions\n"
                          "{\n"
                          "    class East\n"
                          "    {\n"
                          "        side=\"EAST\";\n"
                          "        tiers[]={\"SoldierEB\",\"SoldierESoldier\",\"SoldierECrew\"};\n"
                          "        tierThresholds[]={3,5};\n"
                          "        officer=\"OfficerE\";\n"
                          "        vehicles[]={\"Ural\",\"BMP\"};\n"
                          "        vehicleThreshold=3;\n"
                          "    };\n"
                          "    class Guer\n"
                          "    {\n"
                          "        side=\"GUER\";\n"
                          "        holdClass=\"SoldierGB\";\n"
                          "        recruitFighter=\"SoldierGB\";\n"
                          "        recruitSpecialist=\"SoldierGMG\";\n"
                          "        companionClass=\"SoldierGB\";\n"
                          "        baseWeapon=\"AK47\";\n"
                          "        baseMagazine=\"AK47Mag\";\n"
                          "    };\n"
                          "};\n";

// keeps the parsed ParamFile alive for the duration of a test
struct RegistryFixture
{
    ParamFile file;
    ZoneRegistry registry;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"));
    }
};

ZoneTickInputs MakeInputs(const ZoneRegistry& registry, int playerAtZone, int guerAtZone)
{
    ZoneTickInputs in;
    in.guerPresent.Resize(registry.NZones());
    for (int i = 0; i < in.guerPresent.Size(); i++)
    {
        in.guerPresent[i] = false;
    }
    if (playerAtZone >= 0)
    {
        const ZoneRecord* z = registry.GetZone(playerAtZone);
        REQUIRE(z != nullptr);
        in.playerValid = true;
        in.playerX = z->pos.X();
        in.playerZ = z->pos.Z();
    }
    if (guerAtZone >= 0)
    {
        in.guerPresent[guerAtZone] = true;
    }
    return in;
}

int CountEvents(const AutoArray<ZoneEventRecord>& fired, ZoneEventType type, int zoneIndex)
{
    int count = 0;
    for (int i = 0; i < fired.Size(); i++)
    {
        if (fired[i].type == type && fired[i].zoneIndex == zoneIndex)
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

TEST_CASE("ZoneRegistry - absent config leaves the registry inactive", "[game][guerrilla]")
{
    ZoneRegistry registry;
    registry.LoadFromParams(nullptr, nullptr);

    REQUIRE(registry.NZones() == 0);
    REQUIRE_FALSE(registry.IsActive());
    REQUIRE(registry.GetZone(0) == nullptr);
    REQUIRE(registry.FindZoneIndex("Camp") == -1);
    REQUIRE(Str(registry.FactionTierClass("EAST", 5)).empty());
    REQUIRE(Str(registry.FactionValue("GUER", "holdClass")).empty());
    REQUIRE(Str(registry.FactionVehicle("EAST", 5)).empty());

    // Simulate is a no-op when inactive (must not touch the world)
    registry.Simulate(100.0f);

    ZoneTickInputs in;
    AutoArray<ZoneEventRecord> fired;
    registry.EvaluateTick(in, fired);
    REQUIRE(fired.Size() == 0);
}

TEST_CASE("ZoneRegistry - parses zones, tuning and position mapping", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);

    REQUIRE(f.registry.NZones() == 4);
    REQUIRE(f.registry.IsActive());

    SECTION("zone fields")
    {
        const ZoneRecord* camp = f.registry.GetZone(0);
        REQUIRE(camp != nullptr);
        REQUIRE(Str(camp->name) == "Camp");
        REQUIRE(Str(camp->type) == "CAMP");
        REQUIRE(Str(camp->owner) == "GUER");
        REQUIRE(camp->garrison == Approx(0.0f));
        REQUIRE(camp->support == Approx(100.0f));
        REQUIRE(camp->income == Approx(0.0f));
        REQUIRE(camp->heat == Approx(0.0f));
        REQUIRE(Str(camp->marker) == "gmZoneMarker_0");

        const ZoneRecord* outpost = f.registry.GetZone(2);
        REQUIRE(outpost != nullptr);
        REQUIRE(Str(outpost->owner) == "EAST");
        REQUIRE(outpost->garrison == Approx(8.0f));
    }

    SECTION("position[] getPos order maps to engine X/Z/Y")
    {
        const ZoneRecord* camp = f.registry.GetZone(0);
        REQUIRE(camp != nullptr);
        REQUIRE(camp->pos.X() == Approx(6519.04f)); // easting
        REQUIRE(camp->pos.Z() == Approx(6473.68f)); // northing
        REQUIRE(camp->pos.Y() == Approx(149.66f));  // elevation
    }

    SECTION("tuning")
    {
        const ZoneTuning& t = f.registry.Tuning();
        REQUIRE(t.tickInterval == Approx(3.0f));
        REQUIRE(t.zoneArea == Approx(150.0f));
        REQUIRE(t.revealRadius == Approx(1500.0f));
        REQUIRE(t.cacheRadius == Approx(800.0f));
        REQUIRE(t.supportRate == Approx(5.0f));
        REQUIRE(t.supportFlip == Approx(60.0f));
        REQUIRE(t.heatCapSpike == Approx(40.0f));
        REQUIRE(t.defaultIncome == Approx(25.0f));
        REQUIRE(t.holdCount == Approx(3.0f));
    }

    SECTION("missing tuning entries keep defaults")
    {
        const char* minimalConfig = "class CfgGuerrillaZones\n"
                                    "{\n"
                                    "    zoneArea = 200;\n"
                                    "    class Zones { class A { name=\"A\"; }; };\n"
                                    "};\n";
        RegistryFixture minimal;
        minimal.Load(minimalConfig);
        REQUIRE(minimal.registry.NZones() == 1);
        REQUIRE(minimal.registry.Tuning().zoneArea == Approx(200.0f));
        REQUIRE(minimal.registry.Tuning().revealRadius == Approx(1500.0f)); // default
    }

    SECTION("name lookup")
    {
        REQUIRE(f.registry.FindZoneIndex("Camp") == 0);
        REQUIRE(f.registry.FindZoneIndex("Village") == 1);
        REQUIRE(f.registry.FindZoneIndex("outpost") == 2); // case-insensitive
        REQUIRE(f.registry.FindZoneIndex("NoSuchZone") == -1);
    }
}

TEST_CASE("ZoneRegistry - faction queries", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);

    SECTION("tier selection by war level")
    {
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 1)) == "SoldierEB");
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 2)) == "SoldierEB");
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 3)) == "SoldierESoldier");
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 4)) == "SoldierESoldier");
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 5)) == "SoldierECrew");
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 10)) == "SoldierECrew");
        REQUIRE(Str(f.registry.FactionTierClass("WEST", 1)).empty()); // unknown side
    }

    SECTION("vehicle threshold")
    {
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 1)) == "Ural");
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 2)) == "Ural");
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 3)) == "BMP");
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 8)) == "BMP");
    }

    SECTION("named values")
    {
        REQUIRE(Str(f.registry.FactionValue("EAST", "officer")) == "OfficerE");
        REQUIRE(Str(f.registry.FactionValue("GUER", "holdClass")) == "SoldierGB");
        REQUIRE(Str(f.registry.FactionValue("GUER", "recruitFighter")) == "SoldierGB");
        REQUIRE(Str(f.registry.FactionValue("GUER", "recruitSpecialist")) == "SoldierGMG");
        REQUIRE(Str(f.registry.FactionValue("GUER", "companionClass")) == "SoldierGB");
        REQUIRE(Str(f.registry.FactionValue("GUER", "baseWeapon")) == "AK47");
        REQUIRE(Str(f.registry.FactionValue("GUER", "baseMagazine")) == "AK47Mag");
        REQUIRE(Str(f.registry.FactionValue("GUER", "noSuchKey")).empty());
        REQUIRE(Str(f.registry.FactionValue("WEST", "officer")).empty());
    }
}

TEST_CASE("ZoneRegistry - fog-of-war reveal", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);

    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs in = MakeInputs(f.registry, -1, -1);
    f.registry.EvaluateTick(in, fired);

    // Camp is GUER-owned; Village and Outpost sit within revealRadius of it
    REQUIRE(f.registry.GetZone(0)->revealed);
    REQUIRE(f.registry.GetZone(1)->revealed);
    REQUIRE(f.registry.GetZone(2)->revealed);
    REQUIRE_FALSE(f.registry.GetZone(3)->revealed); // FarPost is out of range

    REQUIRE(CountEvents(fired, ZERevealed, 0) == 1);
    REQUIRE(CountEvents(fired, ZERevealed, 1) == 1);
    REQUIRE(CountEvents(fired, ZERevealed, 2) == 1);
    REQUIRE(CountEvents(fired, ZERevealed, 3) == 0);

    // the revealed event fires only on the first transition
    fired.Clear();
    f.registry.EvaluateTick(in, fired);
    REQUIRE(CountEvents(fired, ZERevealed, 0) == 0);
    REQUIRE(CountEvents(fired, ZERevealed, 1) == 0);

    // a zone flipping to GUER reveals itself on the next tick
    f.registry.GetZoneMutable(3)->owner = "GUER";
    fired.Clear();
    f.registry.EvaluateTick(in, fired);
    REQUIRE(f.registry.GetZone(3)->revealed);
    REQUIRE(CountEvents(fired, ZERevealed, 3) == 1);
}

TEST_CASE("ZoneRegistry - military capture rules", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);
    const int outpost = 2;

    SECTION("no flip while the occupier reserve remains")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired); // garrison == 8
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
        REQUIRE(CountEvents(fired, ZECaptured, outpost) == 0);
    }

    SECTION("no flip while live occupiers remain")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->liveOccupiers = 5;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
    }

    SECTION("no flip without a GUER unit in the zone area")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, -1);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
    }

    SECTION("no capture math when the player is outside cacheRadius")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        // player parked at FarPost, GUER unit present at the outpost
        ZoneTickInputs in = MakeInputs(f.registry, 3, outpost);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
    }

    SECTION("flip: owner, heat spike, income tap, garrison reset, event")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired);

        const ZoneRecord* z = f.registry.GetZone(outpost);
        REQUIRE(Str(z->owner) == "GUER");
        REQUIRE(z->heat == Approx(40.0f));   // heatCapSpike
        REQUIRE(z->income == Approx(25.0f)); // defaultIncome opened (was 0)
        REQUIRE(z->garrison == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptured, outpost) == 1);
        REQUIRE(CountEvents(fired, ZESupportThreshold, outpost) == 0);
    }

    SECTION("an already-open income tap is preserved on capture")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->income = 55;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(f.registry.GetZone(outpost)->income == Approx(55.0f));
    }
}

TEST_CASE("ZoneRegistry - city support and threshold flip", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);
    const int village = 1;

    ZoneTickInputs present = MakeInputs(f.registry, village, village);
    AutoArray<ZoneEventRecord> fired;

    // support 20 -> 55 over 7 ticks, no flip yet (threshold 60)
    for (int tick = 0; tick < 7; tick++)
    {
        f.registry.EvaluateTick(present, fired);
    }
    REQUIRE(f.registry.GetZone(village)->support == Approx(55.0f));
    REQUIRE(Str(f.registry.GetZone(village)->owner) == "NEUTRAL");
    REQUIRE(CountEvents(fired, ZECaptured, village) == 0);

    // support does not accrue without a GUER unit present
    ZoneTickInputs absent = MakeInputs(f.registry, village, -1);
    fired.Clear();
    f.registry.EvaluateTick(absent, fired);
    REQUIRE(f.registry.GetZone(village)->support == Approx(55.0f));

    // the 8th accruing tick crosses the threshold and flips the city
    fired.Clear();
    f.registry.EvaluateTick(present, fired);
    const ZoneRecord* z = f.registry.GetZone(village);
    REQUIRE(z->support == Approx(60.0f));
    REQUIRE(Str(z->owner) == "GUER");
    REQUIRE(z->heat == Approx(40.0f));
    REQUIRE(z->income == Approx(0.0f)); // CITY income stays with the economy script
    REQUIRE(CountEvents(fired, ZECaptured, village) == 1);
    REQUIRE(CountEvents(fired, ZESupportThreshold, village) == 1);

    // once owned, support stops accruing
    fired.Clear();
    f.registry.EvaluateTick(present, fired);
    REQUIRE(f.registry.GetZone(village)->support == Approx(60.0f));
    REQUIRE(CountEvents(fired, ZECaptured, village) == 0);
}

TEST_CASE("ZoneRegistry - heat clamps", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);

    f.registry.HeatRaise(0, 500.0f);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(100.0f));

    f.registry.HeatDecay(0, 30.0f);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(70.0f));

    f.registry.HeatDecay(0, 500.0f);
    REQUIRE(f.registry.GetZone(0)->heat == Approx(0.0f));

    // out-of-range indices are ignored
    f.registry.HeatRaise(99, 10.0f);
    f.registry.HeatDecay(-1, 10.0f);
}

TEST_CASE("ZoneRegistry - event handler bookkeeping", "[game][guerrilla]")
{
    ZoneRegistry registry;

    REQUIRE(ZoneRegistry::EventTypeFromName("captured") == ZECaptured);
    REQUIRE(ZoneRegistry::EventTypeFromName("SUPPORTTHRESHOLD") == ZESupportThreshold);
    REQUIRE(ZoneRegistry::EventTypeFromName("revealed") == ZERevealed);
    REQUIRE(ZoneRegistry::EventTypeFromName("noSuchEvent") == -1);

    registry.SetEventHandler(ZECaptured, "hint \"flip\"");
    REQUIRE(Str(registry.GetEventHandler(ZECaptured)) == "hint \"flip\"");

    // Clear drops handlers together with the zone tables
    registry.Clear();
    REQUIRE(Str(registry.GetEventHandler(ZECaptured)).empty());
}

TEST_CASE("ZoneRegistry - campaignLoaded notification", "[game][guerrilla]")
{
    ZoneRegistry registry;

    REQUIRE(ZoneRegistry::EventTypeFromName("campaignLoaded") == ZECampaignLoaded);
    REQUIRE(ZoneRegistry::EventTypeFromName("CAMPAIGNLOADED") == ZECampaignLoaded);

    // nothing pending on a fresh registry
    int version = -1;
    REQUIRE_FALSE(registry.ConsumeCampaignLoaded(version));

    // queued by Serialize at the end of a load; consumed exactly once
    registry.MarkCampaignLoaded(1);
    REQUIRE(registry.ConsumeCampaignLoaded(version));
    REQUIRE(version == 1);
    REQUIRE_FALSE(registry.ConsumeCampaignLoaded(version));

    // Clear drops both the handler and any queued notification (the handler
    // survives save/load only through serialization)
    registry.SetEventHandler(ZECampaignLoaded, "call gmOnLoad");
    registry.MarkCampaignLoaded(2);
    registry.Clear();
    REQUIRE(Str(registry.GetEventHandler(ZECampaignLoaded)).empty());
    REQUIRE_FALSE(registry.ConsumeCampaignLoaded(version));
}
