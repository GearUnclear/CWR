#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <algorithm>
#include <filesystem>
#include <string.h>
#include <string>
#include <vector>

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
              const char* namesClass = nullptr, const ClassProbe* probe = nullptr)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"),
                                selOccupier, selResistance, namesClass ? file.FindEntry(namesClass) : nullptr, probe);
    }
};

// fake data package for the plan-15 resolution pass: a class exists when its
// name is on the list for the bank
struct FakeProbe final : ClassProbe
{
    std::vector<std::string> vehicles;
    std::vector<std::string> weapons;

    bool Exists(const char* bank, const char* className) const override
    {
        const std::vector<std::string>& list = stricmp(bank, "CfgWeapons") == 0 ? weapons : vehicles;
        for (const std::string& name : list)
        {
            if (stricmp(name.c_str(), className) == 0)
            {
                return true;
            }
        }
        return false;
    }
};

ZoneTickInputs MakeInputs(const ZoneRegistry& registry, int playerAtZone, int guerAtZone, int guerCount = 1)
{
    ZoneTickInputs in;
    in.guerCount.Resize(registry.NZones());
    in.occCount.Resize(registry.NZones());
    for (int i = 0; i < in.guerCount.Size(); i++)
    {
        in.guerCount[i] = 0;
        in.occCount[i] = 0;
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
        in.guerCount[guerAtZone] = guerCount;
    }
    return in;
}

// run EvaluateTick n times with the same inputs, accumulating events
void Ticks(ZoneRegistry& registry, const ZoneTickInputs& in, AutoArray<ZoneEventRecord>& fired, int n)
{
    for (int tick = 0; tick < n; tick++)
    {
        registry.EvaluateTick(in, fired);
    }
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

    SECTION("no progress while the occupier reserve remains")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired); // garrison == 8
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptured, outpost) == 0);
    }

    SECTION("the liveOccupiers bookkeeping mirror decides nothing")
    {
        // stale GarrisonCache membership counts must neither block a clear
        // zone (occCount 0) nor stand in for positional presence
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->liveOccupiers = 5;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(6.0f)); // accrues
    }

    SECTION("a live occupier unit in the zone blocks progress at 0")
    {
        // the QRF fix: positional presence blocks even with counters clear
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        in.occCount[outpost] = 1;
        Ticks(f.registry, in, fired, 5);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptureStarted, outpost) == 0);
    }

    SECTION("no progress without a GUER unit in the zone area")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, -1);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(0.0f));
    }

    SECTION("outside cacheRadius the meter freezes - no gain, no decay")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->capture = 50;
        AutoArray<ZoneEventRecord> fired;
        // player parked at FarPost, GUER unit present at the outpost
        ZoneTickInputs in = MakeInputs(f.registry, 3, outpost);
        Ticks(f.registry, in, fired, 3);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "EAST");
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(50.0f));
    }

    SECTION("consolidation: solo capture accrues 6/tick and flips on the 17th")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        Ticks(f.registry, in, fired, 16);
        const ZoneRecord* z = f.registry.GetZone(outpost);
        REQUIRE(z->capture == Approx(96.0f));
        REQUIRE(Str(z->owner) == "EAST"); // still holding, not yet flipped
        REQUIRE(CountEvents(fired, ZECaptured, outpost) == 0);
        REQUIRE(CountEvents(fired, ZECaptureStarted, outpost) == 1); // 0 -> 6 edge only

        // 17th tick: flip with the full legacy side effects, meter reset
        fired.Clear();
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(z->owner) == "GUER");
        REQUIRE(z->heat == Approx(40.0f));   // heatCapSpike
        REQUIRE(z->income == Approx(25.0f)); // defaultIncome opened (was 0)
        REQUIRE(z->garrison == Approx(0.0f));
        REQUIRE(z->capture == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptured, outpost) == 1);
        REQUIRE(CountEvents(fired, ZESupportThreshold, outpost) == 0);
    }

    SECTION("crew scaling: rate multiplies by attacker count up to captureCrewCap")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs three = MakeInputs(f.registry, outpost, outpost, 3);
        f.registry.EvaluateTick(three, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(18.0f)); // 6 * 3

        ZoneTickInputs five = MakeInputs(f.registry, outpost, outpost, 5);
        f.registry.EvaluateTick(five, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(36.0f)); // capped at 3
    }

    SECTION("contested: both sides present freeze the meter, one edge event")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs securing = MakeInputs(f.registry, outpost, outpost);
        Ticks(f.registry, securing, fired, 8); // capture 48
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(48.0f));

        fired.Clear();
        ZoneTickInputs contested = MakeInputs(f.registry, outpost, outpost);
        contested.occCount[outpost] = 2;
        Ticks(f.registry, contested, fired, 4);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(48.0f)); // frozen, no decay
        REQUIRE(CountEvents(fired, ZEContested, outpost) == 1); // edge only, no repeat

        // defenders die -> securing resumes where it left off
        fired.Clear();
        f.registry.EvaluateTick(securing, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(54.0f));
    }

    SECTION("defended: occupiers alone re-secure at 10/tick, captureLost at 0")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->capture = 25;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs defended = MakeInputs(f.registry, outpost, -1);
        defended.occCount[outpost] = 1;
        Ticks(f.registry, defended, fired, 2);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(5.0f));
        REQUIRE(CountEvents(fired, ZECaptureLost, outpost) == 0);

        Ticks(f.registry, defended, fired, 2);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptureLost, outpost) == 1); // floor edge only
    }

    SECTION("abandoned: unheld progress fades at 2/tick")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->capture = 10;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs abandoned = MakeInputs(f.registry, outpost, -1);
        Ticks(f.registry, abandoned, fired, 3);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(4.0f));
        Ticks(f.registry, abandoned, fired, 2);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptureLost, outpost) == 1);
    }

    SECTION("outnumber override: overwhelming defenders re-secure past a straggler")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->capture = 40;
        AutoArray<ZoneEventRecord> fired;
        // 4 defenders vs 1 attacker >= contestOutnumberRatio (4) -> DEFENDED
        ZoneTickInputs swarmed = MakeInputs(f.registry, outpost, outpost);
        swarmed.occCount[outpost] = 4;
        f.registry.EvaluateTick(swarmed, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(30.0f));

        // 3 vs 1 stays under the ratio -> CONTESTED, frozen
        ZoneTickInputs held = MakeInputs(f.registry, outpost, outpost);
        held.occCount[outpost] = 3;
        f.registry.EvaluateTick(held, fired);
        REQUIRE(f.registry.GetZone(outpost)->capture == Approx(30.0f));
    }

    SECTION("an already-open income tap is preserved on capture")
    {
        f.registry.GetZoneMutable(outpost)->garrison = 0;
        f.registry.GetZoneMutable(outpost)->income = 55;
        f.registry.GetZoneMutable(outpost)->capture = 96;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, outpost, outpost);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(outpost)->owner) == "GUER");
        REQUIRE(f.registry.GetZone(outpost)->income == Approx(55.0f));
    }
}

