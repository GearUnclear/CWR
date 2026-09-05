#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaCharacterSelect.hpp> // IDD contract only (the display needs the live UI stack)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>           // the parity half of the launch decision
#include <Poseidon/Game/Guerrilla/FactionSources.hpp>         // the global+island union the engine actually reads
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/PreprocC/PreprocC.hpp> // the shipped templates are full of // comments
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp> // stricmp
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string.h>
#include <string>
#include <vector>

using Poseidon::GameModuleId;
using Poseidon::GameModuleRegistry;
using Poseidon::GuerrillaDefaultSelections;
using Poseidon::GuerrillaFactionSide;
using Poseidon::GuerrillaIndexOfName;
using Poseidon::GuerrillaIndexOfSelection;
using Poseidon::GuerrillaListFactions;
using Poseidon::GuerrillaListIslands;
using Poseidon::GuerrillaSelectionIsResolvable;
using Poseidon::IDC_MAIN_GUERRILLA;
using Poseidon::Guerrilla::ZoneRegistry;

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
    REQUIRE(Poseidon::IDD_GUERRILLA_CHARACTER_SELECT == 77);
    // Script-visible selection globals the template mission's init.sqs reads.
    REQUIRE(std::string(Poseidon::kGuerrillaVarIsland) == "gmSelIsland");
    REQUIRE(std::string(Poseidon::kGuerrillaVarOccupier) == "gmSelOccupier");
    REQUIRE(std::string(Poseidon::kGuerrillaVarResistance) == "gmSelResistance");
    REQUIRE(std::string(Poseidon::kGuerrillaVarOutfit) == "gmSelOutfit");
    REQUIRE(std::string(Poseidon::kGuerrillaVarPlayerClass) == "gmSelPlayerClass");
}

// ---------------------------------------------------------------------------
// Outfit cycler choices (issue #25) — offered iff the resolved resistance
// block authors playerClassCiv; WARRIOR always index 0 (the no-op default).
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaOutfitChoices: offered only when the resistance block authors playerClassCiv",
          "[UI][Guerrilla][outfit]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* us = factions->AddClass("US");
    us->Add("side", "WEST");
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    fia->Add("playerClassCiv", "SoldierGFakeC");
    ParamClass* zones = cfg.AddClass("CfgGuerrillaZones");
    zones->Add("defaultResistance", "FIA");

    const ParamEntry* f = cfg.FindEntry("CfgGuerrillaFactions");
    const ParamEntry* z = cfg.FindEntry("CfgGuerrillaZones");

    // selected resistance offers the pair, WARRIOR first
    auto offered = Poseidon::GuerrillaOutfitChoices(f, z, RString("FIA"));
    REQUIRE(offered.size() == 2);
    REQUIRE(std::string((const char*)offered[0]) == "WARRIOR");
    REQUIRE(std::string((const char*)offered[1]) == "CIVILIAN");
    // side string resolves the same block
    REQUIRE(Poseidon::GuerrillaOutfitChoices(f, z, RString("GUER")).size() == 2);
    // a resistance without the key offers nothing — "(mission default)"
    REQUIRE(Poseidon::GuerrillaOutfitChoices(f, z, RString("US")).empty());
    // empty selection falls back to defaultResistance
    REQUIRE(Poseidon::GuerrillaOutfitChoices(f, z, RString()).size() == 2);
    // no config at all
    REQUIRE(Poseidon::GuerrillaOutfitChoices(nullptr, z, RString("FIA")).empty());
}

TEST_CASE("GuerrillaOutfitChoices: without zones config the built-in GUER side resolves", "[UI][Guerrilla][outfit]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    fia->Add("playerClassCiv", "SoldierGFakeC");

    REQUIRE(Poseidon::GuerrillaOutfitChoices(cfg.FindEntry("CfgGuerrillaFactions"), nullptr, RString()).size() == 2);
}

// ---------------------------------------------------------------------------
// Outfit preview resolution (issue #25 M4) — which body the mannequin shows,
// and the plan-15 shaped hide-instead-of-substitute gates.
// ---------------------------------------------------------------------------

namespace
{
std::string PreviewStr(const RString& s)
{
    return std::string((const char*)s);
}
} // namespace

TEST_CASE("GuerrillaOutfitPreviewClass: the outfit token picks the descriptor body key", "[UI][Guerrilla][outfit]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* us = factions->AddClass("US");
    us->Add("side", "WEST"); // authors neither body key
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    fia->Add("playerClassWarrior", "SoldierGB");
    fia->Add("playerClassCiv", "SoldierGFakeC");
    ParamClass* zones = cfg.AddClass("CfgGuerrillaZones");
    zones->Add("defaultResistance", "FIA");
    const ParamEntry* f = cfg.FindEntry("CfgGuerrillaFactions");
    const ParamEntry* z = cfg.FindEntry("CfgGuerrillaZones");

    using Poseidon::GuerrillaOutfitPreviewClass;
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString("FIA"), RString("WARRIOR"))) == "SoldierGB");
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString("FIA"), RString("CIVILIAN"))) == "SoldierGFakeC");
    // tokens compare case-insensitively, like the substitution seam's read
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString("FIA"), RString("civilian"))) == "SoldierGFakeC");
    // an EMPTY outfit previews the warrior body: "no pair offered" still has
    // an authored warrior class to show
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString("FIA"), RString())) == "SoldierGB");
    // an unknown token shows nothing rather than guessing
    REQUIRE(GuerrillaOutfitPreviewClass(f, z, RString("FIA"), RString("PILOT")).GetLength() == 0);
    // a side string resolves the same block; an empty resistance takes the
    // defaultResistance rung — the registry's precedence, like the seam
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString("GUER"), RString("WARRIOR"))) == "SoldierGB");
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewClass(f, z, RString(), RString("WARRIOR"))) == "SoldierGB");
    // a resolved block without the requested key hides the preview
    REQUIRE(GuerrillaOutfitPreviewClass(f, z, RString("US"), RString("WARRIOR")).GetLength() == 0);
    // no factions config at all
    REQUIRE(GuerrillaOutfitPreviewClass(nullptr, z, RString("FIA"), RString("WARRIOR")).GetLength() == 0);
}

TEST_CASE("GuerrillaOutfitPreviewClass: without zones config the built-in GUER side resolves",
          "[UI][Guerrilla][outfit]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    fia->Add("playerClassWarrior", "SoldierGB");

    REQUIRE(PreviewStr(Poseidon::GuerrillaOutfitPreviewClass(cfg.FindEntry("CfgGuerrillaFactions"), nullptr, RString(),
                                                             RString("WARRIOR"))) == "SoldierGB");
}

