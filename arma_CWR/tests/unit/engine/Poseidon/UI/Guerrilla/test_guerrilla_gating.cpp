// Menu-time faction availability gating (issue #54 A2): a merged faction whose
// classes the loaded data package does not carry is listed greyed and refused
// on OK, instead of launching into plan-15 fallback bodies.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/Game/Guerrilla/FactionSources.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

struct FakeProbe final : Guerrilla::ClassProbe
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

struct ParsedConfig
{
    ParamFile file;
    explicit ParsedConfig(const char* text)
    {
        QIStream in(text, strlen(text));
        file.Parse(in);
    }
    const ParamEntry* Factions() const { return file.FindEntry("CfgGuerrillaFactions"); }
};

// a vanilla pair plus a mod roster (Hizballah) and a stub with no ladder
const char* kFactions =
    "class CfgGuerrillaFactions\n"
    "{\n"
    "    class EAST { side=\"EAST\"; tiers[]={\"SoldierEB\",\"SoldierEG\"}; };\n"
    "    class GUER { side=\"GUER\"; tiers[]={\"SoldierGB\"}; playerClassWarrior=\"SoldierGB\"; };\n"
    "    class Hizballah { side=\"EAST\"; tiers[]={\"LoBo_HizballahRifle4\"}; "
    "playerClassWarrior=\"LoBo_HizballahRifle4\"; };\n"
    "    class Stub { side=\"WEST\"; };\n"
    "    class Warrior { side=\"WEST\"; tiers[]={\"SoldierWB\"}; playerClassWarrior=\"SoldierWMod\"; };\n"
    "    class CIV { side=\"CIV\"; };\n"
    "};\n";

} // namespace

TEST_CASE("GuerrillaFactionIssue: tiers[0] in the package means launchable", "[UI][Guerrilla][gating]")
{
    ParsedConfig cfg(kFactions);
    FakeProbe vanilla;
    vanilla.vehicles = {"SoldierEB", "SoldierEG", "SoldierGB", "SoldierWB"};
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "EAST", vanilla)).empty());
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "GUER", vanilla)).empty());
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "guer", vanilla)).empty()); // case-insensitive lookup
}

TEST_CASE("GuerrillaFactionIssue: a mod faction on a vanilla-only mount is greyed, and selectable with the mod",
          "[UI][Guerrilla][gating]")
{
    ParsedConfig cfg(kFactions);
    FakeProbe vanilla;
    vanilla.vehicles = {"SoldierEB", "SoldierGB"};
    RString issue = GuerrillaFactionIssue(cfg.Factions(), "Hizballah", vanilla);
    REQUIRE(Str(issue) == "tiers[0] 'LoBo_HizballahRifle4' is not in the loaded data");

    FakeProbe withLobo = vanilla;
    withLobo.vehicles.push_back("LoBo_HizballahRifle4");
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Hizballah", withLobo)).empty());
}

TEST_CASE("GuerrillaFactionIssue: no tiers[], a missing playerClassWarrior, an unknown name", "[UI][Guerrilla][gating]")
{
    ParsedConfig cfg(kFactions);
    FakeProbe probe;
    probe.vehicles = {"SoldierEB", "SoldierGB", "SoldierWB"};
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Stub", probe)) == "authors no tiers[]");
    // the warrior body the resistance pick substitutes (A3) is probed too
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Warrior", probe)) ==
            "playerClassWarrior 'SoldierWMod' is not in the loaded data");
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Nobody", probe)) == "no such faction");
    REQUIRE(Str(GuerrillaFactionIssue(nullptr, "EAST", probe)) == "no such faction");
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), RString(), probe)) == "no such faction");
}

TEST_CASE("GuerrillaFactionIssues: parallel to the cycler list, CIV never listed", "[UI][Guerrilla][gating]")
{
    ParsedConfig cfg(kFactions);
    FakeProbe probe;
    probe.vehicles = {"SoldierEB", "SoldierGB", "SoldierWB"};
    std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
    REQUIRE(list.size() == 5); // EAST GUER Hizballah Stub Warrior
    std::vector<RString> issues = GuerrillaFactionIssues(cfg.Factions(), list, probe);
    REQUIRE(issues.size() == list.size());
    REQUIRE(Str(issues[0]).empty());
    REQUIRE(Str(issues[1]).empty());
    REQUIRE_FALSE(Str(issues[2]).empty());
    REQUIRE_FALSE(Str(issues[3]).empty());
    REQUIRE_FALSE(Str(issues[4]).empty());
}

