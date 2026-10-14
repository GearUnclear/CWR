#pragma once

// Guerrilla Mode garrison distance-cache ("dynamic simulation") - native
// replacement for the spawning.sqs cache loop (guerrilla-mode/mission/
// Guerrilla.Demo/scripts).  Occupier-held zones, and since issue #79 B2 the
// occupier-administered towns (ZoneRegistry::IsGarrisonable), keep their
// garrison as an integer reserve on the ZoneRegistry record while the player
// is far away;
// crossing cacheRadius converts the reserve into live AIGroups, and leaving
// ADDS the survivors back to whatever reserve is still stored (U1,
// OCCUPIER-FORCES.md 5.5: a group-budget remainder, D2 regrowth or a top-up
// written while the garrison was live is kept).  Reserve writes near the
// player go through the S5 pending deposit (Deposit / AddReserve below).
// QRF policy stays in script - this service only fires events.  Inactive
// whenever the ZoneRegistry is inactive.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp> // Vector3Par
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
namespace Poseidon { class AIGroup; } // a Poseidon type: a global forward declaration collides with the using-declaration in Core/Types.hpp on Linux clang

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;
struct ZoneRecord;
class CounterAttackGate;

// optional keys inside class CfgGuerrillaZones
struct GarrisonTuning
{
    float cacheInterval = 5.0f; // s between cache passes
    int groupSize = 12;         // max bodies per garrison group
    // Off-screen contract D2 (issue #71): a distant, calm, untouched
    // occupier post regains one body per garrisonRecoverInterval seconds,
    // up to its garrisonMax (default: the authored garrison).  0 = off.
    float recoverInterval = 600.0f;
};

enum GarrisonEventType
{
    GESpawned,   // "garrisonSpawned"
    GEDespawned, // "garrisonDespawned"
    NGarrisonEventTypes
};

struct GarrisonEventRecord
{
    GarrisonEventType type;
    int zoneIndex;
    int count; // bodies spawned / survivors written back
};

enum GarrisonAction
{
    GActNone,
    GActSpawn,
    GActDespawn
};

// What FlushPending did with a zone's S5 pending deposit
enum PendingFlush
{
    PFNone,           // nothing pending, or it waits (the zone is in the bubble and a spawn point is seen)
    PFReleasedBubble, // moved into the reserve: the player is beyond the despawn edge
    PFReleasedHidden, // moved into the reserve: every sampled spawn point is CAExposeHidden
    PFRefunded        // the occupier lost the zone: refunded to the pool (occupierForces on), else to the origin or discarded
};

// Per-zone observations for the D2 recovery step (StepRecover).
struct GarrisonRecoverInput
{
    bool occupierOwned = false; // ZoneRegistry::IsGarrisonable (post or administered town)
    bool spawned = false;       // live cache groups exist for the zone
    float reserve = 0;          // ZoneRecord::garrison
    float reserveMax = 0;       // ZoneRecord::garrisonMax
    bool playerValid = false;   // real player exists and is alive
    float playerDistSq = 0;     // squared 2D player-to-zone distance
    bool alertGreen = true;     // AlertMachine zone state is GREEN
    float capture = 0;          // ZoneRecord::capture
    int resistancePresent = 0;  // live resistance units in zoneArea (last zone tick)
};

// Per-zone observations for the pure decision step.  The engine path fills
// this from the live world; unit tests inject values directly.
struct GarrisonDecisionInput
{
    bool occupierOwned = false; // ZoneRegistry::IsGarrisonable (post or administered town)
    bool spawned = false;       // live cache groups exist for the zone
    float reserve = 0;          // despawned garrison strength (ZoneRecord::garrison)
    bool playerValid = false;   // real player exists and is alive
    float playerDistSq = 0;     // squared 2D player-to-zone distance
};

class GarrisonCache : public SerializeClass
{
  public:
    GarrisonCache() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static GarrisonCache& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset, including event handlers (drops group refs only)
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    void LoadFromConfig();
    // testable tuning parse; null leaves the defaults
    void LoadFromParams(const ParamEntry* zonesCfg);

