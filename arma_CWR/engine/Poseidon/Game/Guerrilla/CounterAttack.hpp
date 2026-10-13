#pragma once

// Guerrilla Mode occupier counter-attack service (issue #79 B2 follow-up,
// part of #77 scoped to towns).  The rules and the pacing formula are
// written down in guerrilla-mode/OFFSCREEN-CONTRACT.md (decision C and the
// "Pacing" subsection); this header is the code-side summary.
//
// DISPATCH.  When a CITY flips to the resistance (ZoneRegistry's captured
// event; the start-town HQ, exempt from its garrison, is never a target and
// never counts) the service makes one dispatch decision (Decide).  k is the
// number of towns the resistance had already taken before this flip: the
// larger of the towns it ever took (a saved counter) and the non-exempt
// towns it holds right now.  The very first town (k = 0) is answered with
// counterAttackFirstTownChance (0.4): most of the time nothing comes.  From
// the second town on (k >= 1) every flip is answered with
// counterAttackChance (1.0), no decay.  The force is counterAttackSize *
// groupSize, scaled UP (never down) by the war level, capped at one group.
// An answered flip waits a delay drawn from
// [counterAttackDelayMin, counterAttackDelayMax], then LAUNCHES: it picks the
// nearest occupier military zone that can spare bodies (its stored reserve,
// or its live cache garrison when the town is inside the player's bubble),
// keeping counterAttackSourceKeep of the base's strength at home.  No such
// base anywhere: no counter-attack.  One record per target town.
//
// SEAMS for a later occupier forces service: Decide() is the whole dispatch
// decision (town, k, context in; go / no-go and size out), and LaunchFrom()
// sends a force of an explicit size from an explicit source zone.
//
// TWO PATHS.  A source whose garrison is live (inside the bubble) marches its
// own soldiers out: they leave the post's cache groups and walk on the town
// by road (LIVE at once).  A stored reserve is debited and TRAVELS abstractly
// for distance * counterAttackRoadFactor / counterAttackSpeed seconds along
// its road route from the base to the town (RouteOf: the engine's road net,
// a straight line when there is none or it winds too far, RoadIsReasonable).
//
// UNSEEN BIRTHS (the user's rule, 2026-10-13: soldiers always navigate from
// a base where their spawn is not seen by the player to the town they
// retake).  A travelling force becomes a live squad when the town is inside
// the player's bubble, or when its progress point (the route point at
// elapsed / travel time) is.  Its birth point is the FIRST candidate the
// perception gate passes (PickSpawnPoint; CounterAttackGate is Traffic's
// issue #53 gate): a point hidden by distance or terrain wins; a point
// hidden only by the view cone counts only within ConeOnlyBaseRadius of the
// base, and only when no hidden point exists; no point within
// counterAttackBirthMinToTarget of the town is ever a birth point.  A force
// launched with the town already in the bubble (no progress yet) searches
// from the base toward the town.  A force the player walked up to searches
// BACKWARD from its progress point toward the base (never ahead of where it
// has got to, and a column 90% of the way is not re-born at its base); one
// whose progress point is already inside the bound stays abstract at the
// town edge.  No candidate = deferred: retried every RetryInterval with one
// WARN, its arrival clock stopped, and after counterAttackBirthGiveUp
// seconds it turns back and its strength returns to the base.
// While the town is in the bubble an unborn force does nothing to it (no
// arrival, no odds roll, no retake drain: a HOLDING force is suspended, it
// is never born at the town centre).  A born squad marches the road route
// leg by leg with the doMove idiom (SAFE, so the unit planner keeps to the
// road) to the town edge, and only there takes up its combat posture and
// moves in.  Over the live-AI or group budget, or with counterAttackMaxLive
// squads already out, it waits the same way (logged), never skipped.  A
// marching squad that stays out of the player's perception (Traffic's
// despawn rule, every body, for VanishHold seconds) and away from his bubble
// returns to the abstract road at its current progress (ReturnToRoad): its
// survivors are the abstract strength, the arrival clock picks up where the
// march got to, and its live slot and budget are freed.  Out of the bubble
// the force arrives ABSTRACTLY: with resistance fighters in
// the town, one odds roll decides it; otherwise the force HOLDS the town as
// abstract occupier presence and the zone tick drains it down to
// townRevertSupport and reverts it (RevertTown).  A repelled force loses
// counterAttackRepelLoss of its strength; survivors return to the source's
// reserve, and D2 recovery regrows the rest.  A live squad counts as
// occupier presence only in its own target town (GroupTarget): a column
// marching through another held town on its road neither drains it nor
// joins its garrison.
// A live squad stays live until it resolves, whatever the player's bubble
// does (it is never cached back to abstract): wiped out = repelled; the town
// reverts under it = it becomes the town's garrison (GarrisonCache adopts the
// group, the stored reserve tops it up to garrisonMax); still there after
// counterAttackLiveTimeout and out of the player's sight = it withdraws and
// its survivors return to the source (the timeout counts from the town edge;
// the road march gets its own allowance first, see MarchAllowance).
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
    bool enabled = true;             // counterAttack (0 = off)
    float size = 0.5f;               // counterAttackSize: the force, a fraction of groupSize
    float firstTownChance = 0.4f;    // counterAttackFirstTownChance: the first town ever taken (k = 0)
    float chance = 1.0f;             // counterAttackChance: every later town (k >= 1)
    float warLevelStep = 0.25f;      // counterAttackWarLevelStep: force * (1 + step * (warLevel - 1)), never below x1
    float delayMin = 120.0f;         // counterAttackDelayMin (s after the flip)
    float delayMax = 300.0f;         // counterAttackDelayMax
    float sourceKeep = 0.25f;        // counterAttackSourceKeep: share of garrisonMax a base keeps home
    float speed = 4.0f;              // counterAttackSpeed: abstract travel speed (m/s)
    float roadFactor = 1.3f;         // counterAttackRoadFactor: road distance / straight distance
    float defenderWeight = 3.0f;     // counterAttackDefenderWeight: one fighter in his town = this many attackers
    float repelLoss = 0.5f;          // counterAttackRepelLoss: share of a repelled force that does not come home
    float liveTimeout = 1200.0f;     // counterAttackLiveTimeout: s before an unresolved live squad withdraws
    int maxLive = 2;                 // counterAttackMaxLive: live squads out at once
    float birthMinToTarget = 250.0f; // counterAttackBirthMinToTarget: no birth point this close to the town (m)
    float birthGiveUp = 600.0f;      // counterAttackBirthGiveUp: s a force with no birth point waits, then turns back
};