TEST_CASE("ZoneRegistry - capture pacing tunables", "[game][guerrilla]")
{
    SECTION("new tuning keys parse with sane defaults")
    {
        RegistryFixture f;
        f.Load(kDemoConfig); // sets none of the new keys
        const ZoneTuning& t = f.registry.Tuning();
        REQUIRE(t.captureRate == Approx(6.0f));
        REQUIRE(t.captureCrewCap == Approx(3.0f));
        REQUIRE(t.captureDecayDefended == Approx(10.0f));
        REQUIRE(t.captureDecayAbandoned == Approx(2.0f));
        REQUIRE(t.supportDecayOccupied == Approx(0.5f));
        REQUIRE(t.supportDecayFloor == Approx(20.0f));
        REQUIRE(t.contestOutnumberRatio == Approx(4.0f));
    }

    SECTION("tuning keys override the defaults")
    {
        const char* config = "class CfgGuerrillaZones\n"
                             "{\n"
                             "    captureRate = 12; captureCrewCap = 2; captureDecayDefended = 20;\n"
                             "    captureDecayAbandoned = 1; supportDecayOccupied = 2;\n"
                             "    supportDecayFloor = 10; contestOutnumberRatio = 0;\n"
                             "    class Zones { class A { name=\"A\"; }; };\n"
                             "};\n";
        RegistryFixture f;
        f.Load(config);
        const ZoneTuning& t = f.registry.Tuning();
        REQUIRE(t.captureRate == Approx(12.0f));
        REQUIRE(t.captureCrewCap == Approx(2.0f));
        REQUIRE(t.captureDecayDefended == Approx(20.0f));
        REQUIRE(t.captureDecayAbandoned == Approx(1.0f));
        REQUIRE(t.supportDecayOccupied == Approx(2.0f));
        REQUIRE(t.supportDecayFloor == Approx(10.0f));
        REQUIRE(t.contestOutnumberRatio == Approx(0.0f));
    }

    SECTION("contestOutnumberRatio 0 disables the override")
    {
        const char* config = "class CfgGuerrillaZones\n"
                             "{\n"
                             "    contestOutnumberRatio = 0;\n"
                             "    class Zones { class Post { name=\"Post\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                             "garrison=0; position[]={1000.0, 1000.0, 0.0}; }; };\n"
                             "};\n";
        RegistryFixture f;
        f.Load(config);
        f.registry.GetZoneMutable(0)->capture = 40;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, 0, 0);
        in.occCount[0] = 10; // any superiority: still CONTESTED, frozen
        f.registry.EvaluateTick(in, fired);
        REQUIRE(f.registry.GetZone(0)->capture == Approx(40.0f));
    }

    SECTION("per-zone captureRate override beats the tuning rate")
    {
        const char* config = "class CfgGuerrillaZones\n"
                             "{\n"
                             "    class Zones { class Airfield { name=\"Airfield\"; type=\"AIRFIELD\"; "
                             "owner=\"EAST\"; garrison=0; captureRate=3; position[]={1000.0, 1000.0, 0.0}; }; };\n"
                             "};\n";
        RegistryFixture f;
        f.Load(config);
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, 0, 0);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(f.registry.GetZone(0)->capture == Approx(3.0f));
    }

    SECTION("captureRate 100 restores the legacy instant flip")
    {
        const char* config = "class CfgGuerrillaZones\n"
                             "{\n"
                             "    captureRate = 100;\n"
                             "    class Zones { class Post { name=\"Post\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                             "garrison=0; position[]={1000.0, 1000.0, 0.0}; }; };\n"
                             "};\n";
        RegistryFixture f;
        f.Load(config);
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, 0, 0);
        f.registry.EvaluateTick(in, fired);
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "GUER");
        REQUIRE(CountEvents(fired, ZECaptured, 0) == 1);
    }
}

TEST_CASE("ZoneRegistry - city support and threshold flip", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kDemoConfig);
    const int village = 1;

    SECTION("presence-gated accrual, ready threshold, flip on arrival")
    {
        ZoneTickInputs present = MakeInputs(f.registry, village, village);
        AutoArray<ZoneEventRecord> fired;

        // support 20 -> 55 over 7 ticks, no flip yet (threshold 60)
        Ticks(f.registry, present, fired, 7);
        REQUIRE(f.registry.GetZone(village)->support == Approx(55.0f));
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "NEUTRAL");
        REQUIRE(CountEvents(fired, ZECaptured, village) == 0);

        // support does not accrue without a GUER unit present
        ZoneTickInputs absent = MakeInputs(f.registry, village, -1);
        fired.Clear();
        f.registry.EvaluateTick(absent, fired);
        REQUIRE(f.registry.GetZone(village)->support == Approx(55.0f));

        // the 8th accruing tick crosses the threshold; fighters are standing
        // in an occupier-free town, so it also flips on the same tick
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

    SECTION("no accrual while an occupier unit is in the town")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs contested = MakeInputs(f.registry, village, village);
        contested.occCount[village] = 1;
        Ticks(f.registry, contested, fired, 4);
        REQUIRE(f.registry.GetZone(village)->support == Approx(20.0f)); // frozen
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "NEUTRAL");
    }

    SECTION("intimidation: occupier-only presence bleeds support to the floor")
    {
        f.registry.GetZoneMutable(village)->support = 22;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs occupied = MakeInputs(f.registry, village, -1);
        occupied.occCount[village] = 2;
        Ticks(f.registry, occupied, fired, 3);
        REQUIRE(f.registry.GetZone(village)->support == Approx(20.5f)); // 0.5/tick
        Ticks(f.registry, occupied, fired, 5);
        REQUIRE(f.registry.GetZone(village)->support == Approx(20.0f)); // supportDecayFloor
    }

    SECTION("the native floor never raises support left below it by scripts")
    {
        f.registry.GetZoneMutable(village)->support = 5; // atrocity aftermath
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs occupied = MakeInputs(f.registry, village, -1);
        occupied.occCount[village] = 1;
        f.registry.EvaluateTick(occupied, fired);
        REQUIRE(f.registry.GetZone(village)->support == Approx(5.0f));
    }

    SECTION("ready without fighters: threshold event fires once, no flip")
    {
        f.registry.GetZoneMutable(village)->support = 60;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs absent = MakeInputs(f.registry, village, -1);
        Ticks(f.registry, absent, fired, 3);
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "NEUTRAL"); // no spontaneous flip
        REQUIRE(CountEvents(fired, ZESupportThreshold, village) == 1); // announced once

        // fighters arrive in the occupier-free town -> the flip, captured only
        fired.Clear();
        ZoneTickInputs present = MakeInputs(f.registry, village, village);
        f.registry.EvaluateTick(present, fired);
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "GUER");
        REQUIRE(CountEvents(fired, ZECaptured, village) == 1);
        REQUIRE(CountEvents(fired, ZESupportThreshold, village) == 0);
    }

    SECTION("ready but occupied: fighters present with occupiers do not flip")
    {
        f.registry.GetZoneMutable(village)->support = 80;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs contested = MakeInputs(f.registry, village, village);
        contested.occCount[village] = 1;
        Ticks(f.registry, contested, fired, 3);
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "NEUTRAL");
        REQUIRE(CountEvents(fired, ZECaptured, village) == 0);
    }

    SECTION("an occupier-administered town accrues underground support and flips")
    {
        f.registry.GetZoneMutable(village)->owner = "EAST";
        f.registry.GetZoneMutable(village)->support = 55;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs present = MakeInputs(f.registry, village, village);
        f.registry.EvaluateTick(present, fired);
        REQUIRE(f.registry.GetZone(village)->support == Approx(60.0f));
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "GUER");
        REQUIRE(CountEvents(fired, ZECaptured, village) == 1);
    }

    SECTION("an unspawned garrison reserve occupies a town like live units do")
    {
        // mirrors the military predicate: the zone tick runs before
        // GarrisonCache materializes the reserve inside the bubble
        f.registry.GetZoneMutable(village)->owner = "EAST";
        f.registry.GetZoneMutable(village)->garrison = 3;
        f.registry.GetZoneMutable(village)->support = 80;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs present = MakeInputs(f.registry, village, village);
        Ticks(f.registry, present, fired, 3);
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "EAST"); // no flip past the reserve
        REQUIRE(f.registry.GetZone(village)->support == Approx(80.0f)); // contested: frozen
        REQUIRE(CountEvents(fired, ZECaptured, village) == 0);

        // reserve cleared -> the flip lands and clears the counters
        f.registry.GetZoneMutable(village)->garrison = 0;
        fired.Clear();
        f.registry.EvaluateTick(present, fired);
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "GUER");
        REQUIRE(f.registry.GetZone(village)->garrison == Approx(0.0f));
        REQUIRE(CountEvents(fired, ZECaptured, village) == 1);
    }

    SECTION("a third-side-owned town never accrues or flips")
    {
        f.registry.GetZoneMutable(village)->owner = "WEST"; // not the EAST occupier
        f.registry.GetZoneMutable(village)->support = 90;
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs present = MakeInputs(f.registry, village, village);
        Ticks(f.registry, present, fired, 3);
        REQUIRE(f.registry.GetZone(village)->support == Approx(90.0f));
        REQUIRE(Str(f.registry.GetZone(village)->owner) == "WEST");
        REQUIRE(CountEvents(fired, ZECaptured, village) == 0);
        REQUIRE(CountEvents(fired, ZESupportThreshold, village) == 0);
    }
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
    REQUIRE(ZoneRegistry::EventTypeFromName("captureStarted") == ZECaptureStarted);
    REQUIRE(ZoneRegistry::EventTypeFromName("CONTESTED") == ZEContested);
    REQUIRE(ZoneRegistry::EventTypeFromName("captureLost") == ZECaptureLost);
    REQUIRE(ZoneRegistry::EventTypeFromName("noSuchEvent") == -1);

    registry.SetEventHandler(ZECaptured, "hint \"flip\"");
    REQUIRE(Str(registry.GetEventHandler(ZECaptured)) == "hint \"flip\"");

    // registration REPLACES the previous handler - campaign.sqs re-registers
    // on campaignLoaded and must not stack duplicates
    registry.SetEventHandler(ZECaptured, "hint \"flip2\"");
    REQUIRE(Str(registry.GetEventHandler(ZECaptured)) == "hint \"flip2\"");

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

