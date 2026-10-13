// Script command surface for the occupier forces pool (OCCUPIER-FORCES.md
// section 6.6, OccupierForces.hpp).  Registered from its own INIT_MODULE at
// stage 3, the CounterAttackCommands.cpp pattern.
//
//   gmOccupierForces            -> [pool, poolStart, band, tokens, posture,
//                                    control, wavesFired, K, excludedTowns,
//                                    lost]
//                                  band is "STRONG" / "STRETCHED" / "THIN" /
//                                  "SPENT", or "OFF" while the service is
//                                  inert (occupierForces = 0, no roster, or
//                                  not yet sized); posture "DORMANT" /
//                                  "AWAKE", "OFF" when inert; control the
//                                  share 0..1 (C = 0.6 M + 0.4 T);
//                                  excludedTowns the names of the CITY zones
//                                  not counted in K; lost the bodies the
//                                  occupier lost for good.  The numbers are
//                                  the stored values even when inert (all 0
//                                  in a campaign that never ran the service).
//   gmForcesTune [key, value]   runtime override of one tuning value (tests,
//                                  debugging), with test ranges; keys:
//                                  "enabled" (the occupierForces key; turning
//                                  it on sizes the pool on the next frame if
//                                  this session has not tried yet),
//                                  "poolMin", "poolPerTown" (sizing keys:
//                                  read at activation only),
//                                  "refillHours" (0.01..48 h), "poolFloor"
//                                  (0..1), "directorInterval" (1..600 s),
//                                  "tokensMax" (0..6), "controlMilWeight"
//                                  (0..1), "waveWarLevel" (0..3),
//                                  "meanStanding" (0..100, the refill
//                                  factor's input until per-town standing
//                                  exists).  Returns true when the key is
//                                  known.
//   gmForcesForce [what, value] test lever; what:
//                                  "pool"        sets the pool (>= 0) and
//                                                re-steps the band (no-op
//                                                while inert);
//                                  "refillHours" sets the refill duration
//                                                (0.01..48 h);
//                                  "wave"        parks a wave request for
//                                                the wave phase (logged, no
//                                                effect yet).
//                                  Returns true when what is known.

#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
void EnsureOccupierForcesCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static float ClampF(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static GameValue GmOccupierForces(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    const bool live = of.Active();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& row = value;
    row.Resize(10);
    row[0] = (float)of.Pool();
    row[1] = (float)of.PoolStart();
    row[2] = GameStringType(live ? OccupierForces::BandName(of.Band()) : "OFF");
    row[3] = (float)of.Tokens();
    row[4] = GameStringType(live ? OccupierForces::PostureName(of.Posture()) : "OFF");
    row[5] = of.Control();
    row[6] = (float)of.WavesFired();
    row[7] = (float)of.TownCount();
    GameValue excludedValue = state->CreateGameValue(GameArray);
    GameArrayType& excluded = excludedValue;
    for (int i = 0; i < of.ExcludedCount(); i++)
    {
        excluded.Add(GameStringType(of.ExcludedName(i)));
    }
    row[8] = excludedValue;
    row[9] = (float)of.Lost();
    return value;
}

static float ArgValue(const GameValue& v)
{
    return v.GetType() == GameBool ? ((GameBoolType)v ? 1.0f : 0.0f) : (float)v;
}

static GameValue GmForcesTune(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    if (args.Size() < 2 || args[0].GetType() != GameString)
    {
        return GameValue(false);
    }
    const RString key = (GameStringType)args[0];
    const float v = ArgValue(args[1]);
    OccupierForces& of = OccupierForces::Instance();
    OccupierForcesTuning& t = of.TuningMutable();
    if (stricmp(key, "enabled") == 0)
    {
        t.enabled = v != 0;
    }
    else if (stricmp(key, "poolMin") == 0)
    {
        t.poolMin = (int)ClampF(v, 0, 200);
    }
    else if (stricmp(key, "poolPerTown") == 0)
    {
        t.poolPerTown = (int)ClampF(v, 0, 8);
    }
    else if (stricmp(key, "refillHours") == 0)
    {
        t.refillHours = ClampF(v, 0.01f, 48.0f);
    }
    else if (stricmp(key, "poolFloor") == 0)
    {
        t.poolFloor = ClampF(v, 0, 1);
    }
    else if (stricmp(key, "directorInterval") == 0)
    {
        t.directorInterval = ClampF(v, 1.0f, 600.0f);
    }
    else if (stricmp(key, "tokensMax") == 0)
    {
        t.tokensMax = (int)ClampF(v, 0, 6);
    }
    else if (stricmp(key, "controlMilWeight") == 0)
    {
        t.controlMilWeight = ClampF(v, 0, 1);
    }
    else if (stricmp(key, "waveWarLevel") == 0)
    {
        t.waveWarLevel = ClampF(v, 0, 3);
    }
    else if (stricmp(key, "meanStanding") == 0)
    {
        of.SetMeanStanding(v);
    }
    else
    {
        return GameValue(false);
    }
    LOG_INFO(Core, "OccupierForces: gmForcesTune {} = {}", (const char*)key, (double)v);
    return GameValue(true);
}

static GameValue GmForcesForce(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    if (args.Size() < 1 || args[0].GetType() != GameString)
    {
        return GameValue(false);
    }
    const RString what = (GameStringType)args[0];
    const float v = args.Size() >= 2 ? ArgValue(args[1]) : 0.0f;
    OccupierForces& of = OccupierForces::Instance();
    if (stricmp(what, "pool") == 0)
    {
        of.SetPool((int)(v < 0 ? 0 : v + 0.5f));
    }
    else if (stricmp(what, "refillHours") == 0)
    {
        of.TuningMutable().refillHours = ClampF(v, 0.01f, 48.0f);
        LOG_INFO(Core, "OccupierForces: refill forced to {} h", (double)of.Tuning().refillHours);
    }
    else if (stricmp(what, "wave") == 0)
    {
        of.RequestWave();
    }
    else
    {
        return GameValue(false);
    }
    return GameValue(true);
}

INIT_MODULE(GuerrillaOccupierForces, 3)
{
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierForces", GmOccupierForces));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesTune", GmForcesTune, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesForce", GmForcesForce, GameArray));
}
