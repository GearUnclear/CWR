// Transitive addon closure (issue #54 C1): ArcadeUnitInfo::RequiredAddons
// follows weapons[]/magazines[], ArcadeTemplate::RequiredAddonsFrom walks
// every placed unit, and ZoneRegistry::CollectFactionAddons walks every
// class a faction descriptor names - so a template's addOns[] needs only its
// world and what mission.sqm places.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/Asset/Addon/AddonClosure.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <algorithm>
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

std::vector<std::string> Sorted(const FindArrayRStringCI& addons)
{
    std::vector<std::string> out;
    for (int i = 0; i < addons.Size(); i++)
    {
        out.push_back(Str(addons[i]));
    }
    std::sort(out.begin(), out.end());
    return out;
}

// A mod split the way @LoBo is: bodies in one pbo, rifles in a second,
// ammo in a third, a vehicle in a fourth; a vanilla body with empty owners.
const char* kPackage = "class CfgPatches\n"
                       "{\n"
                       "    class modunits { units[] = {\"ModSoldier\"}; weapons[] = {}; };\n"
                       "};\n"
                       "class CfgMagazines\n"
                       "{\n"
                       "    class ModRifleMag {};\n"
                       "    class ModTracerMag {};\n"
                       "    class ModShell {};\n"
                       "    class M16 {};\n"
                       "};\n"
                       "class CfgWeapons\n"
                       "{\n"
                       "    class ModRifle { magazines[] = {\"ModRifleMag\", \"ModTracerMag\"}; };\n"
                       "    class ModCannon { magazines[] = {\"ModShell\"}; };\n"
                       "    class M16 { magazines[] = {\"M16\"}; };\n"
                       "};\n"
                       "class CfgVehicles\n"
                       "{\n"
                       "    class SoldierWB { weapons[] = {\"M16\"}; magazines[] = {\"M16\"}; };\n"
                       "    class ModBase { weapons[] = {\"ModRifle\"}; magazines[] = {\"ModRifleMag\"}; };\n"
                       "    class ModSoldier : ModBase {};\n"
                       "    class ModTank { weapons[] = {\"ModCannon\"}; magazines[] = {\"ModShell\"}; };\n"
                       "    class ModCiv {};\n"
                       "};\n";

// A parsed ParamFile carries no owners (the AddonSystem stamps them when a
// pbo's config merges in), so the fixture stamps them by hand.
struct Package
{
    ParamFile file;

    Package()
    {
        QIStream in(kPackage, strlen(kPackage));
        file.Parse(in);
        Own("CfgVehicles", "ModBase", "modunits");
        Own("CfgVehicles", "ModSoldier", "modunits");
        Own("CfgVehicles", "ModCiv", "modunits");
        Own("CfgVehicles", "ModTank", "modvehicles");
        Own("CfgWeapons", "ModRifle", "modweapons");
        Own("CfgWeapons", "ModCannon", "modvehicles");
        Own("CfgMagazines", "ModRifleMag", "modammo");
        Own("CfgMagazines", "ModTracerMag", "modammo");
        Own("CfgMagazines", "ModShell", "modshells");
    }

    void Own(const char* bank, const char* className, const char* owner)
    {
        ParamEntry* bankEntry = file.FindEntry(bank);
        REQUIRE(bankEntry != nullptr);
        ParamEntry* cls = bankEntry->FindEntry(className);
        REQUIRE(cls != nullptr);
        cls->SetOwner(RString(owner), false);
    }

    const ParamEntry* Patches() const { return file.FindEntry("CfgPatches"); }
    const ParamEntry* Vehicles() const { return file.FindEntry("CfgVehicles"); }
    const ParamEntry* Weapons() const { return file.FindEntry("CfgWeapons"); }
    const ParamEntry* Magazines() const { return file.FindEntry("CfgMagazines"); }
};

ArcadeUnitInfo Unit(const char* vehicle)
{
    ArcadeUnitInfo u;
    u.Init();
    u.vehicle = vehicle;
    return u;
}

} // namespace

