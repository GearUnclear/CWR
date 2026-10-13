#pragma once

// Guerrilla Mode occupier counter-attack service (issue #79 B2 follow-up,
// part of #77 scoped to towns).  The rules and the pacing formula are
// written down in guerrilla-mode/OFFSCREEN-CONTRACT.md (decision C and the
// "Pacing" subsection); this header is the code-side summary.
//
// DISPATCH.  When a CITY flips to the resistance (ZoneRegistry's captured
// event; the start-town HQ, exempt from its garrison, is never a target) the
// service rolls once whether the occupier answers.  The chance and the force
// size shrink with every town the resistance already held at the flip
// (momentum).  An answered flip waits a delay drawn from
// [counterAttackDelayMin, counterAttackDelayMax], then LAUNCHES: it picks the
// nearest occupier military zone that can spare bodies (its stored reserve,
// or its live cache garrison when the town is inside the player's bubble),
// keeping counterAttackSourceKeep of the base's strength at home.  No such
// base anywhere: no counter-attack.  One record per target town.
//
// TWO PATHS.  A source whose garrison is live (inside the bubble) marches its
// own soldiers out: they leave the post's cache groups and walk on the town
// (LIVE at once).  A stored reserve is debited and TRAVELS abstractly for
// (distance - counterAttackStageDistance) * counterAttackRoadFactor /
// counterAttackSpeed seconds.  On arrival:
//   * town inside the bubble: a live squad is spawned at the staging point
//     (on the source's side of the town, road-snapped) and walks in.  Over
//     the live-AI or group budget, or with counterAttackMaxLive squads already
//     out, it waits and retries (logged), never skipped;
//   * town outside the bubble: the force arrives ABSTRACTLY.  With resistance
//     fighters in the town, one odds roll decides it; otherwise the force
//     HOLDS the town as abstract occupier presence and the zone tick drains
//     it down to townRevertSupport and reverts it (RevertTown).  A repelled
//     force loses counterAttackRepelLoss of its strength; survivors return to
//     the source's reserve, and D2 recovery regrows the rest.
// A live squad stays live until it resolves, whatever the player's bubble
// does (it is never cached back to abstract): wiped out = repelled; the town
// reverts under it = it becomes the town's garrison (GarrisonCache adopts the
// group, the stored reserve tops it up to garrisonMax); still there after
// counterAttackLiveTimeout and out of the player's sight = it withdraws and
// its survivors return to the source.
//
// DETERMINISM.  Every roll (dispatch, delay, odds) is a hash of a seed fixed
// at the flip (town name, flip stamp, towns held) and a salt, so a save and
// reload never changes an outcome and the off-screen rules draw no engine
// random numbers.
//
// Script surface (CounterAttackCommands.cpp): gmCounterAttacks,
// gmCounterAttackTune.  Events (ZoneRegistry handler slots): counterAttack
// when a force reaches a town (live squad sent in, or abstract arrival) and
// counterRepelled when one is beaten off or withdraws.  Config keys
// (CfgGuerrillaZones): counterAttack and every counterAttack* key in
// CounterAttackTuning.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
namespace Poseidon
{
class AIGroup;
} // namespace Poseidon

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;
struct ZoneEventRecord;