TEST_CASE("GuerrillaOutfitPreviewModel: probes the class, its model and the shape file", "[UI][Guerrilla][outfit]")
{
    ParamFile cfg;
    ParamClass* vehicles = cfg.AddClass("CfgVehicles");
    ParamClass* base = vehicles->AddClass("Soldier");
    base->Add("model", "MC vojakG2");
    ParamClass* derived = vehicles->AddClass("SoldierGB"); // inherits the model
    derived->SetBase(base->GetClassInterface());
    ParamClass* lobo = vehicles->AddClass("LoBo_Terror_01E");
    lobo->Add("model", "\\LoBo\\LoBo_Men\\terror1");
    ParamClass* shapeless = vehicles->AddClass("Ghost");
    shapeless->Add("model", ""); // broken config: an empty model
    const ParamEntry* v = cfg.FindEntry("CfgVehicles");

    using Poseidon::GuerrillaOutfitPreviewModel;
    std::vector<std::string> probed;
    auto exists = [&](RString p)
    {
        probed.push_back(std::string((const char*)p));
        return true;
    };

    // The RAW model value comes back (it feeds the control's `model` config
    // key, whose constructor runs the full FindShape chain); the probe sees
    // the GetShapeName normalization — data3d\ prefix, .p3d, lowercased —
    // the same bank rung FindShape ends on.
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewModel(v, RString("Soldier"), exists)) == "MC vojakG2");
    REQUIRE(probed.back() == "data3d\\mc vojakg2.p3d");
    // inherited `model` keys resolve (ParamClass::FindEntry follows bases)
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewModel(v, RString("SoldierGB"), exists)) == "MC vojakG2");
    // a fully-qualified addon path keeps its own directory, extension added
    REQUIRE(PreviewStr(GuerrillaOutfitPreviewModel(v, RString("LoBo_Terror_01E"), exists)) ==
            "\\LoBo\\LoBo_Men\\terror1");
    REQUIRE(probed.back() == "lobo\\lobo_men\\terror1.p3d");

    // every failed gate yields EMPTY: hide the preview, never substitute
    auto missing = [](RString) { return false; };
    REQUIRE(GuerrillaOutfitPreviewModel(v, RString("Soldier"), missing).GetLength() == 0); // shape not in package
    REQUIRE(GuerrillaOutfitPreviewModel(v, RString("NoSuchClass"), exists).GetLength() == 0);
    REQUIRE(GuerrillaOutfitPreviewModel(v, RString("Ghost"), exists).GetLength() == 0);
    REQUIRE(GuerrillaOutfitPreviewModel(nullptr, RString("Soldier"), exists).GetLength() == 0);
    REQUIRE(GuerrillaOutfitPreviewModel(v, RString(), exists).GetLength() == 0);
}

TEST_CASE("GuerrillaPreviewIsFlagProxy: flag stems match, the other body proxies do not", "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaPreviewIsFlagProxy;
    // the flag proxy every vanilla body references (Data3D.pbo p3ds)
    REQUIRE(GuerrillaPreviewIsFlagProxy("flag_vojak"));
    REQUIRE(GuerrillaPreviewIsFlagProxy("FLAG_VOJAK")); // case-insensitive
    REQUIRE(GuerrillaPreviewIsFlagProxy("usa_vlajka")); // the Czech stem, mid-name
    REQUIRE(GuerrillaPreviewIsFlagProxy("idf_flag_a")); // English stem, mid-name

    // the full remaining vanilla-body proxy roster must keep drawing
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy("ak_47_v58_proxy"));
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy("rpg7_proxy"));
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy("beretta"));
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy("nvg_proxy"));
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy("dalekohled_proxy"));

    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy(""));
    REQUIRE_FALSE(GuerrillaPreviewIsFlagProxy(nullptr));
}

TEST_CASE("GuerrillaPreviewHideWeaponProxy: only the primary rifle survives", "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaPreviewHideWeaponProxy;

    // the engine's own proxy identity: CfgNonAIVehicles "Proxy<name>", the
    // simulation key inherited through the base chain (the exact shape of
    // Classic CONFIG.BIN + the O.pbo addon config)
    ParamFile cfg;
    ParamClass* nonAI = cfg.AddClass("CfgNonAIVehicles");
    ParamClass* pw = nonAI->AddClass("ProxyWeapon");
    pw->Add("simulation", "ProxyWeapon");
    ParamClass* psw = nonAI->AddClass("ProxySecWeapon");
    psw->Add("simulation", "ProxySecWeapon");
    ParamClass* phg = nonAI->AddClass("ProxyHandGun");
    phg->Add("simulation", "ProxyHandGun");
    nonAI->AddClass("ProxyAK_47_v58_Proxy")->SetBase(pw->GetClassInterface());
    nonAI->AddClass("ProxyRPG7_Proxy")->SetBase(psw->GetClassInterface());
    nonAI->AddClass("ProxyBeretta")->SetBase(phg->GetClassInterface());
    nonAI->AddClass("Proxysome_launcher")->SetBase(psw->GetClassInterface());
    const ParamEntry* v = cfg.FindEntry("CfgNonAIVehicles");

    // the vanilla WEST body's real proxy roster (mc vojakw2.p3d): rifle
    // draws, launcher and pistol hide (civilian=false: the warrior rule)
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, "ak_47_v58_proxy", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "rpg7_proxy", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "beretta", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "RPG7_PROXY", false)); // case-insensitive
    // ShapeLOD's space-underscore normalization is mirrored in the lookup
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "some launcher", false));

    // binocular/NVG stems hide even with no config class (none exists in
    // Classic+AddOns; these proxies never get created there, but a mod that
    // does class them must not put binoculars in the mannequin's hands)
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "dalekohled_proxy", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "nvg_proxy", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(nullptr, "dalekohled_proxy", false));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(nullptr, "binoc_generic", false));

    // everything else draws: unknown gear, unclassified proxies, no config
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, "medic_bag", false));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(nullptr, "ak_47_v58_proxy", false));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, "", false));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, nullptr, false));

    // civilian=true (issue #43): the primary rifle now hides too...
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "ak_47_v58_proxy", true));
    // ...launchers, pistols and the belt-and-braces stems keep hiding...
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "rpg7_proxy", true));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "beretta", true));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(v, "dalekohled_proxy", true));
    REQUIRE(GuerrillaPreviewHideWeaponProxy(nullptr, "nvg_proxy", true));
    // ...non-weapon gear still draws (only WEAPON proxies hide for a
    // civilian), and with no config nothing identifies the rifle as a
    // weapon, so it draws rather than strip mod gear on a guess
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, "medic_bag", true));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(nullptr, "ak_47_v58_proxy", true));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, "", true));
    REQUIRE_FALSE(GuerrillaPreviewHideWeaponProxy(v, nullptr, true));
}

