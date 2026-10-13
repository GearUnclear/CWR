#include <Poseidon/UI/MissionSaves.hpp>
#include <Poseidon/UI/OptionsUICommon.hpp>                // GetMissionSaveDirectory
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp> // LocalizeStringOr
#include <Poseidon/IO/Streams/QStream.hpp>                // QIFStream::FileExists
#include <Poseidon/Foundation/Common/Win.h>               // ::DeleteFile (the call every deletion site used)
#include <Poseidon/Foundation/platform.hpp>               // getFileModTime, getFileLocalTime

#include <stdio.h>
#include <string>

namespace Poseidon
{
namespace
{
// The fixed slot order: the tie-break of NewestMissionSave and the listing
// order of ExistingMissionSaves.
constexpr const char* kSlotOrder[] = {kSaveSlotContinue, kSaveSlotUser, kSaveSlotRetry};

// The island the player agreed to start over on, EMPTY when nobody agreed.
RString& ReplacementConsent()
{
    static RString island;
    return island;
}

// "%1"/"%2" token replacement, so a translated string never becomes a printf
// format (a wrong specifier in a stringtable cell would read past the args).
RString ReplaceTokens(RString text, RString one, RString two)
{
    std::string out((const char*)text);
    const struct
    {
        const char* token;
        const char* value;
    } subs[] = {{"%1", one}, {"%2", two}};
    for (const auto& sub : subs)
    {
        for (size_t at = out.find(sub.token); at != std::string::npos; at = out.find(sub.token, at))
        {
            out.replace(at, 2, sub.value);
            at += strlen(sub.value);
        }
    }
    return RString(out.c_str());
}
} // namespace

MissionSaveProbe DiskMissionSaveProbe()
{
    MissionSaveProbe probe;
    probe.exists = [](RString path) { return QIFStream::FileExists(path); };
    probe.modTime = [](RString path) { return getFileModTime(path); };
    return probe;
}

std::vector<RString> ExistingMissionSaves(RString dir, const MissionSaveProbe& probe)
{
    std::vector<RString> out;
    if (dir.GetLength() == 0 || !probe.exists)
    {
        return out;
    }
    for (const char* slot : kSlotOrder)
    {
        RString path = dir + RString(slot);
        if (probe.exists(path))
        {
            out.push_back(path);
        }
    }
    return out;
}

RString NewestMissionSave(RString dir, const MissionSaveProbe& probe)
{
    RString newest;
    time_t newestTime = 0;
    for (const RString& path : ExistingMissionSaves(dir, probe))
    {
        const time_t t = probe.modTime ? probe.modTime(path) : 0;
        // strictly newer only: an equal time keeps the earlier slot in kSlotOrder
        if (newest.GetLength() == 0 || t > newestTime)
        {
            newest = path;
            newestTime = t;
        }
    }
    return newest;
}

SaveReplacement DecideSaveReplacement(bool guerrillaCampaign, bool savesExist, bool playerConfirmed)
{
    // Stock missions and campaigns keep their stock behaviour: a new start
    // wipes the previous run's saves without asking. Only a Guerrilla campaign
    // is a long-lived world worth a prompt, and only while a save is there.
    if (!guerrillaCampaign || !savesExist || playerConfirmed)
    {
        return SaveReplacement::Delete;
    }
    return SaveReplacement::AskFirst;
}

void DeleteMissionSaves(RString dir)
{
    if (dir.GetLength() == 0)
    {
        return;
    }
    for (const char* slot : kSlotOrder)
    {
        RString path = dir + RString(slot);
        ::DeleteFile(path);
    }
}

bool ApplySaveReplacement(RString dir, bool guerrillaCampaign, bool playerConfirmed, const MissionSaveProbe& probe,
                          const std::function<void(RString)>& deleteDir)
{
    const bool savesExist = !ExistingMissionSaves(dir, probe).empty();
    if (DecideSaveReplacement(guerrillaCampaign, savesExist, playerConfirmed) == SaveReplacement::AskFirst)
    {
        return false;
    }
    if (deleteDir)
    {
        deleteDir(dir);
    }
    return true;
}

bool IsGuerrillaCampaignMission(RString filenameReal)
{
    return filenameReal.GetLength() > 0 && stricmp(filenameReal, "Guerrilla") == 0;
}

RString GuerrillaCampaignSaveDirectory(RString island)
{
    if (island.GetLength() == 0)
    {
        return RString();
    }
    return GetMissionSaveDirectory(RString("Guerrilla.") + island);
}

void GrantGuerrillaSaveReplacement(RString island)
{
    ReplacementConsent() = island;
}

bool ConsumeGuerrillaSaveReplacement(RString island)
{
    RString granted = ReplacementConsent();
    ReplacementConsent() = RString();
    return granted.GetLength() > 0 && island.GetLength() > 0 && stricmp(granted, island) == 0;
}

RString GuerrillaReplaceSaveMessage(RString island, RString when)
{
    RString first = ReplaceTokens(
        LocalizeStringOr("STR_UD_GM_REPLACE_SAVE", "%1 already has a saved campaign from %2."), island, when);
    RString second =
        LocalizeStringOr("STR_UD_GM_REPLACE_SAVE_ASK", "Starting a new campaign deletes it. Start anyway?");
    // one paragraph: MsgBox wraps by width and draws "\n" as a missing glyph
    return first + RString(" ") + second;
}

RString GuerrillaRestartSaveMessage()
{
    return LocalizeStringOr("STR_UD_GM_RESTART_SAVE",
                            "Restarting deletes the saved campaign on this island. Restart anyway?");
}

RString GuerrillaContinueLabel(RString when)
{
    return ReplaceTokens(LocalizeStringOr("STR_UD_GM_CONTINUE", "Continue (%1)"), when, RString());
}

RString GuerrillaContinueFailedMessage()
{
    return LocalizeStringOr("STR_UD_GM_CONTINUE_FAILED", "The saved campaign could not be loaded.");
}

RString FormatMissionSaveTime(RString path)
{
    int day = 0, month = 0, year = 0, hour = 0, minute = 0;
    if (path.GetLength() == 0 || !getFileLocalTime(path, day, month, year, hour, minute))
    {
        return RString();
    }
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d/%d/%d %d:%02d", month, day, year, hour, minute);
    return RString(buffer);
}

} // namespace Poseidon
