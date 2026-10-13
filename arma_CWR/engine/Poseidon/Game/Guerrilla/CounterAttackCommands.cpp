// Script command surface for the occupier counter-attack service (issue #79,
// CounterAttack.hpp).  Registered from its own INIT_MODULE at stage 3, the
// JournalCommands.cpp / CampaignEndCommands.cpp pattern.
//
//   gmCounterAttacks            -> [[town, source, phase, clock, strength,
//                                     size, townsBefore, group], ...]
//                                  one row per pending or running
//                                  counter-attack; phase is SCHEDULED /
//                                  TRAVEL / HOLDING / LIVE, clock the
//                                  seconds left (SCHEDULED, TRAVEL) or alive
//                                  (LIVE), group grpNull unless LIVE
//   gmCounterAttackTune [key, value]
//                                  runtime override of one tuning value
//                                  (tests, debugging); keys: "enabled",
//                                  "firstTownChance", "chance",
//                                  "warLevelStep", "delayMin", "delayMax",
//                                  "size", "speed", "stageDistance",
//                                  "liveTimeout", "sourceKeep", "maxLive",
//                                  "townsTaken" (the saved k counter).  A
//                                  pending SCHEDULED clock is clamped to the
//                                  new delayMax.  Returns true when the key
//                                  is known.

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/AI/AI.hpp> // AIGroup (the group link)
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
void EnsureCounterAttackCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static GameValue GmCounterAttacks(const GameState* state)
{
    const CounterAttack& ca = CounterAttack::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < ca.Count(); i++)
    {
        const CounterAttack::Record* r = ca.Get(i);
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(8);
        row[0] = GameStringType(r->target);
        row[1] = GameStringType(r->source);
        row[2] = GameStringType(CounterAttack::PhaseName(r->phase));
        row[3] = r->clock;
        row[4] = (float)r->strength;
        row[5] = (float)r->size;
        row[6] = (float)r->townsBefore;
        row[7] = GameValueExt(r->phase == CALive ? r->group.GetLink() : (AIGroup*)nullptr);
        rows.Add(rowValue);
    }
    return value;
}

static GameValue GmCounterAttackTune(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    if (args.Size() < 2 || args[0].GetType() != GameString)
    {
        return GameValue(false);
    }
    const RString key = (GameStringType)args[0];
    const float v = args[1].GetType() == GameBool ? ((GameBoolType)args[1] ? 1.0f : 0.0f) : (float)args[1];
    CounterAttack& ca = CounterAttack::Instance();
    CounterAttackTuning& t = ca.TuningMutable();
    if (stricmp(key, "enabled") == 0)
    {
        t.enabled = v != 0;
    }
    else if (stricmp(key, "firstTownChance") == 0)
    {
        t.firstTownChance = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else if (stricmp(key, "chance") == 0)
    {
        t.chance = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else if (stricmp(key, "warLevelStep") == 0)
    {
        t.warLevelStep = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "maxLive") == 0)
    {
        t.maxLive = v < 0 ? 0 : (int)v;
    }
    else if (stricmp(key, "townsTaken") == 0)
    {
        ca.SetTownsTaken((int)v);
    }
    else if (stricmp(key, "delayMin") == 0)
    {
        t.delayMin = v < 0 ? 0 : v;
        if (t.delayMax < t.delayMin)
        {
            t.delayMax = t.delayMin;
        }
    }
    else if (stricmp(key, "delayMax") == 0)
    {
        t.delayMax = v < t.delayMin ? t.delayMin : v;
        ca.ClampScheduled(t.delayMax);
    }
    else if (stricmp(key, "size") == 0)
    {
        t.size = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "speed") == 0)
    {
        t.speed = v < 0.5f ? 0.5f : v;
    }
    else if (stricmp(key, "stageDistance") == 0)
    {
        t.stageDistance = v < 50 ? 50 : v;
    }
    else if (stricmp(key, "liveTimeout") == 0)
    {
        t.liveTimeout = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "sourceKeep") == 0)
    {
        t.sourceKeep = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else
    {
        return GameValue(false);
    }
    return GameValue(true);
}

INIT_MODULE(GuerrillaCounterAttack, 3)
{
    GGameState.NewNularOp(GameNular(GameArray, "gmCounterAttacks", GmCounterAttacks));
    GGameState.NewFunction(GameFunction(GameBool, "gmCounterAttackTune", GmCounterAttackTune, GameArray));
}
