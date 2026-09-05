#pragma once

// Guerrilla Mode garrison distance-cache ("dynamic simulation") - native
// replacement for the spawning.sqs cache loop (guerrilla-mode/mission/
// Guerrilla.Demo/scripts).  Occupier-held zones keep their garrison as an
// integer reserve on the ZoneRegistry record while the player is far away;
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

// Per-zone observations for the pure decision step.  The engine path fills
// this from the live world; unit tests inject values directly.
struct GarrisonDecisionInput
{
    bool occupierOwned = false; // non-CITY zone held by an occupier faction
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

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to cacheInterval
    void Simulate(float deltaT);
    // script escape hatch: immediate despawn write-back for one zone
    void ForceDespawn(int zoneIndex);
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
