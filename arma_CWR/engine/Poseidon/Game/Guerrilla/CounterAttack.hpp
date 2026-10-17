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
// for distance * forcesRoadFactor / counterAttackSpeed seconds along
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
// seconds it turns back and its strength returns to the base (the key off;
// with occupierForces on the birth chain below replaces the turn-back).
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
// reserve, and D2 recovery regrows the rest.  A live squad is occupier
// presence wherever it stands (OCCUPIER-FORCES.md D5): a column marching
// through another held town on its road drains that town too.  A return to
// a base inside the player's bubble is a pending deposit (S5,
// GarrisonCache::AddReserve), never bodies at his feet.
//
// THE HQ (OCCUPIER-FORCES.md 9.1, N4a).  The town the player's headquarters
// stands in (GuerrillaBase::HqZone) is never a target: OnTownFlipped and
// LaunchFrom refuse it, and a scheduled force whose town became the HQ's is
// cancelled at launch.
// A live squad stays live until it resolves, whatever the player's bubble
// does (it is never cached back to abstract): wiped out = repelled; the town
// reverts under it = it becomes the town's garrison (GarrisonCache adopts the
// group, and its bodies come off the revert refill, SubtractAdopted); still
// there after
// counterAttackLiveTimeout = it withdraws (the timeout counts from the town
// edge; the road march gets its own allowance first, see MarchAllowance).
//
// BREAK-OFF (decision 4, the feel assessment's item 7; occupierForces on or a
// funded record).  A live RETAKE, BASE or HQ_ASSAULT whose living infantry
// falls below a third of the strength it was born with (BreaksOff:
// alive x BreakOffDivisor < bornStrength) while the target is still the
// resistance's withdraws at once; the lone survivor of a squad is the same
// rule.  A withdrawal (break-off, live timeout, the HQ moved) settles the
// record NOW: one CAOpRepelled with its reason (CAERBrokenOff /
// CAERWithdrew; the director counts it as a repel), the losses booked, and
// the survivors walk back toward their source as a withdrawing Leftover
// (re-sent home at every check, removed only away from the player and
// unseen, then home as bodies through CrewHome, the dead among them lost).
// The hulls never leave before the infantry: infantry still seated rides
// home in its carrier (the HomeHull carries it as cargo, deleted cargo
// first); infantry on foot is waited for, each hull holding where it stands
// (an armed one still supporting) until no living survivor is within
// ClearRadius of the target or WithdrawHullWait (120 s) has passed, then it
// drives home.  A top-up squad whose town was lost (M5) keeps the same
// bound on its hulls.
//
// DETERMINISM.  Every roll (dispatch, delay, odds) is a hash of a seed fixed
// at the flip (town name, flip stamp, towns held) and a salt, so a save and
// reload never changes an outcome and the off-screen rules draw no engine
// random numbers.
//
// OCCUPIER FORCES (guerrilla-mode/OCCUPIER-FORCES.md phase 3, the movement
// half; CounterAttackOrders.hpp is the contract with the director).
//   Records carry kind / origin / package / opId / price and hull refs
//   (serialized; an old save loads as CAKRetake / CAOLegacy / FPFoot, opId
//   0).  LaunchFunded starts a director operation from a FIXED source; the
//   legacy path (OnTownFlipped -> Decide -> schedule) is unchanged in
//   outcome.  A listener (the director) hears every flip first (the AWAKE
//   hook: it may take the flip, then no legacy record is scheduled) and
//   every state change of every record (CounterAttackOpEvent; legacy records
//   report with origin CAOLegacy and opId 0).  The record ends with exactly
//   one TERMINAL event: Repelled, Settled, CancelledUnseen or Aborted; a
//   Returned (survivors written to a base or the pool) comes before it, and
//   a hull's HullRefund / HullLost may come after it (an intact hull drives
//   home and is deleted only where nobody sees it).
//   With occupierForces on (CounterAttackTuning::forcesOn, read here from the
//   same key OccupierForces reads):
//   * real speeds (3.2): abstract travel at the record's abstractSpeed, or
//     FootSpeed for a legacy record, over the real road length (straight
//     distance x forcesRoadFactor when no road route is known); the
//     gmCounterAttackTune "speed" lever, once set at run time, wins over
//     both (a TEST lever that keeps the tri windows short);
//   * resolution (6.5): an arrival at a town with resistance defenders goes
//     LIVE (born under S at the 250 m bound, even off-screen); over budget
//     it waits at the town edge doing nothing; no abstract Fight; an empty
//     town is held and drained as before;
//   * the birth chain (3.1, D6) for every record, funded or legacy (the
//     director DORMANT): at launch, on the road, then AT ONCE the next base
//     back whose route has a point the gate passes (re-source: the old base
//     refunded, the new one debited, CAOpResourced; nearest the target first,
//     a base whose road is watched too passed over), and only when no base
//     passes a wait of counterAttackBirthGiveUp, then cancel unseen
//     (CAOpCancelledUnseen).  The turn-back stays with the key off only;
//   * a holding force settles as the reverted town's garrison (5.4,
//     ZoneRegistry::RevertTown via ZoneTickInputs::settleOcc); a travelling
//     force whose town reverts under it tops the town up (CAKReinforce) when
//     its garrison is short, else goes home.
//   Pool routing (5.5) goes through OccupierForces::Instance(), which passes
//   everything through while inactive.  Ground hulls (FPTruck now; one hull
//   class, crew, cargo): created on the road at the birth point, crew a
//   group of their own, infantry boarded as cargo, driven with Command::Move
//   to the driver, the infantry dismounts at counterAttackBirthMinToTarget;
//   deleted with no refund on a return to abstract travel and re-created at
//   the next birth.  Born operations (forcesOpsMax) and live hulls
//   (forcesLiveHullsMax) gate births, an arrival birth at the town edge
//   too (6.5: its hulls are born with it); a birth over them waits.  After
//   CampaignEnd's win latch nothing launches or steps (CounterAttackView::
//   won) and RevertTown refuses.
//
// Script surface (CounterAttackCommands.cpp): gmCounterAttacks,
// gmCounterAttackTune, gmCounterAttackLaunchFunded (test lever).  Events
// (ZoneRegistry handler slots): counterAttack when a force reaches a town
// (live squad sent in, or abstract arrival) and counterRepelled when one is
// beaten off or withdraws.  Config keys (CfgGuerrillaZones): counterAttack
// and every counterAttack* key in CounterAttackTuning, plus occupierForces
// and forcesLiveHullsMax (forcesOpsMax is OccupierForces' key: OpsCap).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttackOrders.hpp>

