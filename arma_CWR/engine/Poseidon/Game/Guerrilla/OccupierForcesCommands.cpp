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
//                                  Five director slots follow the ten
//                                  documented ones (phase 3): [10]
//                                  townsEver (the distinct towns taken),
//                                  [11] urgent (the latched zone names),
//                                  [12] cooldowns ([[zone, seconds left]]),
//                                  [13] the operation counter, [14] the
//                                  token clock (s toward the next token).
//                                  Phase 4 (waves, 7.1): [15] a wave is
//                                  inbound, [16] its clock (s to landing),
//                                  [17] its bodies, [18] waves fired so far
//                                  (the "Wave N" count), [19] the final
//                                  stand fired, [20] the surge zone ("" none),
//                                  [21] the surge bodies, [22] waves landed,
//                                  [23] the occupier war level.
//                                  Phase 6: [24] towns struck, [25] strikes
//                                  applied (any zone), [26] alarm answers
//                                  launched, [27] legacySave, [28]
//                                  gmLegacyQrf.  [29] the seeded towns the
//                                  settlement probe kept out of K (5.1;
//                                  their names are in excludedTowns).
//   gmLegacyQrf                 -> true while qrf.sqs fields its script QRF
//                                  (D20): the key off, the service sized
//                                  off, or a legacy save (a block older than
//                                  phase 6, or derived from a save without
//                                  the block).  False with the native alarm
//                                  answers in charge.
//   gmWarLevelOccupier          -> the occupier-side war level (7.1): the
//                                  territory level (gmWarLevel) plus
//                                  forcesWaveWarLevel per landed wave, capped
//                                  at 6; the territory level with the key off.
//                                  qrf.sqs reads it; resistance readers keep
//                                  gmWarLevel.
//   gmOccupierOps               -> one row per pending or active operation
//                                  (every CounterAttack record, legacy ones
//                                  included): [id, kind, origin, package,
//                                  source, target, phase, bodies, hulls,
//                                  eta s, birthReason].  id is the director's
//                                  operation id (0 for a legacy record);
//                                  kind "RETAKE" / "BASE" / "HQ_ASSAULT" /
//                                  "ALARM" / "REINFORCE" / "STRIKE"; origin
//                                  "LEGACY" / "DIRECTOR" / "PUSHOUT" /
//                                  "WAVE" / "ALARM"; package "FOOT" /
//                                  "TRUCK" / ... (a hull package CounterAttack
//                                  sent on foot at launch shows "FOOT");
//                                  source and target zone names; phase
//                                  "SCHEDULED" / "TRAVEL" / "HOLDING" /
//                                  "LIVE"; bodies the infantry (the planned
//                                  size while scheduled); hulls the live
//                                  hulls, the carrier's and the escort's (0
//                                  to 2); eta the seconds left on the
//                                  abstract clock (scheduled or travelling;
//                                  0 otherwise); birthReason the spawn
//                                  reason ("base", "route", ...; "" while
//                                  unborn).  Two slots follow the eleven
//                                  (phase 5): [11] the carrier's hull class
//                                  ("" on foot), [12] the escort's hull
//                                  class ("" with no escort).
//   gmOccupierIntel             -> the director's sightings (6.1, D9): one
//                                  row per zone with a report, [zone, count,
//                                  ageSeconds, [positions...]]; zone "" for
//                                  reports outside every zone; positions in
//                                  getPos order [x, y, z] as the groups
//                                  reported them.
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
//                                  exists); the director (phase 3):
//                                  "wakeTowns" (1..99), "opsMax" (0..6),
//                                  "heatFull" (1..100), "urgentHeat"
//                                  (1..100), "townCooldown" (0..86400 s),
//                                  "pushoutQuiet" (0..86400 s), "retakeMin"
//                                  (1..12), "lethalityMargin" (0..3),
//                                  "urgentCostShare" (1..3), "intelDecay"
//                                  (1..86400 s); phase 4: "hqAssaultMin"
//                                  (1..12), "waveDwell" (1..10 ticks),
//                                  "waveFinal" (0/1), "waveShare" (0..2),
//                                  "waveMin" (0..200), "waveMax" (0..400),
//                                  "waveLead" / "waveLeadFinal" (0..7200 s),
//                                  "waveRetakeShare" (0..1), "waveOps"
//                                  (0..8), "waveCluster" (100..20000 m),
//                                  "waveSurge" (0..48).  Returns true when
//                                  the key is known.
//   gmForcesForce [what, value] test lever; what:
//                                  "pool"        sets the pool (>= 0) and
//                                                re-steps the band (no-op
//                                                while inert);
//                                  "refillHours" sets the refill duration
//                                                (0.01..48 h);
//                                  "wave"        ["wave", n] fires wave n
//                                                now (n omitted or 0: the
//                                                next threshold; n beyond the
//                                                thresholds: the final
//                                                stand); its thresholds are
//                                                marked fired; a wave already
//                                                inbound absorbs it; parked
//                                                (no effect) while inert;
//                                  "cooldown"    ["cooldown", zoneIndex,
//                                                seconds] rests a zone as a
//                                                repel would (false for an
//                                                unknown zone);
//                                  "strike"      ["strike", zoneIndex] buys
//                                                and launches a strike on
//                                                the zone now (the package
//                                                choice and the pool apply,
//                                                the bought-when rule does
//                                                not); false when refused;
//                                  "strikeEffects" ["strikeEffects",
//                                                zoneIndex] applies a
//                                                strike's effects at once
//                                                (7.3; no airframe);
//                                  "alarm"       ["alarm", zoneIndex]
//                                                answers the zone as an
//                                                alarm now (RED or not);
//                                  "legacySave"  ["legacySave", 0/1] sets
//                                                the flag (gmLegacyQrf).
//                                  Returns true when what is known.

