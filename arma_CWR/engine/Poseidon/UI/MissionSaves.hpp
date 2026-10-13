#pragma once

// Per-mission save slots: the one place that names them, picks the newest,
// deletes them for a new start, and decides when a new start must ask first
// (issue #79 B4).
//
// THE SAVE-SLOT CONTRACT. A mission's save directory (GetSaveDirectory(); for
// a Guerrilla campaign "<user>/Saved/missions/Guerrilla.<Island>/", see
// GuerrillaCampaignSaveDirectory) holds up to three slots:
//
//   save.fps      THE USER SLOT. Written by the pause menu's Save and by the
//                 Guerrilla campaign's "Save Game" action (gmSaveGame). Read by
//                 the pause menu's Load and the mission-end dialog's Load.
//   autosave.fps  THE RETRY SLOT. Written by the stock saveGame command
//                 (mission checkpoints). In a Guerrilla campaign every save
//                 (gmSaveGame and the pause menu's Save) is mirrored here, so
//                 Retry after a death goes back to the last save. Read by
//                 Retry.
//   continue.fps  THE QUIT SNAPSHOT. Written when the player aborts a mission
//                 from the pause menu (SaveContinue, DisplayUIMenus.cpp). Read
//                 by the Single Missions browser's Resume.
//
// The GUERRILLA screen's Continue resumes the NEWEST of the three
// (NewestMissionSave), so a campaign continues from wherever the player last
// stood: the quit snapshot after an ordinary quit, the last Save Game after a
// crash or a death.
//
// THE REPLACEMENT POLICY. Every new start of a mission deletes all three slots
// (stock behaviour: a fresh run must not Load or Retry into the previous one).
// DecideSaveReplacement is the one rule each deletion site consults: a stock
// mission or campaign deletes at once; a Guerrilla campaign whose directory
// still holds a save deletes only after the player confirmed it.

#include <Poseidon/Foundation/Strings/RString.hpp>

#include <ctime>
#include <functional>
#include <vector>

namespace Poseidon
{

constexpr const char* kSaveSlotUser = "save.fps";
constexpr const char* kSaveSlotRetry = "autosave.fps";
constexpr const char* kSaveSlotContinue = "continue.fps";

// The file-system seam, injectable for unit tests: `exists` answers whether a
// path is a readable file, `modTime` its last write time.
struct MissionSaveProbe
{
    std::function<bool(RString)> exists;
    std::function<time_t(RString)> modTime;
};

// The probe the game uses (QIFStream::FileExists + getFileModTime).
MissionSaveProbe DiskMissionSaveProbe();

// The slots that exist in `dir` (which ends in a path separator), as full
// paths in the fixed order continue, save, autosave.
std::vector<RString> ExistingMissionSaves(RString dir, const MissionSaveProbe& probe);

// The newest existing slot in `dir` as a full path, EMPTY when there is none.
// A tie on the write time keeps the fixed order above: continue, then save,
// then autosave.
RString NewestMissionSave(RString dir, const MissionSaveProbe& probe);

// What a NEW START must do with the saves in its directory.
enum class SaveReplacement
{
    Delete,   // delete all three slots now (nothing to lose, or already agreed)
    AskFirst, // a Guerrilla campaign save would be lost: confirm before deleting
};

// The one rule. guerrillaCampaign: the mission is a Guerrilla campaign
// (IsGuerrillaCampaignMission). savesExist: any slot is present.
// playerConfirmed: the player already agreed to replace them.
SaveReplacement DecideSaveReplacement(bool guerrillaCampaign, bool savesExist, bool playerConfirmed);

// Delete continue.fps, autosave.fps and save.fps in `dir`. The single deletion
// implementation behind every new-start site.
void DeleteMissionSaves(RString dir);

// The new-start half of the policy, called by every site that begins a run
// from scratch (DisplayMain's campaign restart, IDD_INTRO and the debriefing
// restart, UI/OptionsUIApp.cpp): consults DecideSaveReplacement and, when the
// answer is Delete, deletes through `deleteDir`. Returns false when it is
// AskFirst: nothing was deleted and the caller must confirm with the player
// (then call `deleteDir` itself) or keep the saves. `deleteDir` is injectable
// for unit tests and defaults to DeleteMissionSaves.
bool ApplySaveReplacement(RString dir, bool guerrillaCampaign, bool playerConfirmed, const MissionSaveProbe& probe,
                          const std::function<void(RString)>& deleteDir = DeleteMissionSaves);

// True when the mission header names a Guerrilla campaign template: the launch
// paths (GUERRILLA screen, Single Missions browser) all set filenameReal to
// "Guerrilla" for a "Guerrilla.<Island>" template, banked or not.
bool IsGuerrillaCampaignMission(RString filenameReal);

// Where an island's Guerrilla campaign saves live: the directory
// GetSaveDirectory() resolves once the "Guerrilla.<island>" template is the
// current mission (the same path the Single Missions browser checks).
RString GuerrillaCampaignSaveDirectory(RString island);

// One-shot consent latch between the GUERRILLA screen (which asks) and the
// IDD_INTRO new-start site (which deletes once the mission is set up). Granting
// records the island; consuming returns true when the current island matches
// and clears the latch either way.
void GrantGuerrillaSaveReplacement(RString island);
bool ConsumeGuerrillaSaveReplacement(RString island);

// The player-facing confirmation for replacing a Guerrilla campaign save.
// `island` is the display name, `when` the newest save's local time already
// formatted (FormatMissionSaveTime). Localized (STR_UD_GM_REPLACE_SAVE) with
// an English fallback for data without UD's stringtable shard.
RString GuerrillaReplaceSaveMessage(RString island, RString when);

// The same confirmation for a restart that did not come through the GUERRILLA
// screen (the debriefing's restart, or a template started from another
// browser): STR_UD_GM_RESTART_SAVE.
RString GuerrillaRestartSaveMessage();

// The GUERRILLA screen's Continue label for a save written at `when`
// (STR_UD_GM_CONTINUE), and the refusal when that save fails to load
// (STR_UD_GM_CONTINUE_FAILED).
RString GuerrillaContinueLabel(RString when);
RString GuerrillaContinueFailedMessage();

// "month/day/year hour:minute" of a save file's local write time, EMPTY when
// the file cannot be read. The stock Resume button's date order.
RString FormatMissionSaveTime(RString path);

} // namespace Poseidon
