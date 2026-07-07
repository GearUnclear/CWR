#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
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

// Faction-agnostic campaign: OCCUPIER/RESISTANCE owner tokens plus a WEST
// occupier and a CIV resistance selectable by side or by class name.  The
// Legacy zone keeps a literal side string for backward compatibility.
const char* kSwappableConfig =
    "class CfgGuerrillaZones\n"
    "{\n"
    "    class Zones\n"
    "    {\n"
    "        class Base   { name=\"Base\";   type=\"CAMP\";    owner=\"RESISTANCE\"; "
    "position[]={1000.0, 1000.0, 0.0}; };\n"
    "        class Depot  { name=\"Depot\";  type=\"OUTPOST\"; owner=\"OCCUPIER\";   garrison=0; "
    "position[]={1200.0, 1000.0, 0.0}; };\n"
    "        class Town   { name=\"Town\";   type=\"CITY\";    owner=\"NEUTRAL\";    support=55; "
    "position[]={1000.0, 1200.0, 0.0}; };\n"
    "        class Legacy { name=\"Legacy\"; type=\"OUTPOST\"; owner=\"EAST\";       garrison=0; "
    "position[]={1400.0, 1000.0, 0.0}; };\n"
    "    };\n"
    "};\n"
    "class CfgGuerrillaFactions\n"
    "{\n"
    "    class NATO      { side=\"WEST\"; tiers[]={\"SoldierWB\"}; officer=\"OfficerW\"; };\n"
    "    class Partisans { side=\"CIV\";  holdClass=\"Civilian\"; };\n"
    "};\n";

// keeps the parsed ParamFile alive for the duration of a test
struct RegistryFixture
{
    ParamFile file;
    ZoneRegistry registry;

    void Load(const char* config, const char* selOccupier = nullptr, const char* selResistance = nullptr,
              const char* namesClass = nullptr)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"),
                                selOccupier, selResistance, namesClass ? file.FindEntry(namesClass) : nullptr);
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

    SECTION("vehicleThresholds[] ladder reaches vehicles past index 1")
    {
        const char* config = "class CfgGuerrillaZones { class Zones { class A { name=\"A\"; }; }; };\n"
                             "class CfgGuerrillaFactions\n"
                             "{\n"
                             "    class East\n"
                             "    {\n"
                             "        side=\"EAST\";\n"
                             "        vehicles[]={\"UAZ\",\"Ural\",\"BMP\",\"T80\"};\n"
                             "        vehicleThresholds[]={3,5,7};\n"
                             "        vehicleThreshold=5;\n" // ignored when the array is present
                             "    };\n"
                             "};\n";
        RegistryFixture g;
        g.Load(config);
        REQUIRE(Str(g.registry.FactionVehicle("EAST", 1)) == "UAZ");
        REQUIRE(Str(g.registry.FactionVehicle("EAST", 3)) == "Ural");
        REQUIRE(Str(g.registry.FactionVehicle("EAST", 5)) == "BMP");
        REQUIRE(Str(g.registry.FactionVehicle("EAST", 7)) == "T80");
        REQUIRE(Str(g.registry.FactionVehicle("EAST", 10)) == "T80"); // clamped
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

TEST_CASE("ZoneRegistry - OCCUPIER/RESISTANCE owner tokens resolve to the campaign sides", "[game][guerrilla]")
{
    SECTION("defaults when nothing is selected")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig);
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "GUER"); // RESISTANCE token
        REQUIRE(Str(f.registry.GetZone(1)->owner) == "EAST"); // OCCUPIER token
        REQUIRE(Str(f.registry.GetZone(2)->owner) == "NEUTRAL");
        REQUIRE(Str(f.registry.GetZone(3)->owner) == "EAST"); // literal passthrough
        // raw config value survives for re-resolution on savegame load
        REQUIRE(Str(f.registry.GetZone(0)->ownerConfig) == "RESISTANCE");
        REQUIRE(Str(f.registry.GetZone(1)->ownerConfig) == "OCCUPIER");
    }

    SECTION("tokens follow the selected sides, literals stay put")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig, "WEST", "CIV");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "CIV");
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "CIV");
        REQUIRE(Str(f.registry.GetZone(1)->owner) == "WEST");
        REQUIRE(Str(f.registry.GetZone(3)->owner) == "EAST"); // literal passthrough
    }

    SECTION("tokens are case-insensitive")
    {
        RegistryFixture f;
        const char* cfg = "class CfgGuerrillaZones\n"
                          "{\n"
                          "    class Zones { class A { name=\"A\"; owner=\"occupier\"; }; };\n"
                          "};\n";
        f.Load(cfg);
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "EAST");
    }
}