#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Core/Global.hpp> // Glob.time (strikeEffects)
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
void EnsureOccupierForcesCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static float ClampF(float v, float lo, float hi)
{
    // L14: NaN fails every compare; it takes the low end, never the knob
    if (!(v == v))
    {
        return lo;
    }
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
    // the director (phase 3), after the ten documented slots
    GameValue everValue = state->CreateGameValue(GameArray);
    GameArrayType& ever = everValue;
    for (int i = 0; i < of.TownsEverCount(); i++)
    {
        ever.Add(GameStringType(of.TownsEverName(i)));
    }
    row.Add(everValue);
    GameValue urgentValue = state->CreateGameValue(GameArray);
    GameArrayType& urgent = urgentValue;
    for (int i = 0; i < of.UrgentCount(); i++)
    {
        urgent.Add(GameStringType(of.UrgentName(i)));
    }
    row.Add(urgentValue);
    GameValue coolValue = state->CreateGameValue(GameArray);
    GameArrayType& cool = coolValue;
    for (int i = 0; i < of.CooldownCount(); i++)
    {
        GameValue pairValue = state->CreateGameValue(GameArray);
        GameArrayType& pair = pairValue;
        pair.Resize(2);
        pair[0] = GameStringType(of.CooldownZone(i));
        pair[1] = of.CooldownLeft(i);
        cool.Add(pairValue);
    }
    row.Add(coolValue);
    row.Add(GameValue((float)of.OpCounter()));
    row.Add(GameValue(of.TokenClock()));
    // the waves (phase 4)
    row.Add(GameValue(of.WaveInbound()));
    row.Add(GameValue(of.WaveClock()));
    row.Add(GameValue((float)of.WaveBodies()));
    row.Add(GameValue((float)of.WaveCount()));
    row.Add(GameValue(of.FinalFired()));
    row.Add(GameValue(GameStringType(of.SurgeZone())));
    row.Add(GameValue((float)of.SurgeBodies()));
    row.Add(GameValue((float)of.WavesLanded()));
    row.Add(GameValue(of.OccupierWarLevel()));
    // phase 6
    row.Add(GameValue((float)of.TownStrikes()));
    row.Add(GameValue((float)of.StrikesApplied()));
    row.Add(GameValue((float)of.AlarmAnswers()));
    row.Add(GameValue(of.LegacySave()));
    row.Add(GameValue(of.LegacyQrf()));
    // the settlement probe (5.1)
    row.Add(GameValue((float)of.ProbedOutCount()));
    return value;
}

