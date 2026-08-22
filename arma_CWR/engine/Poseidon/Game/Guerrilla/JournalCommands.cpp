// Script command surface for the Guerrilla field journal.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run - same pattern as StashRegistryCommands.cpp.
//
//   gmJournalLog "<text>"                 diary entry, stamped "Day N HH:MM"
//   gmJournalObjective [id, text, state]  upsert an objective row
//                                         (state "ACTIVE"|"DONE"|"FAILED"|"HIDDEN")
//   gmJournalStatus [key, text]           upsert a Situation line ("" removes)
//   gmJournalCount                        -> scalar, diary entries
//   gmJournalEntry <i>                    -> [stamp, text] (0 = oldest), [] out of range
//   gmJournalObjectiveState "<id>"        -> state name, "" when unknown
//   gmJournalStatusText "<key>"           -> the status line's text, "" when unknown
//   gmIslandName                         -> the world's CfgWorlds description
//                                            ("Malden"), the class name when absent

#include <Poseidon/Game/Guerrilla/Journal.hpp>

#include <Poseidon/Core/Global.hpp>     // Glob.header.worldname
#include <Poseidon/IO/ParamFileExt.hpp> // Pars

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

// gmJournalEntry <i> -> [stamp, text] or []
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
    array.Resize(2);
    array[0] = GameStringType(journal.Entry(index).stamp);
    array[1] = GameStringType(journal.Entry(index).text);
    return value;
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
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalObjective", GmJournalObjective, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmJournalStatus", GmJournalStatus, GameArray));
    GGameState.NewNularOp(GameNular(GameScalar, "gmJournalCount", GmJournalCount));
    GGameState.NewFunction(GameFunction(GameArray, "gmJournalEntry", GmJournalEntry, GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "gmJournalObjectiveState", GmJournalObjectiveState, GameString));
    GGameState.NewNularOp(GameNular(GameString, "gmIslandName", GmIslandName));
}
