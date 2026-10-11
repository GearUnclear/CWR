#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/UI/MainMenuLayout.hpp>

using Poseidon::CalculateMainMenuModsPlacement;
using Poseidon::MainMenuControlRect;

TEST_CASE("main menu Mods placement sits left of Quit when horizontal space exists", "[ui][main-menu][mods]")
{
    const MainMenuControlRect quit{0.70f, 0.80f, 0.20f, 0.05f, true};

    const auto placement = CalculateMainMenuModsPlacement(quit, {});

    CHECK(placement.rightAlign);
    CHECK(placement.x == Catch::Approx(0.49f));
    CHECK(placement.y == Catch::Approx(quit.y));
    CHECK(placement.w == Catch::Approx(quit.w));
    CHECK(placement.h == Catch::Approx(quit.h));
}

TEST_CASE("main menu Mods placement stacks above left-edge Quit when clear", "[ui][main-menu][mods]")
{
    const MainMenuControlRect quit{0.04f, 0.80f, 0.20f, 0.05f, true};

    const auto placement = CalculateMainMenuModsPlacement(quit, {});

    CHECK_FALSE(placement.rightAlign);
    CHECK(placement.x == Catch::Approx(quit.x));
    CHECK(placement.y == Catch::Approx(0.74f));
}

TEST_CASE("main menu Mods placement avoids foreground overlap above Quit", "[ui][main-menu][mods]")
{
    const MainMenuControlRect quit{0.04f, 0.80f, 0.20f, 0.05f, true};
    const std::vector<MainMenuControlRect> foreground{{0.03f, 0.73f, 0.22f, 0.06f, true}};

    const auto placement = CalculateMainMenuModsPlacement(quit, foreground);

    CHECK_FALSE(placement.rightAlign);
    CHECK(placement.x == Catch::Approx(quit.x));
    CHECK(placement.y == Catch::Approx(0.86f));
}

// ---------------------------------------------------------------------------
// Main-menu class selection and the Remaster install mode (issue #67).
//
// setup-guerrilla.ps1 installs bin/ud-mainmenu.hpp and, on the official
// Remaster, appends `#include "ud-mainmenu.hpp"` to Bohemia's own
// bin/resource-extra.cpp. The engine parses that file as ONE ParamFile, so the
// UD menu must not redefine anything Bohemia's mainMenu.hpp already declared
// (the RscDisplayMain placeholder, RscDisplayMainRemaster). These cases parse
// the shipped ud-mainmenu.hpp behind a synthetic Bohemia-shaped header, then
// replay MergeBaseResourceExtra's Update over a locked vanilla RscDisplayMain.
// ---------------------------------------------------------------------------

#include <Poseidon/IO/ParamFile/ParamFile.hpp>

#include <filesystem>
#include <fstream>
#include <random>
#include <string>

using Poseidon::kMainMenuClassRemaster;
using Poseidon::kMainMenuClassUD;
using Poseidon::kMainMenuClassVanilla;
using Poseidon::ParamClass;
using Poseidon::ParamEntry;
using Poseidon::ParamFile;
using Poseidon::SelectMainMenuResourceClass;