TEST_CASE("GuerrillaListPlayerBodies: Man-derived, creatable, playable-side, deduped by look",
          "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaBodyChoice;
    using Poseidon::GuerrillaListPlayerBodies;

    // ParamClass::Add resolves through the base chain, so own entries must
    // be added BEFORE SetBase or they overwrite the base's values - the same
    // rule the DisplayMain MODS-injection comment documents.
    ParamFile cfg;
    ParamClass* vehicles = cfg.AddClass("CfgVehicles");
    ParamClass* land = vehicles->AddClass("Land");
    ParamClass* man = vehicles->AddClass("Man"); // abstract base: never listed
    man->Add("scope", 0);
    man->Add("side", 3);
    ParamClass* wb = vehicles->AddClass("SoldierWB");
    wb->Add("scope", 2);
    wb->Add("side", 1);
    wb->Add("model", "MC vojakW2");
    wb->Add("displayName", "Rifleman");
    wb->SetBase(man->GetClassInterface());
    // same look as SoldierWB (displayName+model): deduped away
    ParamClass* wg = vehicles->AddClass("SoldierWG");
    wg->SetBase(wb->GetClassInterface());
    // different displayName, same model: a distinct row (side/scope/model
    // inherited through the base chain)
    ParamClass* wmedic = vehicles->AddClass("SoldierWMedic");
    wmedic->Add("displayName", "Medic");
    wmedic->SetBase(wb->GetClassInterface());
    ParamClass* eb = vehicles->AddClass("SoldierEB");
    eb->Add("scope", 2);
    eb->Add("side", 0);
    eb->Add("model", "MC vojakE2");
    eb->Add("displayName", "Rifleman");
    eb->SetBase(man->GetClassInterface());
    // scope=1 script-only classes are deliberate picks (SoldierGFakeC)
    ParamClass* fakeC = vehicles->AddClass("SoldierGFakeC");
    fakeC->Add("scope", 1);
    fakeC->Add("side", 2);
    fakeC->Add("model", "MC civil");
    fakeC->Add("displayName", "Civilian");
    fakeC->SetBase(man->GetClassInterface());
    ParamClass* civ = vehicles->AddClass("Civilian");
    civ->Add("scope", 2);
    civ->Add("side", 3);
    civ->Add("model", "MC civil");
    civ->Add("displayName", "Man");
    civ->SetBase(man->GetClassInterface());
    // excluded rows: abstract, logic-side, non-Man, package-missing shape
    ParamClass* abstract = vehicles->AddClass("SoldierAbstract");
    abstract->Add("scope", 0);
    abstract->Add("side", 1);
    abstract->Add("model", "MC vojakW2");
    abstract->SetBase(man->GetClassInterface());
    ParamClass* logician = vehicles->AddClass("Logician");
    logician->Add("scope", 2);
    logician->Add("side", 7);
    logician->Add("model", "MC vojakW2");
    logician->SetBase(man->GetClassInterface());
    ParamClass* car = vehicles->AddClass("Car");
    car->Add("scope", 2);
    car->Add("side", 1);
    car->Add("model", "car");
    car->SetBase(land->GetClassInterface());
    ParamClass* ghost = vehicles->AddClass("GhostBody");
    ghost->Add("scope", 2);
    ghost->Add("side", 1);
    ghost->Add("model", "missing_shape");
    ghost->Add("displayName", "Ghost");
    ghost->SetBase(man->GetClassInterface());

    // Addon attribution: stamp an owner the way AddonSystem::ParseAddonConfig
    // does (ParamClass::SetOwner, which lowercases - so a runtime GetOwner is
    // always lowercase too). The other classes stay base-game (EMPTY owner).
    eb->SetOwner("LoBo_Men");

    auto exists = [](RString path) { return std::string((const char*)path).find("missing") == std::string::npos; };
    std::vector<GuerrillaBodyChoice> bodies = GuerrillaListPlayerBodies(cfg.FindEntry("CfgVehicles"), exists);

    // WEST, EAST, GUER, CIV order; config scan order inside a side
    REQUIRE(bodies.size() == 5);
    REQUIRE(std::string((const char*)bodies[0].className) == "SoldierWB");
    REQUIRE(std::string((const char*)bodies[0].side) == "WEST");
    REQUIRE(std::string((const char*)bodies[1].className) == "SoldierWMedic");
    REQUIRE(std::string((const char*)bodies[1].side) == "WEST");
    REQUIRE(std::string((const char*)bodies[2].className) == "SoldierEB");
    REQUIRE(std::string((const char*)bodies[2].side) == "EAST");
    REQUIRE(std::string((const char*)bodies[3].className) == "SoldierGFakeC");
    REQUIRE(std::string((const char*)bodies[3].side) == "GUER");
    REQUIRE(std::string((const char*)bodies[4].className) == "Civilian");
    REQUIRE(std::string((const char*)bodies[4].side) == "CIV");

    // the display metadata rides along: config displayName (inherited keys
    // resolve through the base chain) and the owning addon (lowercased by
    // SetOwner; EMPTY for base game)
    REQUIRE(std::string((const char*)bodies[0].displayName) == "Rifleman");
    REQUIRE(std::string((const char*)bodies[1].displayName) == "Medic");
    REQUIRE(std::string((const char*)bodies[2].displayName) == "Rifleman");
    REQUIRE(std::string((const char*)bodies[2].addon) == "lobo_men");
    REQUIRE(bodies[0].addon.GetLength() == 0);
    REQUIRE(bodies[4].addon.GetLength() == 0);

    // no config / no Man class: empty, never crash
    REQUIRE(GuerrillaListPlayerBodies(nullptr, exists).empty());
    ParamFile manless;
    manless.AddClass("CfgVehicles");
    REQUIRE(GuerrillaListPlayerBodies(manless.FindEntry("CfgVehicles"), exists).empty());
}

TEST_CASE("GuerrillaListPlayerBodies: the roster is uncapped - every deduped body enumerates in config order",
          "[UI][Guerrilla][outfit]")
{
    // The old kGuerrillaMaxBodiesPerSide=24 cap existed for the flat BODY
    // cycler; the character-select screen scrolls (issue #43), so a fixture
    // bigger than the old cap must come through whole and in order.
    using Poseidon::GuerrillaListPlayerBodies;

    ParamFile cfg;
    ParamClass* vehicles = cfg.AddClass("CfgVehicles");
    ParamClass* man = vehicles->AddClass("Man");
    man->Add("scope", 0);
    const int total = 32; // > the retired 24-per-side cap
    for (int i = 0; i < total; i++)
    {
        char name[64];
        snprintf(name, sizeof(name), "LoBoBody%02d", i);
        ParamClass* body = vehicles->AddClass(name);
        body->Add("scope", 2);
        body->Add("side", 2);
        char model[64];
        snprintf(model, sizeof(model), "lobo_body_%02d", i); // all distinct looks
        body->Add("model", model);
        body->Add("displayName", name);
        body->SetBase(man->GetClassInterface()); // own entries first - see above
    }
    auto exists = [](RString) { return true; };
    auto bodies = GuerrillaListPlayerBodies(cfg.FindEntry("CfgVehicles"), exists);
    REQUIRE((int)bodies.size() == total);
    for (int i = 0; i < total; i++)
    {
        char name[64];
        snprintf(name, sizeof(name), "LoBoBody%02d", i);
        REQUIRE(std::string((const char*)bodies[i].className) == name);
    }
}

// ---------------------------------------------------------------------------
// Character-select row labels (issue #43): human-readable names, never an
// underscore (the menu fonts drop that glyph entirely).
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaSanitizeLabel: underscores become hyphens, nothing else changes", "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaSanitizeLabel;
    REQUIRE(PreviewStr(GuerrillaSanitizeLabel("LoBo_Terror_01E")) == "LoBo-Terror-01E");
    REQUIRE(PreviewStr(GuerrillaSanitizeLabel("_")) == "-");
    REQUIRE(PreviewStr(GuerrillaSanitizeLabel("SoldierWB")) == "SoldierWB"); // untouched
    REQUIRE(PreviewStr(GuerrillaSanitizeLabel("a b (c)")) == "a b (c)");     // only '_' transforms
    REQUIRE(GuerrillaSanitizeLabel(RString()).GetLength() == 0);             // empty stays empty
}

