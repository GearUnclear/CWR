// Guerrilla Mode world-naming contract (guerrilla-mode/WORLD-NAMES.md).
//
// The island listboxes DISPLAY CfgWorlds>>description ("Malden") but ACT on
// the config class name ("Abel"): SelectedIsland() returns the row DATA and
// OptionsUIApp.cpp resolves "missions\Guerrilla.<class>". A template folder
// named for a DISPLAY name (Guerrilla.Malden) would list fine in the repo,
// pass the script-core parity test, and then never be found by the new-game
// launch path - a silent content bug. This test pins the convention:
//
//   * every mission template / test-mission world suffix must be a KNOWN
//     internal world name (the table in WORLD-NAMES.md);
//   * display names are explicitly rejected as suffixes;
//   * WORLD-NAMES.md must document every internal name that templates use
//     (doc and content stay in sync).
//
// Repo root discovery uses TESTS_ROOT_DIR, same idiom as
// test_mission_script_core.cpp.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

namespace fs = std::filesystem;

fs::path RepoRoot()
{
    // TESTS_ROOT_DIR = <repo>/tests
    return fs::path(TESTS_ROOT_DIR).parent_path();
}

std::string Lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

// Internal world names (CfgWorlds class names, lowercased): the shipped
// configs' authoritative set - see WORLD-NAMES.md "Derived from" section.
const std::vector<std::string> kInternalNames = {
    "abel",   // Malden
    "cain",   // Kolgujev
    "eden",   // Everon
    "intro",  // Desert Island
    "noe",    // Nogova (Resistance)
    "demo",   // Malden - Demo (2001 demo dataset only)
    "sinai",  // Southern Sinai (@LoBo)
    "lebanon80", // Lebanon (80's) (@LoBo)
};

// Display names (lowercased): NEVER valid as a mission world suffix.
const std::vector<std::string> kDisplayNames = {
    "malden", "kolgujev", "everon", "desert island", "nogova", "southern sinai",
};

bool Contains(const std::vector<std::string>& v, const std::string& s)
{
    return std::find(v.begin(), v.end(), s) != v.end();
}

// world suffix of a mission dir name like "Guerrilla.Abel" / "guerrilla_native.abel"
std::string WorldSuffix(const std::string& dirName)
{
    const size_t dot = dirName.rfind('.');
    REQUIRE(dot != std::string::npos);
    return Lower(dirName.substr(dot + 1));
}

// mission dirs whose name starts with prefix, directly under parent
std::vector<fs::path> MissionDirsMatching(const fs::path& parent, const std::string& prefix)
{
    std::vector<fs::path> dirs;
    if (!fs::exists(parent))
    {
        return dirs;
    }
    for (const auto& entry : fs::directory_iterator(parent))
    {
        if (entry.is_directory() && entry.path().filename().string().rfind(prefix, 0) == 0)
        {
            dirs.push_back(entry.path());
        }
    }
    std::sort(dirs.begin(), dirs.end());
    return dirs;
}

} // namespace

TEST_CASE("Guerrilla mission world suffixes are internal world names, never display names", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();

    std::vector<fs::path> missions = MissionDirsMatching(repo / "guerrilla-mode" / "mission", "Guerrilla.");
    for (const fs::path& dir : MissionDirsMatching(repo / "tests" / "integration" / "missions", "guerrilla_"))
    {
        missions.push_back(dir);
    }
    REQUIRE(!missions.empty());

    for (const fs::path& mission : missions)
    {
        const std::string name = mission.filename().string();
        INFO("mission: " << mission.string());
        const std::string world = WorldSuffix(name);

        // A display name here means the template can never be launched by
        // the new-game flow (missions\Guerrilla.<class> uses class names).
        REQUIRE(!Contains(kDisplayNames, world));
        REQUIRE(Contains(kInternalNames, world));
    }
}

TEST_CASE("WORLD-NAMES.md documents every world the mission templates use", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path doc = repo / "guerrilla-mode" / "WORLD-NAMES.md";
    REQUIRE(fs::exists(doc));

    std::ifstream in(doc, std::ios::binary);
    REQUIRE(in.good());
    const std::string text = Lower(std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()));

    // the full internal/display tables must be present (doc drift check)
    for (const std::string& internal : kInternalNames)
    {
        INFO("internal name missing from WORLD-NAMES.md: " << internal);
        REQUIRE(text.find(internal) != std::string::npos);
    }
    for (const std::string& display : kDisplayNames)
    {
        INFO("display name missing from WORLD-NAMES.md: " << display);
        REQUIRE(text.find(display) != std::string::npos);
    }

    // every world actually used by a template must be in the doc's table
    for (const fs::path& mission : MissionDirsMatching(repo / "guerrilla-mode" / "mission", "Guerrilla."))
    {
        const std::string world = WorldSuffix(mission.filename().string());
        INFO("template world not documented in WORLD-NAMES.md: " << world);
        REQUIRE(text.find(world) != std::string::npos);
    }
}
