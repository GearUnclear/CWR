// Save slots, the new-start replacement policy and the Guerrilla save
// commands (issue #79 B4, UI/MissionSaves.hpp).

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/UI/MissionSaves.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/Containers/Array.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string.h>
#include <system_error>
#include <vector>

using namespace Poseidon;

namespace
{
// An in-memory save directory: path -> write time.
struct FakeSaveDir
{
    std::map<std::string, time_t> files;

    MissionSaveProbe Probe()
    {
        MissionSaveProbe probe;
        probe.exists = [this](RString path) { return files.count((const char*)path) > 0; };
        probe.modTime = [this](RString path)
        {
            auto it = files.find((const char*)path);
            return it == files.end() ? time_t(0) : it->second;
        };
        return probe;
    }
};

const RString kDir("u/Saved/missions/Guerrilla.Abel/");

std::string At(const char* slot)
{
    return std::string((const char*)kDir) + slot;
}
} // namespace

TEST_CASE("NewestMissionSave picks the most recent of the three slots", "[ui][mission-saves]")
{
    FakeSaveDir dir;
    CHECK(NewestMissionSave(kDir, dir.Probe()).GetLength() == 0);

    dir.files[At(kSaveSlotRetry)] = 100;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotRetry));

    // the campaign's Save Game action writes save.fps later than an old checkpoint
    dir.files[At(kSaveSlotUser)] = 200;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotUser));

    // an Abort's quit snapshot is newer still
    dir.files[At(kSaveSlotContinue)] = 300;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotContinue));

    // a later Save Game after resuming wins over the older snapshot
    dir.files[At(kSaveSlotUser)] = 400;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotUser));
}

TEST_CASE("NewestMissionSave breaks a write-time tie as continue, save, autosave", "[ui][mission-saves]")
{
    FakeSaveDir dir;
    // gmSaveGame mirrors save.fps into autosave.fps within the same second
    dir.files[At(kSaveSlotUser)] = 500;
    dir.files[At(kSaveSlotRetry)] = 500;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotUser));

    dir.files[At(kSaveSlotContinue)] = 500;
    CHECK(std::string((const char*)NewestMissionSave(kDir, dir.Probe())) == At(kSaveSlotContinue));
}

TEST_CASE("ExistingMissionSaves lists only present slots in the fixed order", "[ui][mission-saves]")
{
    FakeSaveDir dir;
    CHECK(ExistingMissionSaves(kDir, dir.Probe()).empty());
    dir.files[At(kSaveSlotRetry)] = 1;
    dir.files[At(kSaveSlotContinue)] = 2;
    dir.files[std::string((const char*)kDir) + "weapons.cfg"] = 3; // not a save slot
    std::vector<RString> found = ExistingMissionSaves(kDir, dir.Probe());
    REQUIRE(found.size() == 2);
    CHECK(std::string((const char*)found[0]) == At(kSaveSlotContinue));
    CHECK(std::string((const char*)found[1]) == At(kSaveSlotRetry));
    // no directory, no probe: nothing, never a crash
    CHECK(ExistingMissionSaves(RString(), dir.Probe()).empty());
    CHECK(ExistingMissionSaves(kDir, MissionSaveProbe{}).empty());
}

TEST_CASE("DecideSaveReplacement asks only before losing a Guerrilla campaign save", "[ui][mission-saves]")
{
    // stock missions and campaigns keep stock behaviour, saves or not
    CHECK(DecideSaveReplacement(false, false, false) == SaveReplacement::Delete);
    CHECK(DecideSaveReplacement(false, true, false) == SaveReplacement::Delete);
    // a Guerrilla campaign with nothing saved has nothing to lose
    CHECK(DecideSaveReplacement(true, false, false) == SaveReplacement::Delete);
    // a Guerrilla campaign save goes only with the player's consent
    CHECK(DecideSaveReplacement(true, true, false) == SaveReplacement::AskFirst);
    CHECK(DecideSaveReplacement(true, true, true) == SaveReplacement::Delete);
}

TEST_CASE("ApplySaveReplacement deletes or defers per the policy", "[ui][mission-saves]")
{
    FakeSaveDir dir;
    dir.files[At(kSaveSlotUser)] = 10;
    std::vector<std::string> deleted;
    auto deleter = [&deleted](RString d) { deleted.push_back((const char*)d); };

    SECTION("a stock new start deletes without asking")
    {
        CHECK(ApplySaveReplacement(kDir, false, false, dir.Probe(), deleter));
        REQUIRE(deleted.size() == 1);
        CHECK(deleted[0] == std::string((const char*)kDir));
    }
    SECTION("an unconfirmed Guerrilla new start keeps the save and asks")
    {
        CHECK_FALSE(ApplySaveReplacement(kDir, true, false, dir.Probe(), deleter));
        CHECK(deleted.empty());
    }
    SECTION("a confirmed Guerrilla new start deletes")
    {
        CHECK(ApplySaveReplacement(kDir, true, true, dir.Probe(), deleter));
        CHECK(deleted.size() == 1);
    }
    SECTION("a Guerrilla new start with no save proceeds")
    {
        FakeSaveDir empty;
        CHECK(ApplySaveReplacement(kDir, true, false, empty.Probe(), deleter));
    }
}

