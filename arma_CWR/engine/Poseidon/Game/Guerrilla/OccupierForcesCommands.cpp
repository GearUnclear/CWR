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
//                                  2026-10-16: [30] the re-armed zones
//                                  (6.3: rested with fighters reported,
//                                  heat held at forcesRearmHeat).
//   gmOccupierRepels            -> the repel ladder (OCCUPIER-FORCES.md 6.3,
//                                  2026-10-16): one row per zone the
//                                  director remembers, [zone, repels,
//                                  lastPackage, lastSource, contained];
//                                  lastPackage "FOOT" / "TRUCK" / ...,
//                                  lastSource the base name ("" unknown).
//   gmOccupierLedger            -> the ledger and the band's input
//                                  (OCCUPIER-FORCES.md 5.5, 5.6, 7.1,
//                                  2026-10-16): [bandStrength, committedAir,
//                                  lossesInWindow, repaidInWindow, banked,
//                                  dropped, stoodDown, [[zone, bodies]...]
//                                  the wave surge, [[zone, package, what,
//                                  units]...] the stored spare hulls,
//                                  surgeOpen, poolCeiling, bankHulls].
//                                  bandStrength is what the band is stepped
//                                  on: min(pool + committedAir, max(poolStart,
//                                  pool) - (lossesInWindow - repaidInWindow)).
//   gmOccupierMassing           -> [target, ticks, forcesMassingMax]: the
//                                  candidate the director is massing for
//                                  ("" none; "@HQ" the HQ) and the director
//                                  ticks it has held its token.
//   gmOccupierReserve           -> [floorUnits, room, budget, [[zone, what,
//                                  plan, package, price, pool, best, line]
//                                  ...]]: the reserve floor (6.3,
//                                  2026-10-17): forcesPoolFloor x poolStart,
//                                  the most a floored launch may cost now
//                                  (the pool kept at the floor), what a
//                                  floored plan is chosen on (the same at
//                                  max(pool, poolStart)), and the targets
//                                  whose plan waits for it (the pool when
//                                  its line was written, the line itself).
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
//                                  (1..86400 s); 2026-10-15: "opsMaxStrong"
//                                  (0..6), "spareCap" (0/1),
//                                  "spareCapFloor" (1..12), "slowEta"
//                                  (0..86400 s), "slowCostShare" (1..5),
//                                  "airEta" (0..86400 s); 2026-10-16:
//                                  "roadFactor" (1..5), "rearmHeat" (0..100);
//                                  phase 4: "hqAssaultMin"
//                                  (1..12), "waveDwell" (1..10 ticks),
//                                  "waveFinal" (0/1), "waveShare" (0..2),
//                                  "waveMin" (0..200), "waveMax" (0..400),
//                                  "waveLead" / "waveLeadFinal" (0..7200 s),
//                                  "waveRetakeShare" (0..1), "waveOps"
//                                  (0..8), "waveCluster" (100..20000 m),
//                                  "waveSurge" (0..48); "log" (0/1, the
//                                  forcesLog director log toggle,
//                                  DirectorLog.hpp); the repel ladder
//                                  (6.3, 2026-10-16; constants, no config
//                                  key): "escalateLethality" (1..3),
//                                  "escalateInfantry" (0..4),
//                                  "approachRepels" / "containRepels"
//                                  (0..9, 0 = never), "massingMax"
//                                  (0..20 ticks), "restJitter" (0..0.5),
//                                  "adjacentTown" (0..5000 m); the ledger
//                                  (5.5, 5.6, 7.1, 2026-10-16): "bankHulls"
//                                  (0/1, the forcesBankHulls key),
//                                  "surgeDecayHours" (0.001..24 h),
//                                  "surgeShare" (0..2), "lossWindowHours"
//                                  (0.01..24 h; constants, no config key);
//                                  the lever round (16): "urgentExit"
//                                  (0..100), "strikeMinFighters" (0..12),
//                                  "alarmMin" (1..12), and the eight
//                                  levers "cadence" (60..7200 s),
//                                  "strength" (0.25..4, at activation
//                                  only), "recovery" (0.5..48 h), "reach"
//                                  (0..86400 s), "aggression" (0.25..4),
//                                  "waves" (0..3), "minimums" (1..10),
//                                  "concurrency" (2..6), plus "difficulty"
//                                  (0..3, every lever at that preset): a
//                                  lever re-derives every field it folds
//                                  (CounterAttack's maxLive,
//                                  forcesLiveHullsMax and firstTownChance
//                                  included), overwriting earlier field
//                                  values.  Returns true when the key is
//                                  known.
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
//                                                the flag (gmLegacyQrf);
//                                  "loss"        ["loss", n] books n units
//                                  lost now (the band's loss window, 5.6;
//                                  the pool does not move);
//                                  "repels"      ["repels", zoneIndex, n]
//                                                makes the director remember
//                                                n repels on the zone (0
//                                                forgets; containRepels or
//                                                more contains it, no surge;
//                                                false for an unknown zone).
//                                  Returns true when what is known.