TEST_CASE("GuerrillaBodyRowLabel: displayName, ambiguous displayName, empty displayName", "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaBodyChoice;
    using Poseidon::GuerrillaBodyRowLabel;

    GuerrillaBodyChoice named{"SoldierWB", "WEST", "Rifleman", ""};
    REQUIRE(PreviewStr(GuerrillaBodyRowLabel(named, false)) == "Rifleman");
    // ambiguous: the classname suffix disambiguates, sanitized
    GuerrillaBodyChoice modded{"LoBo_Rifle_01", "WEST", "Rifleman", "lobo_men"};
    REQUIRE(PreviewStr(GuerrillaBodyRowLabel(modded, true)) == "Rifleman (LoBo-Rifle-01)");
    // no displayName at all: the sanitized classname is the label
    GuerrillaBodyChoice bare{"LoBo_Terror_01E", "GUER", "", "lobo_men"};
    REQUIRE(PreviewStr(GuerrillaBodyRowLabel(bare, false)) == "LoBo-Terror-01E");
    // a displayName carrying an underscore is sanitized too
    GuerrillaBodyChoice oddName{"SoldierEB", "EAST", "Squad_Leader", ""};
    REQUIRE(PreviewStr(GuerrillaBodyRowLabel(oddName, false)) == "Squad-Leader");
}

TEST_CASE("GuerrillaBodyRowLabels: ambiguity is per side, case-insensitive, over the final roster",
          "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaBodyChoice;
    using Poseidon::GuerrillaBodyRowLabels;

    std::vector<GuerrillaBodyChoice> roster = {
        {"SoldierWB", "WEST", "Rifleman", ""},         // shares "Rifleman" with the LoBo row below
        {"LoBo_Rifle_01", "WEST", "RIFLEMAN", "lobo"}, // case-insensitive collision
        {"SoldierWMedic", "WEST", "Medic", ""},        // unique on its side
        {"SoldierEB", "EAST", "Rifleman", ""},         // same name, OTHER side: not ambiguous
        {"LoBo_Terror_01E", "GUER", "", "lobo"},       // no displayName: classname label
    };
    std::vector<RString> labels = GuerrillaBodyRowLabels(roster);
    REQUIRE(labels.size() == roster.size());
    REQUIRE(PreviewStr(labels[0]) == "Rifleman (SoldierWB)");
    REQUIRE(PreviewStr(labels[1]) == "RIFLEMAN (LoBo-Rifle-01)");
    REQUIRE(PreviewStr(labels[2]) == "Medic");
    REQUIRE(PreviewStr(labels[3]) == "Rifleman"); // EAST's only Rifleman
    REQUIRE(PreviewStr(labels[4]) == "LoBo-Terror-01E");
    // the never-an-underscore invariant, over every label
    for (const RString& label : labels)
    {
        REQUIRE(std::string((const char*)label).find('_') == std::string::npos);
    }
}

TEST_CASE("GuerrillaClassIsCivilian: config side 3 and nothing else", "[UI][Guerrilla][outfit]")
{
    using Poseidon::GuerrillaClassIsCivilian;

    ParamFile cfg;
    ParamClass* vehicles = cfg.AddClass("CfgVehicles");
    ParamClass* man = vehicles->AddClass("Man");
    man->Add("scope", 0);
    ParamClass* east = vehicles->AddClass("SoldierEB");
    east->Add("side", 0);
    ParamClass* west = vehicles->AddClass("SoldierWB");
    west->Add("side", 1);
    ParamClass* guer = vehicles->AddClass("SoldierGB");
    guer->Add("side", 2);
    ParamClass* civ = vehicles->AddClass("Civilian");
    civ->Add("side", 3);
    // side inherited through the base chain resolves too
    ParamClass* civ2 = vehicles->AddClass("Civilian2");
    civ2->SetBase(civ->GetClassInterface());
    vehicles->AddClass("NoSide"); // no side key anywhere
    const ParamEntry* v = cfg.FindEntry("CfgVehicles");

    REQUIRE(GuerrillaClassIsCivilian(v, "Civilian"));
    REQUIRE(GuerrillaClassIsCivilian(v, "Civilian2"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, "SoldierEB"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, "SoldierWB"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, "SoldierGB"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, "NoSide"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, "NoSuchClass"));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(v, RString()));
    REQUIRE_FALSE(GuerrillaClassIsCivilian(nullptr, "Civilian"));
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
// RString compares as bytes in the failure output, which is unreadable
std::string Str(const RString& s)
{
    return std::string((const char*)s);
}
} // namespace

TEST_CASE("GuerrillaListFactions: null config yields no entries (no built-in is invented)", "[UI][Guerrilla]")
{
    // An empty list makes the display publish EMPTY selections, so the
    // mission's defaultOccupier/defaultResistance config keys win in
    // ZoneRegistry::LoadFromParams. Inventing "EAST"/"GUER" entries here used
    // to publish those side strings, which matched a mission faction by side
    // and silently overrode the mission's defaults.
    REQUIRE(GuerrillaListFactions(nullptr).empty());
}

TEST_CASE("GuerrillaListFactions: every non-CIV subclass is offered to both cyclers", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* soviets = factions->AddClass("Soviets");
    soviets->Add("side", "EAST");
    ParamClass* partisans = factions->AddClass("Partisans");
    partisans->Add("side", "GUER");
    ParamClass* fia = factions->AddClass("FIA");
    fia->Add("side", "GUER");
    ParamClass* civ = factions->AddClass("CIV");
    civ->Add("side", "CIV");
    factions->Add("someValue", 1); // non-class entries must be ignored

    // No side filter survives: the player picks a ROSTER and the registry
    // rebases whatever it collides with, so both cyclers offer the same list.
    // Declaration order is preserved; only the population is held back.
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"))) ==
            (std::vector<std::string>{"Soviets", "Partisans", "FIA"}));
}

TEST_CASE("GuerrillaListFactions: the side defaults to the class name, and CIV is excluded either way",
          "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    factions->AddClass("EAST"); // no side entry — class name is the side
    ParamClass* guer = factions->AddClass("Rebels");
    guer->Add("side", "guer"); // lowercase on purpose
    ParamClass* civilians = factions->AddClass("Civilians");
    civilians->Add("side", "civ"); // lowercase on purpose: still not a combatant
    factions->AddClass("CIV");     // no side entry — excluded via the class-name default

    // Both CIV spellings must be held back; letting either into a cycler is
    // the regression this pins (a CIV pick has no war semantics at all).
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"))) ==
            (std::vector<std::string>{"EAST", "Rebels"}));
}

