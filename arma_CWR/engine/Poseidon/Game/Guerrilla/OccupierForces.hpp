#pragma once

// Guerrilla Mode occupier forces: the strategic pool and the director
// (guerrilla-mode/OCCUPIER-FORCES.md, phases 2 to 4).  This header is the
// code-side summary of the design's sections 5 (the pool), 6 (the director),
// 7.1 (control and waves), 7.3 (occupier standing, the model) and 11
// (persistence).  Phase 6 (the B half): the director's air buying, town
// strikes and alarm answers; the airframes themselves are OccupierAir's and
// CounterAttack's.
// The service owns the ledger every operation spends from and decides WHAT
// to send; CounterAttack moves it (CounterAttackOrders.hpp is the contract).
//
// KEY.  occupierForces (CfgGuerrillaZones, 0/1, default 1 since phase 7;
// OccupierForcesTuning::KeyDefault, missions with campaignEnd = 0 author
// 0).  With it 0 the
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
// MaxWarLevel.  The occupier-side readers use it (phase 4): garrison tiers
// (GarrisonCache), the counter-attack squad tiers and ForceSize
// (CounterAttack), the convoy / patrol class pick and the traffic
// modulation (Traffic), the undercover detection factor (Undercover), the
// script QRF (qrf.sqs through gmWarLevelOccupier).  The resistance keeps the
// territory level (gmWarLevel: hold squads, the recruit cap, the journal,
// escalation.sqs).  With the key off it equals the territory level.
//
// WAVES (7.1, D7; phase 4).  Each director tick reads the control C and the
// roster bases the occupier holds as Tick measured them.  A trigger is due
// when C is at or below a forcesWaveThresholds[] entry not yet fired, or
// (forcesWaveFinal) when exactly one roster base is left: the final stand.
// A due trigger fires after forcesControlDwell consecutive due ticks; every
// trigger fires once; triggers due together, or due while a wave is
// inbound, merge into one wave (the merged final stand shortens the lead).
// Size = forcesWaveShare x poolStart, clamped to [forcesWaveMin,
// forcesWaveMax]; it lands after forcesWaveLead s (forcesWaveLeadFinal for
// the final stand) through LandWave, then the allocation runs in order:
//   1. the hub top-up: the hub (forcesWaveHub by name, else the
//      occupier-held AIRFIELD, else SEAPORT, else the strongest base) is
//      filled to garrisonMax by a pool-funded deposit;
//   2. the final stand: the last base gets a surge of up to forcesWaveSurge
//      above its strength (a deposit above garrisonMax that nothing trims;
//      it is never spare, and the last base never sources a strike);
//   3. strikes on the best cluster (score of a held zone z: the sum over
//      held zones y within forcesWaveCluster of 4 for the HQ's zone, 3 for
//      a base, 1 for a town, plus heat(y) / 40; an HQ outside every held
//      zone adds 4 when it stands within the radius): the HQ if it is in
//      the cluster, then bases, then towns, up to forcesWaveRetakeShare of
//      the wave on up to forcesWaveOps targets, each its own package, from
//      the hub (with no hub, from the nearest occupier base), launched
//      together with origin CAOWave, only once the director is AWAKE.  Wave
//      operations take no token and come on top of forcesOpsMax
//      (CounterAttack::OpsCap raises the born cap by one per living wave
//      record).  Their infantry is staged: a pool-funded deposit straight
//      into the source's reserve that the launch debits in the same tick;
//   4. REINFORCE columns from the hub to bled bases (CAKReinforce, topping
//      them up on arrival), up to forcesWaveOps columns;
//   5. the rest stays in the pool.
// "In a wave" (the HQ candidate) is a wave inbound or landed less than
// WaveWindow s ago.  gmForcesForce ["wave", n] fires wave n now.
//
// STANDING (7.3, D21: the model only; strikes are phase 6).  Each director
// tick every CITY's ZoneRecord::standing recovers toward 50 at 1 per
// StandingRecoverSeconds, the mean over the K towns feeds the refill
// factor (SetMeanStanding), and a town with standing >= 50 where resistance
// units stand may yield an informant TIP with chance standing / 200 (a
// deterministic hashed roll): a sighting of the true count at the town
// centre jittered +-TipJitter m.  This is the one place the director reads
// the true census: a local talked.  Garrisons spawned in a town with
// standing < 25 fight at skill x 0.8 (GarrisonCache::MoraleFactor).
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
//   Roll01(MakeSeed(town, flipDay, flipMinute, 0), 7).  A held base (D16)
//   by the same rule, only while STRONG (CAKBase, minimum max(4,
//   garrisonMax)).  The HQ (HqCandidate; D13, D14, D24) only while STRONG
//   or in a wave, by the heat of the zone it stands in (none for an HQ
//   outside every zone), and unconditionally (first in the list) once the
//   resistance holds no base and no town besides the HQ's own zone: the
//   final push.  The HQ operation is always a CAKHqAssault of at least
//   forcesHqAssaultMin bodies on HqPos; its cooldown is kept under the name
//   HqCooldownName.
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
//   or base nearest an occupier base with no sighting for
//   forcesPushoutQuiet, with the minimal package.
//   PACKAGES (6.5).  ForcePackages: the cheapest lethal package, the fastest
//   within forcesUrgentCostShare when urgent; phase 3 offers Foot and Truck.
//   The pool pays hull + crew (Spend before LaunchFunded, Refund when it is
//   refused); the source base pays the infantry from its spare.
//   WON / LOST.  Nothing is launched, and no wave fires or lands, once
//   CampaignEnd has latched the win or the loss.
//
// AIR (phase 6B; 6.5, D2, D3, D4).  Insertions: the transport helicopter
// joins the package choice whenever the catalogue offers one, chosen like
// any package (the cheapest meeting L_req and the minimum; URGENT takes the
// fastest within forcesUrgentCostShare, which is where the helicopter
// wins).  Its airOrigin is the occupier-held AIRFIELD nearest the target,
// else -1 (off-map), and off-map it is priced at the off-map price
// (ForcePackages::HullPrice(.., true)); the infantry is still paid by the
// source base.  STRIKES (7.3, D17): a jet or attack helicopter, bought ONLY
// while STRONG, either on an URGENT held town with knownFighters >=
// StrikeMinFighters that is not inside its strike window, or as the opener
// of a director base retake or HQ assault.  It is bought right after the
// ground operation on that target launches (a town strike alone when no
// ground package can go) and takes no extra token.  The package: the
// cheapest offered strike airframe the pool can pay (hull at the airfield
// or off-map price, plus its crew), the fastest within
// forcesUrgentCostShare when urgent.  strikePos is the centroid of the
// target zone's REPORTED positions (D9), VZero (the town centre) without a
// sighting row; never a hidden player position.
// STRIKE EFFECTS (7.3, D21).  On CAOpStruck the director calls
// ZoneRegistry::ApplyStrike (-40 support and standing in the struck town,
// +15 heat, struckUntil = now + 1200 s; -10 / -10 in every town within
// 2 km) and queues ZETownStruck for the scripts (civilians.sqs panics,
// economy.sqs halves the income through gmZoneStruck).  Applied for every
// CAOpStruck, director operation or test lever alike.
// ALARM ANSWERS (8.2, D4).  A zone the resistance does not hold that sits
// RED is an alarm.  At the sighting cadence the director answers ONE alarm
// at a time with a CAKAlarm order (origin CAOAlarm): minimum
// ForcePackages::AlarmMinBodies, always urgent (THIN: the cheapest package
// only), lethality against the zone's sightings, sources every occupier base
// but the alarm zone, ground packages and the transport helicopter.
// alarmPos is the zone's latest reported position (VZero = the centre).
// SPENT does not answer ("Starved" logged once per alarm).  The zone's
// cooldown (a repel) applies.  Deviation from the phase brief's "same
// tokens": an alarm answer takes no director token and is answered while
// DORMANT too (8.2 sets its own limits), so a key-on campaign never goes
// without an answer before the second town.  None while legacySave is set.
// LEGACY QRF (D20, 11).  The block saves "format" (SaveFormat).  A block
// with an older format, or derived from a save without the block, sets
// legacySave: the native alarm answers stay off and gmLegacyQrf is true, so
// an old campaign keeps its own script QRF (whose text the save carries).
// gmLegacyQrf (LegacyQrf) is also true with the key off or the service
// sized off; qrf.sqs fields its QRF only while it is true.
//
// Script surface (OccupierForcesCommands.cpp): gmOccupierForces,
// gmOccupierOps, gmOccupierIntel, gmForcesTune, gmForcesForce, gmLegacyQrf.  Every INFO
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
    // the occupierForces value when the mission does not author the key
    // (phase 7: on).  Every reader of the key (this service, CounterAttack's
    // forcesOn, ZoneRegistry's baseRetake fallback) uses this one default.
    static constexpr float KeyDefault = 1.0f;

    bool enabled = false;           // occupierForces (0/1); false until a CfgGuerrillaZones is read
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
    // phase 4: the HQ assault and the waves (7.1, 16)
    int hqAssaultMin = 6;          // forcesHqAssaultMin, 1..12: minimum HQ assault bodies
    int waveDwell = 2;             // forcesControlDwell, 1..10: consecutive due ticks before a wave fires
    bool waveFinal = true;         // forcesWaveFinal (0/1): the final stand when one roster base is left
    float waveShare = 0.5f;        // forcesWaveShare, 0..2: wave size, share of poolStart
    int waveMin = 12;              // forcesWaveMin, 0..200
    int waveMax = 48;              // forcesWaveMax, 0..400
    float waveLead = 600.0f;       // forcesWaveLead, 0..7200 s inbound
    float waveLeadFinal = 120.0f;  // forcesWaveLeadFinal, 0..7200 s inbound for the final stand
    float waveRetakeShare = 0.5f;  // forcesWaveRetakeShare, 0..1: strikes' share of the wave
    int waveOps = 4;               // forcesWaveOps, 0..8: strike targets (and reinforce columns)
    float waveCluster = 2000.0f;   // forcesWaveCluster, 100..20000 m
    int waveSurge = 12;            // forcesWaveSurge, 0..48: final-stand surge above strength
    RString waveHub;               // forcesWaveHub: the template's hub by zone name ("" = the rule)
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
    int hqZone = -1;             // the zone the player's HQ stands in: never a retake target (N4a)
    bool won = false;            // CampaignEnd latched the win: launch nothing (7.4)
    bool lost = false;           // CampaignEnd latched the loss: the same freeze (9.2)
    // the HQ (9.1): established, and where (the HQ assault's destination)
    bool hqEstablished = false;
    Vector3 hqPos = VZero;
    // per zone (optional, missing = 0): the live GarrisonCache bodies (the
    // wave hub's strength and top-up)
    AutoArray<int> garrisonLive;
    float now = 0;               // mission time (s): the town-strike windows (phase 6)
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
    // an HQ_ASSAULT record exists (phase 4; the default for fakes: none)
    virtual bool HasHqRecord() const { return false; }
};