static GameValue GmLegacyQrf(const GameState* /*state*/)
{
    return GameValue(OccupierForces::Instance().LegacyQrf());
}

static GameValue GmWarLevelOccupier(const GameState* /*state*/)
{
    return GameValue(OccupierForces::Instance().OccupierWarLevel());
}

static GameValue GmOccupierOps(const GameState* state)
{
    const CounterAttack& ca = CounterAttack::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < ca.Count(); i++)
    {
        const CounterAttack::Record* r = ca.Get(i);
        if (!r)
        {
            continue;
        }
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(13);
        row[0] = (float)r->opId;
        row[1] = GameStringType(ForcePackages::KindName(r->kind));
        row[2] = GameStringType(ForcePackages::OriginName(r->origin));
        row[3] = GameStringType(ForcePackages::PackageName(r->package));
        row[4] = GameStringType(r->source);
        row[5] = GameStringType(r->target);
        row[6] = GameStringType(CounterAttack::PhaseName(r->phase));
        row[7] = (float)(r->phase == CAScheduled ? r->size : r->strength);
        row[8] = (float)(CounterAttack::LiveHullOf(*r) + CounterAttack::LiveEscortOf(*r));
        row[9] = (r->phase == CAScheduled || r->phase == CATravel) && r->clock > 0 ? r->clock : 0.0f;
        row[10] = GameStringType(CounterAttack::SpawnReasonName(r->spawnReason));
        row[11] = GameStringType(r->hullClass);
        row[12] = GameStringType(r->escortClass);
        rows.Add(rowValue);
    }
    return value;
}

