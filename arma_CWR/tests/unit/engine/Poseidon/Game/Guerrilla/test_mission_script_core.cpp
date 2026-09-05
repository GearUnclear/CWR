// Guerrilla Mode shared-script-core contract (issue #54 step B1).
//
// The mode's island-agnostic logic lives in ONE place on disk:
//     guerrilla-mode/core/{init.sqs, scripts/*}
// installed by guerrilla-mode/install-missions.ps1 as the loose directory
// <GameDir>\gmcore\, and reached from a mission with a LEADING BACKSLASH:
//     [] exec "\gmcore\init.sqs"
// OpenScript (Game/Scripting/Scripts.cpp) strips that backslash and resolves
// the rest against the game data root; without it FindScript looks inside the
// MISSION folder, which is the retired per-mission copy path.
//
// This test used to walk N mission directories diffing byte-identical copies
// of the core.  There are no copies left, so it now pins the ONE-core shape
// instead:
//   * the core exists and its scripts/ file set equals the manifest;
//   * ARCHITECTURE.md A.6 names every manifest script (doc and tree in sync,
//     same idiom as test_mission_world_names.cpp against WORLD-NAMES.md);
//   * no mission template or guerrilla_* test mission carries a scripts/ dir;
//   * every full-core mission's init.sqs is the two-line bootstrap;
//   * every `exec`/`addAction` target that names the core resolves to a file
//     that actually exists in core/scripts;
//   * no relative "scripts/..." reference survives anywhere in the core or in
//     a template init.sqs - that form would silently resolve to nothing.
//
// Repo root discovery uses the TESTS_ROOT_DIR compile definition
// (${CMAKE_SOURCE_DIR}/tests, see tests/unit/engine/Poseidon/CMakeLists.txt),
// the same idiom as the Graphics audit tests.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
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

// THE MANIFEST. Kept in lockstep with guerrilla-mode/ARCHITECTURE.md A.6 by
// the doc case below: adding or retiring a core script means editing both.
const std::vector<std::string> kCoreScripts = {
    "campaign.sqs",
    "capture.sqs",
    "civilians.sqs",
    "companions.sqs",
    "economy.sqs",
    "escalation.sqs",
    "lib.sqs",
    "loot.sqs",
    "market.sqs",
    "market_action.sqs",
    "qrf.sqs",
    "recruit.sqs",
    "recruit_action.sqs",
    "shakedown.sqs",
    "undercover.sqs",
};

// The line every full-core mission's init.sqs must carry, and nothing else
// but ';' comments and blanks.
const char* const kBootstrapLine = R"([] exec "\gmcore\init.sqs")";

fs::path CoreDir(const fs::path& repo)
{
    return repo / "guerrilla-mode" / "core";
}

std::string ReadText(const fs::path& p)
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

// every mission template + test mission, whatever its prefix
std::vector<fs::path> AllGuerrillaMissions(const fs::path& repo)
{
    std::vector<fs::path> all;
    const fs::path templates = repo / "guerrilla-mode" / "mission";
    for (const char* prefix : {"Guerrilla.", "Showcase.", "Qrf.", "Market.", "Undercover."})
    {
        for (const fs::path& dir : MissionDirsMatching(templates, prefix))
        {
            all.push_back(dir);
        }
    }
    for (const fs::path& dir : MissionDirsMatching(repo / "tests" / "integration" / "missions", "guerrilla_"))
    {
        all.push_back(dir);
    }
    return all;
}

// the missions that boot the WHOLE core through the two-line bootstrap (the
// reference slices Qrf./Market./Undercover. run their own bootstrap instead)
std::vector<fs::path> FullCoreMissions(const fs::path& repo)
{
    std::vector<fs::path> missions;
    const fs::path templates = repo / "guerrilla-mode" / "mission";
    for (const char* prefix : {"Guerrilla.", "Showcase."})
    {
        for (const fs::path& dir : MissionDirsMatching(templates, prefix))
        {
            missions.push_back(dir);
        }
    }
    for (const fs::path& dir : MissionDirsMatching(repo / "tests" / "integration" / "missions", "guerrilla_"))
    {
        missions.push_back(dir);
    }
    return missions;
}

// "\gmcore\scripts\<name>.sqs" wherever it appears in a script (exec targets
// and addAction dispatch paths both go through OpenScript, so both forms are
// the same contract). The doubled backslashes are regex escapes inside a raw
// string literal: the .sqs file on disk holds ONE backslash per separator.
std::vector<std::string> CoreScriptReferences(const std::string& text)
{
    static const std::regex kRef(R"(\\gmcore\\scripts\\([A-Za-z0-9_]+\.sqs))");
    std::vector<std::string> names;
    for (auto it = std::sregex_iterator(text.begin(), text.end(), kRef); it != std::sregex_iterator(); ++it)
    {
        names.push_back((*it)[1].str());
    }
    return names;
}

bool Contains(const std::vector<std::string>& v, const std::string& s)
{
    return std::find(v.begin(), v.end(), s) != v.end();
}

