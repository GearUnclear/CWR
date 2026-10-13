#pragma once

// Guerrilla Mode occupier forces: the strategic pool and the director
// (guerrilla-mode/OCCUPIER-FORCES.md, phases 2 and 3).  This header is the
// code-side summary of the design's sections 5 (the pool), 6 (the director),
// 7.1 (control) and 11 (persistence).  Waves and the HQ are later phases.
// The service owns the ledger every operation spends from and decides WHAT
// to send; CounterAttack moves it (CounterAttackOrders.hpp is the contract).
//
// KEY.  occupierForces (CfgGuerrillaZones, 0/1, default 0).  With it 0 the
// service loads its tuning and stays inert: no pool, HasState() false unless
// a saved block was loaded, gmOccupierForces reports band "OFF", and the
// pool seams (Draw, Deposit, Spend) pass everything through, so a caller
// wired to them behaves exactly as before the service existed.
//
// SIZING (5.1).  On the first tick with an active ZoneRegistry the service
// takes its own roster: every zone of a military type (CampaignEnd::
// IsMilitaryType) whose AUTHORED owner is the occupier (CampaignEnd::
// IsAuthoredOccupier), independent of campaignEnd.  R is the roster's summed
// garrisonMax.  K is the administered, non-exempt CITY zones; a SEEDED town
// counts only when it passes the settlement probe (an authored town always
// counts).  ZoneRecord carries no seeded flag yet, so the engine path passes
// no seeded table and every administered non-exempt CITY counts (one INFO
// line per town).  G is the K towns' summed garrisonMax.
// poolStart = max(forcesPoolMin, R + forcesPoolPerTown * K).  An empty
// roster switches the service off for the session (one INFO line).
//
// REFILL (5.2, D8, D15).  Each director tick (forcesDirectorInterval) the
// pool accrues (poolStart + R + G) / forcesPoolRefillHours per hour, scaled
// by the standing factor 0.75 + 0.5 * meanStanding / 100 (1 at the default
// standing 50), into a fractional accumulator.  Whole units enter the pool
// up to poolStart; waves (LandWave) may push it above.  D2 garrison regrowth
// draws first through Draw(): it takes from the pool between director
// ticks, so an exhausted occupier fills its garrisons before its pool.
//
// BANDS (5.6).  Shares of poolStart with hysteresis: STRONG enter > 66 %,
// leave < 60 %; THIN enter < 33 %, leave > 40 %; SPENT enter at 0, leave
// > 10 %; STRETCHED between.  The band is re-stepped on every pool change.
// D2 keeps drawing in SPENT; the pool then sits near 0 while garrisons are
// short and the band leaves SPENT once, when the pool passes 10 %.
// CanOperate() is false in SPENT and in THIN below forcesPoolFloor.
//
// CONTROL (7.1).  C = forcesControlMilWeight * M + (1 - weight) * T: M the
// share of the roster's garrisonMax the occupier still owns, T the share of
// the K towns still administered.  Computed each director tick.
//
// WAR LEVEL SPLIT (7.1).  OccupierWarLevel() = the territory war level
// (ReadWarLevel) + forcesWaveWarLevel per landed wave, capped at
// MaxWarLevel.  No reader uses it yet (phase 4 switches the occupier-side
// readers; the resistance keeps the territory level).
//
// PERSISTENCE (11).  Block GuerrillaOccupierForces, gated on HasState().  A
// save without the block loaded with the key on is derived from the zone
// state (DeriveFromExistingSave): pool = poolStart * C, wave thresholds
// already crossed marked fired, the towns the resistance holds seeded into
// the distinct-town set, tokens 0, legacySave set.
//
// THE DIRECTOR (6, phase 3).  It knows only what its men saw (D9): flips and
// owner changes, zone heat, alert states, its sightings and its own
// operations' events.  It never reads the player's position, the true
// census (ZoneTickInputs::guerCount, ZoneRecord::lastGuer) or AlertMachine's
// player-position fallback.
//   INTEL (6.1).  Every alertInterval (5 s) the engine scan walks every
//   occupier group with a living member and records each resistance-side
//   unit it holds as a known target (Target::IsKnown): the zone whose
//   zoneArea holds the REPORTED position (-1 none), the reported position
//   and the time.  A zone's row is replaced by its latest scan and kept
//   when no group reports it any more (a destroyed group's last report
//   stays), until forcesIntelDecay (1800 s).  knownFighters(zone) is the
//   row's distinct count.
//   POSTURE (6.2, invariant W).  CounterAttack reports every non-exempt town
//   flip (CounterAttackListener::OnTownFlipped).  The distinct names go into
//   _townsEver; the flip that brings it to forcesWakeTowns (2) latches AWAKE
//   and fills the tokens.  The director takes a flip only when Active and
//   AWAKE; while DORMANT CounterAttack answers with the legacy Decide.
//   CANDIDATES (6.3).  A held town (not the HQ's, not on cooldown, no record
//   on it) is a candidate while heat / forcesHeatFull >= u, u =
//   Roll01(MakeSeed(town, flipDay, flipMinute, 0), 7).  Bases (D16) are
//   phase 4: computed while STRONG, logged once, never ordered here.
//   URGENCY.  Latched by a YELLOW / RED alert edge on a held town (since its
//   flip) or heat >= forcesUrgentHeat; released when heat < UrgentExitHeat.
//   TOKENS.  0..forcesTokensMax, full at the wake, one per
//   forcesTokenInterval[band].  Each director tick with a token, a
//   candidate, CanOperate() and fewer than forcesOpsMax BORN operations
//   launches ONE operation on the hottest candidate (the next one when the
//   hottest cannot be served).  A repelled operation puts its target on
//   forcesTownCooldown with +15 heat unless a RED edge already paid it; a
//   cancelled-unseen one gets the cooldown.
//   PUSH-OUT (6.4).  STRONG, a token and no heat candidate: the held town
//   nearest an occupier base with no sighting for forcesPushoutQuiet, with
//   the minimal package.
//   PACKAGES (6.5).  ForcePackages: the cheapest lethal package, the fastest
//   within forcesUrgentCostShare when urgent; phase 3 offers Foot and Truck.
//   The pool pays hull + crew (Spend before LaunchFunded, Refund when it is
//   refused); the source base pays the infantry from its spare.
//   WON.  Nothing is launched once CampaignEnd has latched the win.
//
// Script surface (OccupierForcesCommands.cpp): gmOccupierForces,
// gmOccupierOps, gmOccupierIntel, gmForcesTune, gmForcesForce.  Every INFO
// line starts "OccupierForces:"; a director action "OccupierForces: [op N]".