TEST_CASE("GuerrillaListFactions: a side nothing declares is no longer special-cased", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    ParamClass* a = factions->AddClass("Alpha");
    a->Add("side", "EAST");
    ParamClass* b = factions->AddClass("Bravo");
    b->Add("side", "EAST");

    // Nothing declares GUER here. This used to be a fall-through case ("no
    // side matched — offer everything"), which is what left Sinai's resistance
    // cycler showing [IDF, EgyptFrontier, CIV]. Offering every non-CIV
    // subclass is now the general rule, not a fallback.
    REQUIRE(AsStrings(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions"))) ==
            (std::vector<std::string>{"Alpha", "Bravo"}));
}

TEST_CASE("GuerrillaListFactions: config class with no subclasses yields no entries", "[UI][Guerrilla]")
{
    ParamFile cfg;
    ParamClass* factions = cfg.AddClass("CfgGuerrillaFactions");
    factions->Add("tickInterval", 5); // values only, no subclasses

    REQUIRE(GuerrillaListFactions(cfg.FindEntry("CfgGuerrillaFactions")).empty());
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

    // Two faction classes resolving to one side, spelled differently: the
    // helper reports the raw strings and every caller compares them
    // case-insensitively. What the IDC_OK guard then DOES with such a pair is
    // covered by the GuerrillaSelectionIsResolvable cases below — this test
    // used to claim it covered that guard, which was false: the guard read
    // Pars, which never carries CfgGuerrillaFactions, so it never ran at all.
    // It is driven by the island's own descriptor (_islandFactions) now.
    ParamClass* idf2 = factions->AddClass("IDF_Reserve");
    idf2->Add("side", "west");
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "IDF")) == "WEST");
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionSide(entry, "IDF_Reserve")) == "west");
}

TEST_CASE("GuerrillaFactionsDescriptionPath: banked mount prefix vs unbanked mission directory", "[UI][Guerrilla]")
{
    // Banked: the template's description.ext lives under whatever prefix
    // CreateSingleMissionBank mounted the .pbo at — not under
    // missions\Guerrilla.<island>\ (that path doesn't exist for a .pbo).
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionsDescriptionPath(
                "Sinai", "missions\\__cur_sp.sinai\\")) == "missions\\__cur_sp.sinai\\description.ext");

    // Unbanked: an empty bank prefix means the unpacked mission directory,
    // i.e. exactly GuerrillaTemplateMissionBase(island)\description.ext.
    REQUIRE(std::string((const char*)Poseidon::GuerrillaFactionsDescriptionPath("Abel", "")) ==
            "missions\\Guerrilla.Abel\\description.ext");
}

// ---------------------------------------------------------------------------
// The IDC_OK launch guard.
//
// The display itself cannot be constructed headless (it needs the whole UI
// stack), so the guard's decision lives in GuerrillaSelectionIsResolvable and
// is tested directly — the same factoring rationale as FactionTwins. What must
// be pinned is that the decision reads a PER-ISLAND descriptor: a test that
// only populated Pars would pass against the old, broken, never-reached guard.
// ---------------------------------------------------------------------------

namespace
{
// island descriptor as authored: a CfgGuerrillaZones carrying playerSide next
// to a CfgGuerrillaFactions, exactly like guerrilla-mode/mission/*/description.ext
struct IslandCfg
{
    ParamFile file;

    void Parse(const char* text)
    {
        QIStream in(text, (int)strlen(text));
        file.Parse(in);
    }
    const ParamEntry* Factions() { return file.FindEntry("CfgGuerrillaFactions"); }
    const ParamEntry* Zones() { return file.FindEntry("CfgGuerrillaZones"); }
};

// Abel-shaped: a GUER player side, one EAST occupier, one GUER resistance.
const char* kPinnedTemplate = "class CfgGuerrillaZones\n"
                              "{\n"
                              "    playerSide = \"GUER\";\n"
                              "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                              "};\n"
                              "class CfgGuerrillaFactions\n"
                              "{\n"
                              "    class EAST { side=\"EAST\"; };\n"
                              "    class WEST { side=\"WEST\"; };\n"
                              "    class GUER { side=\"GUER\"; };\n"
                              "    class CIV  { side=\"CIV\";  };\n"
                              "};\n";

// the same rosters with no playerSide: the legacy templates the registry
// rebases nothing for
const char* kLegacyTemplate = "class CfgGuerrillaZones\n"
                              "{\n"
                              "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                              "};\n"
                              "class CfgGuerrillaFactions\n"
                              "{\n"
                              "    class Alpha { side=\"EAST\"; };\n"
                              "    class Bravo { side=\"EAST\"; };\n"
                              "    class NATO  { side=\"WEST\"; };\n"
                              "};\n";

// legacy AND twinned: the twin is inert here (ResolveSideCollisions never runs
// without a playerSide), which is the one place the UI could plausibly be
// tempted to be cleverer than the registry
const char* kLegacyTwinnedTemplate = "class CfgGuerrillaZones\n"
                                     "{\n"
                                     "    class Zones { class A { name=\"A\"; owner=\"OCCUPIER\"; }; };\n"
                                     "};\n"
                                     "class CfgGuerrillaFactions\n"
                                     "{\n"
                                     "    class Alpha      { side=\"EAST\"; sideTwin=\"Alpha_West\"; };\n"
                                     "    class Alpha_West { side=\"WEST\"; sideTwin=\"Alpha\"; };\n"
                                     "    class Bravo      { side=\"EAST\"; };\n"
                                     "};\n";
} // namespace

TEST_CASE("GuerrillaSelectionIsResolvable: the guard resolves sides from the island descriptor", "[UI][Guerrilla]")
{
    RString message;

    SECTION("distinct sides launch")
    {
        IslandCfg cfg;
        cfg.Parse(kPinnedTemplate);
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "EAST", "GUER", message));
        REQUIRE(message.GetLength() == 0);
    }

    SECTION("same side with a playerSide launches — the registry rebases it")
    {
        IslandCfg cfg;
        cfg.Parse(kPinnedTemplate);
        // Both picks are GUER: the resistance is already pinned there, so the
        // occupier is the one that moves (to the first free war side).
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "GUER", "GUER", message));
        // A WEST occupier against a GUER-pinned resistance: no collision, and
        // no ceasefire either (see ZoneRegistry::ApplyCampaignFriendship).
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "WEST", "GUER", message));
        // A resistance pick on another side is pinned onto playerSide, which
        // is what makes every roster resistance-playable.
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "EAST", "WEST", message));
    }

    SECTION("same side with no playerSide and no twin is blocked, naming the side")
    {
        IslandCfg cfg;
        cfg.Parse(kLegacyTemplate);
        REQUIRE_FALSE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Alpha", "Bravo", message));
        REQUIRE(std::string((const char*)message).find("EAST") != std::string::npos);
        // a pair on two sides is fine on a legacy template too
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "NATO", "Alpha", message));
    }

    SECTION("a sideTwin does NOT rescue a legacy template — the registry would not use it either")
    {
        // A twin is only ever consulted by ResolveSideCollisions, and that pass
        // returns immediately when the template authors no playerSide (the
        // legacy no-op that keeps every pre-existing side test green). So on a
        // legacy template a twinned same-side pair is still a campaign fighting
        // itself, and the guard must still block it — allowing it here because
        // a twin merely EXISTS is precisely the UI/registry drift the parity
        // test below is here to prevent.
        IslandCfg cfg;
        cfg.Parse(kLegacyTwinnedTemplate);
        REQUIRE_FALSE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Alpha", "Bravo", message));
        REQUIRE_FALSE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Bravo", "Alpha", message));
        // ...while the twin's own class, being authored on WEST, needs no rebase
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Alpha_West", "Alpha", message));
    }

    SECTION("empty and unknown selections pass through — the mission's defaults decide")
    {
        IslandCfg cfg;
        cfg.Parse(kLegacyTemplate);
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "", "", message));
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Alpha", "", message));
        REQUIRE(GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), "Martians", "Alpha", message));
        // no descriptor at all (the display's Pars fallback found nothing)
        REQUIRE(GuerrillaSelectionIsResolvable(nullptr, nullptr, "Alpha", "Bravo", message));
    }
}