// One planned operation (PlanFor)
struct DirectorAction
{
    bool go = false;
    int target = -1; // zone index; for CAKHqAssault the HQ's zone, -1 outside every zone
    int kind = CAKRetake;
    int origin = CAODirector;
    bool urgent = false;
    float fighters = 0;
    Vector3 destPos = VZero; // CAKHqAssault: HqPos
    PackageChoice choice;
    // phase 6: air and the new kinds (copied into the order)
    int airOrigin = -1;        // the occupier-held AIRFIELD an airframe starts from, -1 off-map
    int strikeZone = -1;       // CAKStrike
    Vector3 strikePos = VZero; // CAKStrike: reported fighters, VZero = the zone's centre
    int alarmZone = -1;        // CAKAlarm
    Vector3 alarmPos = VZero;  // CAKAlarm: the latest reported position, VZero = the centre
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
    // drops the S5 refund hook when this service is its target
    ~OccupierForces();

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
    // a director operation CounterAttack refuses is re-planned from the next
    // source at most this many times in the same tick (M1)
    static constexpr int MaxSourceRetries = 4;
    // phase 5 offers every ground package (foot, truck, armed car, APC,
    // tank); phase 6 adds air
    static constexpr unsigned int OfferedPackages = ForcePackages::DefaultPackages;
    // phase 4: the HQ in the candidate list (Candidates, PlanFor), its
    // cooldown row's name, the "in a wave" window after a landing (s), the
    // standing recovery (s per point toward 50), the informant tip (roll
    // salt, position jitter m), morale (standing below LowStanding)
    static constexpr int HqCandidate = -2;
    static constexpr const char* HqCooldownName = "@HQ";
    static constexpr float WaveWindow = 1800.0f;
    static constexpr float StandingRecoverSeconds = 600.0f;
    static constexpr int TipSalt = 11;
    static constexpr float TipJitter = 100.0f;
    static constexpr float LowStanding = 25.0f;
    static constexpr float MoraleSkill = 0.8f;
    // phase 6: a town strike needs this many reported fighters (7.3); the
    // block format that knows the native alarm answers (older = legacySave);
    // the rest of an alarm zone after CounterAttack refused its answer (s)
    static constexpr int StrikeMinFighters = 3;
    static constexpr int SaveFormat = 6;
    static constexpr float AlarmRefusedRest = 60.0f;
    // the strike airframes the director may buy
    static constexpr unsigned int StrikePackages = (1u << FPAttackHeli) | (1u << FPJet);

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
    // forcesWaveShare x poolStart, rounded, clamped to [waveMin, waveMax]
    static int WaveSize(const OccupierForcesTuning& t, int poolStart);
    // the cluster score of a resistance-held zone z (7.1): the sum over
    // resistance-held zones y within radius of z of (4 if y is hqZone, 3 if
    // a base, 1 if a town, else 0) + heat(y) / 40, plus 4 when the HQ stands
    // within radius of z and its zone is not a held zone counted above
    // (hqPos VZero: no HQ)
    static float ClusterScore(const ZoneRegistry& registry, int z, float radius, int hqZone, Vector3Par hqPos);
    // the best cluster centre (highest score, zone order on ties), -1 when
    // the resistance holds nothing
    static int BestCluster(const ZoneRegistry& registry, float radius, int hqZone, Vector3Par hqPos);
    // the wave hub (7.1): forcesWaveHub when it names an occupier-held
    // military zone, else the occupier-held AIRFIELD, else SEAPORT, else the
    // strongest (reserve + live) occupier-held military zone; ties go to the
    // stronger, then the zone order; -1 none.  live optional per zone.
    static int PickHub(const ZoneRegistry& registry, const OccupierForcesTuning& t, const AutoArray<int>* live);
    // roster bases the occupier holds now
    int RosterBasesHeld(const ZoneRegistry& registry) const;
    // the final push (6.3): the resistance holds no base and no town besides
    // the HQ's own zone
    static bool FinalPush(const ZoneRegistry& registry, int hqZone);
    // the morale skill factor of a garrison spawned in z: MoraleSkill in a
    // CITY with standing < LowStanding, else 1
    static float MoraleFactor(const ZoneRecord& z);
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
    // (optional) accepts them.  log: one INFO line per town.  authoredOwner:
    // a town counts in K by its AUTHORED owner (ownerConfig resolved: the
    // occupier or NEUTRAL), not the current one; the old-save derivation
    // only (M7), so the towns the resistance holds at load still count.
    static void Size(const ZoneRegistry& registry, const OccupierForcesTuning& t, OccupierForcesSizing& out,
                     const AutoArray<bool>* seeded = nullptr, const SettlementProbe* probe = nullptr, bool log = false,
                     bool authoredOwner = false);
    // a zone's authored owner side: ownerConfig with the OCCUPIER /
    // RESISTANCE tokens resolved (ZoneRegistry::ResolveOwnerToken's rule)
    static RString AuthoredOwnerOf(const ZoneRegistry& registry, const ZoneRecord& z);
    static const char* BandName(int band);
    static const char* PostureName(int posture);