#include <Poseidon/Game/Guerrilla/CounterAttackOrders.hpp>
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;
struct SettlementProbe;
struct ZoneRecord;

// pool bands (5.6), in token-interval order (forcesTokenInterval[])
enum OccupierBand
{
    OFBandStrong,
    OFBandStretched,
    OFBandThin,
    OFBandSpent,
    NOccupierBands
};

// director posture (6.2): DORMANT until the second distinct town (phase 3)
enum OccupierPosture
{
    OFDormant,
    OFAwake,
    NOccupierPostures
};

struct OccupierForcesTuning
{
    static constexpr int MaxWaveThresholds = 3;

    bool enabled = false;           // occupierForces (0/1)
    int poolMin = 24;               // forcesPoolMin, 0..200
    int poolPerTown = 2;            // forcesPoolPerTown, 0..8
    float refillHours = 4.0f;       // forcesPoolRefillHours, 0.5..48 (gmForcesTune: 0.01..48)
    float poolFloor = 0.2f;         // forcesPoolFloor, 0..1: THIN operations floor (share of poolStart)
    float directorInterval = 60.0f; // forcesDirectorInterval, 10..600 s (gmForcesTune: 1..600)
    int tokensMax = 2;              // forcesTokensMax, 0..6
    // forcesTokenInterval[] by band (STRONG, STRETCHED, THIN, SPENT), s;
    // 0 = no tokens in that band
    float tokenInterval[NOccupierBands] = {1800.0f, 2700.0f, 3600.0f, 0.0f};
    float controlMilWeight = 0.6f; // forcesControlMilWeight, 0..1
    // forcesWaveThresholds[], descending control shares, at most 3
    float waveThresholds[MaxWaveThresholds] = {0.75f, 0.50f, 0.0f};
    int nWaveThresholds = 2;
    float waveWarLevel = 1.0f; // forcesWaveWarLevel, 0..3: occupier war level per landed wave
    // the director (6, phase 3)
    int wakeTowns = 2;            // forcesWakeTowns, 2..99: distinct towns to wake
    int opsMax = 2;               // forcesOpsMax, 0..6: born operations in flight
    float heatFull = 40.0f;       // forcesHeatFull, 1..100: candidate heat
    float urgentHeat = 50.0f;     // forcesUrgentHeat, 1..100: urgency entry
    float townCooldown = 1800.0f; // forcesTownCooldown, s: after a repel or a cancel
    float pushoutQuiet = 1800.0f; // forcesPushoutQuiet, s: no sighting this long = quiet
    int retakeMin = 4;            // forcesRetakeMin, 1..12: minimum retake bodies
    float lethalityMargin = 1.2f; // forcesLethalityMargin, 1..3
    float urgentCostShare = 1.5f; // forcesUrgentCostShare, 1..3
    float intelDecay = 1800.0f;   // forcesIntelDecay, s: sightings expire
};