// What the dispatch decision may look at besides the town and k.  The engine
// fills it from the live campaign (GarrisonCache groupSize, the script war
// level); a later occupier forces service can extend it.
struct CounterAttackContext
{
    int groupSize = 12;
    float warLevel = 1.0f;
    unsigned int seed = 0; // the flip's roll seed (MakeSeed)
};

struct CounterAttackDecision
{
    bool go = false;
    float chance = 0; // the dispatch probability used
    float roll = 0;   // the deterministic roll it was compared with
    int size = 0;     // planned bodies (0 when not going)
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

// How a squad appearing at a point would be perceived, Traffic's tiers
// (issue #53 T2): hidden survives the player turning round (beyond the
// object cull + exposeMargin, or behind terrain); cone-only is hidden only by
// the view cone.
enum CounterAttackExposure
{
    CAExposeSeen,     // in view, or inside the audible floor, or in the sea
    CAExposeConeOnly, // outside the +-60 deg view cone only
    CAExposeHidden,   // past the draw distance, or terrain-hidden
};

// Where a counter-attack force may be born without the player noticing.
// The engine gate (CounterAttack.cpp) is Traffic's perception gate (issue
// #53: PerceptionCamera, Traffic::EffectiveBand) with Traffic's config
// minSpawnDist as the hard audible floor and no headlights; it also refuses
// water.  Unit tests inject a predicate.
class CounterAttackGate
{
  public:
    virtual ~CounterAttackGate() = default;
    // CounterAttackExposure of a squad appearing at pos right now
    virtual int Exposure(Vector3Par pos) const = 0;
};

enum CounterAttackSpawnReason
{
    CASpawnNone,     // not instantiated (or restored from a save)
    CASpawnBase,     // at the road-snapped base
    CASpawnRoute,    // at a route point between the base and the town
    CASpawnGarrison, // a live garrison marched out of its base
    CASpawnDeferred, // nothing passed the gate: wait and retry
    CASpawnHold,     // the progress point is inside the bound: stay abstract at the town edge
    NCounterAttackSpawnReasons
};

// PickSpawnPoint's answer
struct CounterAttackSpawnPick
{
    int reason = CASpawnDeferred;
    Vector3 pos = VZero;
    float fraction = 0; // of the route length, where pos sits
};

// The world-touching half (engine implementation in CounterAttack.cpp).
// Null in unit tests: the live path then waits, nothing is spawned.  It is
// the perception gate too.
class CounterAttackOps : public CounterAttackGate
{
  public:
    // spawn n occupier bodies at from as one group in march order, sent on
    // firstLeg (the first road waypoint)
    virtual AIGroup* SpawnSquad(int n, Vector3Par from, Vector3Par firstLeg) = 0;
    // move up to n living bodies out of a zone's cache garrison into a new
    // group in march order sent on firstLeg; taken = bodies moved
    virtual AIGroup* DetachFromGarrison(int sourceZone, int n, Vector3Par firstLeg, int& taken) = 0;
    // living bodies and the leader's position (0 for a dead or null group)
    virtual int Alive(AIGroup* grp, Vector3& leaderPos) = 0;
    // one doMove leg, the posture unchanged
    virtual void MoveOn(AIGroup* grp, Vector3Par target) = 0;
    // at the town edge: combat posture, then into the town
    virtual void Assault(AIGroup* grp, Vector3Par town) = 0;
    // the road route from -> to as a polyline (engine road net); left empty
    // when there is none (the caller falls back to the straight line)
    virtual void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) = 0;
    // delete every body and the group
    virtual void Remove(AIGroup* grp) = 0;
    // hand the group to GarrisonCache as the town's garrison
    virtual void Adopt(int townZone, AIGroup* grp) = 0;
    // every living body of grp could vanish unnoticed right now (Traffic's
    // despawn rule, Traffic::CanExposeDespawn from the camera)
    virtual bool SquadUnseen(AIGroup* grp) const = 0;
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
    // a road route longer than MaxRoadDetour x the straight distance +
    // RoadDetourSlack is not walked: men on foot cut across instead
    static constexpr float MaxRoadDetour = 2.0f;
    static constexpr float RoadDetourSlack = 300.0f;
    // route sampling (m between points; the spawn walk tests each point)
    static constexpr float RouteStep = 50.0f;
    // a pick this close to the route start counts as the base itself
    static constexpr float BaseReach = RouteStep;
    // a point hidden only by the view cone is a birth point only this close
    // to the base (a squad leaving its own post is a plausible sight)
    static constexpr float ConeOnlyBaseRadius = 300.0f;
    // a marching squad unseen this long returns to the abstract road
    static constexpr float VanishHold = 10.0f;
    // live march: the next doMove leg ends this many route points ahead
    // (4 x 50 m), and a leg counts as reached this close to its end point
    static constexpr int WaypointStride = 4;
    static constexpr float WaypointReach = 50.0f;
    // the town edge: zoneArea plus this; there the squad deploys and moves in
    static constexpr float TownEdgeMargin = 100.0f;
    // march allowance: remaining road / MarchSpeed + MarchSlack seconds
    // before counterAttackLiveTimeout starts counting
    static constexpr float MarchSpeed = 1.0f;
    static constexpr float MarchSlack = 120.0f;

