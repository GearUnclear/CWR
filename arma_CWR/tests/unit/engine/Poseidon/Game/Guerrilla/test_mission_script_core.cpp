// Guerrilla Mode shared-script-core parity (issue #3 exit criterion).
//
// The mode's island-agnostic logic lives in ONE canonical place:
//     guerrilla-mode/mission/Guerrilla.Demo/{init.sqs, scripts/*}
// Every other Guerrilla mission - the full-CWA integration-test missions
// (tests/integration/missions/guerrilla_native.*) and the Demo-data test
// missions (guerrilla_capture.Demo / guerrilla_persist.Demo) - must carry a
// BYTE-IDENTICAL copy of that core; only description.ext (island + faction
// data) and mission.sqm (player start) may differ per island.
//
// This test mechanically enforces the contract: it walks every matching
// mission directory and diffs init.sqs plus the full scripts/ file set
// against the canonical copy.  A fix to the core therefore goes into the
// canonical mission first and is re-copied - divergent copies fail here.
//
// Repo root discovery uses the TESTS_ROOT_DIR compile definition
// (${CMAKE_SOURCE_DIR}/tests, see tests/unit/engine/Poseidon/CMakeLists.txt),
// the same idiom as the Graphics audit tests.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
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

std::string ReadBytes(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    REQUIRE(in.good());
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// sorted list of regular-file names directly inside dir (no recursion:
// scripts/ is flat by design)
std::vector<std::string> ListFileNames(const fs::path& dir)
{
    std::vector<std::string> names;
    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
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
        if (!entry.is_directory())
        {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (name.rfind(prefix, 0) == 0)
        {
            dirs.push_back(entry.path());
        }
    }
    std::sort(dirs.begin(), dirs.end());
    return dirs;
}

void RequireSameFile(const fs::path& canonical, const fs::path& copy)
{
    INFO("canonical: " << canonical.string());
    INFO("copy:      " << copy.string());
    REQUIRE(fs::exists(copy));
    REQUIRE(ReadBytes(canonical) == ReadBytes(copy));
}

} // namespace

TEST_CASE("Guerrilla missions share a byte-identical script core", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path canonical = repo / "guerrilla-mode" / "mission" / "Guerrilla.Demo";
    REQUIRE(fs::exists(canonical / "init.sqs"));
    REQUIRE(fs::exists(canonical / "scripts"));

    const std::vector<std::string> canonicalScripts = ListFileNames(canonical / "scripts");
    REQUIRE(!canonicalScripts.empty());

    // every mission that must carry the shared core. Showcase.* templates
    // (issue #9) carry the identical core plus a separate showcase/ overlay
    // directory; only init.sqs and scripts/ are checked here, so the overlay
    // stays out of scope while the core stays drift-proof.
    std::vector<fs::path> missions;
    for (const fs::path& dir : MissionDirsMatching(repo / "guerrilla-mode" / "mission", "Guerrilla."))
    {
        missions.push_back(dir);
    }
    for (const fs::path& dir : MissionDirsMatching(repo / "guerrilla-mode" / "mission", "Showcase."))
    {
        missions.push_back(dir);
    }
    const fs::path testMissions = repo / "tests" / "integration" / "missions";
    for (const fs::path& dir : MissionDirsMatching(testMissions, "guerrilla_native."))
    {
        missions.push_back(dir);
    }
    missions.push_back(testMissions / "guerrilla_capture.Demo");
    missions.push_back(testMissions / "guerrilla_persist.Demo");

    // the canonical mission itself is in the walk (a trivially green diff);
    // at minimum the two .Demo test missions and one guerrilla_native.* copy
    // must exist alongside it
    REQUIRE(missions.size() >= 4);

    for (const fs::path& mission : missions)
    {
        INFO("mission: " << mission.string());
        REQUIRE(fs::exists(mission));

        // init.sqs byte-identical
        RequireSameFile(canonical / "init.sqs", mission / "init.sqs");

        // scripts/: exactly the same file SET (nothing added, nothing
        // dropped - a stray retired script like zones.sqs fails here) ...
        REQUIRE(fs::exists(mission / "scripts"));
        REQUIRE(ListFileNames(mission / "scripts") == canonicalScripts);

        // ... and every file byte-identical
        for (const std::string& script : canonicalScripts)
        {
            RequireSameFile(canonical / "scripts" / script, mission / "scripts" / script);
        }
    }
}

// The reference missions (guerrilla-mode/mission/Qrf.<World> and
// Undercover.<World>) are NOT full shared-core missions: their own init.sqs
// boots only the one system on display. What they DO carry under scripts/
// must be a byte-identical SUBSET of the canonical core (lib.sqs + the one
// policy script: qrf.sqs / undercover.sqs), so each sandbox always exercises
// the campaign's real policy script - never a drifted copy. Mission-local
// files (hud.sqs, act_*.sqs, patrol.sqs) live at the mission root, outside
// scripts/, and are deliberately out of scope here.
TEST_CASE("Reference missions carry a byte-identical subset of the script core", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path canonical = repo / "guerrilla-mode" / "mission" / "Guerrilla.Demo";
    REQUIRE(fs::exists(canonical / "scripts"));
    const std::vector<std::string> canonicalScripts = ListFileNames(canonical / "scripts");

    struct Family
    {
        const char* prefix;
        const char* policyScript; // the one core script the sandbox exists to run
    };
    const Family families[] = {
        {"Qrf.", "qrf.sqs"},
        {"Market.", "market.sqs"},
        {"Undercover.", "undercover.sqs"},
    };

    for (const Family& fam : families)
    {
        const std::vector<fs::path> missions = MissionDirsMatching(repo / "guerrilla-mode" / "mission", fam.prefix);
        // at least the Abel slice of each family exists
        INFO("family: " << fam.prefix);
        REQUIRE(!missions.empty());

        for (const fs::path& mission : missions)
        {
            INFO("mission: " << mission.string());
            REQUIRE(fs::exists(mission / "scripts"));
            const std::vector<std::string> carried = ListFileNames(mission / "scripts");
            // the policy script and its helper library are the minimum
            REQUIRE(std::find(carried.begin(), carried.end(), fam.policyScript) != carried.end());
            REQUIRE(std::find(carried.begin(), carried.end(), "lib.sqs") != carried.end());
            for (const std::string& script : carried)
            {
                INFO("script: " << script);
                // every carried file is a canonical core file ...
                REQUIRE(std::find(canonicalScripts.begin(), canonicalScripts.end(), script) != canonicalScripts.end());
                // ... and byte-identical to it
                RequireSameFile(canonical / "scripts" / script, mission / "scripts" / script);
            }
        }
    }
}