class ParamArchive;
namespace Poseidon
{
class AIGroup;
class Entity;
} // namespace Poseidon

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;
struct ZoneEventRecord;
struct ZoneRecord;

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
    // (counterAttackRoadFactor is retired: every leg with no road route uses
    // the quote's forcesRoadFactor, QuoteRoadFactor)
    float defenderWeight = 3.0f;     // counterAttackDefenderWeight: one fighter in his town = this many attackers
    float repelLoss = 0.5f;          // counterAttackRepelLoss: share of a repelled force that does not come home
    float liveTimeout = 1200.0f;     // counterAttackLiveTimeout: s before an unresolved live squad withdraws
    int maxLive = 2;                 // counterAttackMaxLive: live squads out at once
    float birthMinToTarget = 250.0f; // counterAttackBirthMinToTarget: no birth point this close to the town (m)
    float birthGiveUp = 600.0f;      // counterAttackBirthGiveUp: s a force with no birth point waits, then turns back
    // occupier forces (OCCUPIER-FORCES.md 16) ---------------------------------
    bool forcesOn = false;      // occupierForces: real speeds, key-on resolution, settle on revert
    int forcesOpsMax = 2;       // the default OpsCap uses while OccupierForces is inactive (not read from config)
    int forcesLiveHullsMax = 6; // forcesLiveHullsMax (0..20): live hulls at once
    // phase 6A air (OCCUPIER-FORCES.md 3.1, 3.2, 16)
    float sortieTime = 300.0f; // forcesSortieTime (60..1800 s): a strike's time over its target
    int offMapEdge = -1;       // forcesOffMapEdge: -1 the edge nearest the target, else CAEdgeNorth..CAEdgeWest
    // transient test lever: gmCounterAttackTune "speed" was set at run time,
    // so `speed` (with roadFactor) wins over the real speeds
    bool speedLever = false;
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

// forcesOffMapEdge (3.1): the parent nation's side of the map, where
// off-map air is born and goes home to
enum CounterAttackMapEdge
{
    CAEdgeNorth, // high Z
    CAEdgeSouth, // Z = 0
    CAEdgeEast,  // high X
    CAEdgeWest,  // X = 0
    NCounterAttackMapEdges
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
    // the zone the player's HQ stands in (GuerrillaBase::HqZone), -1 when
    // none: never a target (N4a)
    int hqZone = -1;
    // budget census (GatherSimDiag): live AI bodies, all sides, and occupier groups
    int liveBodies = 0;
    int occGroups = 0;
    // CampaignEnd's win latch (7.4): nothing launches, nothing steps
    bool won = false;
    // CampaignEnd's lose latch (9.2): the same freeze
    bool lost = false;
    // the HQ (OCCUPIER-FORCES.md 9.1; GuerrillaBase::IsEstablished / HqPos):
    // an HQ_ASSAULT record marches on hqPos and resolves there.  hqDefenders
    // is the resistance HOLD presence within zoneArea of hqPos (hold squads,
    // recruits, companions, the player per PlayerHoldsPresence): the HQ's
    // "defenders present" for the arrival rule (6.5), never the HQ meter
    bool hqEstablished = false;
    Vector3 hqPos = VZero;
    int hqDefenders = 0;
    // phase 6A: per-zone AlertMachine state (AlertState; empty = unknown,
    // read as below RED) and the map's side length in metres (0 = unknown:
    // no off-map edge, air is born at its airfield only)
    AutoArray<int> alertState;
    float mapSize = 0;
};

// phase 6A: what CounterAttack reads back about one of its airframes
// (OccupierAir's records through CounterAttackOps::AirStatus)
struct CounterAttackAirStatus
{
    bool sortie = false;     // a strike sortie (else an insertion)
    int state = 0;           // AirInsertionState / AirSortieState
    bool landed = false;     // insertion: touched down at least once
    float landedOffset = -1; // insertion: 2D m from the LZ at touchdown
    Vector3 landedAt = VZero;
    int failReason = 0;      // AirFailReason
    bool loiter = false;     // sortie: reached its target area (orbiting, or past it)
    bool crewRemoved = false;
    bool gone = false;       // crew and hull removed, or the hull lost
    bool hullLost = false;   // destroyed (or deleted under it)
    int cargoBorn = 0;       // insertion: bodies seated at birth
    int cargoSeated = 0;     // ... living, still aboard (last step)
    int crewAlive = 0;       // living crew aboard (last step)
    // the airframe's position while its hull exists (the far census)
    bool hasPos = false;
    Vector3 pos = VZero;
};

// The far census (gmSimDiag, [gmsim]; the abstract-director assessment's
// step 1): this service's live occupier groups with a living member standing
// beyond cacheRadius + GarrisonCache::DespawnHysteresis (850 m) of the
// player, by why they are still in the world.  An airframe counts once (its
// crew); a group is counted under the first cause in field order below
// that holds it.
struct CounterAttackFarCensus
{
    int leftover = 0; // Leftover groups with living stragglers (M3, L7)
    int homeHull = 0; // crews of intact hulls driving home
    int air = 0;      // airframes: an insertion or sortie in flight, or flying home
    int arrival = 0;  // a live arrival fight (Record::arrivalLive: born at the town edge against defenders)
    int march = 0;    // any other born operation group (on its road, at its town, withdrawing)
    int Total() const { return leftover + homeHull + air + arrival + march; }
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
    // move up to n living non-leaders of grp (on foot) into a new group in
    // march order sent on firstLeg; taken = bodies moved.  Default: no
    // split (null), the caller keeps the group whole.
    virtual AIGroup* SplitOff(AIGroup* grp, int n, Vector3Par firstLeg, int& taken)
    {
        (void)grp, (void)n, (void)firstLeg;
        taken = 0;
        return nullptr;
    }
    // every living body of grp could vanish unnoticed right now (Traffic's
    // despawn rule, Traffic::CanExposeDespawn from the camera)
    virtual bool SquadUnseen(AIGroup* grp) const = 0;