    // live squad legs (transient: a reload re-derives it from the leader)
    enum MarchLeg
    {
        LegUnknown,
        LegRoad, // marching the road route
        LegTown  // at the town edge or inside
    };

    struct Record
    {
        RString target;
        RString source;
        int phase = CAScheduled;
        // Scheduled / Travel: seconds left; Live: seconds since the town edge
        // (negative: the road march's allowance left)
        float clock = 0;
        float travel = 0;       // Travel: the whole road leg (s); progress = (travel - clock) / travel
        bool wholeRoad = false; // Travel: launched with the town in the bubble (birth anywhere on the road)
        float deferTime = 0;    // Travel: seconds without a birth point (BirthGiveUp)
        int strength = 0;       // bodies drawn from the source (0 while scheduled)
        int size = 0;           // planned force at the flip
        int townsBefore = 0;    // k: towns the resistance had taken before this flip
        unsigned int seed = 0;
        int salt = 2;              // next roll salt (0 dispatch, 1 delay)
        float resend = 0;          // Live: seconds to the next re-send check
        bool deferLogged = false;  // the budget deferral WARN went out
        bool hiddenLogged = false; // the perception deferral WARN went out
        LLink<AIGroup> group;      // Live only
        // transient (rebuilt after a load) ---------------------------------
        float retry = 0;          // Travel / Holding: seconds to the next live attempt
        AutoArray<Vector3> route; // the road route, base to town, at most RouteStep apart
        int leg = LegUnknown;     // Live: MarchLeg
        int wp = 0;               // Live, LegRoad: the route point the current leg ends at
        bool suspended = false;   // Holding: the town is in the bubble, the force does nothing
        float unseenFor = 0;      // Live, LegRoad: seconds every body has been out of perception
        int spawnReason = CASpawnNone;
        Vector3 spawnPos = VZero; // where the live squad was instantiated
        LSError Serialize(ParamArchive& ar);
    };

