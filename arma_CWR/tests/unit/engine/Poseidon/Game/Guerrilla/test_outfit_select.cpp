// Character-select outfit substitution (issue #25): the pure resolver the
// WorldInit seam runs (ResolveCivilianPlayerClass), and the civTier[]
// half of the plan-15 resolution pass (its own civilian-outfit fallback
// ladder - never kFallbackCiv, whose SoldierWCaptive entry is a WEST-side
// unarmed captive class).

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// fake data package: a CfgVehicles class exists when its name is listed
struct FakeProbe final : ClassProbe
{
    std::vector<std::string> vehicles;

    bool Exists(const char* bank, const char* className) const override
    {
        if (!bank || !className || stricmp(bank, "CfgVehicles") != 0)
        {
            return false;
        }
        for (const std::string& v : vehicles)
        {
            if (stricmp(v.c_str(), className) == 0)
            {
                return true;
            }
        }
        return false;
    }
};

// the Guerrilla.Abel shape: playerSide-pinned GUER resistance offering the
// FakeC pair, plus a WEST faction offering no civilian outfit
const char* kOutfitConfig = "class CfgGuerrillaZones\n"
                            "{\n"
                            "    defaultOccupier = \"EAST\";\n"
                            "    defaultResistance = \"GUER\";\n"
                            "    playerSide = \"GUER\";\n"
                            "    class Zones\n"
                            "    {\n"
                            "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; "
                            "position[]={1000.0, 1000.0, 0.0}; };\n"
                            "    };\n"
                            "};\n"
                            "class CfgGuerrillaFactions\n"
                            "{\n"
                            "    class US { side=\"WEST\"; tiers[]={\"SoldierWB\"}; };\n"
                            "    class FIA\n"
                            "    {\n"
                            "        side=\"GUER\";\n"
                            "        tiers[]={\"SoldierGB\",\"SoldierGG\"};\n"
                            "        tierThresholds[]={4};\n"
                            "        playerClassWarrior=\"SoldierGB\";\n"
                            "        playerClassCiv=\"SoldierGFakeC\";\n"
                            "        civTier[]={\"SoldierGFakeC\",\"SoldierGFakeC2\"};\n"
                            "    };\n"
                            "};\n";

struct ParsedConfig
{
    ParamFile file;

    explicit ParsedConfig(const char* text)
    {
        QIStream in(text, strlen(text));
        file.Parse(in);
    }

    const ParamEntry* Zones() const { return file.FindEntry("CfgGuerrillaZones"); }
    const ParamEntry* Factions() const { return file.FindEntry("CfgGuerrillaFactions"); }
};

} // namespace

// ---------------------------------------------------------------------------
// ResolveCivilianPlayerClass - the WorldInit substitution seam's pure core
// ---------------------------------------------------------------------------

TEST_CASE("OutfitSelect: only the civilian selection substitutes", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC"};

    // nil / warrior / junk selections all keep the authored class
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), nullptr, nullptr, probe)).empty());
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "", nullptr, probe)).empty());
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "WARRIOR", nullptr, probe)).empty());
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "outlandish", nullptr, probe)).empty());
    // the publish token is compared case-insensitively ("CIVILIAN" is what
    // the cycler publishes; the engine reads the lowercased var)
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "CIVILIAN", nullptr, probe)) ==
            "SoldierGFakeC");
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", nullptr, probe)) ==
            "SoldierGFakeC");
}

TEST_CASE("OutfitSelect: resistance-block precedence is selection > defaultResistance > GUER", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC"};

    // no selection: defaultResistance="GUER" resolves the FIA block
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", nullptr, probe)) ==
            "SoldierGFakeC");
    // selection by class name and by side both resolve the same block
    // (side first, then class name - ZoneRegistry::FindFaction order)
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", "FIA", probe)) == "SoldierGFakeC");
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", "GUER", probe)) == "SoldierGFakeC");
    // a resistance whose block authors no playerClassCiv offers no outfit
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", "US", probe)).empty());
    // a selection naming no block falls back to defaultResistance
    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", "NoSuch", probe)) ==
            "SoldierGFakeC");
    // no zones config at all: the built-in GUER side still resolves
    REQUIRE(Str(ResolveCivilianPlayerClass(nullptr, cfg.Factions(), "civilian", nullptr, probe)) == "SoldierGFakeC");
}

TEST_CASE("OutfitSelect: probe failure keeps the authored class, never a fallback body", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe; // package ships NO FakeC (the Demo [Remaster] shape)

    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), cfg.Factions(), "civilian", nullptr, probe)).empty());
}

TEST_CASE("OutfitSelect: null factions config resolves nothing", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC"};

    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), nullptr, "civilian", nullptr, probe)).empty());
}