// One resistance unit an occupier group reports (the 6.1 scan)
struct SightingReport
{
    int zone = -1;             // the zone whose zoneArea holds pos, -1 none
    unsigned long long id = 0; // the unit (distinct count within one scan)
    Vector3 pos = VZero;       // the REPORTED position (Target::posReported)
};

// What the director may look at on a tick besides the registry's flips,
// owners and heat and its own state (D9).  No player position, no census.
struct DirectorView
{
    AutoArray<int> alert;        // per zone: AlertState (index-aligned; missing = GREEN)
    AutoArray<int> spare;        // per zone: bodies an occupier base can spare (CounterAttack::Spare)
    AutoArray<int> spareStored;  // per zone: ... from its stored reserve (hull packages); missing = spare
    int hqZone = -1;             // the zone the player's HQ stands in: never a target (N4a)
    bool won = false;            // CampaignEnd latched the win: launch nothing (7.4)
    float tierFactor = 1.0f;     // the occupier infantry tier's lethality factor
    float defenderWeight = 3.0f; // counterAttackDefenderWeight
};

// The director's hands: CounterAttack::LaunchFunded and the two queries it
// needs (engine wrapper in OccupierForces.cpp; unit tests fake it).
class DirectorLauncher
{
  public:
    virtual ~DirectorLauncher() = default;
    // false when the order is refused
    virtual bool Launch(const CounterAttackOrder& order) = 0;
    // funded operations that are live or hold a hull (unborn ones do not count)
    virtual int BornOps() const = 0;
    // a record already targets this zone
    virtual bool HasRecord(int zoneIndex) const = 0;
};

// One planned operation (PlanFor)
struct DirectorAction
{
    bool go = false;
    int target = -1;
    int kind = CAKRetake;
    int origin = CAODirector;
    bool urgent = false;
    float fighters = 0;
    PackageChoice choice;
};

// The first-tick roster and town census (5.1)
struct OccupierForcesSizing
{
    AutoArray<RString> roster;   // military zones with an authored occupier owner
    AutoArray<RString> towns;    // the K towns
    AutoArray<RString> excluded; // CITY zones not counted in K
    int R = 0;                   // the roster's summed garrisonMax
    int G = 0;                   // the K towns' summed garrisonMax
    int poolStart = 0;
    int K() const { return towns.Size(); }
};

class OccupierForces : public SerializeClass, public CounterAttackListener
{
  public:
    OccupierForces() = default;

    static OccupierForces& Instance();

