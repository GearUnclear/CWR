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

TEST_CASE("OutfitSelect: an explicit player-class pick beats the outfit token", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC", "SoldierWB"};

    // the pick wins even while the outfit token would substitute the civ body
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), "SoldierWB", "civilian", nullptr, probe)) ==
            "SoldierWB");
    // and with no token at all - the pick needs no outfit channel
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), "SoldierWB", nullptr, nullptr, probe)) ==
            "SoldierWB");
    // no pick: the token path decides exactly as before
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), nullptr, "civilian", nullptr, probe)) ==
            "SoldierGFakeC");
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), "", "civilian", nullptr, probe)) ==
            "SoldierGFakeC");
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), nullptr, "WARRIOR", nullptr, probe)).empty());
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), nullptr, nullptr, nullptr, probe)).empty());
}

TEST_CASE("OutfitSelect: a pick that fails the probe keeps the authored class, never the token body",
          "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC"}; // the pick is NOT in the package

    // EMPTY = keep the authored mission.sqm class. Deliberately NOT the
    // civilian body: the pick replaced the outfit resolution, so its failure
    // degrades to the authored class, never to a third body.
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), cfg.Factions(), "SoldierWB", "civilian", nullptr, probe)).empty());
}

TEST_CASE("OutfitSelect: null factions config resolves nothing", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kOutfitConfig);
    FakeProbe probe;
    probe.vehicles = {"SoldierGFakeC"};

    REQUIRE(Str(ResolveCivilianPlayerClass(cfg.Zones(), nullptr, "civilian", nullptr, probe)).empty());
}

// issue #46 seam 1: ApplyPlayerOutfitSelection used to return early unless BOTH
// CfgGuerrillaZones and CfgGuerrillaFactions resolved, so a template authoring
// one block and not the other discarded every body pick in silence. The gate is
// gone; this pins the property that made it removable - the pick path reads
// neither block, so passing nulls straight through is safe and still resolves.
TEST_CASE("OutfitSelect: a body pick resolves with neither descriptor block present", "[guerrilla][outfit]")
{
    FakeProbe probe;
    probe.vehicles = {"SoldierWB"};

    REQUIRE(Str(ResolvePlayerBodyClass(nullptr, nullptr, "SoldierWB", nullptr, nullptr, probe)) == "SoldierWB");
    // a zones-only template (the realistic half-authored shape) resolves too
    ParsedConfig cfg(kOutfitConfig);
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), nullptr, "SoldierWB", "civilian", "GUER", probe)) == "SoldierWB");
    // ...while the token half still degrades to the authored class, with no
    // dereference of the missing block
    REQUIRE(Str(ResolvePlayerBodyClass(cfg.Zones(), nullptr, nullptr, "civilian", "GUER", probe)).empty());
    REQUIRE(Str(ResolvePlayerBodyClass(nullptr, nullptr, nullptr, "civilian", nullptr, probe)).empty());
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
// CollectPlayerBodyAddons - the addon closure the picked body needs active
// (issue #45: a package-wide roster pick is by construction absent from the
// mission's addOns[], so its owner and its loadout's owners get denied)
// ---------------------------------------------------------------------------

namespace
{

// a mod body whose weapon and ammo live in THREE different pbos - the @LoBo
// shape that produced the "Access denied: lobois / loboweapons / loboweapnad"
// warnings on vanilla Guerrilla.Abel
const char* kAddonConfig = "class CfgVehicles\n"
                           "{\n"
                           "    class SoldierWB { weapons[]={\"M16\"}; magazines[]={\"M16\"}; };\n"
                           "    class ModBase { weapons[]={\"ModRifle\"}; magazines[]={\"ModRifleMag\"}; };\n"
                           "    class ModSoldier : ModBase {};\n"
                           "    class ModUnarmed {};\n"
                           "};\n"
                           "class CfgWeapons\n"
                           "{\n"
                           "    class M16 {};\n"
                           "    class ModRifle { magazines[]={\"ModRifleMag\",\"ModTracerMag\"}; };\n"
                           "};\n"
                           "class CfgMagazines\n"
                           "{\n"
                           "    class M16 {};\n"
                           "    class ModRifleMag {};\n"
                           "    class ModTracerMag {};\n"
                           "};\n";

// A parsed ParamFile carries no owners (owners are stamped by the AddonSystem
// when a pbo's config is merged in), so the fixture stamps them by hand to model
// "this class came from that pbo". SetOwner(name, false) touches the class only,
// which is how a real merge marks a single class.
struct AddonConfig
{
    ParamFile file;

    AddonConfig()
    {
        QIStream in(kAddonConfig, strlen(kAddonConfig));
        file.Parse(in);
        Own("CfgVehicles", "ModBase", "modunits");
        Own("CfgVehicles", "ModSoldier", "modunits");
        Own("CfgVehicles", "ModUnarmed", "modunits");
        Own("CfgWeapons", "ModRifle", "modweapons");
        Own("CfgMagazines", "ModRifleMag", "modammo");
        Own("CfgMagazines", "ModTracerMag", "modammo");
    }

    void Own(const char* bank, const char* className, const char* owner)
    {
        ParamEntry* bankEntry = file.FindEntry(bank);
        REQUIRE(bankEntry != nullptr);
        ParamEntry* cls = bankEntry->FindEntry(className);
        REQUIRE(cls != nullptr);
        cls->SetOwner(RString(owner), false);
    }