static GameValue GmOccupierIntel(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < of.SightingRows(); i++)
    {
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(4);
        row[0] = GameStringType(of.SightingZone(i));
        row[1] = (float)of.SightingCount(i);
        row[2] = of.SightingAge(i);
        GameValue posValue = state->CreateGameValue(GameArray);
        GameArrayType& positions = posValue;
        const AutoArray<Vector3>* p = of.SightingPositions(i);
        for (int k = 0; p && k < p->Size(); k++)
        {
            // script order [easting, northing, elevation]
            GameValue xyzValue = state->CreateGameValue(GameArray);
            GameArrayType& xyz = xyzValue;
            xyz.Resize(3);
            xyz[0] = (*p)[k].X();
            xyz[1] = (*p)[k].Z();
            xyz[2] = (*p)[k].Y();
            positions.Add(xyzValue);
        }
        row[3] = posValue;
        rows.Add(rowValue);
    }
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
    else if (stricmp(key, "wakeTowns") == 0)
    {
        t.wakeTowns = (int)ClampF(v, 1, 99);
    }
    else if (stricmp(key, "opsMax") == 0)
    {
        t.opsMax = (int)ClampF(v, 0, 6);
    }
    else if (stricmp(key, "heatFull") == 0)
    {
        t.heatFull = ClampF(v, 1, 100);
    }
    else if (stricmp(key, "urgentHeat") == 0)
    {
        t.urgentHeat = ClampF(v, 1, 100);
    }
    else if (stricmp(key, "townCooldown") == 0)
    {
        t.townCooldown = ClampF(v, 0, 86400);
    }
    else if (stricmp(key, "pushoutQuiet") == 0)
    {
        t.pushoutQuiet = ClampF(v, 0, 86400);
    }
    else if (stricmp(key, "retakeMin") == 0)
    {
        t.retakeMin = (int)ClampF(v, 1, 12);
    }
    else if (stricmp(key, "lethalityMargin") == 0)
    {
        t.lethalityMargin = ClampF(v, 0, 3);
    }
    else if (stricmp(key, "urgentCostShare") == 0)
    {
        t.urgentCostShare = ClampF(v, 1, 3);
    }
    else if (stricmp(key, "intelDecay") == 0)
    {
        t.intelDecay = ClampF(v, 1, 86400);
    }
    else if (stricmp(key, "hqAssaultMin") == 0)
    {
        t.hqAssaultMin = (int)ClampF(v, 1, 12);
    }
    else if (stricmp(key, "waveDwell") == 0)
    {
        t.waveDwell = (int)ClampF(v, 1, 10);
    }
    else if (stricmp(key, "waveFinal") == 0)
    {
        t.waveFinal = v != 0;
    }
    else if (stricmp(key, "waveShare") == 0)
    {
        t.waveShare = ClampF(v, 0, 2);
    }
    else if (stricmp(key, "waveMin") == 0)
    {
        t.waveMin = (int)ClampF(v, 0, 200);
    }
    else if (stricmp(key, "waveMax") == 0)
    {
        t.waveMax = (int)ClampF(v, 0, 400);
    }
    else if (stricmp(key, "waveLead") == 0)
    {
        t.waveLead = ClampF(v, 0, 7200);
    }
    else if (stricmp(key, "waveLeadFinal") == 0)
    {
        t.waveLeadFinal = ClampF(v, 0, 7200);
    }
    else if (stricmp(key, "waveRetakeShare") == 0)
    {
        t.waveRetakeShare = ClampF(v, 0, 1);
    }
    else if (stricmp(key, "waveOps") == 0)
    {
        t.waveOps = (int)ClampF(v, 0, 8);
    }
    else if (stricmp(key, "waveCluster") == 0)
    {
        t.waveCluster = ClampF(v, 100, 20000);
    }
    else if (stricmp(key, "waveSurge") == 0)
    {
        t.waveSurge = (int)ClampF(v, 0, 48);
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
        of.RequestWave((int)(v + 0.5f));
    }
    else if (stricmp(what, "cooldown") == 0)
    {
        // ["cooldown", zoneIndex, seconds]: rest a zone (tests)
        const float seconds = args.Size() >= 3 ? ArgValue(args[2]) : 0.0f;
        const ZoneRecord* z = ZoneRegistry::Instance().GetZone((int)v);
        if (!z || seconds <= 0)
        {
            return GameValue(false);
        }
        of.ForceCooldown(z->name, seconds);
        LOG_INFO(Core, "OccupierForces: {} forced to rest {:.0f} s", (const char*)z->name, (double)seconds);
    }
    else if (stricmp(what, "strike") == 0)
    {
        return GameValue(of.ForceStrike((int)v));
    }
    else if (stricmp(what, "strikeEffects") == 0)
    {
        ZoneRegistry& registry = ZoneRegistry::Instance();
        if (!registry.GetZone((int)v))
        {
            return GameValue(false);
        }
        of.SetMissionNow(Glob.time - Foundation::Time(0));
        of.ApplyStrikeEffects(registry, (int)v, VZero, of.MissionNow(), 0);
    }
    else if (stricmp(what, "alarm") == 0)
    {
        return GameValue(of.ForceAlarm((int)v));
    }
    else if (stricmp(what, "legacySave") == 0)
    {
        of.SetLegacySave(v != 0);
        LOG_INFO(Core, "OccupierForces: legacySave forced {}", v != 0 ? "on" : "off");
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
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierOps", GmOccupierOps));
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierIntel", GmOccupierIntel));
    GGameState.NewNularOp(GameNular(GameScalar, "gmWarLevelOccupier", GmWarLevelOccupier));
    GGameState.NewNularOp(GameNular(GameBool, "gmLegacyQrf", GmLegacyQrf));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesTune", GmForcesTune, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesForce", GmForcesForce, GameArray));
}
