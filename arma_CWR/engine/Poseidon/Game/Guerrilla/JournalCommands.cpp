// Script command surface for the Guerrilla field journal.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run - same pattern as StashRegistryCommands.cpp.
//
//   gmJournalLog "<text>"                 diary entry, stamped "Day N HH:MM"
//   gmJournalNote [text, zone, kind, charId]
//                                         diary entry tagged with a zone, a kind
//                                         ("plain"|"good"|"warn"|"danger") and the
//                                         Legend registry character it is about
//   gmJournalObjective [id, text, state]  upsert an objective row
//                                         (state "ACTIVE"|"DONE"|"FAILED"|"HIDDEN")
//   gmJournalStatus [key, text]           upsert a Situation line ("" removes)
//   gmJournalCount                        -> scalar, diary entries
//   gmJournalEntry <i>                    -> [stamp, text, zone, kind] (0 = oldest),
//                                            [] out of range.  Four elements, not
//                                            five: the shipped scripts index it
//   gmJournalEntryChar <i>                -> the entry's charId, "" when the line
//                                            names no character or i is out of range
//   gmJournalObjectiveState "<id>"        -> state name, "" when unknown
//   gmJournalStatusText "<key>"           -> the status line's text, "" when unknown
//   gmDisplayName "<class>"               -> the package's displayName for a
//                                            weapon / magazine / vehicle class
//   gmIslandName                         -> the world's CfgWorlds description
//                                            ("Malden"), the class name when absent

#include <Poseidon/Game/Guerrilla/Journal.hpp>

#include <Poseidon/Core/Global.hpp>                  // Glob.header.worldname
#include <Poseidon/IO/ParamFileExt.hpp>              // Pars
#include <Poseidon/IO/ParamFile/LocalizedString.hpp> // displayName ($STR_) resolution

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from Journal.cpp to keep this TU (only content besides this:
// file-static commands + module registration) in the link.
void EnsureJournalCommandsLinked() {}

// The island's player-facing name: CfgWorlds >> <world> >> description
// ("Malden", "Southern Sinai"), falling back to the world class name.
RString IslandDisplayName()
{
    RString world = Glob.header.worldname;
    if (world.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* worlds = Pars.FindEntry("CfgWorlds");
    if (worlds)
    {
        if (const ParamEntry* entry = worlds->FindEntry(world))
        {
            RString description = entry->ReadValue("description", world);
            if (description.GetLength() > 0)
            {
                return description;
            }
        }
    }
    return world;
}
} // namespace Poseidon::Guerrilla

// gmJournalLog "<text>"
static GameValue GmJournalLog(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType text = oper1;
    Journal::Instance().AddEntry(JournalStampNow(), RString(text));
    return NOTHING;
}

// gmJournalNote [text, zone, kind, charId]: a diary line tagged with the zone it
// is about, a kind ("plain" | "good" | "warn" | "danger") that colours the page,
// and the Legend registry character id the line is about.  Every trailing
// argument may be omitted (["text"] == gmJournalLog "text"), and arity 1 to 3
// behaves exactly as it did before the id arrived.
//
// External linkage, unlike its neighbours: the arity contract above is the one
// piece of this surface a script can break silently, so the unit suite calls it
// directly rather than through the evaluator (the pattern the tri* asserts in
// GameStateExtTest.cpp already use).
GameValue GmJournalNote(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (array.Size() < 1 || array.Size() > 4)
    {
        state->SetError(EvalGen, "gmJournalNote: [text, zone, kind, charId]");
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameString))
    {
        return NOTHING;
    }
    GameStringType text = array[0];
    RString zone;
    RString charId;
    int kind = JKPlain;
    if (array.Size() >= 2)
    {
        if (!CheckType(state, array[1], GameString))
        {
            return NOTHING;
        }
        zone = RString((GameStringType)array[1]);
    }
    if (array.Size() >= 3)
    {
        if (array[2].GetType() == GameScalar)
        {
            kind = toInt((float)array[2]);
        }
        else if (array[2].GetType() == GameString)
        {
            kind = Journal::EntryKindFromName((GameStringType)array[2]);
            if (kind < 0)
            {
                state->SetError(EvalGen, "gmJournalNote: unknown kind");
                return NOTHING;
            }
        }
        else
        {
            state->SetError(EvalGen, "gmJournalNote: kind must be a string or a number");
            return NOTHING;
        }
    }
    if (array.Size() >= 4)
    {
        if (!CheckType(state, array[3], GameString))
        {
            return NOTHING;
        }
        charId = RString((GameStringType)array[3]);
    }
    Journal::Instance().AddEntry(JournalStampNow(), RString(text), zone, kind, charId);
    return NOTHING;
}