// ---------------------------------------------------------------------------
// UI/registry parity — the anti-drift test.
//
// The UI decides whether to launch a pair; ZoneRegistry decides what that pair
// becomes. Nothing in the type system ties the two together, and the last time
// they diverged the guard silently stopped running for two releases. So: feed
// the SAME config to both and assert the UI allows a pair IFF the registry
// resolves it to two distinct sides.
// ---------------------------------------------------------------------------

namespace
{
// every non-CIV pick the cyclers would offer, run through both halves
void RequireParity(const char* configText)
{
    IslandCfg cfg;
    cfg.Parse(configText);
    std::vector<RString> picks = GuerrillaListFactions(cfg.Factions());
    REQUIRE_FALSE(picks.empty());

    for (const RString& occ : picks)
    {
        for (const RString& res : picks)
        {
            RString message;
            bool uiAllows = GuerrillaSelectionIsResolvable(cfg.Factions(), cfg.Zones(), occ, res, message);

            ZoneRegistry registry;
            registry.LoadFromParams(cfg.Zones(), cfg.Factions(), occ, res);
            bool registryResolved = stricmp(registry.OccupierSide(), registry.ResistanceSide()) != 0;

            INFO("occupier=" << (const char*)occ << " resistance=" << (const char*)res
                             << " -> occSide=" << (const char*)registry.OccupierSide()
                             << " resSide=" << (const char*)registry.ResistanceSide());
            REQUIRE(uiAllows == registryResolved);
            REQUIRE(uiAllows == (message.GetLength() == 0));
        }
    }
}
} // namespace

TEST_CASE("Guerrilla launch: the UI block and the registry's rebase agree on every pair", "[UI][Guerrilla]")
{
    SECTION("a playerSide template resolves every pair")
    {
        RequireParity(kPinnedTemplate);
    }

    SECTION("a legacy template blocks exactly the pairs the registry cannot separate")
    {
        RequireParity(kLegacyTemplate);
    }

    SECTION("a legacy template's sideTwins are inert to BOTH halves")
    {
        // A twin the registry never reads must not make the UI any braver.
        RequireParity(kLegacyTwinnedTemplate);
    }
}

// ---------------------------------------------------------------------------
// What the cyclers OPEN on.
//
// This is a contract, not a cosmetic default. Whatever the cyclers show is
// what IDC_OK publishes into gmSelOccupier/gmSelResistance, and a published
// selection OUTRANKS the template's own defaultOccupier/defaultResistance keys
// in ZoneRegistry::LoadFromParams. So an untouched screen must open on exactly
// the pair those keys resolve to, or merely visiting the new-game menu
// silently rewrites the campaign the template author specified.
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaIndexOfSelection: side first, then class name, as FindFaction scans", "[UI][Guerrilla]")
{
    IslandCfg cfg;
    cfg.Parse(kPinnedTemplate);
    std::vector<RString> list = GuerrillaListFactions(cfg.Factions()); // EAST, WEST, GUER (CIV filtered)

    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "WEST") == 1);
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "guer") == 2); // case-insensitive
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "") == -1);
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "CIV") == -1); // filtered off the cyclers
    REQUIRE(GuerrillaIndexOfSelection(cfg.Factions(), list, "Martians") == -1);

    SECTION("a class name that is not a side still resolves")
    {
        IslandCfg sinai;
        sinai.Parse("class CfgGuerrillaFactions\n"
                    "{\n"
                    "    class IDF           { side=\"WEST\"; };\n"
                    "    class EgyptFrontier { side=\"EAST\"; };\n"
                    "    class EgyptArmy     { side=\"EAST\"; };\n"
                    "};\n");
        std::vector<RString> picks = GuerrillaListFactions(sinai.Factions());
        REQUIRE(GuerrillaIndexOfSelection(sinai.Factions(), picks, "IDF") == 0);
        // two rosters share EAST: the class name has to pick the right one, and
        // a bare side string takes the first, exactly as FindFaction does
        REQUIRE(GuerrillaIndexOfSelection(sinai.Factions(), picks, "EgyptArmy") == 2);
        REQUIRE(GuerrillaIndexOfSelection(sinai.Factions(), picks, "EAST") == 1);
    }
}

// ---------------------------------------------------------------------------
// The island-switch keep is an IDENTITY test, not a resolution test.
//
// RefreshFactionsForIsland carries the occupier/resistance picks across an
// island change by name. Running that through GuerrillaIndexOfSelection (side
// first) makes a SIDE STRING match whatever faction fills that role on the new
// island, which is a different faction, and on the shipped templates it lands
// the occupier cycler on the island's own defaultResistance. Observed on the
// real menu: Abel (occupier = the class literally named EAST) -> Lebanon80
// opened on OCCUPIER: Hizballah / RESISTANCE: Hizballah.
// ---------------------------------------------------------------------------

TEST_CASE("GuerrillaIndexOfName: exact class name only, no side rung", "[UI][Guerrilla]")
{
    IslandCfg cfg;
    cfg.Parse(kPinnedTemplate);
    std::vector<RString> list = GuerrillaListFactions(cfg.Factions()); // EAST, WEST, GUER

    REQUIRE(GuerrillaIndexOfName(list, "WEST") == 1);
    REQUIRE(GuerrillaIndexOfName(list, "guer") == 2); // case-insensitive
    REQUIRE(GuerrillaIndexOfName(list, "") == -1);
    REQUIRE(GuerrillaIndexOfName(list, "Martians") == -1);

    SECTION("a side string does NOT alias onto the faction that fills that role")
    {
        // Lebanon80's roster: IDF is the occupier (WEST), Hizballah the
        // resistance (EAST). Carrying Abel's occupier "EAST" over must NOT
        // keep, or both cyclers land on Hizballah.
        IslandCfg leb;
        leb.Parse("class CfgGuerrillaFactions\n"
                  "{\n"
                  "    class IDF       { side=\"WEST\"; };\n"
                  "    class Hizballah { side=\"EAST\"; };\n"
                  "};\n");
        std::vector<RString> picks = GuerrillaListFactions(leb.Factions());
        REQUIRE(GuerrillaIndexOfName(picks, "EAST") == -1);
        REQUIRE(GuerrillaIndexOfName(picks, "GUER") == -1);
        // the resolution helper is the one that DOES alias - kept as the
        // contrast, because it stays correct for default*/gmSel* resolution
        REQUIRE(GuerrillaIndexOfSelection(leb.Factions(), picks, "EAST") == 1);
        // a genuine same-named pick still survives
        REQUIRE(GuerrillaIndexOfName(picks, "Hizballah") == 1);
        REQUIRE(GuerrillaIndexOfName(picks, "idf") == 0);
    }
}