// ---------------------------------------------------------------------------
// sideTwin resolution (FactionTwins) - the shared half of the rebase, used by
// ZoneRegistry below and by the new-game UI's launch guard.
// ---------------------------------------------------------------------------

namespace
{
// parse a bare CfgGuerrillaFactions and hand back the entry (kept alive by the
// caller's ParamFile)
const ParamEntry* ParseFactions(ParamFile& file, const char* text)
{
    QIStream in(text, strlen(text));
    file.Parse(in);
    return file.FindEntry("CfgGuerrillaFactions");
}

const char* kTwinConfig = "class CfgGuerrillaFactions\n"
                          "{\n"
                          "    class PLO      { side=\"GUER\"; sideTwin=\"PLO_East\"; };\n"
                          "    class PLO_East { side=\"EAST\"; sideTwin=\"PLO\"; };\n"
                          "    class Lonely   { side=\"EAST\"; };\n"
                          "    class Dangling { side=\"EAST\"; sideTwin=\"NoSuchClass\"; };\n"
                          "    class Plain    { };\n"
                          "};\n";
} // namespace

TEST_CASE("FactionTwins - FactionSideOf reads the side key, defaulting to the class name", "[game][guerrilla]")
{
    ParamFile file;
    const ParamEntry* cfg = ParseFactions(file, kTwinConfig);

    REQUIRE(Str(FactionSideOf(cfg, "PLO")) == "GUER");
    REQUIRE(Str(FactionSideOf(cfg, "Plain")) == "Plain"); // no side key: the class name IS the side
    REQUIRE(Str(FactionSideOf(cfg, "NoSuchFaction")).empty());
    REQUIRE(Str(FactionSideOf(cfg, "")).empty());
    REQUIRE(Str(FactionSideOf(nullptr, "PLO")).empty());
}

TEST_CASE("FactionTwins - TwinOnSide / TwinOffSide walk the chain", "[game][guerrilla]")
{
    ParamFile file;
    const ParamEntry* cfg = ParseFactions(file, kTwinConfig);

    SECTION("the faction itself counts as a hit")
    {
        REQUIRE(Str(TwinOnSide(cfg, "PLO", "GUER")) == "PLO");
        REQUIRE(Str(TwinOffSide(cfg, "PLO", "EAST")) == "PLO");
    }

    SECTION("one hop finds the re-authored roster")
    {
        REQUIRE(Str(TwinOnSide(cfg, "PLO", "EAST")) == "PLO_East"); // the Sinai case
        REQUIRE(Str(TwinOnSide(cfg, "PLO_East", "guer")) == "PLO"); // case-insensitive
        REQUIRE(Str(TwinOffSide(cfg, "PLO", "GUER")) == "PLO_East");
    }

    SECTION("no twin on the wanted side")
    {
        REQUIRE(Str(TwinOnSide(cfg, "Lonely", "WEST")).empty());
        REQUIRE(Str(TwinOffSide(cfg, "Lonely", "EAST")).empty());
    }

    SECTION("a sideTwin naming a class the config lacks ends the walk, it does not abort")
    {
        // plan-15: unknown incoming descriptor data degrades, never fatal
        REQUIRE(Str(TwinOnSide(cfg, "Dangling", "WEST")).empty());
        REQUIRE(Str(TwinOffSide(cfg, "Dangling", "EAST")).empty());
    }

    SECTION("null config / empty selection / empty side")
    {
        REQUIRE(Str(TwinOnSide(nullptr, "PLO", "EAST")).empty());
        REQUIRE(Str(TwinOnSide(cfg, "", "EAST")).empty());
        REQUIRE(Str(TwinOnSide(cfg, "PLO", "")).empty());
        REQUIRE(Str(TwinOffSide(cfg, "PLO", nullptr)).empty());
    }
}