TEST_CASE("ZoneRegistry - faction selection resolves occupier/resistance sides", "[game][guerrilla]")
{
    SECTION("selection by side string")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig, "WEST", "CIV");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "CIV");
    }

    SECTION("selection by faction class name")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig, "NATO", "Partisans");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "CIV");
    }

    SECTION("unmatched or empty selections keep the defaults")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig, "Martians", "");
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
    }

    SECTION("faction queries accept the class name as well as the side")
    {
        RegistryFixture f;
        f.Load(kSwappableConfig);
        REQUIRE(Str(f.registry.FactionTierClass("WEST", 1)) == "SoldierWB");
        REQUIRE(Str(f.registry.FactionTierClass("nato", 1)) == "SoldierWB");
        REQUIRE(Str(f.registry.FactionValue("Partisans", "holdClass")) == "Civilian");
        REQUIRE(Str(f.registry.FactionValue("CIV", "holdClass")) == "Civilian");
        REQUIRE(Str(f.registry.FactionValue("NoSuchFaction", "holdClass")).empty());
    }
}

TEST_CASE("ZoneRegistry - defaultOccupier/defaultResistance config keys", "[game][guerrilla]")
{
    // Sinai-style direct-launch mission: sides picked by the mission config
    // (no new-game UI, so no gmSel* selections), plus a second faction pair
    // so the selection-beats-config precedence is observable.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    defaultOccupier = \"IDF\"; defaultResistance = \"EgyptFrontier\";\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Base  { name=\"Base\";  type=\"CAMP\";    owner=\"RESISTANCE\"; "
                         "position[]={1000.0, 1000.0, 0.0}; };\n"
                         "        class Depot { name=\"Depot\"; type=\"OUTPOST\"; owner=\"OCCUPIER\"; "
                         "position[]={1200.0, 1000.0, 0.0}; };\n"
                         "    };\n"
                         "};\n"
                         "class CfgGuerrillaFactions\n"
                         "{\n"
                         "    class IDF           { side=\"WEST\"; };\n"
                         "    class EgyptFrontier { side=\"EAST\"; };\n"
                         "    class NATO          { side=\"WEST\"; };\n"
                         "    class Partisans     { side=\"CIV\";  };\n"
                         "};\n";

    SECTION("config keys beat the built-in defaults")
    {
        RegistryFixture f;
        f.Load(config);
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "EAST");
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "EAST"); // RESISTANCE token
        REQUIRE(Str(f.registry.GetZone(1)->owner) == "WEST"); // OCCUPIER token
    }

    SECTION("gmSel selections beat the config keys")
    {
        RegistryFixture f;
        f.Load(config, "NATO", "Partisans");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "CIV"); // Partisans, not EgyptFrontier
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "CIV");
    }

    SECTION("unmatched selections fall back to the config keys")
    {
        RegistryFixture f;
        f.Load(config, "Martians", "Martians");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "EAST");
    }

    SECTION("empty selections fall through to the config keys")
    {
        // The new-game UI publishes EMPTY selections when it had no real
        // CfgGuerrillaFactions to offer (built-in fallback cyclers) - the
        // mission's defaultOccupier/defaultResistance must win, exactly as
        // if no selection variables existed at all.
        RegistryFixture f;
        f.Load(config, "", "");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "EAST");
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "EAST"); // RESISTANCE token
        REQUIRE(Str(f.registry.GetZone(1)->owner) == "WEST"); // OCCUPIER token
    }

    SECTION("unmatched config keys keep the built-in defaults")
    {
        const char* badKeys = "class CfgGuerrillaZones\n"
                              "{\n"
                              "    defaultOccupier = \"NoSuchFaction\"; defaultResistance = \"AlsoMissing\";\n"
                              "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                              "};\n"
                              "class CfgGuerrillaFactions\n"
                              "{\n"
                              "    class NATO { side=\"WEST\"; };\n"
                              "};\n";
        RegistryFixture f;
        f.Load(badKeys);
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "EAST");
    }
}

TEST_CASE("ZoneRegistry - capture and reveal follow the swapped sides", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kSwappableConfig, "NATO", "Partisans"); // WEST occupier, CIV resistance
    const int base = 0;
    const int depot = 1;
    const int town = 2;
    const int legacy = 3;

    SECTION("fog-of-war keys on the resistance side")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, -1, -1);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(f.registry.GetZone(base)->revealed); // CIV == resistance
        REQUIRE(f.registry.GetZone(depot)->revealed);
    }

    SECTION("military capture flips occupier-owned zones to the resistance")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, depot, depot);
        f.registry.EvaluateTick(in, fired);
        const ZoneRecord* z = f.registry.GetZone(depot);
        REQUIRE(Str(z->owner) == "CIV");
        REQUIRE(z->income == Approx(25.0f)); // defaultIncome tap opened
        REQUIRE(CountEvents(fired, ZECaptured, depot) == 1);
    }

    SECTION("a zone owned by a third side is not capturable")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, legacy, legacy);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(legacy)->owner) == "EAST"); // not the WEST occupier
        REQUIRE(CountEvents(fired, ZECaptured, legacy) == 0);
    }

    SECTION("city support flip targets the resistance side")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, town, town);
        f.registry.EvaluateTick(in, fired); // support 55 -> 60 >= supportFlip
        REQUIRE(Str(f.registry.GetZone(town)->owner) == "CIV");
        REQUIRE(CountEvents(fired, ZESupportThreshold, town) == 1);
    }
}