    // the territory war level runs 1..6 (the share-of-zones ladder)
    static constexpr float MaxWarLevel = 6.0f;
    // band thresholds in percent of poolStart (5.6; constants, not knobs).
    // Compared in integers: STRONG while pool * 100 > 66 * poolStart, and so on.
    static constexpr int StrongEnterPct = 66;
    static constexpr int StrongLeavePct = 60;
    static constexpr int ThinEnterPct = 33;
    static constexpr int ThinLeavePct = 40;
    static constexpr int SpentLeavePct = 10;
    // the default occupier standing (7.3); the refill factor is 1 here
    static constexpr float DefaultStanding = 50.0f;
    // shortest gap between two rate-limited INFO lines (s of simulation)
    static constexpr float LogQuiet = 60.0f;
    // the director (6.3): the urgency exit heat, the candidate roll salt,
    // the heat a repel adds when no RED edge paid it, positions kept per
    // sighting row, director operations kept in the ledger
    static constexpr float UrgentExitHeat = 35.0f;
    static constexpr int CandidateSalt = 7;
    static constexpr float RepelHeat = 15.0f;
    static constexpr int MaxSightingPositions = 12;
    static constexpr int MaxOpRows = 32;
    // phase 3 offers only foot and truck packages (phase 5 armour, phase 6 air)
    static constexpr unsigned int OfferedPackages = ForcePackages::Phase3Packages;

    // lifecycle ---------------------------------------------------------------
    void Clear();
    void InitMission(); // Clear + read the CfgGuerrillaZones keys
    void LoadFromParams(const ParamEntry* zonesCfg);
    const OccupierForcesTuning& Tuning() const { return _tuning; }
    OccupierForcesTuning& TuningMutable() { return _tuning; }

    // pure rules (unit-tested) ------------------------------------------------
    // max(forcesPoolMin, R + forcesPoolPerTown * K)
    static int PoolStartOf(const OccupierForcesTuning& t, int R, int K);
    // 0.75 + 0.5 * meanStanding / 100 (standing clamped to 0..100)
    static float StandingFactor(float meanStanding);
    // (poolStart + R + G) / refillHours * StandingFactor(meanStanding)
    static float RefillPerHour(int poolStart, int R, int G, float refillHours, float meanStanding);
    // the band for a pool with no history (activation, a derived save)
    static int InitialBand(int pool, int poolStart);
    // one hysteresis step from prev (a jump may cross several bands)
    static int StepBand(int prev, int pool, int poolStart);
    // false in SPENT, and in THIN below floor * poolStart
    static bool CanOperateIn(int band, int pool, int poolStart, float floor);
    // seconds per token in a band (0 = no tokens)
    static float TokenInterval(const OccupierForcesTuning& t, int band);
    // weight * M + (1 - weight) * T.  M = rosterHeld / rosterTotal (1 for an
    // empty total), T = townsAdministered / townsTotal; with no towns C = M
    static float ControlOf(float rosterHeld, float rosterTotal, int townsAdministered, int townsTotal, float milWeight);
    // how many wave thresholds control is at or below (thresholds descend,
    // so they are crossed in order)
    static int WavesCrossed(const OccupierForcesTuning& t, float control);
    // territory + step * wavesLanded, never above MaxWarLevel (a territory
    // level already above it is kept)
    static float OccupierWarLevelOf(float territory, int wavesLanded, float step);
    // the ledger (5.5): (pool + reserves + pending + opStrength) -
    // (poolStart + R + G + refill + waves - losses); 0 when conserved
    static int ConservationImbalance(int pool, int reserves, int pending, int opStrength, int poolStart, int R, int G,
                                     int refill, int waves, int losses);
    static bool ConservationCheck(int pool, int reserves, int pending, int opStrength, int poolStart, int R, int G,
                                  int refill, int waves, int losses)
    {
        return ConservationImbalance(pool, reserves, pending, opStrength, poolStart, R, G, refill, waves, losses) == 0;
    }
    // the roster, K, G and poolStart of a registry.  seeded (optional, per
    // zone index) marks auto-seeded towns, which count only when the probe
    // (optional) accepts them.  log: one INFO line per town.
    static void Size(const ZoneRegistry& registry, const OccupierForcesTuning& t, OccupierForcesSizing& out,
                     const AutoArray<bool>* seeded = nullptr, const SettlementProbe* probe = nullptr, bool log = false);
    static const char* BandName(int band);
    static const char* PostureName(int posture);

