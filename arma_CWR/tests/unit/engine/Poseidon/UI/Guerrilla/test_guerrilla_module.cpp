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

TEST_CASE("GuerrillaListFactions: null config yields no entries (no built-in is invented)", "[UI][Guerrilla]")
{
    // An empty list makes the display publish EMPTY selections, so the
    // mission's defaultOccupier/defaultResistance config keys win in
    // ZoneRegistry::LoadFromParams. Inventing "EAST"/"GUER" entries here used
    // to publish those side strings, which matched a mission faction by side
    // and silently overrode the mission's defaults.
    REQUIRE(GuerrillaListFactions(nullptr, Poseidon::kGuerrillaDefaultOccupier).empty());
    REQUIRE(GuerrillaListFactions(nullptr, Poseidon::kGuerrillaDefaultResistance).empty());
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

TEST_CASE("GuerrillaListFactions: config class with no subclasses yields no entries", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    factions->Add("tickInterval", 5); // values only, no subclasses

    REQUIRE(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"), "GUER").empty());
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

// ---------------------------------------------------------------------------
// Template resolution + faction-side helpers (injected predicates/config).
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaTemplateMissionBase: the exact base the launch path resolves", "[UI][Guerrilla]")
{
    REQUIRE(std::string((const char*)Poseidon::GuerrillaTemplateMissionBase("Abel")) == "missions\\Guerrilla.Abel");
}

TEST_CASE("GuerrillaTemplateExists: banked, unbanked and missing templates", "[UI][Guerrilla]")
{
    std::set<std::string> files;
    auto exists = [&](RString p) { return files.count(std::string((const char*)p)) > 0; };

    SECTION("banked .pbo wins without touching the unbanked check")
    {
        files = {"missions\\Guerrilla.Abel.pbo"};
        bool unbankedAsked = false;
        REQUIRE(Poseidon::GuerrillaTemplateExists("Abel", exists,
                                                  [&](RString)
                                                  {
                                                      unbankedAsked = true;
                                                      return false;
                                                  }));
        REQUIRE_FALSE(unbankedAsked);
    }

    SECTION("unbanked directory form")
    {
        files = {"missions\\Guerrilla.Cain\\mission.sqm"};
        REQUIRE(Poseidon::GuerrillaTemplateExists("Cain", exists, exists));
    }

    SECTION("neither form installed")
    {
        files = {"missions\\Guerrilla.Abel.pbo"};
        REQUIRE_FALSE(Poseidon::GuerrillaTemplateExists("Eden", exists, exists));
    }
}

TEST_CASE("GuerrillaFactionSide: resolves a selection to its side for pair validation", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* idf = factions->AddClass("IDF");
    idf->Add("side", "WEST");
    factions->AddClass("EAST"); // no side entry — class name is the side
    const ParamEntry* entry = cfg.FindEntry("CfgGuerrillaFactions");

    // side key, class-name default, and the unvalidatable cases (empty
    // selection / unknown class / no config) all resolve empty
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "IDF")) == "WEST");
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "EAST")) == "EAST");
    REQUIRE(Poseidon::GuerrillaFactionSide(entry, "").GetLength() == 0);
    REQUIRE(Poseidon::GuerrillaFactionSide(entry, "NoSuchFaction").GetLength() == 0);
    REQUIRE(Poseidon::GuerrillaFactionSide(nullptr, "IDF").GetLength() == 0);

    // the same-side pair the IDC_OK guard must catch: two different faction
    // classes resolving to one side (the guard compares case-insensitively)
    ParamClass* idf2 = factions->AddClass("IDF_Reserve");
    idf2->Add("side", "west");
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "IDF")) == "WEST");
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "IDF_Reserve")) == "west");
}
