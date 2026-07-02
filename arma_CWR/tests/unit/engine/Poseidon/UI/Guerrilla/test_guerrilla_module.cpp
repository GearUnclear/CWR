#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <vector>

using Poseidon::GameModuleId;
using Poseidon::GameModuleRegistry;
using Poseidon::GuerrillaListFactions;
using Poseidon::GuerrillaListIslands;
using Poseidon::IDC_MAIN_GUERRILLA;

// ---------------------------------------------------------------------------
// Module registration — same explicit XxxModule::Register() init mechanism the
// application uses for the sibling modules (GameApplication.cpp).
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaModule: Register puts Guerrilla into the registry", "[UI][GameModule][Guerrilla]")
{
    GameModuleRegistry::Clear();
    REQUIRE_FALSE(GameModuleRegistry::IsRegistered(GameModuleId::Guerrilla));

    Poseidon::GuerrillaModule::Register();

    REQUIRE(GameModuleRegistry::IsRegistered(GameModuleId::Guerrilla));
    const auto* mod = GameModuleRegistry::Find(GameModuleId::Guerrilla);
    REQUIRE(mod != nullptr);
    REQUIRE(std::string(mod->name) == "Guerrilla");
    REQUIRE(mod->menuButtonIDC == IDC_MAIN_GUERRILLA);
    REQUIRE(mod->menuAction != nullptr); // don't invoke — needs the live UI stack

    GameModuleRegistry::Clear();
}

TEST_CASE("GuerrillaModule: FindByIDC round-trip", "[UI][GameModule][Guerrilla]")
{
    GameModuleRegistry::Clear();
    Poseidon::GuerrillaModule::Register();

    const auto* byIDC = GameModuleRegistry::FindByIDC(IDC_MAIN_GUERRILLA);
    REQUIRE(byIDC != nullptr);
    REQUIRE(byIDC->id == GameModuleId::Guerrilla);
    REQUIRE(byIDC == GameModuleRegistry::Find(GameModuleId::Guerrilla));

    GameModuleRegistry::Clear();
}

TEST_CASE("GuerrillaModule: coexists with the sibling modules", "[UI][GameModule][Guerrilla]")
{
    GameModuleRegistry::Clear();
    GameModuleRegistry::Register({GameModuleId::Missions, "Single Missions", 117, nullptr});
    GameModuleRegistry::Register({GameModuleId::Mods, "Mods", 119, nullptr});
    Poseidon::GuerrillaModule::Register();

    REQUIRE(GameModuleRegistry::All().size() == 3);
    REQUIRE(GameModuleRegistry::IsRegistered(GameModuleId::Guerrilla));
    // The Guerrilla button idc must not collide with any other module's
    const auto* byIDC = GameModuleRegistry::FindByIDC(IDC_MAIN_GUERRILLA);
    REQUIRE(byIDC != nullptr);
    REQUIRE(byIDC->id == GameModuleId::Guerrilla);

    GameModuleRegistry::Clear();
}

TEST_CASE("GuerrillaModule: main-menu contract constants", "[UI][GameModule][Guerrilla]")
{
    // 120 is the unused slot between IDC_MAIN_MODS (119) and IDC_MAIN_LOAD
    // (121) — DisplayMain injects the button with this idc at runtime.
    REQUIRE(IDC_MAIN_GUERRILLA == 120);
    // Above the vanilla dialog-id range (IDD_JOIN_REQUIREMENTS == 75), below
    // IDD_UNITINFO (100).
    REQUIRE(Poseidon::IDD_GUERRILLA_NEW_GAME == 76);
    // Script-visible selection globals the template mission's init.sqs reads.
    REQUIRE(std::string(Poseidon::kGuerrillaVarIsland) == "gmSelIsland");
    REQUIRE(std::string(Poseidon::kGuerrillaVarOccupier) == "gmSelOccupier");
    REQUIRE(std::string(Poseidon::kGuerrillaVarResistance) == "gmSelResistance");
}