    // the service ---------------------------------------------------------------
    bool Enabled() const { return _tuning.enabled; }
    // sized and running (the key on, a roster found); a loaded block with
    // the key now off is restored but inert
    bool Active() const { return _active && _tuning.enabled; }
    // take the roster and size the pool (the first active tick).  False and
    // off for the session on an empty roster.  authoredOwner: Size's flag
    // (the old-save derivation).
    bool Activate(const ZoneRegistry& registry, const AutoArray<bool>* seeded = nullptr,
                  const SettlementProbe* probe = nullptr, bool authoredOwner = false);
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
    // S5 (3.3): a pending deposit whose zone changed owner is refunded to
    // this service's pool (GarrisonCache::SetPendingRefundHook).  The hook
    // credits the pool and claims the bodies only while Active(); otherwise
    // GarrisonCache's phase-1 path runs (origin post, else discarded).
    // InitMission and Clear of the process-wide instance install it; a unit
    // test's local service installs it explicitly and drops it on Remove or
    // destruction.
    void InstallPendingRefundHook();
    void RemovePendingRefundHook();
    // the hook body: true when the pool took the bodies
    static bool RefundPendingToPool(ZoneRegistry& registry, int zone, int originZone, int bodies);

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
    // the same over a territory level the caller already read
    float OccupierWarLevelFrom(float territory) const;
    // waves (phase 4)
    bool WaveInbound() const { return _waveInbound; }
    float WaveClock() const { return _waveClock; }
    int WaveBodies() const { return _waveBodies; }
    bool WaveFinalStand() const { return _waveFinalStand; }
    int WaveCount() const { return _waveCount; }
    bool FinalFired() const { return _finalFired; }
    int ControlDwell() const { return _controlDwell; }
    float WaveWindowLeft() const { return _waveWindow; }
    bool InWave() const { return _waveInbound || _waveWindow > 0; }
    RString SurgeZone() const { return _surgeZone; }
    int SurgeBodies() const { return _surgeBodies; }
    // the surge bodies stored at zone (0 elsewhere)
    int SurgeAt(const char* zone) const;

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
    // the package and source for an operation on target (no side effects).
    // target HqCandidate plans the HQ assault.  fixedSource >= 0 offers only
    // that base, with fixedSpare bodies to spare (a wave strike's staging).
    DirectorAction PlanFor(const ZoneRegistry& registry, const DirectorView& view, int target, int origin,
                           int fixedSource = -1, int fixedSpare = 0) const;
    // one director step: tokens, urgency, cooldowns, intel decay, standing,
    // the waves, then at most one launch.  No-op unless Active().
    void Direct(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, float elapsed);
    // the waves' step (7.1): triggers with dwell and merge, the inbound
    // clock, the landing and its allocation.  Called by Direct.
    void StepWaves(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, float elapsed);
    // fire a wave now (the triggers due, or gmForcesForce "wave"): final
    // marks the final stand.  A wave already inbound absorbs it (merge).
    void FireWave(const ZoneRegistry& registry, const DirectorView& view, bool final, const char* why);
    // the landing's allocation (7.1 order); returns the bodies left in the pool
    int AllocateWave(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int bodies,
                     bool final);
    // standing (7.3): recovery toward 50, the K towns' mean into the refill
    // factor, informant tips.  Called by Direct.
    void StepStanding(ZoneRegistry& registry, float elapsed);
    // one informant tip into the sighting table (a local talked: count at pos)
    void IngestTip(const ZoneRegistry& registry, int zone, int count, Vector3Par pos);
    // phase 6 (air, strikes, alarms) ------------------------------------------
    // the occupier-held AIRFIELD nearest pos, -1 none (air is off-map)
    static int HeldAirfield(const ZoneRegistry& registry, Vector3Par pos);
    // the bought-when rule (7.3): may a strike go with an operation of
    // launchedKind on target (HqCandidate for the HQ)?  STRONG, a strike
    // airframe offered, and either a base / HQ opener, or an URGENT held
    // town with >= StrikeMinFighters reported and outside its strike window
    bool StrikeDue(const ZoneRegistry& registry, const DirectorView& view, int target, int launchedKind) const;
    // the strike package on target (HqCandidate for the HQ): go false when
    // no strike airframe is offered or the pool cannot pay any
    DirectorAction PlanStrike(const ZoneRegistry& registry, const DirectorView& view, int target, bool urgent) const;
    // the centroid of a zone's reported positions, VZero without a row
    Vector3 ReportedCentroid(const ZoneRegistry& registry, int zone) const;
    // the latest reported position in a zone, VZero without a row
    Vector3 LatestReport(const ZoneRegistry& registry, int zone) const;
    // the alarm to answer now (8.2): a zone the resistance does not hold,
    // RED, not on cooldown, no record on it; the most reported fighters
    // first, then the zone order; -1 none
    int AlarmTarget(const ZoneRegistry& registry, const DirectorView& view, const DirectorLauncher& launcher) const;
    // the alarm answer's package and source (no side effects)
    DirectorAction PlanAlarm(const ZoneRegistry& registry, const DirectorView& view, int zone) const;
    // an alarm answer of the director's is still in flight (its record
    // lives); rows whose record is gone are closed here
    bool AlarmInFlight(const ZoneRegistry& registry, const DirectorLauncher& launcher);
    // one alarm step: at most one answer (Direct calls it; the engine also at
    // the sighting cadence).  No-op unless Active() and not legacySave.
    void AnswerAlarms(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher);
    // the strike effects (CAOpStruck): ZoneRegistry::ApplyStrike, the
    // ZETownStruck event queued, the tallies, one INFO line.  Returns the
    // ring towns cut.
    int ApplyStrikeEffects(ZoneRegistry& registry, int zone, Vector3Par pos, float now, int opId);
    // test levers (gmForcesForce "strike" / "alarm"): plan and launch now,
    // bypassing the bought-when rule / the RED requirement (the package
    // choice, the pool and the launcher still apply).  The engine path.
    bool ForceStrike(int zone);
    bool ForceAlarm(int zone);
    // the same over an explicit registry, view and launcher (unit tests)
    bool ForceStrike(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int zone);
    bool ForceAlarm(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int zone);
    // D20: the script QRF runs (gmLegacyQrf): the key off, the service sized
    // off, or a legacy save.  False before the first sizing with the key on.
    bool LegacyQrf() const;
    void SetLegacySave(bool on) { _legacySave = on; }
    // mission time (s) for the strike stamps: the engine sets it from
    // Glob.time each frame; unit tests set it
    void SetMissionNow(float now) { _missionNow = now; }
    float MissionNow() const { return _missionNow; }
    int TownStrikes() const { return _townStrikes; }
    int StrikesApplied() const { return _strikesApplied; }
    int AlarmAnswers() const { return _alarmAnswers; }
    int LoadedFormat() const { return _loadedFormat; }

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
    // the ledger row's package (follows a launch-time downgrade to foot)
    int OpPackage(int row) const { return row >= 0 && row < _ops.Size() ? _ops[row].package : FPFoot; }