struct CounterAttackTuning
{
    bool enabled = true;          // counterAttack (0 = off, and momentum off too)
    float size = 0.5f;            // counterAttackSize: the first force, a fraction of groupSize
    float chance = 1.0f;          // counterAttackChance: dispatch probability for the first town
    float delayMin = 120.0f;      // counterAttackDelayMin (s after the flip)
    float delayMax = 300.0f;      // counterAttackDelayMax
    float sizeDecay = 0.15f;      // counterAttackSizeDecay: force * (1 - d)^townsHeldBefore
    float chanceDecay = 0.15f;    // counterAttackChanceDecay: chance * (1 - d)^townsHeldBefore
    float sourceKeep = 0.25f;     // counterAttackSourceKeep: share of garrisonMax a base keeps home
    float speed = 4.0f;           // counterAttackSpeed: abstract travel speed (m/s)
    float roadFactor = 1.3f;      // counterAttackRoadFactor: road distance / straight distance
    float stageDistance = 500.0f; // counterAttackStageDistance: live squads appear this far out (m)
    float defenderWeight = 3.0f;  // counterAttackDefenderWeight: one fighter in his town = this many attackers
    float repelLoss = 0.5f;       // counterAttackRepelLoss: share of a repelled force that does not come home
    float liveTimeout = 1200.0f;  // counterAttackLiveTimeout: s before an unresolved live squad withdraws
    int maxLive = 2;              // counterAttackMaxLive: live squads out at once
};

enum CounterAttackPhase
{
    CAScheduled, // waiting out the delay after the flip
    CATravel,    // a stored reserve on the road (abstract)
    CAHolding,   // an abstract force standing in an empty town, draining it
    CALive,      // a live squad
    NCounterAttackPhases
};

// Per-step observations.  The engine path fills it from the live world
// (Simulate); unit tests inject values.
struct CounterAttackView
{
    bool playerValid = false;
    float playerX = 0;
    float playerZ = 0;
    // per zone (optional, missing = 0): the zone's live GarrisonCache bodies
    AutoArray<int> garrisonLive;
    // budget census (GatherSimDiag): live AI bodies, all sides, and occupier groups
    int liveBodies = 0;
    int occGroups = 0;
};

// The world-touching half (engine implementation in CounterAttack.cpp).
// Null in unit tests: the live path then waits, nothing is spawned.
class CounterAttackOps
{
  public:
    virtual ~CounterAttackOps() = default;
    // spawn n occupier bodies at from as one group sent on target
    virtual AIGroup* SpawnSquad(int n, Vector3Par from, Vector3Par target) = 0;
    // move up to n living bodies out of a zone's cache garrison into a new
    // group sent on target; taken = bodies moved
    virtual AIGroup* DetachFromGarrison(int sourceZone, int n, Vector3Par target, int& taken) = 0;
    // living bodies and the leader's position (0 for a dead or null group)
    virtual int Alive(AIGroup* grp, Vector3& leaderPos) = 0;
    virtual void MoveOn(AIGroup* grp, Vector3Par target) = 0;
    // delete every body and the group
    virtual void Remove(AIGroup* grp) = 0;
    // hand the group to GarrisonCache as the town's garrison
    virtual void Adopt(int townZone, AIGroup* grp) = 0;
};

class CounterAttack : public SerializeClass
{
  public:
    CounterAttack() = default;

    static CounterAttack& Instance();

    // seconds between steps
    static constexpr float TickInterval = 1.0f;
    // retry delay for a launch or live spawn that has to wait (budget,
    // spawn failure, no world)
    static constexpr float RetryInterval = 30.0f;
    // a live squad farther than this from the town centre is re-sent
    static constexpr float ResendInterval = 30.0f;
    // shortest abstract road leg (s)
    static constexpr float MinTravel = 10.0f;

    struct Record
    {
        RString target;
        RString source;
        int phase = CAScheduled;
        // Scheduled / Travel: seconds left; Live: seconds alive
        float clock = 0;
        int strength = 0;    // bodies drawn from the source (0 while scheduled)
        int size = 0;        // planned force at the flip
        int townsBefore = 0; // towns the resistance held before this flip
        unsigned int seed = 0;
        int salt = 2;     // next roll salt (0 dispatch, 1 delay)
        float resend = 0; // Live: seconds to the next re-send check
        bool deferLogged = false;
        LLink<AIGroup> group; // Live only
        LSError Serialize(ParamArchive& ar);
    };