TEST_CASE("FactionTwins - a cyclic or over-long chain terminates", "[game][guerrilla]")
{
    SECTION("A -> B -> A with no match must not hang")
    {
        // PLO <-> PLO_East is already a cycle; asking for a side neither
        // declares walks it until the hop budget runs out
        ParamFile file;
        const ParamEntry* cfg = ParseFactions(file, kTwinConfig);
        REQUIRE(Str(TwinOnSide(cfg, "PLO", "WEST")).empty());
    }

    SECTION("a chain longer than kMaxTwinHops gives up rather than resolving")
    {
        std::string text = "class CfgGuerrillaFactions\n{\n";
        const int chain = kMaxTwinHops + 3;
        for (int i = 0; i < chain; i++)
        {
            // every rung is EAST except the last, which is out of reach
            text += "    class F" + std::to_string(i) + " { side=\"" + (i == chain - 1 ? "WEST" : "EAST") +
                    "\"; sideTwin=\"F" + std::to_string(i + 1) + "\"; };\n";
        }
        text += "};\n";
        ParamFile file;
        const ParamEntry* cfg = ParseFactions(file, text.c_str());
        REQUIRE(Str(TwinOnSide(cfg, "F0", "WEST")).empty());
        REQUIRE(Str(TwinOffSide(cfg, "F0", "EAST")).empty());
    }
}

TEST_CASE("FactionTwins - FirstFreeWarSide never offers CIV", "[game][guerrilla]")
{
    // CIV is not a war side: its center is friendly to everyone
    // (AICenterImpl::BeginArcade), so an occupier rebased onto it never fights.
    REQUIRE(Str(FirstFreeWarSide("GUER")) == "WEST");
    REQUIRE(Str(FirstFreeWarSide("WEST")) == "GUER");
    REQUIRE(Str(FirstFreeWarSide("EAST")) == "GUER");
    REQUIRE(Str(FirstFreeWarSide("CIV")) == "GUER"); // pinning CIV frees all three war sides
}

// ---------------------------------------------------------------------------
// playerSide: the resistance pin and the occupier collision rebase.
// ---------------------------------------------------------------------------

namespace
{
// Abel-shaped: the template welds the player to GUER, so the resistance side
// is fixed there and a resistance PICK only chooses which roster fills it.
// PLO/PLO_Guer are the twinned pair (the Sinai case).
const char* kPinnedConfig = "class CfgGuerrillaZones\n"
                            "{\n"
                            "    playerSide = \"GUER\";\n"
                            "    class Zones\n"
                            "    {\n"
                            "        class Base  { name=\"Base\";  owner=\"RESISTANCE\"; "
                            "position[]={1000.0, 1000.0, 0.0}; };\n"
                            "        class Depot { name=\"Depot\"; owner=\"OCCUPIER\";   "
                            "position[]={1200.0, 1000.0, 0.0}; };\n"
                            "    };\n"
                            "};\n"
                            "class CfgGuerrillaFactions\n"
                            "{\n"
                            "    class Soviets   { side=\"EAST\"; tiers[]={\"SoldierEB\"}; flag=\"east.pac\"; };\n"
                            "    class NATO      { side=\"WEST\"; tiers[]={\"SoldierWB\"}; flag=\"west.pac\"; };\n"
                            "    class Partisans { side=\"GUER\"; tiers[]={\"SoldierGB\"}; flag=\"guer.pac\"; };\n"
                            "    class PLO       { side=\"EAST\"; sideTwin=\"PLO_Guer\"; tiers[]={\"SoldierEB\"}; };\n"
                            "    class PLO_Guer  { side=\"GUER\"; sideTwin=\"PLO\";      tiers[]={\"SoldierGB\"}; };\n"
                            "};\n";

// the same rosters with no playerSide - the legacy shape every pre-existing
// test uses, where the pass must not run at all
const char* kUnpinnedConfig = "class CfgGuerrillaZones\n"
                              "{\n"
                              "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                              "};\n"
                              "class CfgGuerrillaFactions\n"
                              "{\n"
                              "    class Soviets   { side=\"EAST\"; };\n"
                              "    class Partisans { side=\"GUER\"; };\n"
                              "};\n";
} // namespace

TEST_CASE("ZoneRegistry - playerSide pins the resistance side (roster pick, not side pick)", "[game][guerrilla]")
{
    SECTION("no playerSide: the pass is a strict no-op")
    {
        RegistryFixture f;
        f.Load(kUnpinnedConfig, "Soviets", "Soviets");
        // Two picks on one side WOULD deadlock - but a legacy template gets no
        // rebase, exactly as before this pass existed. The new-game UI blocks
        // the pair instead. This section is what the whole playerSide guard
        // buys: every pre-existing side test stays green untouched.
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "EAST");
        REQUIRE(Str(f.registry.FindFactionForSide("EAST")->side) == "EAST");
    }

    SECTION("resistance already on playerSide: no-op")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "Soviets", "Partisans");
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.ResistanceFaction()) == "Partisans");
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
        REQUIRE(Str(f.registry.OccupierFaction()) == "Soviets");
    }

    SECTION("a sideTwin on playerSide is substituted, and overrides nothing")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "Soviets", "PLO"); // PLO is EAST; its twin is GUER
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.ResistanceFaction()) == "PLO_Guer");
        // the twin already DECLARES GUER: the config-clean path
        const FactionRecord* res = f.registry.FindFactionForSide("GUER");
        REQUIRE(res != nullptr);
        REQUIRE(Str(res->className) == "PLO_Guer");
        REQUIRE(Str(res->side) == "GUER");
    }

    SECTION("no twin: the picked roster is rebased onto playerSide")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "NATO", "Soviets"); // Soviets are EAST and have no twin
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.ResistanceFaction()) == "Soviets");
        // the RECORD moved with the side: FindFaction matches side first, so a
        // record left on EAST would still be handed to anyone asking for EAST
        // while GUER found nothing at all
        const FactionRecord* res = f.registry.FindFactionForSide("GUER");
        REQUIRE(res != nullptr);
        REQUIRE(Str(res->className) == "Soviets");
        REQUIRE(Str(res->side) == "GUER");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST"); // no collision: untouched
    }

    SECTION("occupier collides and steps onto its twin")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "PLO_Guer", "Partisans"); // both GUER
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.OccupierFaction()) == "PLO"); // the EAST twin of the pick
        REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
    }

    SECTION("occupier collides with no twin: rebased to the first free war side, never CIV")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "Partisans", "Partisans"); // one roster, both slots
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST"); // {GUER,WEST,EAST} minus GUER
        REQUIRE(Str(f.registry.OccupierSide()) != "CIV");
    }

    SECTION("a mirror match gets TWO records: one roster cannot sit on two sides")
    {
        // Both slots on one descriptor. Rebasing the occupier to WEST leaves
        // both picks naming the single Partisans record, so writing each pick's
        // side onto "its" record wrote both onto the same one and the occupier's
        // WEST lost to the resistance's GUER. Symptoms: the WEST occupier's
        // roster resolved against the EAST/Soviet fallback list, and
        // FindFaction("WEST") found nothing at all.
        RegistryFixture f;
        f.Load(kPinnedConfig, "Partisans", "Partisans");
        REQUIRE(Str(f.registry.OccupierSide()) == "WEST");
        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        // the resistance keeps the authored descriptor...
        REQUIRE(Str(f.registry.ResistanceFaction()) == "Partisans");
        // ...and the occupier gets its own copy of it, on its own side
        REQUIRE(Str(f.registry.OccupierFaction()) != Str(f.registry.ResistanceFaction()));

        const FactionRecord* occ = f.registry.FindFactionForSide("WEST");
        const FactionRecord* res = f.registry.FindFactionForSide("GUER");
        REQUIRE(occ != nullptr);
        REQUIRE(res != nullptr);
        REQUIRE(occ != res); // two records, not one aliased twice
        REQUIRE(Str(occ->side) == "WEST");
        REQUIRE(Str(res->side) == "GUER");
        // same order of battle on both sides — that IS what a mirror match is
        REQUIRE(Str(occ->tiers[0]) == "SoldierGB");
        REQUIRE(Str(res->tiers[0]) == "SoldierGB");
        REQUIRE(Str(f.registry.FactionValue("WEST", "flag")) == "guer.pac");
        REQUIRE(Str(f.registry.FactionValue("GUER", "flag")) == "guer.pac");
    }

    SECTION("a mirror match is idempotent: the clone is not re-cloned per load")
    {
        RegistryFixture first;
        first.Load(kPinnedConfig, "Partisans", "Partisans");
        RegistryFixture second;
        second.Load(kPinnedConfig, "Partisans", "Partisans");
        // the clone's name is a pure function of (class, side), which is what
        // lets Serialize restore _occupierFaction by name across a save/load
        REQUIRE(Str(first.registry.OccupierFaction()) == Str(second.registry.OccupierFaction()));
        REQUIRE(first.registry.NFactions() == second.registry.NFactions());
    }

    SECTION("a mirror match with no playerSide is left alone: one side, one record")
    {
        // Legacy: ResolveSideCollisions is a strict no-op, so the two picks
        // really are one side and splitting the record would invent a side
        // nobody resolved. The UI blocks this pair instead.
        RegistryFixture f;
        f.Load(kUnpinnedConfig, "Soviets", "Soviets");
        REQUIRE(Str(f.registry.OccupierFaction()) == Str(f.registry.ResistanceFaction()));
        REQUIRE(Str(f.registry.OccupierSide()) == Str(f.registry.ResistanceSide()));
    }

    SECTION("owner tokens resolve against the REBASED sides")
    {
        RegistryFixture f;
        f.Load(kPinnedConfig, "NATO", "Soviets");
        // LoadZones runs after the pass, so RESISTANCE resolves to the pinned
        // side rather than the descriptor's authored EAST
        REQUIRE(Str(f.registry.GetZone(0)->owner) == "GUER");
        REQUIRE(Str(f.registry.GetZone(1)->owner) == "WEST");
    }

    SECTION("the pass is idempotent: the same inputs resolve the same way twice")
    {
        // LoadFromParams re-runs on every savegame load pass, so the pass has
        // to be a pure function of (playerSide, selections, faction table) or
        // a campaign drifts a side further on each reload
        RegistryFixture first;
        first.Load(kPinnedConfig, "Partisans", "Soviets");
        RegistryFixture second;
        second.Load(kPinnedConfig, "Partisans", "Soviets");
        REQUIRE(Str(first.registry.OccupierSide()) == Str(second.registry.OccupierSide()));
        REQUIRE(Str(first.registry.ResistanceSide()) == Str(second.registry.ResistanceSide()));
        REQUIRE(Str(first.registry.OccupierFaction()) == Str(second.registry.OccupierFaction()));
        REQUIRE(Str(first.registry.ResistanceFaction()) == Str(second.registry.ResistanceFaction()));
    }
}