    // lifecycle ---------------------------------------------------------------
    void Clear();
    void InitMission(); // Clear + read the CfgGuerrillaZones keys
    void LoadFromParams(const ParamEntry* zonesCfg);
    const CounterAttackTuning& Tuning() const { return _tuning; }
    CounterAttackTuning& TuningMutable() { return _tuning; }

    // pure rules (unit-tested) ------------------------------------------------
    // the largest force: one group (MAX_UNITS_PER_GROUP)
    static constexpr int MaxForce = 12;
    // counterAttackFirstTownChance for k = 0, counterAttackChance after; 0 when off
    static float DispatchChance(const CounterAttackTuning& t, int townsBefore);
    // counterAttackSize * groupSize * max(1, 1 + warLevelStep * (warLevel - 1)),
    // rounded, 1..MaxForce
    static int ForceSize(const CounterAttackTuning& t, int groupSize, float warLevel);
    // the dispatch decision for one town: go / no-go and the force size (the
    // seam a later occupier forces service would wrap or replace)
    static CounterAttackDecision Decide(const CounterAttackTuning& t, const char* town, int townsBefore,
                                        const CounterAttackContext& ctx);
    // probability the town holds: D*w / (A + D*w); 0 with no defenders, 1
    // with no attackers
    static float HoldOdds(const CounterAttackTuning& t, int defenders, int attackers);
    // straightDistance * counterAttackRoadFactor / counterAttackSpeed, at
    // least MinTravel: the whole road leg from the base to the town
    static float TravelTime(const CounterAttackTuning& t, float straightDistance);
    // route geometry (2D lengths) ---------------------------------------------
    static float RouteLength(const AutoArray<Vector3>& route);
    // the point at fraction (0 = start, 1 = end) of the route's length
    static Vector3 RoutePoint(const AutoArray<Vector3>& route, float fraction);
    // in resampled at most step apart (every input vertex kept)
    static void ResampleRoute(const AutoArray<Vector3>& in, float step, AutoArray<Vector3>& out);
    // the birth rule.  launch (no progress yet): candidates are the route
    // points from the base (route start) toward the town.  Otherwise: the
    // point at progress, then the route points behind it back to the base
    // (CASpawnHold when the progress point is already within minToTarget of
    // the town).  Never a point within minToTarget of the route's end (the
    // town).  The first CAExposeHidden candidate wins; else the first
    // CAExposeConeOnly one within ConeOnlyBaseRadius of the base; else
    // CASpawnDeferred.  CASpawnBase within BaseReach of the start, else
    // CASpawnRoute.
    static CounterAttackSpawnPick PickSpawnPoint(const AutoArray<Vector3>& route, float progress, bool launch,
                                                 float minToTarget, const CounterAttackGate& gate);
    // the fraction of the route's length at its point nearest pos
    static float RouteFractionNear(const AutoArray<Vector3>& route, Vector3Par pos);
    // a live squad back to the abstract road at fraction of its route with
    // alive bodies: TRAVEL, clock = travel * (1 - fraction) (travel = the
    // whole leg, set from the base distance when the squad never travelled)
    static void ReturnToRoad(Record& r, float fraction, int alive, float wholeTravel);
    // the road route is walked when it is no longer than MaxRoadDetour x the
    // straight distance + RoadDetourSlack
    static bool RoadIsReasonable(float roadLength, float straightDistance);
    // a travelling force's progress along its road: (travel - clock) / travel
    static float Progress(const Record& r);
    // seconds the live march may take from fraction to the town
    static float MarchAllowance(const AutoArray<Vector3>& route, float fraction);
    // a live counter-attack squad counts as occupier presence only in its
    // own target town; groupTarget -1 = not a counter-attack group
    static bool CountsInZone(int groupTarget, int zone) { return groupTarget < 0 || groupTarget == zone; }
    // the target zone of the live counter-attack squad grp, -1 when grp is
    // not one (ZoneRegistry's occupier census)
    int GroupTarget(const ZoneRegistry& registry, const AIGroup* grp) const;
    static const char* SpawnReasonName(int reason);
    // bodies a base can spare: (stored + live) - floor(garrisonMax * keep),
    // drawn from the live garrison when useLive, else from the stored reserve
    static int Spare(const CounterAttackTuning& t, float stored, int live, float garrisonMax, bool useLive);
    // deterministic uniform [0, 1) from a seed and a salt
    static float Roll01(unsigned int seed, int salt);
    static unsigned int MakeSeed(const char* town, int flipDay, int flipMinute, int townsBefore);