    // the service ---------------------------------------------------------------
    bool Enabled() const { return _tuning.enabled; }
    // sized and running (the key on, a roster found); a loaded block with
    // the key now off is restored but inert
    bool Active() const { return _active && _tuning.enabled; }
    // take the roster and size the pool (the first active tick).  False and
    // off for the session on an empty roster.
    bool Activate(const ZoneRegistry& registry, const AutoArray<bool>* seeded = nullptr,
                  const SettlementProbe* probe = nullptr);
    // one director tick: the refill, control and the band.  No-op while not
    // Active().
    void Tick(const ZoneRegistry& registry, float elapsed);
    // control over the stored roster and towns
    float ComputeControl(const ZoneRegistry& registry) const;

    // the pool seams ------------------------------------------------------------
    // D2 regrowth (5.3): grants min(bodies, pool) and debits it.  An empty
    // pool grants nothing.  Inactive: grants everything (legacy D2).
    int Draw(int bodies);
    // a pool-funded garrison deposit (a reverted town or base, 5.4): as Draw,
    // but SPENT grants nothing (reverts left hollow).  Inactive: everything.
    int Deposit(int bodies, const char* why);
    // an operation's price (5.3): false (nothing debited) when units exceed
    // the pool or !CanOperate().  Inactive: true, nothing debited.
    bool Spend(int units, const char* why);
    // units back into the pool (survivors with no base, surplus, a flipped
    // base's live garrison, partial-spawn failures, hull refunds: 5.5).
    // May exceed poolStart.  Inactive: no-op.
    void Refund(int units, const char* why);
    // diagnostics: bodies the occupier lost for good (the `lost` field)
    void NoteLoss(int units, const char* why);
    // a reinforcement wave lands (the phase-4 seam): bodies into the pool,
    // above poolStart if need be, and the occupier war level rises
    void LandWave(int bodies);

    // queries -------------------------------------------------------------------
    int Pool() const { return _pool; }
    int PoolStart() const { return _poolStart; }
    int RosterStrength() const { return _R; }
    int TownCount() const { return _towns.Size(); }
    int TownGarrisons() const { return _G; }
    int Band() const { return _band; }
    bool CanOperate() const { return CanOperateIn(_band, _pool, _poolStart, _tuning.poolFloor); }
    float Control() const { return _control; }
    int Posture() const { return _posture; }
    int Tokens() const { return _tokens; }
    float TokenClock() const { return _tokenClock; }
    int WavesFired() const { return _wavesFired; }
    int WavesLanded() const { return _wavesLanded; }
    int WaveRequests() const { return _waveRequests; }
    int Lost() const { return _lost; }
    int RefillTotal() const { return _refillTotal; }
    int WavesTotal() const { return _wavesTotal; }
    float RefillAccum() const { return _refillAccum; }
    bool LegacySave() const { return _legacySave; }
    float MeanStanding() const { return _meanStanding; }
    int RosterCount() const { return _roster.Size(); }
    RString RosterName(int i) const { return i >= 0 && i < _roster.Size() ? _roster[i].name : RString(); }
    RString TownName(int i) const { return i >= 0 && i < _towns.Size() ? _towns[i].name : RString(); }
    int ExcludedCount() const { return _excluded.Size(); }
    RString ExcludedName(int i) const { return i >= 0 && i < _excluded.Size() ? _excluded[i].name : RString(); }
    int TownsEverCount() const { return _townsEver.Size(); }
    bool TownEver(const char* name) const;
    // the occupier-side war level (live: ReadWarLevel + landed waves)
    float OccupierWarLevel() const;