TEST_CASE("ZoneRegistry - FindFactionForSide binds the selected descriptor, not the first on that side",
          "[game][guerrilla]")
{
    // Sinai ships five EAST rosters. FindFaction scans by side FIRST and
    // returns whichever was DECLARED first, so picking Syria would silently
    // field EgyptArmy's roster - the pick would appear to work and quietly do
    // nothing.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                         "};\n"
                         "class CfgGuerrillaFactions\n"
                         "{\n"
                         "    class EgyptArmy { side=\"EAST\"; tiers[]={\"SoldierEB\"};   vehicles[]={\"BMP\"}; "
                         "holdClass=\"SoldierEB\"; };\n"
                         "    class Hizballah { side=\"EAST\"; tiers[]={\"SoldierEHiz\"}; vehicles[]={\"UAZ\"}; "
                         "holdClass=\"SoldierEHiz\"; };\n"
                         "    class Syria     { side=\"EAST\"; tiers[]={\"SoldierESyr\"}; vehicles[]={\"T72\"}; "
                         "holdClass=\"SoldierESyr\"; };\n"
                         "    class IDF       { side=\"WEST\"; tiers[]={\"SoldierWB\"}; };\n"
                         "};\n";
    RegistryFixture f;
    f.Load(config, "Syria", "IDF");

    REQUIRE(Str(f.registry.OccupierSide()) == "EAST");
    REQUIRE(Str(f.registry.OccupierFaction()) == "Syria");
    const FactionRecord* occ = f.registry.FindFactionForSide("EAST");
    REQUIRE(occ != nullptr);
    REQUIRE(Str(occ->className) == "Syria"); // NOT EgyptArmy, the first declared

    // every side-keyed query must agree, or a zone's infantry and its armour
    // come from two different armies
    REQUIRE(Str(f.registry.FactionTierClass("EAST", 1)) == "SoldierESyr");
    REQUIRE(Str(f.registry.FactionVehicle("EAST", 1)) == "T72");
    REQUIRE(Str(f.registry.FactionValue("EAST", "holdClass")) == "SoldierESyr");
    AutoArray<RString> squad;
    f.registry.FactionSquad("EAST", 1, 3, squad);
    REQUIRE(squad.Size() == 3);
    REQUIRE(Str(squad[0]) == "SoldierESyr");

    // an unselected third party still resolves through the plain scan
    REQUIRE(Str(f.registry.FactionTierClass("Hizballah", 1)) == "SoldierEHiz");
    REQUIRE(Str(f.registry.FactionTierClass("WEST", 1)) == "SoldierWB");
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
        Ticks(f.registry, in, fired, 17); // consolidation hold at 6/tick
        const ZoneRecord* z = f.registry.GetZone(depot);
        REQUIRE(Str(z->owner) == "CIV");
        REQUIRE(z->income == Approx(25.0f)); // defaultIncome tap opened
        REQUIRE(CountEvents(fired, ZECaptured, depot) == 1);
    }

    SECTION("a zone owned by a third side is not capturable")
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(f.registry, legacy, legacy);
        Ticks(f.registry, in, fired, 20);
        REQUIRE(Str(f.registry.GetZone(legacy)->owner) == "EAST"); // not the WEST occupier
        REQUIRE(f.registry.GetZone(legacy)->capture == Approx(0.0f));
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