TEST_CASE("DeleteMissionSaves removes the three slots and nothing else", "[ui][mission-saves]")
{
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path root = fs::temp_directory_path() / "ud_mission_saves_test";
    fs::remove_all(root, ec);
    fs::create_directories(root);
    for (const char* name : {kSaveSlotContinue, kSaveSlotUser, kSaveSlotRetry, "weapons.cfg", "objects.sav"})
    {
        std::ofstream(root / name) << "x";
    }
    RString dir((root.generic_string() + "/").c_str());
    REQUIRE(ExistingMissionSaves(dir, DiskMissionSaveProbe()).size() == 3);

    DeleteMissionSaves(dir);

    CHECK(ExistingMissionSaves(dir, DiskMissionSaveProbe()).empty());
    CHECK(fs::exists(root / "weapons.cfg"));
    CHECK(fs::exists(root / "objects.sav"));
    // an empty directory string is a no-op, not a delete at the working dir
    DeleteMissionSaves(RString());
    fs::remove_all(root, ec);
}

TEST_CASE("The Guerrilla replacement consent is one-shot and island-scoped", "[ui][mission-saves]")
{
    // nothing granted
    ConsumeGuerrillaSaveReplacement(RString());
    CHECK_FALSE(ConsumeGuerrillaSaveReplacement("Abel"));

    // granted for Abel; banked templates report the world lowercased
    GrantGuerrillaSaveReplacement("Abel");
    CHECK(ConsumeGuerrillaSaveReplacement("abel"));
    CHECK_FALSE(ConsumeGuerrillaSaveReplacement("Abel")); // consumed

    // a consent for one island never covers another, and is gone afterwards
    GrantGuerrillaSaveReplacement("Abel");
    CHECK_FALSE(ConsumeGuerrillaSaveReplacement("Sinai"));
    CHECK_FALSE(ConsumeGuerrillaSaveReplacement("Abel"));
}

TEST_CASE("Guerrilla campaigns are recognised by the template name", "[ui][mission-saves]")
{
    CHECK(IsGuerrillaCampaignMission("Guerrilla"));
    CHECK(IsGuerrillaCampaignMission("guerrilla"));
    CHECK_FALSE(IsGuerrillaCampaignMission(""));
    CHECK_FALSE(IsGuerrillaCampaignMission("Showcase"));
    CHECK_FALSE(IsGuerrillaCampaignMission("Guerrilla2"));
    CHECK(GuerrillaCampaignSaveDirectory("").GetLength() == 0);
    const std::string dir((const char*)GuerrillaCampaignSaveDirectory("Malden"));
    CHECK(dir.size() > strlen("Saved/missions/Guerrilla.Malden/"));
    CHECK(dir.compare(dir.size() - strlen("Saved/missions/Guerrilla.Malden/"), std::string::npos,
                      "Saved/missions/Guerrilla.Malden/") == 0);
}

TEST_CASE("Guerrilla save messages carry the island and the save time", "[ui][mission-saves]")
{
    // no UD stringtable shard is loaded here: the English fallbacks apply
    const std::string ask((const char*)GuerrillaReplaceSaveMessage("Malden", "10/12/2026 14:05"));
    CHECK(ask.find("Malden") != std::string::npos);
    CHECK(ask.find("10/12/2026 14:05") != std::string::npos);
    CHECK(ask.find("Start anyway?") != std::string::npos);
    CHECK(ask.find('%') == std::string::npos);

    const std::string label((const char*)GuerrillaContinueLabel("10/12/2026 14:05"));
    CHECK(label == "Continue (10/12/2026 14:05)");
    CHECK(GuerrillaRestartSaveMessage().GetLength() > 0);
    CHECK(GuerrillaContinueFailedMessage().GetLength() > 0);
    // a save that cannot be read has no time
    CHECK(FormatMissionSaveTime("no/such/save.fps").GetLength() == 0);
}

TEST_CASE("gmSaveGame is registered next to saveGame", "[ui][mission-saves][gameStateExt]")
{
    GGameState.Reset();
    Poseidon::Foundation::InitModules();
    AutoArray<RStringS> nulars;
    GGameState.AppendNularOpList(nulars, [](const char*) { return true; });
    bool saveGame = false;
    bool gmSaveGame = false;
    for (int i = 0; i < nulars.Size(); ++i)
    {
        saveGame = saveGame || strcmp((const char*)nulars[i], "saveGame") == 0;
        gmSaveGame = gmSaveGame || strcmp((const char*)nulars[i], "gmSaveGame") == 0;
    }
    CHECK(saveGame);
    CHECK(gmSaveGame);
}