TEST_CASE("AddonClosure: a vehicle class pulls its weapons' and magazines' owners", "[guerrilla][addons]")
{
    Package pkg;
    FindArrayRStringCI addons;
    CollectVehicleClassAddons(pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), "ModSoldier", addons);
    // weapons[]/magazines[] are INHERITED from ModBase and still followed
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modammo", "modunits", "modweapons"});

    addons.Clear();
    CollectVehicleClassAddons(pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), "SoldierWB", addons);
    REQUIRE(addons.Size() == 0); // base game: empty owners contribute nothing

    addons.Clear();
    CollectVehicleClassAddons(pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), "NoSuchClass", addons);
    REQUIRE(addons.Size() == 0);

    addons.Clear();
    CollectWeaponAddons(pkg.Weapons(), pkg.Magazines(), "ModCannon", addons);
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modshells", "modvehicles"});

    addons.Clear();
    CollectMagazineAddons(pkg.Magazines(), "ModTracerMag", addons);
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modammo"});
}

TEST_CASE("ArcadeUnitInfo::RequiredAddonsFrom: CfgPatches units[] hit, owner, and the transitive closure",
          "[guerrilla][addons]")
{
    Package pkg;
    FindArrayRStringCI addons;
    Unit("ModSoldier").RequiredAddonsFrom(pkg.Patches(), pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), addons);
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modammo", "modunits", "modweapons"});

    // a class no CfgPatches lists is still attributed through its owner
    addons.Clear();
    Unit("ModTank").RequiredAddonsFrom(pkg.Patches(), pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), addons);
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modshells", "modvehicles"});

    // null roots are tolerated (a package with no CfgPatches at all)
    addons.Clear();
    Unit("ModSoldier").RequiredAddonsFrom(nullptr, pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), addons);
    REQUIRE(Sorted(addons) == std::vector<std::string>{"modammo", "modunits", "modweapons"});
}

TEST_CASE("ArcadeTemplate::RequiredAddonsFrom: every placed unit and empty vehicle, de-duplicated",
          "[guerrilla][addons]")
{
    Package pkg;
    ArcadeTemplate t;
    ArcadeGroupInfo g;
    g.units.Add(Unit("SoldierWB"));
    g.units.Add(Unit("ModSoldier"));
    g.units.Add(Unit("ModSoldier"));
    t.groups.Add(g);
    t.emptyVehicles.Add(Unit("ModTank"));

    FindArrayRStringCI addons;
    t.RequiredAddonsFrom(pkg.Patches(), pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), addons);
    REQUIRE(Sorted(addons) ==
            std::vector<std::string>{"modammo", "modshells", "modunits", "modvehicles", "modweapons"});
}

TEST_CASE("ZoneRegistry::CollectFactionAddons: every class a descriptor names", "[guerrilla][addons]")
{
    Package pkg;
    const char* factions = "class CfgGuerrillaFactions\n"
                           "{\n"
                           "    class EAST { side=\"EAST\"; tiers[]={\"SoldierWB\"}; officer=\"SoldierWB\"; };\n"
                           "    class Mod\n"
                           "    {\n"
                           "        side=\"GUER\";\n"
                           "        tiers[]={\"ModSoldier\"};\n"
                           "        vehicles[]={\"ModTank\"};\n"
                           "        officer=\"ModSoldier\";\n"
                           "        lootMGWeapon=\"ModCannon\";\n"
                           "        lootATMag=\"ModTracerMag\";\n"
                           "        recruitFighterCiv=\"ModCiv\";\n"
                           "    };\n"
                           "    class CIV { side=\"CIV\"; civClassCount=1; civClass1=\"ModCiv\"; };\n"
                           "};\n";
    ParamFile f;
    QIStream in(factions, strlen(factions));
    f.Parse(in);

    ZoneRegistry reg;
    reg.LoadFromParams(nullptr, f.FindEntry("CfgGuerrillaFactions"));
    REQUIRE(reg.NFactions() == 3);
    REQUIRE(reg.GetFaction(1) != nullptr);
    REQUIRE(Str(reg.GetFaction(1)->className) == "Mod");
    REQUIRE(reg.GetFaction(3) == nullptr);

    FindArrayRStringCI addons;
    reg.CollectFactionAddons(pkg.Vehicles(), pkg.Weapons(), pkg.Magazines(), addons);
    REQUIRE(Sorted(addons) ==
            std::vector<std::string>{"modammo", "modshells", "modunits", "modvehicles", "modweapons"});
}