TEST_CASE("ZoneRegistry - the side rebase is idempotent across save/load", "[game][guerrilla][save][load]")
{
    // LoadFromConfig re-runs on the load's second pass and the saved sides are
    // applied on top of it, so a rebase that is not a pure function of its
    // inputs drifts a little further on every reload. A single round-trip
    // would miss a drift that needs two hops to show - do three.
    {
        QIStream in(kPinnedConfig, strlen(kPinnedConfig));
        ExtParsMission.Parse(in);
    }
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "zone-registry-rebase.bin";

    RegistryFixture f;
    // the occupier is rebased off GUER and the resistance onto it: both halves
    // of the pass are in flight
    f.Load(kPinnedConfig, "Partisans", "Soviets");
    const std::string occSide = Str(f.registry.OccupierSide());
    const std::string resSide = Str(f.registry.ResistanceSide());
    const std::string occFaction = Str(f.registry.OccupierFaction());
    const std::string resFaction = Str(f.registry.ResistanceFaction());
    REQUIRE(occSide != resSide);
    REQUIRE(occFaction == "Partisans");
    REQUIRE(resFaction == "Soviets");

    ZoneRegistry* current = &f.registry;
    ZoneRegistry loaded[3];
    for (int pass = 0; pass < 3; pass++)
    {
        {
            ParamArchiveSave ar(WorldSerializeVersion);
            REQUIRE(current->Serialize(ar) == LSOK);
            REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        }
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded[pass].Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded[pass].Serialize(ar) == LSOK);

        INFO("round-trip " << pass);
        CHECK(Str(loaded[pass].OccupierSide()) == occSide);
        CHECK(Str(loaded[pass].ResistanceSide()) == resSide);
        CHECK(Str(loaded[pass].OccupierFaction()) == occFaction);
        CHECK(Str(loaded[pass].ResistanceFaction()) == resFaction);
        // RebindFactionSides ran on the load: the roster a side fields is the
        // roster that was picked, not whatever the config declared there
        const FactionRecord* res = loaded[pass].FindFactionForSide(resSide.c_str());
        REQUIRE(res != nullptr);
        CHECK(Str(res->className) == resFaction);
        current = &loaded[pass];
    }

    std::filesystem::remove(archivePath);
}

TEST_CASE("ZoneRegistry - a pre-rebase save without the faction keys still loads", "[game][guerrilla][save][load]")
{
    // occupierFaction/resistanceFaction are additive and presence-tolerant, so
    // no GuerrillaSaveVersion bump: a campaign saved before they existed
    // carries the two sides only and must keep whatever the config resolves.
    // (The archive writer omits any value equal to its default, so a registry
    // with no matched selections produces exactly that byte layout.)
    const char* defaultsConfig = "class CfgGuerrillaZones\n"
                                 "{\n"
                                 "    defaultOccupier = \"Soviets\"; defaultResistance = \"Partisans\";\n"
                                 "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                                 "};\n"
                                 "class CfgGuerrillaFactions\n"
                                 "{\n"
                                 "    class Soviets   { side=\"EAST\"; };\n"
                                 "    class Partisans { side=\"GUER\"; };\n"
                                 "};\n";
    {
        QIStream in(defaultsConfig, strlen(defaultsConfig));
        ExtParsMission.Parse(in);
    }
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "zone-registry-no-faction-keys.bin";

    {
        // no CfgGuerrillaFactions at all -> both faction names stay empty ->
        // neither key is written
        const char* factionlessConfig = "class CfgGuerrillaZones\n"
                                        "{\n"
                                        "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                                        "};\n";
        RegistryFixture f;
        f.Load(factionlessConfig);
        REQUIRE(Str(f.registry.OccupierFaction()).empty());
        REQUIRE(Str(f.registry.ResistanceFaction()).empty());

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.registry.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    ZoneRegistry loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK); // no error on the absent keys
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(Str(loaded.OccupierSide()) == "EAST");
    CHECK(Str(loaded.ResistanceSide()) == "GUER");
    // absent keys leave the config-resolved picks in place rather than
    // blanking them
    CHECK(Str(loaded.OccupierFaction()) == "Soviets");
    CHECK(Str(loaded.ResistanceFaction()) == "Partisans");
    REQUIRE(loaded.FindFactionForSide("EAST") != nullptr);
    CHECK(Str(loaded.FindFactionForSide("EAST")->className) == "Soviets");

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
        z->capture = 33; // mid-consolidation progress must survive a reload
        z->revealed = true;
        // 70 >= supportFlip: the town is saved in the READY-waiting state
        f.registry.GetZoneMutable(village)->support = 70;
        f.registry.HeatRaise(outpost, 55);

        f.registry.SetEventHandler(ZECaptured, "gmEvtCaptured = gmEvtCaptured + [_this]");
        f.registry.SetEventHandler(ZESupportThreshold, "hSupport");
        f.registry.SetEventHandler(ZERevealed, "hRevealed");
        f.registry.SetEventHandler(ZECampaignLoaded, "hLoaded");
        f.registry.SetEventHandler(ZECaptureStarted, "hCapStart");
        f.registry.SetEventHandler(ZEContested, "hContested");
        f.registry.SetEventHandler(ZECaptureLost, "hCapLost");

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
    CHECK(z->capture == Approx(33.0f));
    CHECK(z->revealed);
    CHECK(loaded.GetZone(village)->support == Approx(70.0f));
    // village had no capture in progress; the row default is 0
    CHECK(loaded.GetZone(village)->capture == Approx(0.0f));

    // the READY one-shot edge is reconstructed from the loaded support: a
    // town saved at/past supportFlip must NOT re-announce after the load
    {
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs in = MakeInputs(loaded, village, -1);
        loaded.EvaluateTick(in, fired);
        CHECK(CountEvents(fired, ZESupportThreshold, village) == 0);
        CHECK(Str(loaded.GetZone(village)->owner) == "NEUTRAL"); // and no flip
    }

    CHECK(Str(loaded.GetEventHandler(ZECaptured)) == "gmEvtCaptured = gmEvtCaptured + [_this]");
    CHECK(Str(loaded.GetEventHandler(ZESupportThreshold)) == "hSupport");
    CHECK(Str(loaded.GetEventHandler(ZERevealed)) == "hRevealed");
    CHECK(Str(loaded.GetEventHandler(ZECampaignLoaded)) == "hLoaded");
    CHECK(Str(loaded.GetEventHandler(ZECaptureStarted)) == "hCapStart");
    CHECK(Str(loaded.GetEventHandler(ZEContested)) == "hContested");
    CHECK(Str(loaded.GetEventHandler(ZECaptureLost)) == "hCapLost");

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