    // ground hulls (OCCUPIER-FORCES.md 3.2).  Defaults do nothing, so a
    // world without hulls (the unit tests' fakes) never mounts a force.
    // A crewed hull of hullClass on the road at from, facing dir, its crew
    // (crew bodies) a group of their own, and n infantry in a second group
    // boarded as its cargo (the Traffic idiom).  Returns the infantry group,
    // or null when the hull or its driver could not be created (nothing left
    // in the world).  hull / crewGroup are set on success.
    virtual AIGroup* SpawnMounted(const RString& hullClass, int crew, int n, Vector3Par from, Vector3Par dir,
                                  Entity*& hull, AIGroup*& crewGroup)
    {
        (void)hullClass, (void)crew, (void)n, (void)from, (void)dir;
        hull = nullptr;
        crewGroup = nullptr;
        return nullptr;
    }
    // one Command::Move to the hull's driver through IssueCommand
    virtual void DriveOn(AIGroup* crewGroup, Vector3Par target) { (void)crewGroup, (void)target; }
    // the infantry of grp gets out of the hull (cargo of another group)
    virtual void Dismount(AIGroup* grp, Entity* hull) { (void)grp, (void)hull; }
    // an armed hull at the town edge (6.5, phase 5; ForcePackages::EdgeRole
    // FERSupport): its crew AWARE and firing, the hull driven up to edgePos
    // through the driver (the Traffic Move idiom).  A truck never gets it.
    virtual void Support(AIGroup* crewGroup, Vector3Par edgePos) { (void)crewGroup, (void)edgePos; }
    // a column's escort hull (6.5, phase 5): a crewed hull of hullClass on
    // the road at from facing dir, its crew (crew bodies) a group of their
    // own, no cargo.  Returns the hull, or null when the hull or its driver
    // could not be created (nothing left in the world); crewGroup is set on
    // success.
    virtual Entity* SpawnEscort(const RString& hullClass, int crew, Vector3Par from, Vector3Par dir,
                                AIGroup*& crewGroup)
    {
        (void)hullClass, (void)crew, (void)from, (void)dir;
        crewGroup = nullptr;
        return nullptr;
    }
    enum HullStatus
    {
        HullGone,      // the link is dead (deleted)
        HullDestroyed, // wrecked
        HullIntact
    };
    // the hull's HullStatus; crewAlive = living crewmen of crewGroup
    virtual int HullState(Entity* hull, AIGroup* crewGroup, int& crewAlive) const
    {
        (void)hull, (void)crewGroup;
        crewAlive = 0;
        return HullGone;
    }
    // the hull's speed (m/s); 0 without a hull
    virtual float HullSpeed(Entity* hull) const
    {
        (void)hull;
        return 0;
    }
    // living bodies of grp within radius (2D) of centre.  The default counts
    // every living body when the leader stands within radius (the unit
    // tests' fakes); the engine counts body by body.
    virtual int AliveWithin(AIGroup* grp, Vector3Par centre, float radius)
    {
        Vector3 leader = VZero;
        const int alive = Alive(grp, leader);
        const float dx = leader.X() - centre.X();
        const float dz = leader.Z() - centre.Z();
        return alive > 0 && dx * dx + dz * dz <= radius * radius ? alive : 0;
    }
    // the hull and every member of crewGroup (living or dead, seated or
    // not) could vanish unnoticed right now (M4: a crewman who bailed out
    // next to the player keeps the whole crew in the world)
    virtual bool HullUnseen(Entity* hull, AIGroup* crewGroup) const
    {
        (void)hull, (void)crewGroup;
        return false;
    }
    // every member of grp, living or dead, could vanish unnoticed right now
    // (the leftover cleanup deletes corpses too).  Default: SquadUnseen.
    virtual bool BodiesUnseen(AIGroup* grp) const { return SquadUnseen(grp); }
    // anyone sits in the hull (crew or cargo, the player included): an
    // abandoned hull is never deleted from under him.  Default: nobody.
    virtual bool HullOccupied(Entity* hull) const
    {
        (void)hull;
        return false;
    }
    // the gate's mode for the next Exposure calls: on, a ground hull is
    // being born (its headlights show at night, M8: the night light band);
    // off, men on foot.  Default: nothing (the unit tests' fakes).
    virtual void SetHullGate(bool on) { (void)on; }
    // delete the crew FIRST (deleting a transport ejects its crew), then the hull
    virtual void RemoveHull(Entity* hull, AIGroup* crewGroup) { (void)hull, (void)crewGroup; }

