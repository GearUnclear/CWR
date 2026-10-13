#pragma once

// Guerrilla Mode garrison distance-cache ("dynamic simulation") - native
// replacement for the spawning.sqs cache loop (guerrilla-mode/mission/
// Guerrilla.Demo/scripts).  Occupier-held zones, and since issue #79 B2 the
// occupier-administered towns (ZoneRegistry::IsGarrisonable), keep their
// garrison as an integer reserve on the ZoneRegistry record while the player
// is far away;
// crossing cacheRadius converts the reserve into live AIGroups, and leaving
// converts the survivors back.  QRF policy stays in script - this service
// only fires events.  Inactive whenever the ZoneRegistry is inactive.

#include <Poseidon/Foundation/Containers/Array.hpp>
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