TEST_CASE("ZoneRegistry - CollectTownNames lists the campaign's CITY zones: authored first, then seeded",
          "[game][guerrilla]")
{
    // The START TOWN cycler's list rule. Village is an authored CITY sitting on
    // top of the Houdan Names entry (deduped by distance), NearCamp is ~141 m
    // from the explicit Camp, Sainte 50 m from the authored OUTPOST (every
    // authored zone counts for the dedup, as in SeedCityZones), LeVille is an
    // Arma-style typed city, Rocher a typed non-town, Arudy a plain town.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    seedCities = 1;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Camp    { name=\"Camp\";    type=\"CAMP\"; owner=\"GUER\"; "
                         "position[]={6500.0, 6500.0, 100.0}; };\n"
                         "        class Village { name=\"Village\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={3500.0, 4200.0, 35.0}; };\n"
                         "        class Post    { type=\"OUTPOST\"; owner=\"EAST\"; "
                         "position[]={9000.0, 9050.0, 10.0}; };\n"
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
                         "    class Arudy    { name=\"Arudy\";    position[]={2000.0, 9000.0}; };\n"
                         "};\n";

    SECTION("authored CITY zones first, then the seeded Names towns in Names order")
    {
        ParamFile file;
        QIStream in(config, strlen(config));
        file.Parse(in);
        AutoArray<RString> names;
        ZoneRegistry::CollectTownNames(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("Names"), names);
        REQUIRE(names.Size() == 3);
        CHECK(Str(names[0]) == "Village");
        CHECK(Str(names[1]) == "Le Ville");
        CHECK(Str(names[2]) == "Arudy");
    }

    SECTION("the list IS the registry's CITY zone table, in its order")
    {
        RegistryFixture f;
        f.Load(config, nullptr, nullptr, "Names");
        AutoArray<RString> names;
        ZoneRegistry::CollectTownNames(f.file.FindEntry("CfgGuerrillaZones"), f.file.FindEntry("Names"), names);
        int cities = 0;
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            const ZoneRecord* z = f.registry.GetZone(i);
            if (stricmp(z->type, "CITY") != 0)
            {
                continue;
            }
            REQUIRE(cities < names.Size());
            CHECK(Str(names[cities]) == Str(z->name));
            cities++;
        }
        CHECK(cities == names.Size());
    }

    SECTION("seedCities off: the authored CITY zones only")
    {
        std::string off(config);
        off.replace(off.find("seedCities = 1"), strlen("seedCities = 1"), "seedCities = 0");
        ParamFile file;
        QIStream in(off.c_str(), off.size());
        file.Parse(in);
        AutoArray<RString> names;
        ZoneRegistry::CollectTownNames(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("Names"), names);
        REQUIRE(names.Size() == 1);
        CHECK(Str(names[0]) == "Village");
    }

    SECTION("null configs")
    {
        ParamFile file;
        QIStream in(config, strlen(config));
        file.Parse(in);
        AutoArray<RString> names;
        ZoneRegistry::CollectTownNames(nullptr, nullptr, names);
        CHECK(names.Size() == 0);
        ZoneRegistry::CollectTownNames(file.FindEntry("CfgGuerrillaZones"), nullptr, names);
        REQUIRE(names.Size() == 1);
        CHECK(Str(names[0]) == "Village");
        ZoneRegistry::CollectTownNames(nullptr, file.FindEntry("Names"), names);
        // no zones config = no seedCities key = Auto (issue #54 C3): the
        // Names towns are listed (the mission-time classifier may seed fewer)
        CHECK(names.Size() >= 1);
    }
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

    SECTION("seeding is Auto by default: with no landscape to classify against every town seeds")
    {
        // issue #54 C3: an absent seedCities key means Auto; the engine's
        // settlement probe (dry land + buildings) does the filtering, and
        // without one (these config-only tests) the pre-C3 answer stands
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
        REQUIRE(f.registry.Tuning().seedCities == ZoneTuning::SeedCities::Auto);
        REQUIRE(f.registry.NZones() == 2);
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

// ---------------------------------------------------------------------------
// plan 15: descriptor class resolution + role-diverse squad composition
// ---------------------------------------------------------------------------

namespace
{

// occupier with a full role kit and a 4-rung vehicle ladder; resistance with
// tiers only; civilians with two classes.  Deliberately references classes a
// FakeProbe can selectively "unship".
const char* kPlan15Config = "class CfgGuerrillaZones { class Zones { class A { name=\"A\"; }; }; };\n"
                            "class CfgGuerrillaFactions\n"
                            "{\n"
                            "    class East\n"
                            "    {\n"
                            "        side=\"EAST\";\n"
                            "        tiers[]={\"SoldierEB\",\"SoldierEG\",\"SoldierECrew\"};\n"
                            "        tierThresholds[]={3,5};\n"
                            "        tiersMG[]={\"SoldierEMG\",\"SoldierEMG\",\"SoldierEMG\"};\n"
                            "        tiersAT[]={\"SoldierELAW\",\"SoldierELAW\",\"SoldierELAW\"};\n"
                            "        tiersMedic[]={\"SoldierEMedic\",\"SoldierEMedic\",\"SoldierEMedic\"};\n"
                            "        tiersSniper[]={\"\",\"\",\"SoldierESniper\"};\n"
                            "        vehicles[]={\"UAZ\",\"Ural\",\"BMP\",\"T72\"};\n"
                            "        vehicleThresholds[]={3,6,8};\n"
                            "        officer=\"OfficerE\";\n"
                            "        baseWeapon=\"AK74\";\n"
                            "        baseMagazine=\"AK74\";\n"
                            "        lootSniperWeapon=\"SVDDragunov\";\n"
                            "        lootSniperMag=\"SVDDragunov\";\n"
                            "    };\n"
                            "    class Guer\n"
                            "    {\n"
                            "        side=\"GUER\";\n"
                            "        tiers[]={\"SoldierGB\"};\n"
                            "        officer=\"SoldierGB\";\n"
                            "        holdClass=\"SoldierGB\";\n"
                            "    };\n"
                            "    class Civ\n"
                            "    {\n"
                            "        side=\"CIV\";\n"
                            "        civClassCount=2;\n"
                            "        civClass1=\"Civilian\";\n"
                            "        civClass2=\"Civilian2\";\n"
                            "    };\n"
                            "};\n";

FakeProbe FullClassicProbe()
{
    FakeProbe probe;
    probe.vehicles = {"SoldierEB", "SoldierEG",      "SoldierECrew", "SoldierEMG", "SoldierELAW", "SoldierEMedic",
                      "OfficerE",  "SoldierESniper", "UAZ",          "Ural",       "BMP",         "T72",
                      "SoldierGB", "SoldierGFakeE",  "SoldierWB",    "Civilian",   "Civilian2",   "SoldierWCaptive"};
    probe.weapons = {"AK74", "SVDDragunov"};
    return probe;
}

void Unship(FakeProbe& probe, const char* className)
{
    probe.vehicles.erase(std::remove(probe.vehicles.begin(), probe.vehicles.end(), std::string(className)),
                         probe.vehicles.end());
}

} // namespace

TEST_CASE("ZoneRegistry - plan-15 resolution keeps a fully-shipped descriptor verbatim", "[game][guerrilla]")
{
    FakeProbe probe = FullClassicProbe();
    RegistryFixture f;
    f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);

    REQUIRE(Str(f.registry.FactionTierClass("EAST", 1)) == "SoldierEB");
    REQUIRE(Str(f.registry.FactionTierClass("EAST", 10)) == "SoldierECrew");
    REQUIRE(Str(f.registry.FactionValue("EAST", "officer")) == "OfficerE");
    REQUIRE(Str(f.registry.FactionVehicle("EAST", 10)) == "T72");
    REQUIRE(Str(f.registry.FactionValue("CIV", "civClassCount")) == "2");
}