    // the service ---------------------------------------------------------------
    // a CITY just flipped to the resistance: count it, decide and schedule.
    // True when a counter-attack was scheduled.
    bool OnTownFlipped(const ZoneRegistry& registry, int zoneIndex, int groupSize, float warLevel = 1.0f);
    // send a force of up to size bodies from an explicit source zone on a
    // held town NOW (no delay, no roll): the launch seam for a later occupier
    // forces service.  The source's spare still caps it.  True when a force
    // left (a record is added); false when the town is not a valid target or
    // already has a record, or the source cannot spare a man or cannot march
    // its live garrison out.
    bool LaunchFrom(ZoneRegistry& registry, const CounterAttackView& view, int target, int source, int size,
                    CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired);
    // towns the resistance took so far (non-exempt flips counted by
    // OnTownFlipped; saved).  k of the next flip is max(this, towns held).
    int TownsTaken() const { return _townsTaken; }
    void SetTownsTaken(int n) { _townsTaken = n > 0 ? n : 0; }
    // non-exempt CITY zones the resistance holds now
    static int HeldTowns(const ZoneRegistry& registry);
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

    bool HasState() const { return _records.Size() > 0 || _townsTaken > 0; }
    LSError Serialize(ParamArchive& ar) override;

  private:
    void LoadFromConfig();
    // nearest occupier military zone with bodies to spare; -1 when none
    int PickSource(const ZoneRegistry& registry, const CounterAttackView& view, int target, bool allowLive, int& spare,
                   bool& live) const;
    // r's road route (built once per record: ops' road net, else the
    // straight line from the source to the town), at most RouteStep apart
    static const AutoArray<Vector3>& RouteOf(const ZoneRegistry& registry, Record& r, CounterAttackOps* ops);
    // a travelling force must be live now: the town is in the bubble, or
    // its progress point is
    bool MustGoLive(const ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                    CounterAttackOps* ops) const;
    // the live squad's road march: legs, the town edge, re-sends
    static void March(const ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, Vector3Par leader,
                      float elapsed);
    // survivors go home, capped at the base's strength
    static void ReturnToSource(ZoneRegistry& registry, const CounterAttackView& view, const Record& r, int bodies);
    bool InBubble(const ZoneRegistry& registry, const CounterAttackView& view, int zone) const;
    int LiveSquads() const;
    // room for one more live group (live squads and the side's group budget)
    bool RoomLive(const ZoneRegistry& registry, const CounterAttackView& view) const;
    // send r's force of n from src: live (march the base's garrison out) or
    // abstract (debit the stored reserve, travel).  False = not launched (a
    // live detach failed); the caller retries or gives up.
    bool Launch(ZoneRegistry& registry, int target, int src, int n, bool live, CounterAttackOps* ops, Record& r,
                AutoArray<ZoneEventRecord>& fired);
    // the abstract fight on arrival: true when the record is finished
    bool Fight(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
               AutoArray<ZoneEventRecord>& fired);
    // GoLive outcomes
    enum BirthResult
    {
        BirthDone,    // the squad is live
        BirthWait,    // budget, spawn failure or no world: retry
        BirthNoPoint, // nothing on the road passes the gate: retry, counts toward BirthGiveUp
        BirthHold     // the progress point is inside the bound: stay abstract at the town edge
    };
    // birth the live squad at its first unseen road point up to fraction
    // (PickSpawnPoint)
    int GoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops,
               float fraction, AutoArray<ZoneEventRecord>& fired);
    // one record's step: true when it is finished
    bool Advance(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops, Record& r,
                 AutoArray<ZoneEventRecord>& fired);
    // a live squad in the town it retook becomes the town's garrison
    static void AdoptLive(ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, int alive);

    CounterAttackTuning _tuning;
    AutoArray<Record> _records;
    float _accum = 0;
    int _townsTaken = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