TEST_CASE("ZoneRegistry - occupier/resistance sides survive save/load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "zone-registry-sides.bin";

    {
        RegistryFixture f;
        f.Load(kSwappableConfig, "NATO", "Partisans");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "CIV");

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.registry.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    // fresh registry; on load the second pass rebuilds from config (empty in
    // this process) and then applies the saved sides on top
    ZoneRegistry loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(Str(loaded.OccupierSide()) == "WEST");
    CHECK(Str(loaded.ResistanceSide()) == "CIV");

    std::filesystem::remove(archivePath);
}

TEST_CASE("ZoneRegistry - zone rows, handlers and the load notification survive save/load",
          "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "zone-registry-rows.bin";

    // The load's second pass rebuilds the static table via LoadFromConfig(),
    // which reads the global ExtParsMission - park the config there exactly
    // as SetMission's description.ext reparse would during a real load.
    {
        QIStream in(kDemoConfig, strlen(kDemoConfig));
        ExtParsMission.Parse(in);
    }

    {
        RegistryFixture f;
        f.Load(kDemoConfig);
        const int outpost = f.registry.FindZoneIndex("Outpost");
        const int village = f.registry.FindZoneIndex("Village");
        REQUIRE(outpost >= 0);
        REQUIRE(village >= 0);

        // mutate every serialized dynamic field away from its config value
        ZoneRecord* z = f.registry.GetZoneMutable(outpost);
        REQUIRE(z != nullptr);
        z->owner = "GUER";
        z->garrison = 3;
        z->income = 40;
        z->liveOccupiers = 2;
        z->revealed = true;
        f.registry.GetZoneMutable(village)->support = 44;
        f.registry.HeatRaise(outpost, 55);

        f.registry.SetEventHandler(ZECaptured, "gmEvtCaptured = gmEvtCaptured + [_this]");
        f.registry.SetEventHandler(ZESupportThreshold, "hSupport");
        f.registry.SetEventHandler(ZERevealed, "hRevealed");
        f.registry.SetEventHandler(ZECampaignLoaded, "hLoaded");

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.registry.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    ZoneRegistry loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    // the static table came back from ExtParsMission, the dynamic rows from
    // the archive (matched by zone name)
    REQUIRE(loaded.NZones() == 4);
    const int outpost = loaded.FindZoneIndex("Outpost");
    const int village = loaded.FindZoneIndex("Village");
    REQUIRE(outpost >= 0);
    REQUIRE(village >= 0);

    const ZoneRecord* z = loaded.GetZone(outpost);
    CHECK(Str(z->owner) == "GUER");
    CHECK(z->garrison == Approx(3.0f));
    CHECK(z->income == Approx(40.0f));
    CHECK(z->heat == Approx(55.0f));
    CHECK(z->liveOccupiers == Approx(2.0f));
    CHECK(z->revealed);
    CHECK(loaded.GetZone(village)->support == Approx(44.0f));

    CHECK(Str(loaded.GetEventHandler(ZECaptured)) == "gmEvtCaptured = gmEvtCaptured + [_this]");
    CHECK(Str(loaded.GetEventHandler(ZESupportThreshold)) == "hSupport");
    CHECK(Str(loaded.GetEventHandler(ZERevealed)) == "hRevealed");
    CHECK(Str(loaded.GetEventHandler(ZECampaignLoaded)) == "hLoaded");

    // the campaignLoaded notification is queued exactly once, carrying the
    // archive's save-format version
    int version = -1;
    CHECK(loaded.ConsumeCampaignLoaded(version));
    CHECK(version == GuerrillaSaveVersion);
    CHECK_FALSE(loaded.ConsumeCampaignLoaded(version));

    // scrub the process-wide mission config other tests expect to be empty
    ExtParsMission.Clear();
    std::filesystem::remove(archivePath);
}

TEST_CASE("ZoneRegistry - seedCities appends CITY zones from the world's Names", "[game][guerrilla]")
{
    // Houdan/Sainte are OFP-style entries (no type; Sainte with the common
    // empty name), LeVille is an Arma-style typed city, Rocher a typed
    // non-town, NearCamp sits ~141 m from the explicit Camp zone.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    seedCities = 1; seedCitySupport = 35;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"GUER\"; "
                         "position[]={6500.0, 6500.0, 100.0}; };\n"
                         "    };\n"
                         "};\n"
                         "class Names\n"
                         "{\n"
                         "    class Houdan   { name=\"Houdan\";   position[]={3500.0, 4200.0}; };\n"
                         "    class NearCamp { name=\"NearCamp\"; position[]={6600.0, 6400.0}; };\n"
                         "    class Sainte   { name=\"\";         position[]={9000.0, 9000.0}; };\n"
                         "    class LeVille  { name=\"Le Ville\"; type=\"NameCity\"; "
                         "position[]={12000.0, 12000.0}; };\n"
                         "    class Rocher   { name=\"Rocher\";   type=\"RockArea\"; "
                         "position[]={15000.0, 15000.0}; };\n"
                         "};\n";

    SECTION("seeded zone fields, type filter and dedup")
    {
        RegistryFixture f;
        f.Load(config, nullptr, nullptr, "Names");

        // Camp + Houdan + Sainte + LeVille; NearCamp deduped, Rocher filtered
        REQUIRE(f.registry.NZones() == 4);
        REQUIRE(f.registry.FindZoneIndex("NearCamp") == -1);
        REQUIRE(f.registry.FindZoneIndex("Rocher") == -1);

        const ZoneRecord* houdan = f.registry.GetZone(f.registry.FindZoneIndex("Houdan"));
        REQUIRE(houdan != nullptr);
        REQUIRE(Str(houdan->type) == "CITY");
        REQUIRE(Str(houdan->owner) == "NEUTRAL");
        REQUIRE(houdan->support == Approx(35.0f)); // seedCitySupport
        REQUIRE(houdan->income == Approx(0.0f));
        REQUIRE(houdan->heat == Approx(0.0f));
        REQUIRE(Str(houdan->marker) == "gmZoneCity_0");
        // Names position[] is 2D map coords [x, z]; elevation 0
        REQUIRE(houdan->pos.X() == Approx(3500.0f));
        REQUIRE(houdan->pos.Z() == Approx(4200.0f));
        REQUIRE(houdan->pos.Y() == Approx(0.0f));

        // empty name falls back to the class name; markers keep counting
        const ZoneRecord* sainte = f.registry.GetZone(f.registry.FindZoneIndex("Sainte"));
        REQUIRE(sainte != nullptr);
        REQUIRE(Str(sainte->marker) == "gmZoneCity_1");

        // typed city-like locations pass the filter
        const ZoneRecord* leVille = f.registry.GetZone(f.registry.FindZoneIndex("Le Ville"));
        REQUIRE(leVille != nullptr);
        REQUIRE(Str(leVille->marker) == "gmZoneCity_2");
    }

    SECTION("seeding is off by default")
    {
        const char* noSeedConfig = "class CfgGuerrillaZones\n"
                                   "{\n"
                                   "    class Zones\n"
                                   "    {\n"
                                   "        class Camp { name=\"Camp\"; position[]={6500.0, 6500.0, 100.0}; };\n"
                                   "    };\n"
                                   "};\n"
                                   "class Names\n"
                                   "{\n"
                                   "    class Houdan { name=\"Houdan\"; position[]={3500.0, 4200.0}; };\n"
                                   "};\n";
        RegistryFixture f;
        f.Load(noSeedConfig, nullptr, nullptr, "Names");
        REQUIRE(f.registry.NZones() == 1);
    }

    SECTION("total zone count is capped at MaxZones")
    {
        std::string bigConfig = "class CfgGuerrillaZones\n"
                                "{\n"
                                "    seedCities = 1;\n"
                                "    class Zones\n"
                                "    {\n"
                                "        class Camp { name=\"Camp\"; position[]={100.0, 100.0, 0.0}; };\n"
                                "    };\n"
                                "};\n"
                                "class Names\n"
                                "{\n";
        for (int i = 0; i < ZoneRegistry::MaxZones + 10; i++)
        {
            // 1 km apart on a row far from Camp - clear of the 300 m dedup
            bigConfig += "    class N" + std::to_string(i) + " { name=\"N" + std::to_string(i) + "\"; position[]={" +
                         std::to_string(5000 + i * 1000) + ".0, 50000.0}; };\n";
        }
        bigConfig += "};\n";

        RegistryFixture f;
        f.Load(bigConfig.c_str(), nullptr, nullptr, "Names");
        REQUIRE(f.registry.NZones() == ZoneRegistry::MaxZones);
    }
}