    // lifecycle ---------------------------------------------------------------
    void Clear();
    void InitMission(); // Clear + read the CfgGuerrillaZones keys
    void LoadFromParams(const ParamEntry* zonesCfg);
    const CounterAttackTuning& Tuning() const { return _tuning; }
    CounterAttackTuning& TuningMutable() { return _tuning; }

    // pure rules (unit-tested) ------------------------------------------------
    static float DispatchChance(const CounterAttackTuning& t, int townsBefore);
    static int ForceSize(const CounterAttackTuning& t, int groupSize, int townsBefore);
    // probability the town holds: D*w / (A + D*w); 0 with no defenders, 1
    // with no attackers
    static float HoldOdds(const CounterAttackTuning& t, int defenders, int attackers);
    static float TravelTime(const CounterAttackTuning& t, float straightDistance);
    // bodies a base can spare: (stored + live) - floor(garrisonMax * keep),
    // drawn from the live garrison when useLive, else from the stored reserve
    static int Spare(const CounterAttackTuning& t, float stored, int live, float garrisonMax, bool useLive);
    // deterministic uniform [0, 1) from a seed and a salt
    static float Roll01(unsigned int seed, int salt);
    static unsigned int MakeSeed(const char* town, int flipDay, int flipMinute, int townsBefore);

    // the service ---------------------------------------------------------------
    // a CITY just flipped to the resistance: roll and schedule.  True when a
    // counter-attack was scheduled.
    bool OnTownFlipped(const ZoneRegistry& registry, int zoneIndex, int groupSize);
    // the registry's events of one zone tick (engine wrapper: flips and
    // town losses)
    void OnZoneEvents(const AutoArray<ZoneEventRecord>& fired);
    // advance every record by elapsed seconds.  ops may be null (no world:
    // the live path waits).  Raises counterAttack / counterRepelled into
    // fired, and townLost through RevertTown.
    void Step(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
              AutoArray<ZoneEventRecord>& fired);
    // abstract occupier bodies holding a zone (ZoneTickInputs::abstractOcc)
    int AbstractPresence(const ZoneRegistry& registry, int zoneIndex) const;

    // runtime knob (gmCounterAttackTune "delayMax"): a pending delay never
    // outlasts the new ceiling
    void ClampScheduled(float maxDelay);

    int Count() const { return _records.Size(); }
    const Record* Get(int i) const { return i >= 0 && i < _records.Size() ? &_records[i] : nullptr; }
    int FindByTarget(const char* town) const;
    static const char* PhaseName(int phase);

    // engine hook: per frame, throttled to TickInterval
    void Simulate(float deltaT);

    bool HasState() const { return _records.Size() > 0; }
    LSError Serialize(ParamArchive& ar) override;

  private:
    void LoadFromConfig();
    // nearest occupier military zone with bodies to spare; -1 when none
    int PickSource(const ZoneRegistry& registry, const CounterAttackView& view, int target, bool allowLive, int& spare,
                   bool& live) const;
    // the staging point on the source's side of the town
    static Vector3 StagePoint(const CounterAttackTuning& t, Vector3Par town, Vector3Par source);
    // survivors go home, capped at the base's strength
    static void ReturnToSource(ZoneRegistry& registry, const CounterAttackView& view, const Record& r, int bodies);
    bool InBubble(const ZoneRegistry& registry, const CounterAttackView& view, int zone) const;
    int LiveSquads() const;
    // the abstract fight on arrival: true when the record is finished
    bool Fight(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
               AutoArray<ZoneEventRecord>& fired);
    // spawn the live squad at the staging point; false = wait and retry
    bool GoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops,
                bool announce, AutoArray<ZoneEventRecord>& fired);
    // one record's step: true when it is finished
    bool Advance(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops, Record& r,
                 AutoArray<ZoneEventRecord>& fired);
    // a live squad in the town it retook becomes the town's garrison
    static void AdoptLive(ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, int alive);

    CounterAttackTuning _tuning;
    AutoArray<Record> _records;
    float _accum = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