TEST_CASE("GuerrillaDefaultSelections: the cyclers open on the template's default* pair", "[UI][Guerrilla]")
{
    int occ = -99;
    int res = -99;

    SECTION("the default* keys decide, by class name or by side")
    {
        IslandCfg cfg;
        cfg.Parse("class CfgGuerrillaZones\n"
                  "{\n"
                  "    playerSide = \"GUER\";\n"
                  "    defaultOccupier = \"WEST\";\n"
                  "    defaultResistance = \"Partisans\";\n"
                  "};\n"
                  "class CfgGuerrillaFactions\n"
                  "{\n"
                  "    class EAST      { side=\"EAST\"; };\n"
                  "    class WEST      { side=\"WEST\"; };\n"
                  "    class Partisans { side=\"GUER\"; };\n"
                  "};\n");
        std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), list, occ, res);
        REQUIRE(std::string((const char*)list[occ]) == "WEST");
        REQUIRE(std::string((const char*)list[res]) == "Partisans");
    }

    SECTION("no default* keys: the registry's own built-in EAST/GUER pair")
    {
        IslandCfg cfg;
        cfg.Parse(kPinnedTemplate); // EAST, WEST, GUER, CIV — no default* keys
        std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), list, occ, res);
        // ZoneRegistry::LoadFromParams seeds _occupierSide="EAST",
        // _resistanceSide="GUER" before anything else touches them
        REQUIRE(std::string((const char*)list[occ]) == "EAST");
        REQUIRE(std::string((const char*)list[res]) == "GUER");
    }

    SECTION("nothing resolves: two DISTINCT indices, never 0 and 0")
    {
        IslandCfg cfg;
        cfg.Parse("class CfgGuerrillaZones { };\n"
                  "class CfgGuerrillaFactions\n"
                  "{\n"
                  "    class Alpha { side=\"WEST\"; };\n"
                  "    class Bravo { side=\"WEST\"; };\n"
                  "};\n");
        std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), list, occ, res);
        REQUIRE(occ != res); // the whole point: never occupier == resistance
        REQUIRE(occ >= 0);
        REQUIRE(res >= 0);
    }

    SECTION("a one-faction roster is the only case that may repeat an index")
    {
        IslandCfg cfg;
        cfg.Parse("class CfgGuerrillaZones { };\n"
                  "class CfgGuerrillaFactions { class Alpha { side=\"WEST\"; }; };\n");
        std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), list, occ, res);
        REQUIRE(occ == 0);
        REQUIRE(res == 0); // there is no other index to offer; the guard blocks OK
    }

    SECTION("an empty roster selects nothing at all")
    {
        IslandCfg cfg;
        cfg.Parse(kPinnedTemplate);
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), std::vector<RString>(), occ, res);
        // SelectedOccupier/SelectedResistance must return EMPTY here so the
        // launch path publishes nothing and the mission's defaults keep control
        REQUIRE(occ == -1);
        REQUIRE(res == -1);
    }

    SECTION("a template deliberately pointing both keys at one roster is honoured")
    {
        IslandCfg cfg;
        cfg.Parse("class CfgGuerrillaZones\n"
                  "{\n"
                  "    playerSide = \"GUER\";\n"
                  "    defaultOccupier = \"Alpha\";\n"
                  "    defaultResistance = \"Alpha\";\n"
                  "};\n"
                  "class CfgGuerrillaFactions { class Alpha { side=\"EAST\"; }; class Bravo { side=\"WEST\"; }; };\n");
        std::vector<RString> list = GuerrillaListFactions(cfg.Factions());
        GuerrillaDefaultSelections(cfg.Factions(), cfg.Zones(), list, occ, res);
        // reproducing the template's launch beats the distinctness floor;
        // ZoneRegistry::DivergeAliasedFactions is what makes the pair coherent
        REQUIRE(occ == 0);
        REQUIRE(res == 0);
    }
}

// ---------------------------------------------------------------------------
// The SHIPPED templates, read the way the engine reads them (issue #54 A4).
//
// A template's description.ext is only the ISLAND half of the faction table.
// The war rosters live in a GLOBAL config that lands in Pars - the vanilla
// library the installer drops into <GameDir>\bin\guerrilla-factions.hpp for
// the stock islands, the @LoBo mod config for the @LoBo ones - and the engine
// merges the two through FactionSources (island wins on a class-name
// collision). So parsing only the description.ext, as these cases used to,
// now tests a roster no player is ever offered: an empty one.
// ---------------------------------------------------------------------------

namespace
{

// repo-relative, forward slashes (std::filesystem normalizes them)
const char* kVanillaLibrary = "guerrilla-mode/config/guerrilla-factions.hpp";
const char* kLoBoLibrary = "tests/fixtures/mods-lobo/@lobofixup/bin/config.cpp";

std::filesystem::path RepoFile(const char* relative)
{
    // TESTS_ROOT_DIR = <repo>/tests
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / relative;
}

struct ShippedTemplate
{
    const char* dir;    // guerrilla-mode/mission/<dir>
    const char* global; // the global faction source a session of it has mounted
};

// The stock islands run on the vanilla library, the @LoBo islands on the mod
// config. A machine with both mounted merges both; one per template here
// keeps each case's expected roster the roster that island really offers.
const ShippedTemplate kShippedTemplates[] = {
    {"Guerrilla.Abel", kVanillaLibrary},
    {"Guerrilla.Demo", kVanillaLibrary},
    {"Guerrilla.Sinai", kLoBoLibrary},
    {"Guerrilla.Lebanon80", kLoBoLibrary},
};

// One shipped template plus its global source, merged exactly as
// GuerrillaNewGame::RefreshFactionsForIsland / ZoneRegistry::LoadFromConfig do.
//
// Both halves are parsed through ParamFile's FILENAME overload, which
// preprocesses first. Feeding the raw bytes to Parse(QIStream&) instead
// silently yields an EMPTY ParamFile: all of these files are heavily commented
// and stripping `//` is the preprocessor's job, so the classes these tests are
// about never appear. The default PreprocessorFunctions is a pass-through
// copy, so the real CPreprocessorFunctions has to be installed first.
struct MergedTemplate
{
    ParamFile islandFile;
    ParamFile globalFile;
    Poseidon::Guerrilla::FactionSources sources;