    // queries (zone-indexed against the ZoneRegistry) -----------------------
    bool IsActive() const; // mirrors ZoneRegistry::IsActive
    const GarrisonTuning& Tuning() const { return _tuning; }
    bool IsSpawned(int zoneIndex) const;
    int LiveCount(int zoneIndex) const; // alive bodies; 0 when despawned
    // alive bodies of the zone's cache garrison within radius (2D) of pos;
    // 0 when despawned (U4: the military capture's defenders)
    int LiveCountWithin(int zoneIndex, Vector3Par pos, float radius) const;
    int NGroups(int zoneIndex) const;
    AIGroup* GetGroup(int zoneIndex, int i) const; // may be null (dead link)

    // events ----------------------------------------------------------------
    void SetEventHandler(GarrisonEventType type, RString handler);
    RString GetEventHandler(GarrisonEventType type) const;
    static int EventTypeFromName(const char* name); // -1 when unknown

    // pure logic (unit-tested; no world access) ------------------------------
    // spawning.sqs has no hysteresis (spawn <= R, despawn > R); +50 m on the
    // despawn edge avoids flapping when the player hovers at the radius
    static constexpr float DespawnHysteresis = 50.0f;
    static GarrisonAction Decide(const GarrisonDecisionInput& in, float cacheRadius, float despawnHysteresis);
    // split a reserve into per-group body counts; returns the group count
    static int PlanGroups(int reserve, int groupSize, AutoArray<int>& takes);
    // D2 garrison recovery (issue #71): advance a zone's recovery clock by
    // elapsed seconds and return the bodies it regains (never past
    // reserveMax).  Eligible = occupier-owned, not spawned, the player alive
    // and beyond the despawn edge (cacheRadius + hysteresis), alert GREEN,
    // no capture in progress, no live resistance in the zone and short of
    // reserveMax; an ineligible zone's clock restarts at 0, so recovery
    // needs a full interval of calm.  A dead player freezes the clock.
    // interval <= 0 disables recovery.
    static int StepRecover(const GarrisonRecoverInput& in, float elapsed, float interval, float cacheRadius,
                           float despawnHysteresis, float& clock);
    // apply StepRecover's gained bodies through the occupier pool
    // (OCCUPIER-FORCES.md 5.3): granted = OccupierForces::Draw(gained) joins
    // the reserve; when the pool grants less the recovery clock is clamped
    // to interval (due again at once, nothing banked).  Inactive pool: all
    // granted (the legacy D2).  Returns granted.
    static int ApplyRecover(ZoneRecord& z, int gained, float interval);
    // a despawn's write-back (U1: added to the reserve).  With the occupier
    // pool active a zone the occupier no longer holds (a flipped base or
    // town) refunds its survivors to the pool instead of writing them onto
    // the new owner's record (5.5); dead bodies are NoteLoss diagnostics.
    static void WriteBackSurvivors(ZoneRegistry& registry, int zone, int survivors, int dead);