#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp> // GM_DIRECTOR_INFO
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
    // the re-armed zones (6.3, 2026-10-16)
    GameValue rearmValue = state->CreateGameValue(GameArray);
    GameArrayType& rearm = rearmValue;
    for (int i = 0; i < of.RearmedCount(); i++)
    {
        rearm.Add(GameStringType(of.RearmedName(i)));
    }
    row.Add(rearmValue);
    return value;
}

namespace Poseidon::Guerrilla
{
// the "repels" lever's body (gmForcesForce, OccupierForces.hpp ForceRepels):
// kept with the command surface it serves
void OccupierForces::ForceRepels(const char* zone, int count)
{
    if (!zone || !*zone)
    {
        return;
    }
    int r = FindRepel(zone);
    if (count <= 0)
    {
        if (r >= 0)
        {
            _repels.Delete(r);
        }
        return;
    }
    if (r < 0)
    {
        RepelRow row;
        row.zone = zone;
        _repels.Add(row);
        r = _repels.Size() - 1;
    }
    _repels[r].count = count;
    _repels[r].contained = _tuning.containRepels > 0 && count >= _tuning.containRepels;
    GM_DIRECTOR_INFO(Core, "OccupierForces: {} forced to remember {} repel(s){}", zone, count,
                     _repels[r].contained ? ", contained" : "");
}
} // namespace Poseidon::Guerrilla

static GameValue GmOccupierRepels(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < of.RepelRows(); i++)
    {
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(5);
        row[0] = GameStringType(of.RepelZone(i));
        row[1] = (float)of.RepelCount(i);
        row[2] = GameStringType(ForcePackages::PackageName(of.RepelPackage(i)));
        row[3] = GameStringType(of.RepelSource(i));
        row[4] = GameValue(of.RepelContained(i));
        rows.Add(rowValue);
    }
    return value;
}

static GameValue GmOccupierMassing(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& row = value;
    row.Resize(3);
    row[0] = GameStringType(of.MassingTarget());
    row[1] = (float)of.MassingTicks();
    row[2] = (float)of.Tuning().massingMax;
    return value;
}

static GameValue GmOccupierReserve(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    out.Resize(4);
    out[0] = of.ReserveFloorUnits();
    out[1] = (float)of.ReserveRoom();
    out[2] = (float)of.ReserveBudget();
    GameValue rowsValue = state->CreateGameValue(GameArray);
    GameArrayType& rows = rowsValue;
    for (int i = 0; i < of.ReserveWaitRows(); i++)
    {
        const OccupierForces::ReserveWaitRow* w = of.ReserveWait(i);
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(8);
        row[0] = GameStringType(w->zone);
        row[1] = GameStringType(w->what);
        row[2] = GameStringType(w->plan);
        row[3] = GameStringType(ForcePackages::PackageName(w->package));
        row[4] = (float)w->price;
        row[5] = (float)w->pool;
        row[6] = GameValue(w->best);
        row[7] = GameStringType(w->line);
        rows.Add(rowValue);
    }
    out[3] = rowsValue;
    return value;
}

static GameValue GmLegacyQrf(const GameState* /*state*/)
{
    return GameValue(OccupierForces::Instance().LegacyQrf());
}

