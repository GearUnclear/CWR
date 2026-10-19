// The UD Controls page shard (bin/ud-options.hpp) parsed beside a Bohemia-shaped
// Options shell, the way setup's Remaster mode installs it: Bohemia's
// optionsShell.hpp stays as shipped and resource-extra.cpp gains one appended
// #include. ControlsPage mounts RscOptionsPageControlsUD when it is merged.
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/UI/Options/ControlsPage.hpp>

#include <filesystem>
#include <fstream>
#include <random>
#include <string>

using Poseidon::kControlsPageClassStock;
using Poseidon::kControlsPageClassUD;
using Poseidon::ParamClass;
using Poseidon::ParamEntry;
using Poseidon::ParamFile;
using Poseidon::SelectControlsPageResourceClass;

namespace
{
namespace fs = std::filesystem;

struct OptionsCwdGuard
{
    fs::path prev;
    explicit OptionsCwdGuard(const fs::path& to) : prev(fs::current_path()) { fs::current_path(to); }
    ~OptionsCwdGuard() { fs::current_path(prev); }
};

struct OptionsTempDir
{
    fs::path path;
    OptionsTempDir()
    {
        std::random_device rd;
        path = fs::temp_directory_path() / ("ud-options-" + std::to_string(rd()) + "-" + std::to_string(rd()));
        fs::create_directories(path);
    }
    ~OptionsTempDir()
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

fs::path ShippedUdOptions()
{
    // TESTS_ROOT_DIR = <repo>/tests
    return fs::path(TESTS_ROOT_DIR).parent_path() / "guerrilla-mode" / "runtime" / "common" / "bin" / "ud-options.hpp";
}

// Bohemia-shaped stand-in for the Remaster's BIN/optionsShell.hpp chain: the row
// macros and nav template from optionsUI.hpp / optionsTemplates.hpp, and the
// six-row Controls page exactly as the official package defines it.
const char* const kRemasterOptionsStandIn = "#ifndef OPTIONS_TEST_UI_HPP\n"
                                            "#define OPTIONS_TEST_UI_HPP\n"
                                            // Bohemia composes these from OPT_ROW_BASE_Y + n * OPT_ROW_PITCH; the
                                            // arithmetic needs the script evaluator, so the stand-in uses the sums.
                                            "#define OPT_ROW_0_Y 0.20\n"
                                            "#define OPT_ROW_5_Y 0.55\n"
                                            "#define OPT_ROW_6_Y 0.62\n"
                                            "class RscObjNotebookButton {};\n"
                                            "class OptIdxMenuItem: RscObjNotebookButton\n"
                                            "{\n"
                                            "    type=11;\n"
                                            "    x=0.1;\n"
                                            "    w=0.8;\n"
                                            "    h=0.06;\n"
                                            "};\n"
                                            "class RscOptionsPageControls\n"
                                            "{\n"
                                            "    controls[]={\"NavKbm\",\"NavMouse\",\"NavGamepad\",\"NavGamepadTuning\","
                                            "\"NavResetAll\",\"NavClose\"};\n"
                                            "    class NavKbm: OptIdxMenuItem { idc=1401; y=OPT_ROW_0_Y; text=\"KBM\"; };\n"
                                            "    class NavMouse: OptIdxMenuItem { idc=1402; text=\"Mouse\"; };\n"
                                            "    class NavGamepad: OptIdxMenuItem { idc=1405; text=\"Gamepad\"; };\n"
                                            "    class NavGamepadTuning: OptIdxMenuItem { idc=1406; text=\"Tuning\"; };\n"
                                            "    class NavResetAll: OptIdxMenuItem { idc=1403; text=\"Reset\"; };\n"
                                            "    class NavClose: OptIdxMenuItem { idc=1404; y=OPT_ROW_5_Y; text=\"$STR_DISP_CLOSE\"; };\n"
                                            "};\n"
                                            "#endif\n";

bool ArrayContains(const ParamEntry& array, const char* value)
{
    for (int i = 0; i < array.GetSize(); i++)
        if (strcmpi(array[i].GetValue(), value) == 0)
            return true;
    return false;
}

int ArrayIndexOf(const ParamEntry& array, const char* value)
{
    for (int i = 0; i < array.GetSize(); i++)
        if (strcmpi(array[i].GetValue(), value) == 0)
            return i;
    return -1;
}

// Parse <bin>/resource-extra.cpp the way ParseTextFileFromResolvedPath does
// (cwd = bin/, so the #includes resolve) and merge it over an empty Res.
void ParseAndMerge(const fs::path& bin, ParamFile& extra, ParamFile& res)
{
    {
        OptionsCwdGuard cwd(bin);
        REQUIRE(extra.Parse("resource-extra.cpp") == Poseidon::LSOK);
    }
    res.Update(extra);
    res.SetFile(&res);
}
} // namespace

TEST_CASE("controls page class prefers the UD page, then the stock page", "[ui][options][setup]")
{
    ParamFile res;
    CHECK(std::string(SelectControlsPageResourceClass(res)) == kControlsPageClassStock);
    res.AddClass(kControlsPageClassStock);
    CHECK(std::string(SelectControlsPageResourceClass(res)) == kControlsPageClassStock);
    res.AddClass(kControlsPageClassUD);
    CHECK(std::string(SelectControlsPageResourceClass(res)) == kControlsPageClassUD);
}

TEST_CASE("UD options shard appended to a Remaster resource-extra adds the Arma 3 row beside Bohemia's page",
          "[ui][options][setup]")
{
    REQUIRE(fs::is_regular_file(ShippedUdOptions()));
    OptionsTempDir dir;
    const fs::path bin = dir.path / "bin";
    fs::create_directories(bin);
    fs::copy_file(ShippedUdOptions(), bin / "ud-options.hpp");
    WriteText(bin / "optionsShell.hpp", kRemasterOptionsStandIn);
    // Bohemia's file plus exactly what setup appends.
    WriteText(bin / "resource-extra.cpp", "#include \"optionsShell.hpp\"\n"
                                          "\n"
                                          "// Uslu dur! Options additions; appended by arma_CWR/setup-guerrilla.ps1 (Remaster mode).\n"
                                          "#include \"ud-options.hpp\"\n");
    ParamFile extra;
    ParamFile res;
    ParseAndMerge(bin, extra, res);

    // Bohemia's page survives the one parse untouched: six rows, no Arma 3 entry.
    const ParamEntry* stock = res.FindEntry(kControlsPageClassStock);
    REQUIRE(stock != nullptr);
    CHECK(stock->FindEntry("controls")->GetSize() == 6);
    CHECK_FALSE(ArrayContains(*stock->FindEntry("controls"), "NavArma3"));
    CHECK(stock->FindEntry("NavArma3") == nullptr);

    const ParamEntry* ud = res.FindEntry(kControlsPageClassUD);
    REQUIRE(ud != nullptr);
    CHECK(std::string(SelectControlsPageResourceClass(res)) == kControlsPageClassUD);
    const ParamEntry* controls = ud->FindEntry("controls");
    REQUIRE(controls != nullptr);
    CHECK(controls->GetSize() == 7);
    // The Arma 3 row sits above Close, matching ControlsPage::kCycle.
    CHECK(ArrayIndexOf(*controls, "NavArma3") == 5);
    CHECK(ArrayIndexOf(*controls, "NavClose") == 6);

    // Every row the page mounts by name resolves: the six Bohemia rows through the
    // base class, the UD row and the moved Close row from the shard.
    for (int i = 0; i < controls->GetSize(); i++)
    {
        const ParamEntry* row = ud->FindEntry((*controls)[i].GetValue());
        REQUIRE(row != nullptr);
        CHECK(row->IsClass());
        CHECK(row->FindEntry("idc") != nullptr);
        CHECK(row->FindEntry("type") != nullptr);
    }
    const ParamEntry* arma3 = ud->FindEntry("NavArma3");
    REQUIRE(arma3 != nullptr);
    CHECK(int(*arma3->FindEntry("idc")) == 1407);
    // "$STR_..." values localize at parse time (no stringtable here); the key
    // itself is pinned by the Remaster-lane Trident test on the shadow.
    CHECK(arma3->FindEntry("text") != nullptr);
    const ParamEntry* close = ud->FindEntry("NavClose");
    REQUIRE(close != nullptr);
    CHECK(int(*close->FindEntry("idc")) == 1404);
    // Close moved one row down (OPT_ROW_6_Y) to make room; the base keeps OPT_ROW_5_Y.
    CHECK(float(*close->FindEntry("y")) > float(*stock->FindEntry("NavClose")->FindEntry("y")));
    CHECK(float(*arma3->FindEntry("y")) == float(*stock->FindEntry("NavClose")->FindEntry("y")));
    CHECK(int(*ud->FindEntry("NavKbm")->FindEntry("idc")) == 1401);
}