    const ParamEntry* Vehicles() const { return file.FindEntry("CfgVehicles"); }
    const ParamEntry* Weapons() const { return file.FindEntry("CfgWeapons"); }
    const ParamEntry* Magazines() const { return file.FindEntry("CfgMagazines"); }
};

std::vector<std::string> Collected(const AddonConfig& cfg, const char* className)
{
    FindArrayRStringCI addons;
    CollectPlayerBodyAddons(cfg.Vehicles(), cfg.Weapons(), cfg.Magazines(), RString(className), addons);
    std::vector<std::string> out;
    for (int i = 0; i < addons.Size(); i++)
    {
        out.push_back(Str(addons[i]));
    }
    return out;
}

bool Has(const std::vector<std::string>& v, const char* name)
{
    for (const std::string& s : v)
    {
        if (s == name)
        {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("CollectPlayerBodyAddons: base-game content contributes no owners", "[guerrilla][outfit]")
{
    AddonConfig cfg;

    // SoldierWB and its M16/M16 mag are unowned (the base game), and an EMPTY
    // owner is always visible to ParamOwnerList - collecting it would only put a
    // meaningless empty name into the active list.
    REQUIRE(Collected(cfg, "SoldierWB").empty());
    // an unknown class resolves nothing at all
    REQUIRE(Collected(cfg, "NoSuchBody").empty());
    REQUIRE(Collected(cfg, "").empty());
}

TEST_CASE("CollectPlayerBodyAddons: the class owner plus the loadout's owners are collected", "[guerrilla][outfit]")
{
    AddonConfig cfg;

    // A body with no weapons[]/magazines[] of its own still needs its own owner.
    std::vector<std::string> bare = Collected(cfg, "ModUnarmed");
    REQUIRE(bare.size() == 1);
    REQUIRE(bare[0] == "modunits");

    // ModSoldier inherits weapons[]/magazines[] from ModBase, so the walk has to
    // use FindEntry (which resolves through the base class), not
    // FindEntryNoInheritance - most soldier bodies never restate their loadout.
    std::vector<std::string> full = Collected(cfg, "ModSoldier");
    REQUIRE(Has(full, "modunits"));   // the body class itself
    REQUIRE(Has(full, "modweapons")); // weapons[] -> CfgWeapons owner
    REQUIRE(Has(full, "modammo"));    // magazines[] + the weapon's own magazines[]
    // three distinct pbos, each named ONCE: AddUnique de-duplicates the owner
    // that both ModRifleMag and ModTracerMag share, and the one ModBase's
    // magazines[] reaches through a second path.
    REQUIRE(full.size() == 3);
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

// ---------------------------------------------------------------------------
// PlayerBodyModelIssue - the shape half the mission-time ClassProbe never
// checks (issue #46 seam 4). The menu roster is built behind a model+.p3d gate
// (GuerrillaListPlayerBodies), the substitution seam's probe asks CfgVehicles
// only, and a descriptor's playerClassCiv is shape-probed nowhere at all - so
// the seam can be handed a class that resolves and cannot be drawn - which is
// not cosmetic: a model naming a .p3d the package does not ship access-violates
// in Man::Init during CreateVehicle (reproduced by pointing Abel's
// playerClassCiv at the base class "Man"). The caller treats a non-empty reason
// as a failed existence test and keeps the AUTHORED class; this function itself
// only names the reason, and never proposes a different body.
// ---------------------------------------------------------------------------

namespace
{

const char* kShapeConfig = "class CfgVehicles\n"
                           "{\n"
                           "    class Shipped { model=\"golani\"; };\n"
                           "    class Absent { model=\"missingbody\"; };\n"
                           "    class Shapeless { model=\"\"; };\n"
                           "    class NoModelKey { scope=2; };\n"
                           "};\n";

// the package ships exactly one p3d, under the name GetShapeName produces
// (unrooted model values get the "data3d\" prefix and the ".p3d" extension,
// lowercased - the exact string the engine's QIFStreamB::FileExist is handed)
bool ShippedShapes(RString path)
{
    return Str(path) == "data3d\\golani.p3d";
}

} // namespace

TEST_CASE("PlayerBodyModelIssue: a class the package can draw reports nothing", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kShapeConfig);
    const ParamEntry* vehicles = cfg.file.FindEntry("CfgVehicles");
    REQUIRE(vehicles != nullptr);

    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("Shipped"), ShippedShapes)).empty());
    // an unknown class is the ClassProbe's verdict to give, not this one's
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("NeverHeardOfIt"), ShippedShapes)).empty());
    // no config / no class name / no probe: nothing to report either
    REQUIRE(Str(PlayerBodyModelIssue(nullptr, RString("Shipped"), ShippedShapes)).empty());
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString(), ShippedShapes)).empty());
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("Absent"), nullptr)).empty());
}

TEST_CASE("PlayerBodyModelIssue: a missing or empty model is named, not substituted", "[guerrilla][outfit]")
{
    ParsedConfig cfg(kShapeConfig);
    const ParamEntry* vehicles = cfg.file.FindEntry("CfgVehicles");

    // model[] names a p3d the package does not ship: the unit spawns as an
    // empty LODShape, i.e. an invisible player, with nothing else in the log
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("Absent"), ShippedShapes)) ==
            "shape file 'data3d\\missingbody.p3d' is not in the loaded data package");
    // model="" and no model key at all both leave the type shapeless, which
    // VehicleTypes refuses to create for a crewed simulation - no player unit
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("Shapeless"), ShippedShapes)) == "the class authors no model");
    REQUIRE(Str(PlayerBodyModelIssue(vehicles, RString("NoModelKey"), ShippedShapes)) == "the class authors no model");
}