static GameValue GmOccupierLedger(const GameState* state)
{
    const OccupierForces& of = OccupierForces::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& row = value;
    row.Add(GameValue((float)of.BandStrength()));
    row.Add(GameValue((float)of.CommittedAir()));
    row.Add(GameValue((float)of.LossesInWindow()));
    row.Add(GameValue((float)of.RepaidInWindow()));
    row.Add(GameValue((float)of.Banked()));
    row.Add(GameValue((float)of.Dropped()));
    row.Add(GameValue((float)of.StoodDown()));
    GameValue surgeValue = state->CreateGameValue(GameArray);
    GameArrayType& surge = surgeValue;
    for (int i = 0; i < of.SurgeRows(); i++)
    {
        GameValue pairValue = state->CreateGameValue(GameArray);
        GameArrayType& pair = pairValue;
        pair.Resize(2);
        pair[0] = GameStringType(of.SurgeRowZone(i));
        pair[1] = (float)of.SurgeRowBodies(i);
        surge.Add(pairValue);
    }
    row.Add(surgeValue);
    GameValue hullsValue = state->CreateGameValue(GameArray);
    GameArrayType& hulls = hullsValue;
    for (int i = 0; i < of.StoredHullRows(); i++)
    {
        GameValue hValue = state->CreateGameValue(GameArray);
        GameArrayType& h = hValue;
        h.Resize(4);
        h[0] = GameStringType(of.StoredHullZone(i));
        h[1] = GameStringType(ForcePackages::PackageName(of.StoredHullPackage(i)));
        h[2] = GameStringType(of.StoredHullWhat(i));
        h[3] = (float)of.StoredHullUnits(i);
        hulls.Add(hValue);
    }
    row.Add(hullsValue);
    row.Add(GameValue(of.SurgeOpen()));
    row.Add(GameValue((float)of.PoolCeiling()));
    row.Add(GameValue(of.Tuning().bankHulls));
    return value;
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
    else if (stricmp(key, "opsMaxStrong") == 0)
    {
        t.opsMaxStrong = (int)ClampF(v, 0, 6);
    }
    else if (stricmp(key, "spareCap") == 0)
    {
        t.spareCap = v != 0;
    }
    else if (stricmp(key, "spareCapFloor") == 0)
    {
        t.spareCapFloor = (int)ClampF(v, 1, 12);
    }
    else if (stricmp(key, "slowEta") == 0)
    {
        t.slowEta = ClampF(v, 0, 86400);
    }
    else if (stricmp(key, "slowCostShare") == 0)
    {
        t.slowCostShare = ClampF(v, 1, 5);
    }
    else if (stricmp(key, "airEta") == 0)
    {
        t.airEta = ClampF(v, 0, 86400);
    }
    else if (stricmp(key, "roadFactor") == 0)
    {
        t.roadFactor = ClampF(v, 1, 5);
        ForcePackages::SetRoadFactor(t.roadFactor);
    }
    else if (stricmp(key, "rearmHeat") == 0)
    {
        t.rearmHeat = ClampF(v, 0, 100);
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
    else if (stricmp(key, "log") == 0)
    {
        t.log = v != 0;
    }
    else if (stricmp(key, "escalateLethality") == 0)
    {
        t.escalateLethality = ClampF(v, 1, 3);
    }
    else if (stricmp(key, "escalateInfantry") == 0)
    {
        t.escalateInfantry = ClampF(v, 0, 4);
    }
    else if (stricmp(key, "approachRepels") == 0)
    {
        t.approachRepels = (int)ClampF(v, 0, 9);
    }
    else if (stricmp(key, "containRepels") == 0)
    {
        t.containRepels = (int)ClampF(v, 0, 9);
    }
    else if (stricmp(key, "massingMax") == 0)
    {
        t.massingMax = (int)ClampF(v, 0, 20);
    }
    else if (stricmp(key, "restJitter") == 0)
    {
        t.restJitter = ClampF(v, 0, 0.5f);
    }
    else if (stricmp(key, "adjacentTown") == 0)
    {
        t.adjacentTown = ClampF(v, 0, 5000);
    }
    else if (stricmp(key, "bankHulls") == 0)
    {
        t.bankHulls = v != 0;
    }
    else if (stricmp(key, "surgeDecayHours") == 0)
    {
        t.surgeDecayHours = ClampF(v, 0.001f, 24.0f);
    }
    else if (stricmp(key, "surgeShare") == 0)
    {
        t.surgeShare = ClampF(v, 0, 2);
    }
    else if (stricmp(key, "lossWindowHours") == 0)
    {
        t.lossWindowHours = ClampF(v, 0.01f, 24.0f);
    }
    else if (stricmp(key, "urgentExit") == 0)
    {
        t.urgentExit = ClampF(v, 0, 100);
    }
    else if (stricmp(key, "strikeMinFighters") == 0)
    {
        t.strikeMinFighters = (int)ClampF(v, 0, 12);
    }
    else if (stricmp(key, "alarmMin") == 0)
    {
        t.alarmMin = (int)ClampF(v, 1, 12);
    }
    else if (t.levers.Set(key, v))
    {
        // one of the eight levers or "difficulty": re-derive every field it
        // folds (overwriting earlier field levers), CounterAttack's too.
        // strength sizes the pool at activation only; heatCapSpike is read
        // at mission load only
        const OccupierForcesLevers levers = t.levers;
        levers.Derive(t);
        levers.Derive(CounterAttack::Instance().TuningMutable());
    }
    else
    {
        return GameValue(false);
    }
    GM_DIRECTOR_INFO(Core, "OccupierForces: gmForcesTune {} = {}", (const char*)key, (double)v);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: refill forced to {} h", (double)of.Tuning().refillHours);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: {} forced to rest {:.0f} s", (const char*)z->name, (double)seconds);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: legacySave forced {}", v != 0 ? "on" : "off");
    }
    else if (stricmp(what, "loss") == 0)
    {
        // ["loss", n]: n units booked lost now (the band's loss window)
        if (!of.Active() || v < 1)
        {
            return GameValue(false);
        }
        of.NoteLoss((int)(v + 0.5f), "forced loss");
        GM_DIRECTOR_INFO(Core, "OccupierForces: {} unit(s) forced lost, band strength {}", (int)(v + 0.5f),
                         of.BandStrength());
    }
    else if (stricmp(what, "repels") == 0)
    {
        // ["repels", zoneIndex, n]: the repel ladder remembers n repels
        const ZoneRecord* z = ZoneRegistry::Instance().GetZone((int)v);
        if (!z)
        {
            return GameValue(false);
        }
        of.ForceRepels(z->name, args.Size() >= 3 ? (int)(ArgValue(args[2]) + 0.5f) : 0);
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
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierRepels", GmOccupierRepels));
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierMassing", GmOccupierMassing));
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierReserve", GmOccupierReserve));
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierLedger", GmOccupierLedger));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesTune", GmForcesTune, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmForcesForce", GmForcesForce, GameArray));
}