TEST_CASE("OutfitSelect: FindGuerrillaFactionEntry matches side before class name", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);

    const ParamEntry* bySide = FindGuerrillaFactionEntry(cfg.Factions(), "GUER");
    REQUIRE(bySide != nullptr);
    REQUIRE(Str(RString(bySide->GetName())) == "FIA");
    const ParamEntry* byName = FindGuerrillaFactionEntry(cfg.Factions(), "US");
    REQUIRE(byName != nullptr);
    REQUIRE(Str(RString(byName->GetName())) == "US");
    REQUIRE(FindGuerrillaFactionEntry(cfg.Factions(), "NoSuch") == nullptr);
    REQUIRE(FindGuerrillaFactionEntry(cfg.Factions(), "") == nullptr);
    REQUIRE(FindGuerrillaFactionEntry(nullptr, "GUER") == nullptr);
}

// ---------------------------------------------------------------------------
// civTier[] - plan-15 resolution with the civilian-OUTFIT ladder
// ---------------------------------------------------------------------------

namespace
{

struct RegistryFixture
{
    ParamFile file;
    ZoneRegistry registry;

    void Load(const char* config, const ClassProbe* probe)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"), nullptr,
                                nullptr, nullptr, probe);
    }
};

} // namespace

TEST_CASE("civTier: resolved ladder is served by war level with its own clamp", "[guerrilla][outfit]")
{
    FakeProbe probe;
    probe.vehicles = {"SoldierGB", "SoldierGG", "SoldierGFakeC", "SoldierGFakeC2"};
    RegistryFixture fix;
    fix.Load(kOutfitConfig, &probe);

    REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 1)) == "SoldierGFakeC");
    REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 4)) == "SoldierGFakeC2");
    REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 10)) == "SoldierGFakeC2");
    // a faction without the key serves "" - callers keep warrior classes
    REQUIRE(Str(fix.registry.FactionCivTierClass("WEST", 1)).empty());
}

TEST_CASE("civTier: unresolved entries substitute the nearest resolved civ rung", "[guerrilla][outfit]")
{
    FakeProbe probe;
    probe.vehicles = {"SoldierGB", "SoldierGG", "SoldierGFakeC"}; // no FakeC2
    RegistryFixture fix;
    fix.Load(kOutfitConfig, &probe);

    // the missing high rung degrades to the resolved low rung, not to a
    // warrior body and not to kFallbackCiv's captive class
    REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 10)) == "SoldierGFakeC");
}

TEST_CASE("civTier: an all-unresolved ladder falls back to the civilian-outfit ladder, then tiers[0]",
          "[guerrilla][outfit]")
{
    // package ships neither authored entry but does ship plain Civilian
    {
        FakeProbe probe;
        probe.vehicles = {"SoldierGB", "SoldierGG", "Civilian"};
        RegistryFixture fix;
        fix.Load(kOutfitConfig, &probe);
        REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 1)) == "Civilian");
    }
    // package ships no civilian-outfit candidate at all: the warrior tier 0
    // is the last rung (spawnable beats sartorially correct)
    {
        FakeProbe probe;
        probe.vehicles = {"SoldierGB", "SoldierGG"};
        RegistryFixture fix;
        fix.Load(kOutfitConfig, &probe);
        REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 1)) == "SoldierGB");
    }
    // nothing spawnable anywhere: honest inert ("")
    {
        FakeProbe probe;
        RegistryFixture fix;
        fix.Load(kOutfitConfig, &probe);
        REQUIRE(Str(fix.registry.FactionCivTierClass("GUER", 1)).empty());
    }
}

TEST_CASE("Civ-family scalar keys ride the plan-15 unit-key resolution", "[guerrilla][outfit]")
{
    const char* config = "class CfgGuerrillaFactions\n"
                         "{\n"
                         "    class FIA\n"
                         "    {\n"
                         "        side=\"GUER\";\n"
                         "        tiers[]={\"SoldierGB\"};\n"
                         "        recruitFighterCiv=\"SoldierGFakeC\";\n"
                         "        holdClassCiv=\"NoSuchClass\";\n"
                         "    };\n"
                         "};\n";
    FakeProbe probe;
    probe.vehicles = {"SoldierGB", "SoldierGFakeC"};
    RegistryFixture fix;
    fix.Load(config, &probe);

    // a resolvable Civ key survives untouched
    REQUIRE(Str(fix.registry.FactionValue("GUER", "recruitFighterCiv")) == "SoldierGFakeC");
    // an unresolvable one degrades to the warrior tier-0 fallback (accepted
    // trade-off, issue #25 Part 4 - scripts get a spawnable body; the
    // gmClassExists probe in the scripts is belt-and-braces on top)
    REQUIRE(Str(fix.registry.FactionValue("GUER", "holdClassCiv")) == "SoldierGB");
}