    // the director (phase 3) -----------------------------------------------------
    // CounterAttackListener: CounterAttack calls these from its own tick
    bool OnTownFlipped(int zoneIndex, int k) override;
    void OnCounterAttackOp(const CounterAttackOpEvent& ev) override;
    void OnZoneEvents(const AutoArray<ZoneEventRecord>& fired) override;
    // the same with an explicit registry (unit tests)
    bool NoteTownFlipped(const ZoneRegistry& registry, int zoneIndex);
    void NoteOpEvent(ZoneRegistry& registry, const CounterAttackOpEvent& ev);
    void NoteZoneEvents(const ZoneRegistry& registry, const AutoArray<ZoneEventRecord>& fired);
    // one sighting scan (6.1): replaces the rows of the zones it reports
    void IngestSightings(const ZoneRegistry& registry, const AutoArray<SightingReport>& reports);
    // the alert states of one scan: YELLOW / RED edges latch urgency on held
    // towns, a RED edge pays a later repel's heat
    void NoteAlertStates(const ZoneRegistry& registry, const AutoArray<int>& states);
    // the zone whose zoneArea holds pos (nearest centre), -1 none
    static int ZoneOfPosition(const ZoneRegistry& registry, Vector3Par pos);
    // a resistance-held, non-exempt CITY
    static bool IsHeldTown(const ZoneRegistry& registry, const ZoneRecord& z);
    // the candidate roll u of a held zone's current flip (6.3)
    static float CandidateRoll(const ZoneRecord& z);
    // the held towns that are heat candidates now, hottest first
    void Candidates(const ZoneRegistry& registry, const DirectorView& view, const DirectorLauncher& launcher,
                    AutoArray<int>& out);
    // the push-out target (6.4), -1 none (STRONG is the caller's check)
    int PushoutTarget(const ZoneRegistry& registry, const DirectorView& view, const DirectorLauncher& launcher);
    // the package and source for an operation on target (no side effects)
    DirectorAction PlanFor(const ZoneRegistry& registry, const DirectorView& view, int target, int origin) const;
    // one director step: tokens, urgency, cooldowns, intel decay, then at
    // most one launch.  No-op unless Active().
    void Direct(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, float elapsed);
    // the package catalogue (built from the occupier's ladder at the first
    // director tick on the engine path; tests set it)
    void SetCatalogue(const ForcePackageCatalogue& cat)
    {
        _catalogue = cat;
        _catalogueBuilt = true;
    }
    const ForcePackageCatalogue& Catalogue() const { return _catalogue; }
    // queries
    float Now() const { return _clock + _directorAccum; }
    int KnownFighters(const ZoneRegistry& registry, int zoneIndex) const;
    bool Urgent(const char* zone) const { return HasName(_urgent, zone); }
    bool RedPaid(const char* zone) const { return HasName(_redPaid, zone); }
    float Cooldown(const char* zone) const;
    int OpCounter() const { return _opCounter; }
    int SightingRows() const { return _sightings.Size(); }
    RString SightingZone(int i) const { return i >= 0 && i < _sightings.Size() ? _sightings[i].zone : RString(); }
    int SightingCount(int i) const { return i >= 0 && i < _sightings.Size() ? _sightings[i].count : 0; }
    float SightingAge(int i) const { return i >= 0 && i < _sightings.Size() ? Now() - _sightings[i].stamp : 0.0f; }
    const AutoArray<Vector3>* SightingPositions(int i) const
    {
        return i >= 0 && i < _sightings.Size() ? &_sightings[i].positions : nullptr;
    }
    RString TownsEverName(int i) const { return i >= 0 && i < _townsEver.Size() ? _townsEver[i].name : RString(); }
    int UrgentCount() const { return _urgent.Size(); }
    RString UrgentName(int i) const { return i >= 0 && i < _urgent.Size() ? _urgent[i].name : RString(); }
    int CooldownCount() const { return _cooldowns.Size(); }
    RString CooldownZone(int i) const { return i >= 0 && i < _cooldowns.Size() ? _cooldowns[i].zone : RString(); }
    float CooldownLeft(int i) const { return i >= 0 && i < _cooldowns.Size() ? _cooldowns[i].seconds : 0.0f; }
    // test lever (gmForcesForce "cooldown"): rest a zone for seconds
    void ForceCooldown(const char* zone, float seconds) { SetCooldown(zone, seconds); }
    int OpRows() const { return _ops.Size(); }
    // the ledger row of a director operation, -1 none
    int FindOp(int opId) const;
    int OpOutstanding(int row) const { return row >= 0 && row < _ops.Size() ? _ops[row].outstanding : 0; }

    // test levers (gmForcesForce / gmForcesTune) --------------------------------
    void SetPool(int units);
    void SetMeanStanding(float standing);
    // gmForcesForce "wave": a request parked for phase 4 (no effect yet)
    void RequestWave();

    // engine hook: activation on the first active frame, then a director
    // tick every forcesDirectorInterval
    void Simulate(float deltaT);