// ---------------------------------------------------------------------------
// Pure list builders (injected ParamFile — no engine/resource state needed).
// ---------------------------------------------------------------------------

namespace
{
std::vector<std::string> AsStrings(const std::vector<RString>& in)
{
    std::vector<std::string> out;
    for (const RString& s : in)
        out.push_back(std::string((const char*)s));
    return out;
}
} // namespace

TEST_CASE("GuerrillaListFactions: null config falls back to the built-in side defaults", "[UI][Guerrilla]")
{
    REQUIRE(AsStrings(GuerrillaListFactions(nullptr, Poseidon::kGuerrillaDefaultOccupier)) ==
            std::vector<std::string>{"EAST"});
    REQUIRE(AsStrings(GuerrillaListFactions(nullptr, Poseidon::kGuerrillaDefaultResistance)) ==
            std::vector<std::string>{"GUER"});
}

TEST_CASE("GuerrillaListFactions: subclasses are filtered by side", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* soviets = factions->AddClass("Soviets");
    soviets->Add("side", "EAST");
    ParamClass* partisans = factions->AddClass("Partisans");
    partisans->Add("side", "GUER");
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    factions->Add("someValue", 1); // non-class entries must be ignored

    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "EAST")) ==
            std::vector<std::string>{"Soviets"});
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "GUER")) ==
            (std::vector<std::string>{"Partisans", "FIA"}));
}

TEST_CASE("GuerrillaListFactions: side matching is case-insensitive and defaults to the class name", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* east = factions->AddClass("EAST"); // no side entry — class name is the side
    (void)east;
    ParamClass* guer = factions->AddClass("Rebels");
    guer->Add("side", "guer"); // lowercase on purpose

    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "EAST")) ==
            std::vector<std::string>{"EAST"});
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "GUER")) ==
            std::vector<std::string>{"Rebels"});
}

TEST_CASE("GuerrillaListFactions: no side match offers every subclass", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* a = factions->AddClass("Alpha");
    a->Add("side", "EAST");
    ParamClass* b = factions->AddClass("Bravo");
    b->Add("side", "EAST");

    // Nothing declares side GUER — let the player pick any faction instead of
    // silently inventing one.
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "GUER")) ==
            (std::vector<std::string>{"Alpha", "Bravo"}));
}

TEST_CASE("GuerrillaListFactions: config class with no subclasses falls back to the default", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    factions->Add("tickInterval", 5); // values only, no subclasses

    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "GUER")) ==
            std::vector<std::string>{"GUER"});
}

TEST_CASE("GuerrillaListIslands: null world list yields nothing", "[UI][Guerrilla]")
{
    REQUIRE(GuerrillaListIslands(nullptr, [](RString) { return true; }).empty());
}

TEST_CASE("GuerrillaListIslands: subclass names filtered by the exists predicate", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* worlds = cfg.AddClass("CfgWorldList");
    worlds->AddClass("Abel");
    worlds->AddClass("Cain");
    worlds->AddClass("Eden");
    worlds->Add("stray", "value"); // non-class entries must be ignored

    std::set<std::string> onDisk = {"Abel", "Eden"};
    std::vector<RString> islands = GuerrillaListIslands(cfg.FindEntry("CfgWorldList"), [&](RString name)
                                                        { return onDisk.count(std::string((const char*)name)) > 0; });

    REQUIRE(AsStrings(islands) == (std::vector<std::string>{"Abel", "Eden"}));
}

TEST_CASE("GuerrillaListIslands: predicate sees the raw world class name", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* worlds = cfg.AddClass("CfgWorldList");
    worlds->AddClass("Intro");

    std::vector<std::string> seen;
    GuerrillaListIslands(cfg.FindEntry("CfgWorldList"),
                         [&](RString name)
                         {
                             seen.push_back(std::string((const char*)name));
                             return true;
                         });
    REQUIRE(seen == std::vector<std::string>{"Intro"});
}