    // phase 6A air (OccupierAir).  Defaults: a world without air (the plain
    // unit-test fakes), where every air birth fails and the force goes on
    // under the ground rules instead.
    // the altitude-aware gate (OccupierAir::ExposureAt): posAsl.Y is the
    // height above sea level.  Default: the ground gate.
    virtual int AirExposure(Vector3Par posAsl) const { return Exposure(posAsl); }
    // the surface height (sea = 0)
    virtual float GroundY(float x, float z) const
    {
        (void)x, (void)z;
        return 0;
    }
    // an LZ 300..500 m from target, the side away from awayFrom first
    virtual bool AirPickLz(Vector3Par target, const Vector3* awayFrom, Vector3& lz)
    {
        (void)target, (void)awayFrom;
        lz = VZero;
        return false;
    }
    // a transport helicopter born airborne at birth (altAgl above ground),
    // n infantry as cargo of a second group, flown to lz, unloading on
    // target, home = birth.  Returns its OccupierAir id (0 = failed, nothing
    // left in the world) and sets cargo / hull / crewGroup.
    virtual int AirInsert(const RString& hullClass, int n, Vector3Par birth, float altAgl, Vector3Par lz,
                          Vector3Par target, AIGroup*& cargo, Entity*& hull, AIGroup*& crewGroup)
    {
        (void)hullClass, (void)n, (void)birth, (void)altAgl, (void)lz, (void)target;
        cargo = nullptr;
        hull = nullptr;
        crewGroup = nullptr;
        return 0;
    }
    // a jet or attack helicopter sortie over target for sortieTime
    virtual int AirSortie(const RString& hullClass, Vector3Par birth, float altAgl, Vector3Par target,
                          float sortieTime, Entity*& hull, AIGroup*& crewGroup)
    {
        (void)hullClass, (void)birth, (void)altAgl, (void)target, (void)sortieTime;
        hull = nullptr;
        crewGroup = nullptr;
        return 0;
    }
    // the airframe's status; false = unknown id (dropped or never born)
    virtual bool AirStatus(int airId, CounterAttackAirStatus& out) const
    {
        (void)airId;
        out = CounterAttackAirStatus();
        return false;
    }
    // the limited reveal (3.2, D9): reported fighters near pos, sighted
    // ageSeconds ago, to the sortie's crew (OccupierAir::RevealAllowed).
    // Returns the targets revealed.
    virtual int AirRevealNear(int airId, Vector3Par pos, float ageSeconds)
    {
        (void)airId, (void)pos, (void)ageSeconds;
        return 0;
    }
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
    // phase 6A: s an alarm answer holds quiet (below RED, no resistance
    // presence) at the reported position before it settles (8.2)
    static constexpr float AlarmQuietHold = 60.0f;
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
    // the break-off (decision 4, item 7): a live operation whose living
    // infantry x BreakOffDivisor is below its born strength withdraws (below
    // a third)
    static constexpr int BreakOffDivisor = 3;
    // a withdrawing force's hulls wait at most this long (s) for the infantry
    // on foot to clear the target's edge (ClearRadius), then drive home
    static constexpr float WithdrawHullWait = 120.0f;
    // real speeds (OCCUPIER-FORCES.md 3.2; constants, occupierForces on
    // only): men on foot 1.5 m/s, a ground hull 0.6 x and an aircraft 0.8 x
    // its config maxSpeed (km/h, converted to m/s)
    static constexpr float FootSpeed = 1.5f;
    static constexpr float GroundCruise = 0.6f;
    static constexpr float AirCruise = 0.8f;
    // the abstract speed of a package (m/s): FootSpeed for foot (or an
    // unknown maxSpeed), else cruise x maxSpeedKmh / 3.6
    static float RealSpeed(int package, float maxSpeedKmh);

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
        // occupier forces (OCCUPIER-FORCES.md 14 items 1 and 14; serialized,
        // an old save reads the defaults: a legacy foot retake, opId 0)
        int opId = 0;             // the director's operation id (0: legacy)
        int kind = CAKRetake;     // CounterAttackKind
        int origin = CAOLegacy;   // CounterAttackOrigin
        int package = FPFoot;     // ForcePackage
        int priceUnits = 0;       // what the director debited (hull + crew)
        RString hullClass;        // empty for foot
        int crew = 0;             // the hull's crew bodies (re-created with it at the next birth)
        bool urgent = false;      // the urgency flag at launch
        bool funded = false;      // a director operation with the key on (birth chain, pool routing)
        float abstractSpeed = 0;  // m/s for the abstract leg (0: legacy pacing, FootSpeed with the key on)
        float lethality = 1.0f;   // A_lethal per body (6.5, item 12): odds use strength x this
        bool arrivalLive = false; // born at the town edge to fight present defenders (never returns to the road)
        bool hullDone = false;    // the hull was settled (lost, refunded or cancelled): no further hull event
        bool dismounted = false;  // Live with a hull: the infantry got out
        RString tried;            // bases the birth chain already used, ';'-joined (D6)
        LLink<Entity> hull;       // the live hull (second-pass ref; the hull rides the vehicle serializer)
        LLink<AIGroup> crewGroup; // its crew (counts 1 against the group budget)
        // phase 5 (serialized; absent in older saves: no escort).  A column's
        // escort hull (6.5): born behind the carrier, driven with it,
        // supporting at the town edge.  priceUnits is the carrier's share
        // only; the escort's hull + crew are escortPriceUnits.
        RString escortClass;              // empty: no escort
        int escortCrew = 0;               // its crew bodies (re-created with it at the next birth)
        int escortPriceUnits = 0;         // escort hull + crew
        bool escortDone = false;          // the escort was settled (lost, refunded or cancelled)
        LLink<Entity> escort;             // the live escort hull (second-pass ref)
        LLink<AIGroup> escortCrewGroup;   // its crew (counts 1 against the group budget)
        // phase 4 (serialized): the explicit march destination (an
        // HQ_ASSAULT's HqPos; VZero = the target zone's centre), and an
        // HQ_ASSAULT whose HQ fell: its live squad leaves the world unseen,
        // its bodies to the pool
        Vector3 destPos = VZero;
        bool hqDone = false;
        // serialized (absent: false): a live top-up squad whose town was
        // lost walks home and leaves the world only unseen (M5)
        bool withdrawing = false;
        // the break-off (serialized; absent: 0, set from strength at the
        // next live step): the infantry bodies the operation was first born
        // with, and the seconds a withdrawing top-up squad (M5) has been
        // walking home (its hulls wait at most WithdrawHullWait for it)
        int bornStrength = 0;
        float withdrawClock = 0;
        // phase 6A (serialized; absent in older saves: no air, no strike, no
        // alarm).  CounterAttackOrders.hpp documents each field.
        int airOrigin = -1;       // an occupier-held AIRFIELD zone index, -1 = off-map
        int strikeZone = -1;      // CAKStrike: the struck zone (-1 none)
        Vector3 strikePos = VZero; // CAKStrike: the reported position or the town centre
        int alarmZone = -1;       // CAKAlarm: the RED zone answered
        Vector3 alarmPos = VZero; // CAKAlarm: the reported position (VZero = the zone centre)
        bool struck = false;      // CAKStrike: CAOpStruck went out (once per op)
        bool answered = false;    // CAKAlarm: CAOpAlarmAnswered went out
        int airId = 0;            // the live airframe's OccupierAir id (0 none)
        Vector3 airLz = VZero;    // an insertion's LZ (its squad's route starts there)
        Vector3 airFrom = VZero;  // where its straight flight line starts (airfield or off-map edge)
        int airReplans = 0;       // failed insertions re-planned (once, 3.2)
        bool airLanded = false;   // CAOpLanded went out for the current airframe
        // the air re-plan of the birth chain (3.1 step 6) bought the
        // airframe and no ledger row took it (OccupierForces::SpendOnOp: no
        // row, or a save older than the ledger booking): its price is
        // refunded (or lost) straight to the pool.  A re-plan the row took
        // leaves this false and settles like any hull, through the row.
        bool airReplanned = false;
        float alarmQuiet = 0;     // CAKAlarm, live at the alarm: s below RED with no contact
        float revealClock = 0;    // CAKStrike, live: s to the next limited reveal
        // transient (rebuilt after a load) ---------------------------------
        // CounterAttackEndReason of the withdrawal that ends the record this
        // step (Withdraw); SendHullHome reads it: the hulls wait for the
        // infantry, or carry it home
        int withdrawReason = CAERNone;
        bool edgeHold = false;    // Travel: the last birth attempt was BirthHold (at the town edge)
        bool escortWait = false;  // Live, mounted: the carrier waits for its escort to close up
        float retry = 0;          // Travel / Holding: seconds to the next live attempt
        AutoArray<Vector3> route; // the road route, base to town, at most RouteStep apart
        int leg = LegUnknown;     // Live: MarchLeg
        int wp = 0;               // Live, LegRoad: the route point the current leg ends at
        bool suspended = false;   // Holding: the town is in the bubble, the force does nothing
        float unseenFor = 0;      // Live, LegRoad: seconds every body has been out of perception
        float stopping = -1;      // Live, mounted at the edge: seconds the hull has been braking (-1 not yet)
        int spawnReason = CASpawnNone;
        Vector3 spawnPos = VZero; // where the live squad was instantiated
        bool routeIsRoad = false; // route came from the road net (no roadFactor on its length)
        // the hull's share of priceUnits (the crew are bodies, priced 1 each)
        int HullPrice() const { return priceUnits - crew > 0 ? priceUnits - crew : 0; }
        bool HasHullPackage() const { return hullClass.GetLength() > 0; }
        bool HasEscort() const { return escortClass.GetLength() > 0; }
        bool IsAir() const { return package == FPTransportHeli || package == FPAttackHeli || package == FPJet; }
        int EscortHullPrice() const { return escortPriceUnits - escortCrew > 0 ? escortPriceUnits - escortCrew : 0; }
        // the pool units the director is still owed for unsettled hulls:
        // the carrier's priceUnits while its hull is unsettled, plus the
        // escort's escortPriceUnits while it is (crew already booked lost or
        // refunded are no longer in either: priceUnits drops with crew)
        int OwedUnits() const
        {
            return (HasHullPackage() && !hullDone ? priceUnits : 0) + (HasEscort() && !escortDone ? escortPriceUnits : 0);
        }
        LSError Serialize(ParamArchive& ar);
    };

    // an airframe whose insertion is done (unloaded, or the record ended):
    // OccupierAir flies it home and removes it crew first where unseen;
    // then CAOpHullRefund (crew home as bodies), or CAOpHullLost.  Serialized.
    struct AirHome
    {
        int opId = 0;
        int kind = CAKRetake;
        int origin = CAOLegacy;
        RString target;
        RString source;
        int units = 0;       // the airframe's price
        int crew = 0;        // crew bodies
        int airId = 0;
        bool direct = false; // airReplanned (no ledger row): refund / loss straight to the pool
        LSError Serialize(ParamArchive& ar);
    };

    // what a lost hull or a wiped squad leaves in the world (M3, L7): an
    // abandoned intact hull, a crew or squad group whose members are dead or
    // stragglers.  Each part is deleted where nobody sees it (an abandoned
    // hull by HullUnseen, a group by BodiesUnseen, living stragglers only
    // away from the player); a wreck stays.  Living stragglers go home as
    // bodies (CrewHome) when removed; those who die meanwhile are lost.
    // Serialized.
    struct Leftover
    {
        RString source;  // where living stragglers go home
        int living = 0;  // living members when it was left (booked when it ends)
        float retry = 0;
        LLink<Entity> hull;      // an abandoned intact hull (null: none, or a wreck)
        LLink<AIGroup> group;    // the crew or squad group (null: none)
        // a withdrawing force's survivors (break-off, live timeout): where
        // they walk, re-sent at every check while one lives; VZero =
        // stragglers left where they stand.  Serialized (absent: VZero)
        Vector3 home = VZero;
        LSError Serialize(ParamArchive& ar);
    };

    // why a withdrawing force's hull left (HomeHull::released)
    enum HullRelease
    {
        HRNone,       // it never waited (a normal end, or its infantry rides in it)
        HRWaiting,    // still holding for the infantry
        HRCleared,    // no living survivor within the radius any more
        HRWaitOver,   // WithdrawHullWait passed
        HRNoInfantry, // the survivors' group is gone (removed, or nobody alive)
        NHullReleases
    };
    static const char* HullReleaseName(int released);

    // an intact hull driving home after its operation ended: deleted where
    // nobody sees it (HullUnseen), then CAOpHullRefund, its crew home as
    // bodies; destroyed on the way: CAOpHullLost.  Serialized.
    struct HomeHull
    {
        int opId = 0;
        int kind = CAKRetake;
        int origin = CAOLegacy;
        RString target;
        RString source;
        int units = 0; // the hull's price
        int crew = 0;  // living crew when it left
        float retry = 0;
        LLink<Entity> hull;
        LLink<AIGroup> crewGroup;
        // a withdrawal (serialized; absent: no wait, no cargo).  wait > 0:
        // the hull holds where it stands (an armed one still supporting)
        // while living members of waitGroup stand within waitRadius of
        // waitAt, for at most wait more seconds; then it drives home.
        // waited counts the seconds held; released is a HullRelease.
        float wait = 0;
        float waited = 0;
        float waitRadius = 0;
        Vector3 waitAt = VZero;
        LLink<AIGroup> waitGroup;
        int released = HRNone;
        // the infantry still seated in the carrier rides home with it:
        // deleted first, its living home as bodies, its dead lost
        LLink<AIGroup> cargo;
        int cargoLiving = 0;
        LSError Serialize(ParamArchive& ar);
    };
    // a withdrawing force's infantry has cleared the target's edge beyond
    // this radius (m): the larger of the 250 m dismount bound and the town
    // edge (zoneArea + TownEdgeMargin), so it is past where its hulls stand
    float ClearRadius(const ZoneRegistry& registry) const;

    // the last operations that ended (terminal events), newest last, for
    // the script surface (gmCounterAttackEnds; transient, EndsKept at most)
    struct EndNote
    {
        int opId = 0;
        RString target;
        int type = CAOpAborted; // CounterAttackOpEventType
        int reason = CAERNone;  // CounterAttackEndReason
        int bodies = 0;
        int units = 0;
        int born = 0; // the infantry strength born (0: never live)
        RString note;
        RString line; // a withdrawal's "Broken off at X: N lost, M going home" line
    };
    static constexpr int EndsKept = 16;
    int EndCount() const { return _ends.Size(); }
    const EndNote* GetEnd(int i) const { return i >= 0 && i < _ends.Size() ? &_ends[i] : nullptr; }

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
    // straightDistance * QuoteRoadFactor() / counterAttackSpeed, at
    // least MinTravel: the whole road leg from the base to the town
    static float TravelTime(const CounterAttackTuning& t, float straightDistance);
    // the record's abstract leg (3.2).  The speed lever (t.speedLever) or a
    // legacy record with the key off: TravelTime.  Otherwise at
    // r.abstractSpeed (FootSpeed when 0): roadLength / speed when the route
    // is a real road route (roadLength > 0), else straightDistance x
    // lineFactor / speed; at least MinTravel.  lineFactor is the quote's
    // road factor (QuoteRoadFactor), so the march and the quote agree.
    static float TravelTimeFor(const CounterAttackTuning& t, const Record& r, float straightDistance, float roadLength,
                               float lineFactor);
    // the one road factor of a ground leg with no road route known:
    // ForcePackages::RoadFactor(), the theatre's forcesRoadFactor the
    // director sets and quotes with (PackageRequest::roadFactor), at least 1
    static float QuoteRoadFactor();
    // the break-off rule: alive > 0 and alive x BreakOffDivisor < born
    static bool BreaksOff(int bornStrength, int alive);
    // HoldOdds with the attackers' lethality (6.5, item 12): D*w / (A_lethal + D*w)
    static float HoldOddsLethal(const CounterAttackTuning& t, int defenders, float attackLethal);
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
    // a point of route at or beyond fraction (by arc) lies within radius
    // (2D) of (px, pz): the road still ahead passes there (L13)
    static bool RouteAheadWithin(const AutoArray<Vector3>& route, float fraction, float px, float pz, float radius);
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
    static const char* SpawnReasonName(int reason);
    // U2 (OCCUPIER-FORCES.md 5.5): a squad of alive bodies adopted as the
    // garrison of a town RevertTown just refilled is subtracted from that
    // refill (ZoneRecord::revertRefill): from the pending deposit first,
    // then from the reserve, never below 0.  Any reserve beyond the refill
    // is kept, so reserve + pending + adopted <= garrisonMax unless the town
    // already held more.  Returns the bodies taken off; clears revertRefill.
    static int SubtractAdopted(ZoneRecord& town, int alive);
    // bodies a base can spare: (stored + live) - floor(garrisonMax * keep),
    // drawn from the live garrison when useLive, else from the stored reserve
    static int Spare(const CounterAttackTuning& t, float stored, int live, float garrisonMax, bool useLive);
    // deterministic uniform [0, 1) from a seed and a salt
    static float Roll01(unsigned int seed, int salt);
    static unsigned int MakeSeed(const char* town, int flipDay, int flipMinute, int townsBefore);

    // the service ---------------------------------------------------------------
    // a CITY just flipped to the resistance: count it, decide and schedule.
    // True when a counter-attack was scheduled.  hqZone is the zone the
    // player's HQ stands in (GuerrillaBase::HqZone at the engine call site,
    // -1 none): that town is never a target, like the exempt start town.
    // laterTownFlips are eligible captures already applied by the registry
    // in this event batch but not yet counted by this service.
    bool OnTownFlipped(const ZoneRegistry& registry, int zoneIndex, int groupSize, float warLevel = 1.0f,
                       int hqZone = -1, int laterTownFlips = 0);
    // send a force of up to size bodies from an explicit source zone on a
    // held town NOW (no delay, no roll): the launch seam for a later occupier
    // forces service.  The source's spare still caps it.  True when a force
    // left (a record is added); false when the town is not a valid target
    // (view.hqZone included) or already has a record, or the source cannot
    // spare a man or cannot march its live garrison out.
    bool LaunchFrom(ZoneRegistry& registry, const CounterAttackView& view, int target, int source, int size,
                    CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired);
    // the director's launch (CounterAttackOrders.hpp): validates the target
    // by kind (CAKRetake: a held non-exempt CITY, not view.hqZone, no record
    // on it; CAKBase: a resistance-held military zone, no record on it;
    // CAKHqAssault: an established HQ, no other HQ_ASSAULT, target the HQ's
    // zone or -1, destination order.destPos or view.hqPos; CAKReinforce: an
    // occupier-held non-exempt zone other than the source, no record on it;
    // CAKAlarm (phase 6A, 8.2): an occupier-held zone (order.alarmZone, or
    // order.target when -1) other than the source, one at a time, travelling
    // to order.alarmPos; CAKStrike (phase 6A, D17): an FPJet / FPAttackHeli
    // order with a hullClass and a strikeZone or strikePos, no source or
    // bodies needed; an air package's airOrigin is -1 or an occupier-held
    // AIRFIELD), the source
    // (occupier-held) and the slots (BornOps() < OpsCap(), one more for a
    // CAOWave order: wave operations come on top); refuses an order its
    // source cannot grant whole (M1: the stored reserve's FundedSpare, the
    // figure the director quoted) and starts the record: it travels
    // abstractly, its bodies debited from the stored reserve, at
    // order.abstractSpeed (> 0) or the legacy pacing (0).  Only when the
    // stored reserve cannot pay it and the source's live garrison can (its
    // town in the bubble) does a foot force march out live, and a hull
    // package goes on foot first (6.5 downgrade, the hull price refunded).  Live
    // and hull budgets do not refuse: a birth over them waits (6.5).  False
    // = nothing launched, nothing debited.
    bool LaunchFunded(ZoneRegistry& registry, const CounterAttackView& view, const CounterAttackOrder& order,
                      CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired);
    // phase 6A queries: the record of the one alarm answer (-1 none); a
    // strike record already on the order's strikeZone / strikePos (-1 none)
    int FindAlarm() const;
    int FindStrikeOn(const CounterAttackOrder& order) const;
    // the director's ear (CounterAttackOrders.hpp); null = none (unit tests,
    // the key off).  Not owned.
    static void SetListener(CounterAttackListener* listener);
    static CounterAttackListener* Listener();
    // record queries for the director
    int FindByOpId(int opId) const;
    // funded records that are LIVE or hold a live hull (forcesOpsMax counts
    // these); includeWave false leaves out CAOWave records (the director's
    // own slots: a wave's strikes come on top of them)
    int BornOps(bool includeWave = true) const;
    // records with origin CAOWave
    int WaveRecords() const;
    // the born-operation cap: forcesOpsMax, raised by one per living wave
    // record (7.1: wave operations are in addition to the director's).
    // forcesOpsMax has one reader and one writer: OccupierForces' tuning
    // (config forcesOpsMax, gmForcesTune "opsMax") while that service is
    // active; the local default (2) otherwise (a CounterAttack on its own,
    // the unit tests)
    int OpsCap() const;
    // the bodies a funded order may take from a source (M1): its STORED
    // reserve's spare (Spare with useLive false).  The director quotes
    // exactly this (OccupierForces::FillEngineView), and LaunchFunded
    // refuses an order it cannot grant whole.
    static int FundedSpare(const CounterAttackTuning& t, float stored, int live, float garrisonMax)
    {
        return Spare(t, stored, live, garrisonMax, false);
    }
    // the HQ_ASSAULT record, -1 none (at most one exists)
    int FindHqAssault() const;
    // OCCUPIER-FORCES.md 9.1, D14: the bodies that count at the HQ meter
    // right now.  Live units of CAKHqAssault records within radius (2D) of
    // hqPos, plus the strength of an abstract CAKHqAssault record HOLDING
    // within radius of hqPos (not while suspended in the player's bubble).
    // Passers-by, columns, QRFs and every other kind never count.  The
    // engine overload counts live bodies through the engine ops.
    int HqAssaultPresence(Vector3Par hqPos, float radius) const;
    int HqAssaultPresence(Vector3Par hqPos, float radius, CounterAttackOps* ops) const;
    // where a record marches and resolves: destPos when set, else the
    // target zone's centre (VZero when neither exists)
    static Vector3 DestOf(const ZoneRegistry& registry, const Record& r);
    // live hulls: records' hulls plus hulls driving home
    int LiveHulls() const;
    int HomeHullCount() const { return _homeHulls.Size(); }
    // lost hulls and dead or straggling groups awaiting an unseen deletion
    int LeftoverCount() const { return _leftovers.Size(); }
    const Leftover* GetLeftover(int i) const { return i >= 0 && i < _leftovers.Size() ? &_leftovers[i] : nullptr; }
    const HomeHull* GetHomeHull(int i) const { return i >= 0 && i < _homeHulls.Size() ? &_homeHulls[i] : nullptr; }
    // the view and ops Simulate steps with (the engine wrapper of the
    // director launches with the same pair; it fires the returned events
    // through ZoneRegistry::FireEvents itself).  BuildView refreshes the
    // engine perception snapshot too.
    void BuildView(CounterAttackView& view) const;
    CounterAttackOps* EngineOps();
    // the far census (CounterAttackFarCensus) against view's player; zero
    // without a player or ops.  GatherFarCensus is the engine path
    // (GatherSimDiag): the real player and the engine ops.
    void FarCensus(const ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops,
                   CounterAttackFarCensus& out) const;
    void GatherFarCensus(CounterAttackFarCensus& out) const;
    // the HOLDING bodies on a zone a revert settles as its garrison (5.4):
    // every holding record on it (suspended ones too), 0 with the key off
    int SettlePresence(const ZoneRegistry& registry, int zoneIndex) const;
    // the record's live hull (0 / 1), for diagnostics (gmCounterAttacks, gmOccupierOps)
    static int LiveHullOf(const Record& r);
    // the record's live escort hull (0 / 1); LiveHulls counts both
    static int LiveEscortOf(const Record& r);
    // engine diagnostics (gmCounterAttacks): living infantry seated in the
    // record's hull, and the squad's leader (null when none)
    static int MountedOf(const Record& r);
    static int AliveOf(const Record& r);
    static Entity* LeaderOf(const Record& r);
    // a mounted force stops before its infantry gets out (a cargo seat
    // ejects with the hull's speed): the dismount waits for the hull to
    // slow below this, or StopMax seconds
    static constexpr float DismountSpeed = 1.5f;
    static constexpr float StopMax = 15.0f;
    // a column's escort is born this far behind the carrier on its road (m);
    // the carrier waits while the escort is more than EscortSpread behind
    // and drives on once it is within EscortRejoin
    static constexpr float EscortGap = 20.0f;
    static constexpr float EscortSpread = 150.0f;
    static constexpr float EscortRejoin = 60.0f;
    // send an intact hull home (the end of its operation); the engine path
    // and the unit tests' fake ops
    // units / hullClass: the escort's price and class (defaults: the carrier's)
    void SendHullHome(const Record& r, Entity* hull, AIGroup* crewGroup, int crewAlive, CounterAttackOps* ops,
                      const ZoneRegistry& registry, int units = -1, const char* hullClass = nullptr);
    // towns the resistance took so far (non-exempt flips counted by
    // OnTownFlipped; saved).  k of the next flip is max(this, towns held).
    int TownsTaken() const { return _townsTaken; }
    void SetTownsTaken(int n) { _townsTaken = n > 0 ? n : 0; }
    // non-exempt CITY zones the resistance holds now
    static int HeldTowns(const ZoneRegistry& registry);
    // the registry's events of one zone tick (engine wrapper: flips, town
    // and base losses, the HQ's fall)
    void OnZoneEvents(const AutoArray<ZoneEventRecord>& fired);
    // the HQ fell (ZEHqLost, or the HQ is no longer established): every
    // HQ_ASSAULT record succeeded.  Abstract ones settle now (CAOpSettled,
    // their bodies to the pool: no garrison to join); a live one is marked
    // hqDone and leaves the world once nobody sees it.  Returns the records
    // settled now.
    int SettleHqAssaults(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops);
    // advance every record by elapsed seconds.  ops may be null (no world:
    // the live path waits).  Raises counterAttack / counterRepelled into
    // fired, and townLost through RevertTown.
    void Step(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
              AutoArray<ZoneEventRecord>& fired);
    // abstract occupier bodies holding a zone (ZoneTickInputs::abstractOcc).
    // The zone census supplies its current player view: it runs before our
    // Step, so the record's suspended snapshot may still describe the last
    // position.  Other callers may omit it to inspect that snapshot.
    int AbstractPresence(const ZoneRegistry& registry, int zoneIndex,
                         const CounterAttackView* currentView = nullptr) const;

    // runtime knob (gmCounterAttackTune "delayMax"): a pending delay never
    // outlasts the new ceiling
    void ClampScheduled(float maxDelay);

    // The engine's perception gate (Traffic's issue #53 tiers, the gate the
    // counter-attack births use) refreshed for the player at (px, pz) now.
    // GarrisonCache's S5 pending release samples it.  Engine path only: the
    // reference stays valid for the process, its snapshot until the next
    // refresh.
    static const CounterAttackGate& EngineGate(bool playerValid, float px, float pz);

    int Count() const { return _records.Size(); }
    const Record* Get(int i) const { return i >= 0 && i < _records.Size() ? &_records[i] : nullptr; }
    int FindByTarget(const char* town) const;
    static const char* PhaseName(int phase);

    // engine hook: per frame, throttled to TickInterval
    void Simulate(float deltaT);

    bool HasState() const
    {
        return _records.Size() > 0 || _townsTaken > 0 || _homeHulls.Size() > 0 || _leftovers.Size() > 0 ||
               _airHomes.Size() > 0;
    }
    LSError Serialize(ParamArchive& ar) override;

  private:
    void LoadFromConfig();
    // one event to the listener (queued while a listener call is running)
    // (terminal events are kept in _ends too; reason: a CAOpRepelled's
    // CounterAttackEndReason)
    void Emit(const ZoneRegistry& registry, const Record& r, int type, int bodies, int units, const char* note,
              int reason = CAERNone);
    // crew leave the operation's price (home as bodies, refunded, lost, or
    // handed to the Leftover list): CAOpCrewSettled, so the ledger row lets
    // go of them without crediting the pool (5.5).  Nothing for crew <= 0 or
    // an airReplanned record (no row holds its airframe)
    void SettleCrew(const ZoneRegistry& registry, const Record& r, int crew, const char* note);
    static void Dispatch(const CounterAttackOpEvent& ev);
    // the shared launch core (LaunchFrom, LaunchFunded): r carries the
    // order's fields; false = nothing launched
    bool LaunchCore(ZoneRegistry& registry, const CounterAttackView& view, int target, int source, int size, Record& r,
                    CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired);
    // the birth chain's next base back (D6): the nearest other occupier base
    // (straight distance to the target) that can spare the whole force and
    // whose route (ops' road net, else the straight line) has a point the
    // gate passes; without ops the nearest.  True when re-sourced
    bool Resource(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops);
    // the chain's end: bodies back to their source (or the pool), the
    // terminal CAOpCancelledUnseen
    void CancelUnseen(ZoneRegistry& registry, const CounterAttackView& view, Record& r);
    // a record ended: its hull (live: home; abstract: refunded now)
    void FinishHull(ZoneRegistry& registry, const CounterAttackView& view, Record& r, CounterAttackOps* ops);
    // a live record's hull check (destroyed, crewless): true when it was lost now
    bool CheckHull(ZoneRegistry& registry, Record& r, CounterAttackOps* ops);
    // the same for a column's escort (its loss leaves the infantry mounted)
    bool CheckEscort(ZoneRegistry& registry, Record& r, CounterAttackOps* ops);
    // a record ended: its escort (live: home; abstract: refunded now)
    void FinishEscort(ZoneRegistry& registry, const CounterAttackView& view, Record& r, CounterAttackOps* ops);
    // hulls driving home
    void StepHomeHulls(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops);
    // crew bodies home to the source (room-capped, surplus and a lost base to the pool)
    static void CrewHome(ZoneRegistry& registry, const CounterAttackView& view, const char* source, int crew);
    // bodies back into a zone's reserve, room-capped (garrisonMax - live -
    // reserve - pending; a pending deposit inside the player's bubble), the
    // surplus and a zone no longer the occupier's to the pool (L2, 5.5).
    // Returns the bodies that went into the zone.
    static int ReturnBodies(ZoneRegistry& registry, const CounterAttackView& view, int zone, int bodies);
    // what a lost hull or a dead group leaves: tracked until an unseen
    // deletion (M3, L7)
    void AddLeftover(const RString& source, Entity* hull, AIGroup* group, int living, Vector3Par home = VZero);
    void StepLeftovers(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops);
    // the key-on arrival at a town with defenders: born at the edge (a hull
    // package with its carrier, escort and crews, as on a route birth), or
    // wait (over any budget, the hull gate included; never on foot to fit).
    // Returns the GoLive outcome (BirthDone when born)
    int ArriveLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops,
                   AutoArray<ZoneEventRecord>& fired);
    // a live force withdraws (break-off, live timeout, the HQ moved; reason
    // a CounterAttackEndReason, rule the why for the INFO line): the losses
    // booked, the survivors walking home as a Leftover (or seated in their
    // carrier), the terminal CAOpRepelled; the hulls wait for the infantry
    // (SendHullHome through FinishHull).  Always true: the record ends
    bool Withdraw(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops,
                  int alive, Vector3Par leader, int reason, const char* rule, AutoArray<ZoneEventRecord>& fired);
    // 5.4: the town reverted under a travelling record
    bool RevertedUnder(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r);
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
    void March(const ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
               CounterAttackOps* ops, Vector3Par leader, float elapsed);
    // the playtest trail (assessment step 1, director log level): one line
    // per CAOpArrived, "abstract" or "live" (how = what follows the
    // arrival), and per CAOpBorn with its spawn reason, each with the
    // player's distance from at
    static void TraceArrived(const CounterAttackView& view, const Record& r, const char* mode, const char* how,
                             Vector3Par at);
    static void TraceBorn(const CounterAttackView& view, const Record& r, const char* reason, Vector3Par at);
    // survivors go home, whole bodies, capped at the base's room
    // (garrisonMax - live - reserve - pending); inside the player's deposit
    // bubble they are a pending deposit (S5), else a reserve write.  The
    // surplus over the room and a lost base go to the pool (5.5); one
    // CAOpReturned.
    void ReturnToSource(ZoneRegistry& registry, const CounterAttackView& view, const Record& r, int bodies);
    bool InBubble(const ZoneRegistry& registry, const CounterAttackView& view, int zone) const;
    static bool InBubbleAt(const ZoneRegistry& registry, const CounterAttackView& view, Vector3Par pos);
    // the HQ_ASSAULT record's step (9.1): true when it is finished
    bool AdvanceHq(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
                   Record& r, AutoArray<ZoneEventRecord>& fired);
    int LiveSquads() const;
    // room for one more live group (live squads and the side's group budget)
    bool RoomLive(const ZoneRegistry& registry, const CounterAttackView& view) const;
    // send r's force of n from src: live (march the base's garrison out) or
    // abstract (debit the stored reserve, travel).  False = not launched (a
    // live detach failed); the caller retries or gives up.
    // the whole abstract leg from r's source to its town (TravelTimeFor)
    float LegTime(const ZoneRegistry& registry, Record& r, CounterAttackOps* ops) const;
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
    // phase 6A (CounterAttackAir.cpp) ------------------------------------
  public:
    // the off-map edge point for a target (3.1, forcesOffMapEdge): on the
    // chosen edge (edge < 0: the one nearest the target) beyond the map by
    // margin, level with the target along the edge.  mapSize 0 = no map:
    // VZero.
    static Vector3 OffMapEdgePoint(Vector3Par target, float mapSize, int edge, float margin);
    // the height above ground air is born at: a jet 250 m, a helicopter 100 m
    static float AirBirthAlt(int package);
    // the air re-plan of the birth chain (3.1 step 4): no base's road passes
    // the gate, so a ground operation becomes a helicopter insertion when
    // the catalogue offers a transport helicopter, its seats hold the
    // bodies and the pool pays it (OccupierForces::SpendOnOp: the
    // operation's ledger row holds the airframe like a ground hull).  True =
    // re-planned.
    bool AirReplan(ZoneRegistry& registry, const CounterAttackView& view, Record& r,
                   const ForcePackageCatalogue& catalogue);
    // some occupier base other than r's source (and not yet tried, D6)
    // stands on the target's landmass: a ground force could still come from
    // there.  False = every other base is air-only for this target
    bool GroundSourceShares(const ZoneRegistry& registry, const Record& r) const;
    int AirHomes() const { return _airHomes.Size(); }
    // the pool units CounterAttack still owes operation opId's ledger row:
    // its records' unsettled hull prices (crew included, OwedUnits), its
    // hulls and airframes driving home (price + living crew); a re-plan no
    // row took (airReplanned, direct) is not on a row.  0 for op 0.
    // crew (optional): the part of it that is crew, settled as bodies
    int OwedOnOp(int opId, int* crew = nullptr) const;
    const AirHome* GetAirHome(int i) const { return i >= 0 && i < _airHomes.Size() ? &_airHomes[i] : nullptr; }

  private:
    // the point air comes from: the occupier-held airfield (r.airOrigin), else the off-map edge
    Vector3 AirOriginPos(const ZoneRegistry& registry, const CounterAttackView& view, const Record& r,
                         Vector3Par dest) const;
    // the unseen birth point at altitude, searched back from the abstract
    // progress point to the origin, then off-map; false = none passes
    bool PickAirBirth(const ZoneRegistry& registry, const CounterAttackView& view, const Record& r,
                      const CounterAttackOps& ops, Vector3Par dest, float progress, Vector3& birth) const;
    // a helicopter insertion goes live (GoLive's air branch): BirthDone /
    // BirthNoPoint / BirthWait
    int AirGoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r, CounterAttackOps* ops,
                  float fraction);
    // a live insertion before its infantry is out; true = the record ends
    bool AdvanceAirLive(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops, Record& r);
    // hand a record's airframe to the home list (the record keeps its infantry)
    void SendAirHome(Record& r);
    void StepAirHomes(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops);
    // the reported fighters' centre near a zone (OccupierForces sightings),
    // for the LZ side and the limited reveal; age = the youngest sighting
    static bool ReportedNear(const ZoneRegistry& registry, int zone, Vector3& centre, float* age = nullptr);
    // why a CAKStrike order is refused (null = valid)
    static const char* StrikeRefusal(const ZoneRegistry& registry, const CounterAttackOrder& order);
    // start a strike record (no source, no bodies): true when it exists
    bool LaunchStrike(ZoneRegistry& registry, const CounterAttackView& view, const CounterAttackOrder& order);
    // one step of a strike record; true = the record ends
    bool AdvanceStrike(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
                       Record& r);
    // an event with a place (CAOpAirborne, CAOpLanded, CAOpStruck, CAOpAlarmAnswered)
    void EmitAt(const ZoneRegistry& registry, const Record& r, int type, int zone, Vector3Par pos, int bodies,
                int units, const char* note);
    bool Advance(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops, Record& r,
                 AutoArray<ZoneEventRecord>& fired);
    // a live squad in the town it retook becomes the town's garrison; the
    // squad is subtracted from the revert refill (U2), never assigned over
    // the reserve
    void AdoptLive(ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, int alive);
    // a live top-up (REINFORCE or alarm answer) squad at its town joins the
    // garrison up to the room (garrisonMax - live - reserve - pending): the
    // stored reserve, then the pending deposit, give way to it first (back
    // to the pool, nothing visible), and what still does not fit splits off
    // and walks home as a Leftover (removed unseen, home through CrewHome up
    // to its source's room, the rest to the pool).  Returns the bodies that
    // joined.  The 5.5 "live top-up" row.
    int AdoptTopUp(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                   CounterAttackOps* ops, int alive);
    // the occupier base nearest at on its landmass (-1 none): a hull whose
    // base fell drives there instead (L13)
    static int NearestOccupierBase(const ZoneRegistry& registry, Vector3Par at);

    CounterAttackTuning _tuning;
    AutoArray<Record> _records;
    AutoArray<HomeHull> _homeHulls;
    AutoArray<AirHome> _airHomes; // phase 6A: airframes flying home after an insertion
    float _mapSize = 0;           // phase 6A: the last map size a view reported (the off-map edge)
    AutoArray<Leftover> _leftovers;
    AutoArray<EndNote> _ends; // transient: the last EndsKept terminal events
    RString _endLine;         // transient: the line the next terminal event carries (Withdraw)
    float _accum = 0;
    int _townsTaken = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