    void Load(const ShippedTemplate& tpl)
    {
        const std::filesystem::path desc = RepoFile("guerrilla-mode/mission") / tpl.dir / "description.ext";
        INFO(desc.string());
        REQUIRE(std::filesystem::exists(desc));
        REQUIRE(islandFile.Parse(desc.string().c_str()) == LSOK);

        const std::filesystem::path global = RepoFile(tpl.global);
        INFO(global.string());
        REQUIRE(std::filesystem::exists(global));
        REQUIRE(globalFile.Parse(global.string().c_str()) == LSOK);
        // the @lobofixup config also carries CfgPatches/CfgAddons/CfgAmmo -
        // ParamFile parses them fine and Build only reads the one class
        REQUIRE(globalFile.FindEntry("CfgGuerrillaFactions") != nullptr);

        sources.Build(globalFile.FindEntry("CfgGuerrillaFactions"), islandFile.FindEntry("CfgGuerrillaFactions"));
        REQUIRE(sources.Factions() != nullptr);
    }

    const ParamEntry* Factions() { return sources.Factions(); }
    const ParamEntry* Zones() { return islandFile.FindEntry("CfgGuerrillaZones"); }
    const ParamEntry* IslandFactions() { return islandFile.FindEntry("CfgGuerrillaFactions"); }
};

} // namespace

TEST_CASE("Guerrilla launch: the shipped templates' cyclers open on their own default* pair", "[UI][Guerrilla]")
{
    // The regression, executable: before the cyclers were fed a real per-island
    // roster the lists were always empty, so nothing was published and the
    // default* keys always won. Now that the lists are real, opening the screen
    // and pressing OK must still launch precisely that campaign.
    static CPreprocessorFunctions s_preproc;
    ParamFile::SetDefaultPreprocFunctions(&s_preproc);

    for (const ShippedTemplate& tpl : kShippedTemplates)
    {
        INFO(tpl.dir);
        MergedTemplate merged;
        merged.Load(tpl);
        const ParamEntry* factions = merged.Factions();
        const ParamEntry* zones = merged.Zones();
        REQUIRE(zones != nullptr);

        std::vector<RString> picks = GuerrillaListFactions(factions);
        int occ = -1;
        int res = -1;
        GuerrillaDefaultSelections(factions, zones, picks, occ, res);
        REQUIRE(occ >= 0);
        REQUIRE(res >= 0);

        // What the UI publishes...
        ZoneRegistry published;
        published.LoadFromParams(zones, factions, picks[occ], picks[res]);
        // ...must equal what a direct, no-UI launch of the same template does.
        ZoneRegistry direct;
        direct.LoadFromParams(zones, factions, "", "");

        INFO("occupier=" << (const char*)picks[occ] << " resistance=" << (const char*)picks[res]);
        REQUIRE(Str(published.OccupierSide()) == Str(direct.OccupierSide()));
        REQUIRE(Str(published.ResistanceSide()) == Str(direct.ResistanceSide()));
        REQUIRE(Str(published.OccupierFaction()) == Str(direct.OccupierFaction()));
        REQUIRE(Str(published.ResistanceFaction()) == Str(direct.ResistanceFaction()));
        // and the shipped templates must not ship a mirror match
        REQUIRE(Str(published.OccupierSide()) != Str(published.ResistanceSide()));
    }
}

TEST_CASE("Guerrilla launch: every faction the shipped templates offer is playable on both sides", "[UI][Guerrilla]")
{
    // This assertion IS the goal, executable: on every shipped template, every
    // faction the cyclers offer must be launchable in either slot against any
    // other - including against itself.
    //
    // The one documented exception is a template that authors NO playerSide:
    // ZoneRegistry::ResolveSideCollisions returns early there, so a mirror pick
    // really cannot be separated and the UI is right to refuse it. That is a
    // property of the template, so it is derived from the config here rather
    // than hardcoded per template name.
    static CPreprocessorFunctions s_preproc;
    ParamFile::SetDefaultPreprocFunctions(&s_preproc);

    for (const ShippedTemplate& tpl : kShippedTemplates)
    {
        INFO(tpl.dir);
        MergedTemplate merged;
        merged.Load(tpl);
        const ParamEntry* factions = merged.Factions();
        const ParamEntry* zones = merged.Zones();
        REQUIRE(zones != nullptr);
        const bool pinned = zones->ReadValue("playerSide", RString()).GetLength() > 0;

        std::vector<RString> picks = GuerrillaListFactions(factions);
        REQUIRE_FALSE(picks.empty());
        for (const RString& occ : picks)
        {
            for (const RString& res : picks)
            {
                RString message;
                INFO("occupier=" << (const char*)occ << " resistance=" << (const char*)res);
                const bool sameSide =
                    stricmp(GuerrillaFactionSide(factions, occ), GuerrillaFactionSide(factions, res)) == 0;
                const bool expected = pinned || !sameSide;
                REQUIRE(GuerrillaSelectionIsResolvable(factions, zones, occ, res, message) == expected);
            }
        }
    }
}

TEST_CASE("Guerrilla data: a shipped template's own faction block holds only CIV", "[UI][Guerrilla]")
{
    // The A4 contract, executable. A war roster is not an island fact, so it
    // belongs in the global library (installed to <GameDir>\bin, or shipped by
    // a mod) and NOT in a template's description.ext. Copying one back into a
    // template is exactly how IDF came to exist twice and the two Hizballah
    // copies drifted, so it is pinned here: an island block may declare CIV -
    // which IS an island fact, the population models that data set ships - and
    // otherwise only a deliberate, listed OVERRIDE of a global class.
    static CPreprocessorFunctions s_preproc;
    ParamFile::SetDefaultPreprocFunctions(&s_preproc);

    // The complete list of island overrides in the repo, each with the reason
    // the template's description.ext states. Adding a row here is the cost of
    // adding an override, on purpose.
    struct Override
    {
        const char* templateDir;
        const char* faction;
    };
    const Override kOverrides[] = {
        // Guerrilla.Demo runs on the 2001 demo data set, which ships neither
        // the library GUER's second tier rung nor its officer/APC classes.
        {"Guerrilla.Demo", "GUER"},
    };

    for (const ShippedTemplate& tpl : kShippedTemplates)
    {
        INFO(tpl.dir);
        MergedTemplate merged;
        merged.Load(tpl);
        const ParamEntry* island = merged.IslandFactions();
        REQUIRE(island != nullptr);

        bool sawCiv = false;
        for (int i = 0; i < island->GetEntryCount(); i++)
        {
            const ParamEntry& e = island->GetEntry(i);
            if (!e.IsClass())
            {
                continue;
            }
            const std::string name = Str(RString(e.GetName()));
            INFO("island-declared faction: " << name);
            if (stricmp(name.c_str(), "CIV") == 0)
            {
                sawCiv = true;
                continue;
            }
            bool listed = false;
            for (const Override& o : kOverrides)
            {
                listed |= stricmp(o.templateDir, tpl.dir) == 0 && stricmp(o.faction, name.c_str()) == 0;
            }
            // Either move it to the global library, or list it above with the
            // reason it has to differ on this island.
            REQUIRE(listed);
            // An override overrides: the global source must actually declare
            // the class, or this is a stray roster wearing an override's name.
            REQUIRE(merged.sources.FindRecord(name.c_str()) != nullptr);
            REQUIRE(merged.sources.FindRecord(name.c_str())->overrodeGlobal);
        }
        // Every island owns its population; a template without CIV would fall
        // through to whatever global block a mounted mod happens to ship.
        REQUIRE(sawCiv);
    }
}
