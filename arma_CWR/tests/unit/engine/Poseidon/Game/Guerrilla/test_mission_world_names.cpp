// Guerrilla Mode world-naming contract (guerrilla-mode/WORLD-NAMES.md).
//
// The island listboxes DISPLAY CfgWorlds>>description ("Malden") but ACT on
// the config class name ("Abel"): SelectedIsland() returns the row DATA and
// OptionsUIApp.cpp resolves "missions\Guerrilla.<class>". A template folder
// named for a DISPLAY name (Guerrilla.Malden) would list fine in the repo,
// pass the script-core parity test, and then never be found by the new-game
// launch path - a silent content bug. This test pins that convention:
//
//   * a template's world suffix must be a plausible CONFIG CLASS NAME;
//   * the display names we know about are explicitly rejected as suffixes.
//
// WHAT THIS DELIBERATELY NO LONGER CHECKS (issue #54 C4): that the suffix
// appears in an allow-list of known internal names, and that WORLD-NAMES.md
// documents it. Both were name tables, and a name table means a new island
// pack cannot be added without editing this file and a doc - which is exactly
// the coupling C4 removed from the installer's world gate too (it now reads
// pbo entry tables instead of being told the names). WORLD-NAMES.md is a
// CONVENTION doc now, not a registry, so there is nothing here to keep in
// sync with it. The denylist stays because it is not a registry: it encodes
// the specific mistake this test exists to catch, and a name that is not on
// it still has to pass the class-name shape check.
//
// Repo root discovery uses TESTS_ROOT_DIR, same idiom as
// test_mission_script_core.cpp.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
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

// Display names (lowercased): NEVER valid as a mission world suffix. Not an
// exhaustive registry - just the ones a template author is most likely to
// reach for, since they are what the island listbox shows.
const std::vector<std::string> kDisplayNames = {
    "malden", "kolgujev", "everon", "desert island", "nogova", "southern sinai",
};

bool Contains(const std::vector<std::string>& v, const std::string& s)
{
    return std::find(v.begin(), v.end(), s) != v.end();
}

// A CfgWorlds class name is a config identifier: letters, digits, underscore.
// Anything else (a space, a hyphen, an apostrophe) is a display name wearing
// a folder name's clothes.
bool IsPlausibleConfigClassName(const std::string& s)
{
    if (s.empty())
    {
        return false;
    }
    for (unsigned char c : s)
    {
        if (!std::isalnum(c) && c != '_')
        {
            return false;
        }
    }
    return true;
}

// world suffix of a mission dir name like "Guerrilla.Abel" / "guerrilla_native.abel"
std::string WorldSuffix(const std::string& dirName)
{
    const size_t dot = dirName.rfind('.');
    REQUIRE(dot != std::string::npos);
    return dirName.substr(dot + 1);
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
    for (const fs::path& dir : MissionDirsMatching(repo / "guerrilla-mode" / "mission", "Showcase."))
    {
        missions.push_back(dir);
    }
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
        REQUIRE(!Contains(kDisplayNames, Lower(world)));
        REQUIRE(IsPlausibleConfigClassName(world));
    }
}