// non-blank, non-comment lines of an SQS file. A ';' only starts a comment at
// the start of a line: mid-line it is a STATEMENT SEPARATOR.
std::vector<std::string> CodeLines(const std::string& text)
{
    std::vector<std::string> code;
    std::string line;
    for (size_t i = 0; i <= text.size(); ++i)
    {
        if (i == text.size() || text[i] == '\n')
        {
            // trim CR and surrounding spaces
            const size_t first = line.find_first_not_of(" \t\r");
            const size_t last = line.find_last_not_of(" \t\r");
            const std::string trimmed = (first == std::string::npos) ? "" : line.substr(first, last - first + 1);
            if (!trimmed.empty() && trimmed[0] != ';')
            {
                code.push_back(trimmed);
            }
            line.clear();
            continue;
        }
        line.push_back(text[i]);
    }
    return code;
}

} // namespace

TEST_CASE("The Guerrilla script core is one directory holding exactly the manifest", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path core = CoreDir(repo);

    INFO("core: " << core.string());
    REQUIRE(fs::exists(core / "init.sqs"));
    REQUIRE(fs::is_directory(core / "scripts"));

    // exactly the manifest: nothing added without a doc + test edit, and a
    // retired script (zones.sqs, spawning.sqs, ...) left behind fails here
    std::vector<std::string> expected = kCoreScripts;
    std::sort(expected.begin(), expected.end());
    REQUIRE(ListFileNames(core / "scripts") == expected);
}

TEST_CASE("ARCHITECTURE.md A.6 names every core script", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path doc = repo / "guerrilla-mode" / "ARCHITECTURE.md";
    REQUIRE(fs::exists(doc));

    const std::string text = ReadText(doc);

    // isolate section A.6 so a stray mention elsewhere in the document cannot
    // stand in for the manifest table
    const size_t begin = text.find("### A.6 Script layer file map");
    REQUIRE(begin != std::string::npos);
    const size_t end = text.find("### A.7", begin);
    REQUIRE(end != std::string::npos);
    const std::string section = text.substr(begin, end - begin);

    for (const std::string& script : kCoreScripts)
    {
        INFO("core script missing from ARCHITECTURE.md A.6: " << script);
        REQUIRE(section.find(script) != std::string::npos);
    }

    // the install location and the path convention are the other half of the
    // contract: a reader who misses the leading backslash writes dead execs
    REQUIRE(section.find("guerrilla-mode/core") != std::string::npos);
    REQUIRE(section.find(R"(\gmcore)") != std::string::npos);
}

TEST_CASE("No Guerrilla mission carries its own copy of the script core", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const std::vector<fs::path> missions = AllGuerrillaMissions(repo);
    // the four campaign templates, the showcase, three reference slices and
    // three integration missions at minimum
    REQUIRE(missions.size() >= 8);

    for (const fs::path& mission : missions)
    {
        INFO("mission: " << mission.string());
        REQUIRE(fs::exists(mission));
        // a scripts/ dir here means someone re-copied the core
        REQUIRE(!fs::exists(mission / "scripts"));
    }
}

TEST_CASE("Full-core missions boot through the two-line bootstrap", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const std::vector<fs::path> missions = FullCoreMissions(repo);
    // Guerrilla.{Abel,Demo,Lebanon80,Sinai} + Showcase.Abel + the three
    // guerrilla_* integration missions
    REQUIRE(missions.size() >= 8);

    for (const fs::path& mission : missions)
    {
        INFO("mission: " << mission.string());
        const fs::path init = mission / "init.sqs";
        REQUIRE(fs::exists(init));

        const std::vector<std::string> code = CodeLines(ReadText(init));
        INFO("init.sqs code lines: " << code.size());
        REQUIRE(code.size() == 1);
        REQUIRE(code[0] == kBootstrapLine);
    }
}

TEST_CASE("Every core-script reference resolves to a file in the core", "[guerrilla][missions]")
{
    const fs::path repo = RepoRoot();
    const fs::path core = CoreDir(repo);

    // the core itself (init.sqs execs the managers, campaign/market/recruit
    // dispatch addActions back into it) ...
    std::vector<fs::path> sources = {core / "init.sqs"};
    for (const std::string& script : kCoreScripts)
    {
        sources.push_back(core / "scripts" / script);
    }
    // ... plus every mission's own init.sqs (the reference slices name the one
    // policy script they boot)
    for (const fs::path& mission : AllGuerrillaMissions(repo))
    {
        sources.push_back(mission / "init.sqs");
    }

    size_t seen = 0;
    for (const fs::path& source : sources)
    {
        INFO("source: " << source.string());
        REQUIRE(fs::exists(source));
        const std::string text = ReadText(source);

        for (const std::string& target : CoreScriptReferences(text))
        {
            INFO("reference: \\gmcore\\scripts\\" << target);
            REQUIRE(Contains(kCoreScripts, target));
            REQUIRE(fs::exists(core / "scripts" / target));
            ++seen;
        }

        // A relative "scripts/..." would resolve through FindScript inside the
        // mission folder, which no longer holds anything - a silent no-op that
        // costs a whole manager. Comments naming a core script by its bare
        // filename are fine; the quoted relative path is what fails.
        INFO("a relative \"scripts/... path survives in this file");
        REQUIRE(text.find("\"scripts/") == std::string::npos);
    }

    // the walk found real references, not an empty regex sweep (init.sqs alone
    // execs 13 managers)
    REQUIRE(seen >= 13);
}