    // persistence -----------------------------------------------------------------
    // a save without the block, loaded with the key on (D20): size, then
    // derive the pool and wave state from the zone state
    void DeriveFromExistingSave(const ZoneRegistry& registry);
    bool HasState() const { return _active || _restored; }
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct NameRow
    {
        RString name;
        LSError Serialize(ParamArchive& ar);
    };
    // a zone and a clock (cooldowns, phase 3)
    struct ZoneClockRow
    {
        RString zone;
        float seconds = 0;
        LSError Serialize(ParamArchive& ar);
    };
    // one sighting report (6.1): the zone ("" = outside every zone), the
    // distinct units its latest scan saw, when (Now()), where
    struct SightingRow
    {
        RString zone;
        int count = 0;
        float stamp = 0;
        AutoArray<Vector3> positions;
        LSError Serialize(ParamArchive& ar);
    };
    // the ledger of one director operation: the pool units it still holds
    // (hull + crew) until they are refunded or lost
    struct OpRow
    {
        int opId = 0;
        RString target;
        int kind = CAKRetake;
        int origin = CAODirector;
        int package = FPFoot;
        int outstanding = 0;
        bool born = false;
        bool finished = false;
        LSError Serialize(ParamArchive& ar);
    };

    void LoadFromConfig();
    // re-step the band after a pool change; logs a change
    void UpdateBand();
    void Refill(float elapsed);
    // an INFO line at most every LogQuiet seconds (others counted)
    bool LogAllowed();
    // the director's pieces
    void RefillTokens(float elapsed);
    void UpdateUrgency(const ZoneRegistry& registry);
    void CountDownCooldowns(float elapsed);
    void DecaySightings();
    bool TryLaunch(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int target,
                   int origin);
    static bool HasName(const AutoArray<NameRow>& rows, const char* name);
    static void AddName(AutoArray<NameRow>& rows, const char* name);
    static void RemoveName(AutoArray<NameRow>& rows, const char* name);
    void SetCooldown(const char* zone, float seconds);
    // a fresh flip or a lost town: its urgency latch, cooldown and RED payment go
    void ForgetZone(const char* zone);
    void PruneOps();
    // engine path: the catalogue, the view, the scan
    void BuildEngineCatalogue(const ZoneRegistry& registry);
    void FillEngineView(const ZoneRegistry& registry, DirectorView& view) const;
    void ScanWorld(const ZoneRegistry& registry);

    OccupierForcesTuning _tuning;
    bool _active = false;
    bool _restored = false;        // a saved block was loaded
    bool _activationTried = false; // transient: one activation attempt per session
    int _pool = 0;
    int _poolStart = 0;
    int _R = 0;
    int _G = 0;
    AutoArray<NameRow> _roster;
    AutoArray<NameRow> _towns;
    AutoArray<NameRow> _excluded;
    AutoArray<NameRow> _townsEver;      // distinct towns ever taken
    AutoArray<NameRow> _urgent;         // urgency latches
    AutoArray<NameRow> _redPaid;        // held zones whose RED edge already paid a repel's heat
    AutoArray<ZoneClockRow> _cooldowns; // seconds left
    AutoArray<SightingRow> _sightings;
    AutoArray<OpRow> _ops;
    float _clock = 0; // seconds of active director time (sighting stamps)
    int _posture = OFDormant;
    int _tokens = 0;
    float _tokenClock = 0;
    float _directorAccum = 0;
    int _opCounter = 0;
    float _refillAccum = 0;
    int _band = OFBandStrong;
    float _control = 1.0f;
    int _wavesFired = 0;
    bool _finalFired = false;
    int _wavesLanded = 0;
    int _waveRequests = 0;
    int _lost = 0;
    int _refillTotal = 0;
    int _wavesTotal = 0;
    bool _legacySave = false;
    float _meanStanding = DefaultStanding; // transient until phase 4's per-town standing
    float _logClock = 0;                   // transient: time since the last rate-limited line
    int _logSuppressed = 0;
    // transient
    float _scanAccum = 0;
    AutoArray<int> _alertPrev; // the last scan's alert states (-1 unknown)
    bool _basesDeferredLogged = false;
    bool _catalogueBuilt = false;
    ForcePackageCatalogue _catalogue;
};

} // namespace Guerrilla
} // namespace Poseidon