    // S5 pending deposits (OCCUPIER-FORCES.md 3.3, no pop-in) ----------------
    // A body may enter a zone near the player only by walking in or by a
    // materialization whose every spawn point is hidden.  Every other write
    // toward a reserve inside the deposit bubble (cacheRadius +
    // DespawnHysteresis of the player, the edge Decide despawns at) is a
    // PENDING deposit on ZoneRecord::pending: it does not defend, does not
    // materialize, is saved on the zone row, and moves into the reserve when
    // the zone leaves the bubble or every sampled spawn point passes the
    // gate's turn-proof tier (CAExposeHidden).  Callers: the revert refill
    // (ZoneRegistry::RevertTown), the return of a counter-attack force to its
    // base (CounterAttack::ReturnToSource) and the reserve left waiting
    // beside a wiped garrison (Simulate, and the save reconciliation).
    //
    // Add bodies (whole, >= 1) to the zone's pending deposit.  origin is the
    // zone they came from (-1 none); a deposit from a different origin than
    // the one already pending marks the deposit mixed (-1).
    static void Deposit(ZoneRegistry& registry, int zone, int bodies, int originZone);
    // the same on the engine-wide ZoneRegistry
    void Deposit(int zone, int bodies, int originZone);
    // the zone is inside the deposit bubble: within cacheRadius +
    // DespawnHysteresis (2D) of a valid player
    static bool InDepositBubble(const ZoneRecord& z, bool playerValid, float px, float pz, float cacheRadius);
    // the routing every S5 writer uses: inside the deposit bubble the bodies
    // are Deposited, else added to the reserve directly (as before S5).
    // Returns true when they went to the pending deposit.
    static bool AddReserve(ZoneRegistry& registry, int zone, int bodies, int originZone, bool playerValid, float px,
                           float pz);
    // the turn-proof test for a release: every sample of the +-spread
    // placement square SpawnGarrison draws from (a 3 x 3 grid: the centre,
    // the four edge midpoints and the four corners) rates CAExposeHidden.
    // A single cone-only or seen sample keeps the deposit pending.
    static bool PendingSpawnHidden(Vector3Par centre, float spread, const CounterAttackGate& gate);
    // one zone's pending check (run every cache pass before Decide and before
    // StepRecover): refund when the occupier no longer holds the zone, else
    // release into the reserve when the zone is out of the deposit bubble,
    // or when gate is non-null and PendingSpawnHidden passes.  A dead
    // player freezes the deposit (no release).  Returns a PendingFlush.
    static int FlushPending(ZoneRegistry& registry, int zone, bool playerValid, float px, float pz,
                            const CounterAttackGate* gate);
    // the occupier lost a zone with a pending deposit: the refund hook gets
    // it first (phase 2: the strategic pool); without a hook (or when it
    // declines) the bodies go back to the origin zone's reserve, routed by
    // AddReserve and capped at its garrisonMax, if the occupier still holds
    // it and it is not the zone itself; otherwise they are discarded.  One
    // INFO line either way.
    static void RefundPending(ZoneRegistry& registry, int zone, bool playerValid, float px, float pz);
    // the phase-2 seam for the refund (OccupierForces routes it to the pool
    // when its key is on): return true when the hook took the bodies.  Null
    // (the default) keeps the key-off rule above.
    using PendingRefundHook = bool (*)(ZoneRegistry& registry, int zone, int originZone, int bodies);
    static void SetPendingRefundHook(PendingRefundHook hook);
    static PendingRefundHook GetPendingRefundHook();
    // the occupier holds the zone: an occupier-owned non-CITY zone, or an
    // administered town not exempt from its garrison (IsGarrisonable without
    // the faction-tier check)
    static bool OccupierHolds(const ZoneRegistry& registry, const ZoneRecord& z);

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to cacheInterval
    void Simulate(float deltaT);
    // script escape hatch: immediate despawn write-back for one zone
    void ForceDespawn(int zoneIndex);
    // take a live occupier group as (part of) a zone's garrison: the
    // counter-attack squad that retook a town stays as its defenders
    // (issue #79).  It gets the hold posture and is written back, cached
    // and counted like any garrison group from here on.
    void AdoptGroup(int zoneIndex, AIGroup* grp);
    // test aid: force a zone's spawned flag without world access, so the
    // save/load bookkeeping (spawned rows, dead-group reconciliation) is
    // unit-testable
    void MarkSpawnedForTest(int zoneIndex, bool spawned);

    // save/load; spawned-zone bookkeeping keyed by zone name (the garrison
    // units themselves are saved by the world's vehicle serializer)
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct GarrisonState
    {
        bool spawned = false;
        // OLink semantics: deleted groups read back as null and are pruned
        LLinkArray<AIGroup> groups;
    };

    // one savegame row per spawned zone
    struct GarrisonSaveState
    {
        RString name;
        bool spawned = false;
        LLinkArray<AIGroup> groups;

        LSError Serialize(ParamArchive& ar);
    };

    void SyncStates(); // keep _states index-aligned with the ZoneRegistry
    void SpawnGarrison(int zoneIndex, float warLevel, AutoArray<GarrisonEventRecord>& fired);
    void DespawnGarrison(int zoneIndex, AutoArray<GarrisonEventRecord>& fired);
    int CountAlive(const GarrisonState& state) const;
    void DispatchEvents(const AutoArray<GarrisonEventRecord>& fired);
    void ApplyPendingLoad();

    GarrisonTuning _tuning;
    AutoArray<GarrisonState> _states; // index-aligned with ZoneRegistry zones
    RString _handlers[NGarrisonEventTypes];
    float _accum = 0;
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<GarrisonSaveState> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