TEST_CASE("GuerrillaFactionIssue: works over the merged (global U island) table", "[UI][Guerrilla][gating]")
{
    // the issue-#54 done-when: a faction pack's block appears on an island
    // that never authored it, greyed until its addon is mounted
    ParsedConfig global("class CfgGuerrillaFactions { class IDF { side=\"WEST\"; tiers[]={\"LoBoGolaniWB\"}; }; };");
    ParsedConfig island(kFactions);
    Guerrilla::FactionSources merged;
    merged.Build(global.Factions(), island.Factions());
    std::vector<RString> list = GuerrillaListFactions(merged.Factions());
    REQUIRE(list.size() == 6);
    REQUIRE(Str(list.back()) == "IDF");

    FakeProbe vanilla;
    vanilla.vehicles = {"SoldierEB", "SoldierGB", "SoldierWB"};
    REQUIRE(Str(GuerrillaFactionIssue(merged.Factions(), "IDF", vanilla)) ==
            "tiers[0] 'LoBoGolaniWB' is not in the loaded data");
    vanilla.vehicles.push_back("LoBoGolaniWB");
    REQUIRE(Str(GuerrillaFactionIssue(merged.Factions(), "IDF", vanilla)).empty());
}

TEST_CASE("GuerrillaUnavailableMessage: names the role, the faction and the reason", "[UI][Guerrilla][gating]")
{
    RString msg = GuerrillaUnavailableMessage("RESISTANCE", "Hizballah", "tiers[0] 'X' is not in the loaded data");
    REQUIRE(Str(msg).find("RESISTANCE faction 'Hizballah'") == 0);
    REQUIRE(Str(msg).find("tiers[0] 'X'") != std::string::npos);
    REQUIRE(std::string(kGuerrillaFactionUnavailableSuffix) == " (not in loaded data)");
}

TEST_CASE("GuerrillaFactionIssue: the menu gate holds playerClassWarrior to the launch seam's shape gate",
          "[UI][Guerrilla][gating]")
{
    // the #46 seam-4 shape: a class CfgVehicles carries whose .p3d the
    // package does not ship passes the class probe and fails at launch; with
    // the shape predicate the menu greys it out instead of offering it
    ParsedConfig cfg("class CfgVehicles\n"
                     "{\n"
                     "    class SoldierWB { model=\"\\men\\soldier.p3d\"; };\n"
                     "    class GhostBody { model=\"\\nowhere\\ghost.p3d\"; };\n"
                     "};\n"
                     "class CfgGuerrillaFactions\n"
                     "{\n"
                     "    class Real  { side=\"WEST\"; tiers[]={\"SoldierWB\"}; playerClassWarrior=\"SoldierWB\"; };\n"
                     "    class Ghost { side=\"EAST\"; tiers[]={\"SoldierWB\"}; playerClassWarrior=\"GhostBody\"; };\n"
                     "};\n");
    const ParamEntry* vehicles = cfg.file.FindEntry("CfgVehicles");
    FakeProbe probe;
    probe.vehicles = {"SoldierWB", "GhostBody"};
    auto shapeExists = [](RString path) { return std::string((const char*)path).find("soldier") != std::string::npos; };

    // without the shape predicate (the old verdict) both are offered
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Ghost", probe)).empty());
    // with it, the launch's verdict is the menu's verdict
    REQUIRE(Str(GuerrillaFactionIssue(cfg.Factions(), "Real", probe, vehicles, shapeExists)).empty());
    RString issue = GuerrillaFactionIssue(cfg.Factions(), "Ghost", probe, vehicles, shapeExists);
    REQUIRE(Str(issue).find("playerClassWarrior 'GhostBody'") == 0);
    REQUIRE(Str(issue).find("not in the loaded data package") != std::string::npos);
}

TEST_CASE("GuerrillaIndexOfSelection: a class name beats a same-side mod faction declared earlier",
          "[UI][Guerrilla][gating]")
{
    // the menu half of the same regression the registry case in
    // test_faction_sources.cpp pins: with the faction library global, the
    // cycler list carries the mod rosters before the vanilla classes, so
    // seeding Abel from defaultOccupier = "EAST" must not land on the first
    // EAST-SIDE faction (EgyptFrontier) instead of the class named EAST
    ParsedConfig cfg("class CfgGuerrillaFactions\n"
                     "{\n"
                     "    class EgyptFrontier { side=\"EAST\"; };\n"
                     "    class Jordan        { side=\"GUER\"; };\n"
                     "    class EAST          { side=\"EAST\"; };\n"
                     "    class GUER          { side=\"GUER\"; };\n"
                     "};\n");
    std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "EAST") == 2);
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "GUER") == 3);
    // a class name that is not a side still resolves, and a side string with
    // no class of that name still falls through to the side rung
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "Jordan") == 1);
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "WEST") == -1);
}