// gmDisplayName "<class>" -> the package's displayName for a weapon /
// magazine / vehicle class (CfgWeapons, then CfgMagazines, then CfgVehicles),
// the class name itself when none is configured.  Lets the scripts publish
// player-facing names on the journal without knowing the config.
static GameValue GmDisplayName(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType className = oper1;
    const char* banks[] = {"CfgWeapons", "CfgMagazines", "CfgVehicles"};
    for (const char* bankName : banks)
    {
        const ParamEntry* bank = Pars.FindEntry(bankName);
        const ParamEntry* cls = bank ? bank->FindEntry(className) : nullptr;
        const ParamEntry* dn = cls ? cls->FindEntry("displayName") : nullptr;
        if (dn)
        {
            LocalizedString text;
            text.Bind(*dn);
            RString name = (const char*)text.Get();
            if (name.GetLength() > 0)
            {
                return GameStringType(name);
            }
        }
    }
    return className;
}

// gmJournalObjective [id, text, state]
static GameValue GmJournalObjective(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 3))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameString) || !CheckType(state, array[1], GameString) ||
        !CheckType(state, array[2], GameString))
    {
        return NOTHING;
    }
    GameStringType id = array[0];
    GameStringType text = array[1];
    GameStringType stateName = array[2];
    int value = Journal::ObjectiveStateFromName(stateName);
    if (value < 0)
    {
        state->SetError(EvalGen, "gmJournalObjective: unknown state");
        return NOTHING;
    }
    Journal::Instance().SetObjective(RString(id), RString(text), value);
    return NOTHING;
}

// gmJournalStatus [key, text]
static GameValue GmJournalStatus(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameString) || !CheckType(state, array[1], GameString))
    {
        return NOTHING;
    }
    GameStringType key = array[0];
    GameStringType text = array[1];
    Journal::Instance().SetStatus(RString(key), RString(text));
    return NOTHING;
}

// gmJournalCount -> scalar
static GameValue GmJournalCount(const GameState* /*state*/)
{
    return (float)Journal::Instance().EntryCount();
}

// gmJournalEntry <i> -> [stamp, text, zone, kind] or []
static GameValue GmJournalEntry(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const Journal& journal = Journal::Instance();
    int index = toInt((float)oper1);
    if (index < 0 || index >= journal.EntryCount())
    {
        return value;
    }
    array.Resize(4);
    array[0] = GameStringType(journal.Entry(index).stamp);
    array[1] = GameStringType(journal.Entry(index).text);
    array[2] = GameStringType(journal.Entry(index).zone);
    array[3] = (float)journal.Entry(index).kind;
    return value;
}

// gmJournalEntryChar <i> -> the entry's charId, "" when the line names no
// character or the index is out of range.  Kept off gmJournalEntry on purpose:
// the shipped scripts index that array, so it stays four elements wide.
// External linkage for the same reason as GmJournalNote above.
GameValue GmJournalEntryChar(const GameState* /*state*/, GameValuePar oper1)
{
    const Journal& journal = Journal::Instance();
    int index = toInt((float)oper1);
    if (index < 0 || index >= journal.EntryCount())
    {
        return GameStringType("");
    }
    return GameStringType(journal.Entry(index).charId);
}

// gmJournalObjectiveState "<id>" -> "ACTIVE"|"DONE"|"FAILED"|"HIDDEN", "" unknown
static GameValue GmJournalObjectiveState(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType id = oper1;
    const Journal& journal = Journal::Instance();
    int i = journal.FindObjective(id);
    if (i < 0)
    {
        return GameStringType("");
    }
    return GameStringType(Journal::ObjectiveStateName(journal.Objective(i).state));
}

// gmJournalStatusText "<key>" -> the status line's text, "" when unknown
static GameValue GmJournalStatusText(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType key = oper1;
    const Journal& journal = Journal::Instance();
    int i = journal.FindStatus(key);
    if (i < 0)
    {
        return GameStringType("");
    }
    return GameStringType(journal.Status(i).text);
}

// gmIslandName -> string
static GameValue GmIslandName(const GameState* /*state*/)
{
    return GameStringType(IslandDisplayName());
}

INIT_MODULE(GuerrillaJournal, 3)
{
    GGameState.NewFunction(GameFunction(GameString, "gmJournalStatusText", GmJournalStatusText, GameString));
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalLog", GmJournalLog, GameString));
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalNote", GmJournalNote, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmDisplayName", GmDisplayName, GameString));
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalObjective", GmJournalObjective, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalStatus", GmJournalStatus, GameArray));
    GGameState.NewNularOp(GameNular(GameScalar, "gmJournalCount", GmJournalCount));
    GGameState.NewFunction(GameFunction(GameArray, "gmJournalEntry", GmJournalEntry, GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "gmJournalEntryChar", GmJournalEntryChar, GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "gmJournalObjectiveState", GmJournalObjectiveState, GameString));
    GGameState.NewNularOp(GameNular(GameString, "gmIslandName", GmIslandName));
}