    // test levers (gmForcesForce / gmForcesTune) --------------------------------
    void SetPool(int units);
    void SetMeanStanding(float standing);
    // gmForcesForce ["wave", n]: wave n fires now (n beyond the thresholds:
    // the final stand); its thresholds are marked fired.  Inactive: parked
    // (counted, no effect).  The engine path; the test path takes a view.
    void RequestWave(int n = 0);
    void RequestWave(const ZoneRegistry& registry, const DirectorView& view, int n);

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
    // launch a planned action: the pool pays hull and crew (and, staged, the
    // infantry: a deposit into the source's reserve the launch debits), the
    // ledger row, LaunchFunded, the INFO line.  spendToken: a director token
    // (not for wave operations).  waveNo > 0 labels a wave operation.
    // launcherRefused (optional): set true when CounterAttack refused the
    // order (not the pool), so the caller may try another source.
    bool LaunchAction(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                      const DirectorAction& a, bool spendToken, bool staged, int waveNo,
                      bool* launcherRefused = nullptr);
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
    // the occupier bases an operation on dest may come from (PlanFor's list):
    // fixedSource >= 0 offers only that base with fixedSpare; exclude (>= 0)
    // is never offered (an alarm's own zone)
    void BuildSources(const ZoneRegistry& registry, const DirectorView& view, Vector3Par dest, int fixedSource,
                      int fixedSpare, int exclude, AutoArray<PackageSource>& sources) const;
    // ForcePackages::Choose with the transport helicopter offered when the
    // catalogue has one, priced off-map when airOrigin < 0 (phase 6)
    PackageChoice ChooseWithAir(const PackageRequest& req, const AutoArray<PackageSource>& sources,
                                int airOrigin) const;
    // after a director launch on target: the strike that goes with it (7.3)
    void MaybeStrike(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int target,
                     int launchedKind);
    // the engine view (FillEngineView) for the levers and the alarm cadence
    void EngineView(DirectorView& view);

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
    bool _catalogueBuilt = false;
    ForcePackageCatalogue _catalogue;
    // waves (phase 4; serialized)
    bool _waveInbound = false;
    float _waveClock = 0;        // s until the inbound wave lands
    int _waveBodies = 0;         // its size
    bool _waveFinalStand = false; // it carries the final stand (surge)
    int _waveCount = 0;          // waves fired so far ("Wave N")
    int _controlDwell = 0;       // consecutive director ticks with a trigger due
    float _waveWindow = 0;       // s left of "in a wave" after the last landing
    RString _surgeZone;          // the final stand's base
    int _surgeBodies = 0;        // the surge stored there (never spare)
    // transient: the roster bases held as of the last Tick (-1 before one)
    int _basesHeld = -1;
    // phase 6 (serialized): the strike tallies (CITY strikes, every applied
    // strike) and the alarm answers launched
    int _townStrikes = 0;
    int _strikesApplied = 0;
    int _alarmAnswers = 0;
    // transient: the block format loaded (0: none or older than phase 6),
    // mission time, the alarm zones whose starvation was logged
    int _loadedFormat = 0;
    float _missionNow = 0;
    AutoArray<NameRow> _alarmStarved;
};

} // namespace Guerrilla
} // namespace Poseidon