namespace
{
namespace fs = std::filesystem;

struct MenuCwdGuard
{
    fs::path prev;
    explicit MenuCwdGuard(const fs::path& to) : prev(fs::current_path()) { fs::current_path(to); }
    ~MenuCwdGuard() { fs::current_path(prev); }
};

struct MenuTempDir
{
    fs::path path;
    MenuTempDir()
    {
        std::random_device rd;
        path = fs::temp_directory_path() / ("ud-menu-" + std::to_string(rd()) + "-" + std::to_string(rd()));
        fs::create_directories(path);
    }
    ~MenuTempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void WriteText(const fs::path& file, const std::string& text)
{
    std::ofstream out(file, std::ios::binary);
    out << text;
}

fs::path ShippedUdMenu()
{
    // TESTS_ROOT_DIR = <repo>/tests
    return fs::path(TESTS_ROOT_DIR).parent_path() / "guerrilla-mode" / "runtime" / "common" / "bin" / "ud-mainmenu.hpp";
}

// Bohemia-shaped stand-in for the Remaster's BIN/mainMenu.hpp: same include
// guard, same placeholder, its own RscDisplayMainRemaster with a Mods entry.
const char* const kRemasterMenuStandIn = "#ifndef MAIN_MENU_HPP\n"
                                         "#define MAIN_MENU_HPP\n"
                                         "class RscDisplayMain {};\n"
                                         "class RscDisplayMainRemaster: RscDisplayMain\n"
                                         "{\n"
                                         "    controls[]={\"CWA\",\"Options\",\"Mods\",\"Quit\"};\n"
                                         "    class Mods { idc=119; text=\"MODS\"; };\n"
                                         "};\n"
                                         "#endif\n";

// The locked vanilla menu every RESOURCE.BIN carries, reduced to one control.
void AddLockedVanillaMenu(ParamFile& res)
{
    ParamClass* main = res.AddClass(kMainMenuClassVanilla);
    ParamClass* quit = main->AddClass("Quit");
    quit->Add("idc", 106);
    res.FindEntry(kMainMenuClassVanilla)->SetAccessModeForAll(Poseidon::PAReadOnlyVerified);
}

bool ArrayContains(const ParamEntry& array, const char* value)
{
    for (int i = 0; i < array.GetSize(); i++)
        if (strcmpi(array[i].GetValue(), value) == 0)
            return true;
    return false;
}

// Parse <dir>/bin/resource-extra.cpp the way ParseTextFileFromResolvedPath does
// (cwd = bin/, so the #includes resolve) and merge it over a vanilla Res.
void ParseAndMerge(const fs::path& bin, ParamFile& extra, ParamFile& res)
{
    {
        MenuCwdGuard cwd(bin);
        REQUIRE(extra.Parse("resource-extra.cpp") == Poseidon::LSOK);
    }
    AddLockedVanillaMenu(res);
    res.Update(extra);
    res.SetFile(&res);
}
} // namespace

TEST_CASE("main menu class prefers the UD menu, then the Remaster menu, then vanilla", "[ui][main-menu][setup]")
{
    ParamFile res;
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassVanilla);
    res.AddClass(kMainMenuClassVanilla);
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassVanilla);
    res.AddClass(kMainMenuClassRemaster);
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassRemaster);
    res.AddClass(kMainMenuClassUD);
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassUD);
}

TEST_CASE("UD menu appended to a Remaster resource-extra parses beside Bohemia's menu", "[ui][main-menu][setup]")
{
    REQUIRE(fs::is_regular_file(ShippedUdMenu()));
    MenuTempDir dir;
    const fs::path bin = dir.path / "bin";
    fs::create_directories(bin);
    fs::copy_file(ShippedUdMenu(), bin / "ud-mainmenu.hpp");
    WriteText(bin / "mainMenu.hpp", kRemasterMenuStandIn);
    // Bohemia's file plus exactly what setup appends.
    WriteText(bin / "resource-extra.cpp", "#include \"mainMenu.hpp\"\n"
                                          "class RscDisplaySave { class Title { text=\"save\"; }; };\n"
                                          "\n// Uslu dur! main menu; appended by setup-guerrilla.ps1 (Remaster mode).\n"
                                          "#include \"ud-mainmenu.hpp\"\n");

    ParamFile extra;
    ParamFile res;
    ParseAndMerge(bin, extra, res);

    // Both menus survive the one parse: nothing of Bohemia's was redefined.
    const ParamEntry* remaster = res.FindEntry(kMainMenuClassRemaster);
    REQUIRE(remaster != nullptr);
    CHECK(remaster->FindEntry("Mods") != nullptr);
    CHECK(ArrayContains(*remaster->FindEntry("controls"), "Mods"));
    CHECK(res.FindEntry("RscDisplaySave") != nullptr);

    const ParamEntry* ud = res.FindEntry(kMainMenuClassUD);
    REQUIRE(ud != nullptr);
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassUD);
    CHECK(ud->FindEntry("Guerrilla") != nullptr);
    CHECK(ArrayContains(*ud->FindEntry("controls"), "Guerrilla"));
    CHECK_FALSE(ArrayContains(*ud->FindEntry("controls"), "Mods"));
    // The merge rebound the placeholder base to the locked vanilla class, so the
    // inherited Quit control (what DisplayMain clones for MODS) resolves.
    CHECK(ud->FindEntry("Quit") != nullptr);
}

TEST_CASE("UD menu included from the Classic resource-extra declares its own placeholder", "[ui][main-menu][setup]")
{
    REQUIRE(fs::is_regular_file(ShippedUdMenu()));
    MenuTempDir dir;
    const fs::path bin = dir.path / "bin";
    fs::create_directories(bin);
    fs::copy_file(ShippedUdMenu(), bin / "ud-mainmenu.hpp");
    WriteText(bin / "resource-extra.cpp", "#include \"ud-mainmenu.hpp\"\n");

    ParamFile extra;
    ParamFile res;
    ParseAndMerge(bin, extra, res);

    CHECK(res.FindEntry(kMainMenuClassRemaster) == nullptr);
    const ParamEntry* ud = res.FindEntry(kMainMenuClassUD);
    REQUIRE(ud != nullptr);
    CHECK(std::string(SelectMainMenuResourceClass(res)) == kMainMenuClassUD);
    CHECK(ud->FindEntry("Quit") != nullptr);
}