TEST_CASE("ZoneRegistry - plan-15 resolution substitutes unknown incoming faction data", "[game][guerrilla]")
{
    SECTION("missing tier falls back to the nearest resolved tier")
    {
        FakeProbe probe = FullClassicProbe();
        // the elite tier is not shipped by this package
        Unship(probe, "SoldierECrew");
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 10)) == "SoldierEG"); // nearest lower rung
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 1)) == "SoldierEB");  // untouched
    }

    SECTION("missing officer falls back to tier 0")
    {
        FakeProbe probe = FullClassicProbe();
        Unship(probe, "OfficerE");
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        REQUIRE(Str(f.registry.FactionValue("EAST", "officer")) == "SoldierEB");
    }

    SECTION("missing vehicle rung is dropped and the ladder compacts")
    {
        FakeProbe probe = FullClassicProbe();
        Unship(probe, "BMP");
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 1)) == "UAZ");
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 3)) == "Ural");
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 6)) == "T72"); // BMP's WL6 rung inherited by T72
        REQUIRE(Str(f.registry.FactionVehicle("EAST", 10)) == "T72");
    }

    SECTION("GUER faction on a GUER-less package degrades along the built-in fallback list")
    {
        FakeProbe probe = FullClassicProbe();
        Unship(probe, "SoldierGB");
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        // SoldierGFakeE is next on the GUER fallback list and "shipped"
        REQUIRE(Str(f.registry.FactionTierClass("GUER", 1)) == "SoldierGFakeE");
        REQUIRE(Str(f.registry.FactionValue("GUER", "officer")) == "SoldierGFakeE");
        REQUIRE(Str(f.registry.FactionValue("GUER", "holdClass")) == "SoldierGFakeE");
    }

    SECTION("unresolvable weapon and magazine keys fall back to baseWeapon / the weapon sibling")
    {
        FakeProbe probe = FullClassicProbe();
        probe.weapons = {"AK74"}; // no SVDDragunov in this package
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        REQUIRE(Str(f.registry.FactionValue("EAST", "lootSniperWeapon")) == "AK74"); // baseWeapon
        REQUIRE(Str(f.registry.FactionValue("EAST", "lootSniperMag")) == "AK74");
    }

    SECTION("fully unresolvable CIV roster soft-disables the ambience layer")
    {
        FakeProbe probe = FullClassicProbe();
        Unship(probe, "Civilian");
        Unship(probe, "Civilian2");
        Unship(probe, "SoldierWCaptive");
        RegistryFixture f;
        f.Load(kPlan15Config, nullptr, nullptr, nullptr, &probe);
        REQUIRE(Str(f.registry.FactionValue("CIV", "civClassCount")) == "0");
    }

    SECTION("no probe = no resolution (config-less unit-test path unchanged)")
    {
        RegistryFixture f;
        f.Load(kPlan15Config);
        REQUIRE(Str(f.registry.FactionTierClass("EAST", 10)) == "SoldierECrew");
        REQUIRE(Str(f.registry.FactionValue("EAST", "officer")) == "OfficerE");
    }

    SECTION("a rebased faction takes its fallbacks from the NEW side")
    {
        // Pins the LoadFromParams ordering the whole rebase depends on:
        // ResolveSideCollisions must run BEFORE ResolveFactionClasses, because
        // the resolution pass keys its candidate list off FactionRecord::side.
        // Reject is a WEST roster with an unshippable tier, picked as the
        // resistance on a GUER-pinned template - so it rebases to GUER, and
        // its fallback must come off the GUER list (SoldierGB), not the WEST
        // one (SoldierWB, deliberately unshipped by this package).
        const char* config = "class CfgGuerrillaZones\n"
                             "{\n"
                             "    playerSide = \"GUER\";\n"
                             "    class Zones { class A { name=\"A\"; owner=\"RESISTANCE\"; }; };\n"
                             "};\n"
                             "class CfgGuerrillaFactions\n"
                             "{\n"
                             "    class Reject  { side=\"WEST\"; tiers[]={\"NoSuchClass\"}; };\n"
                             "    class Soviets { side=\"EAST\"; tiers[]={\"SoldierEB\"}; };\n"
                             "};\n";
        FakeProbe probe = FullClassicProbe();
        Unship(probe, "SoldierWB");
        RegistryFixture f;
        f.Load(config, "Soviets", "Reject", nullptr, &probe);

        REQUIRE(Str(f.registry.ResistanceSide()) == "GUER");
        REQUIRE(Str(f.registry.ResistanceFaction()) == "Reject");
        REQUIRE(Str(f.registry.FactionTierClass("GUER", 1)) == "SoldierGB"); // the GUER fallback list
    }
}

TEST_CASE("ZoneRegistry - plan-15 squad composition", "[game][guerrilla]")
{
    RegistryFixture f;
    f.Load(kPlan15Config);

    SECTION("12-man garrison group composes the full template")
    {
        AutoArray<RString> squad;
        f.registry.FactionSquad("EAST", 1, 12, squad);
        REQUIRE(squad.Size() == 12);
        REQUIRE(Str(squad[0]) == "SoldierEB"); // leader slot (caller substitutes the officer)
        int mg = 0, at = 0, medic = 0, rifle = 0;
        for (int i = 0; i < squad.Size(); i++)
        {
            if (Str(squad[i]) == "SoldierEMG")
            {
                mg++;
            }
            else if (Str(squad[i]) == "SoldierELAW")
            {
                at++;
            }
            else if (Str(squad[i]) == "SoldierEMedic")
            {
                medic++;
            }
            else if (Str(squad[i]) == "SoldierEB")
            {
                rifle++;
            }
        }
        REQUIRE(mg == 2);
        REQUIRE(at == 2);
        REQUIRE(medic == 1);
        REQUIRE(rifle == 7); // tier 0 fields no sniper -> that slot stays a rifleman
    }

    SECTION("sniper slot only where the tier authors one")
    {
        AutoArray<RString> low, elite;
        f.registry.FactionSquad("EAST", 1, 12, low);
        f.registry.FactionSquad("EAST", 10, 12, elite);
        for (int i = 0; i < low.Size(); i++)
        {
            REQUIRE(Str(low[i]) != "SoldierESniper");
        }
        int sniper = 0;
        for (int i = 0; i < elite.Size(); i++)
        {
            if (Str(elite[i]) == "SoldierESniper")
            {
                sniper++;
            }
        }
        REQUIRE(sniper == 1);
    }

    SECTION("small groups stay mixed but gated")
    {
        AutoArray<RString> three;
        f.registry.FactionSquad("EAST", 1, 3, three);
        REQUIRE(three.Size() == 3);
        REQUIRE(Str(three[0]) == "SoldierEB");
        REQUIRE(Str(three[1]) == "SoldierEMG"); // MG unlocks from 3 men
        REQUIRE(Str(three[2]) == "SoldierEB");  // AT gates at 5, medic at 6
    }

    SECTION("descriptor without role arrays composes the pre-plan-15 monoculture")
    {
        AutoArray<RString> squad;
        f.registry.FactionSquad("GUER", 1, 6, squad);
        REQUIRE(squad.Size() == 6);
        for (int i = 0; i < squad.Size(); i++)
        {
            REQUIRE(Str(squad[i]) == "SoldierGB");
        }
    }

    SECTION("unknown side / zero count come back empty")
    {
        AutoArray<RString> squad;
        f.registry.FactionSquad("WEST", 1, 6, squad);
        REQUIRE(squad.Size() == 0);
        f.registry.FactionSquad("EAST", 1, 0, squad);
        REQUIRE(squad.Size() == 0);
    }

    SECTION("deterministic: same inputs, same squad")
    {
        AutoArray<RString> a, b;
        f.registry.FactionSquad("EAST", 4, 9, a);
        f.registry.FactionSquad("EAST", 4, 9, b);
        REQUIRE(a.Size() == b.Size());
        for (int i = 0; i < a.Size(); i++)
        {
            REQUIRE(Str(a[i]) == Str(b[i]));
        }
    }
}
