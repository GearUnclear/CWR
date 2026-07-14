#pragma once

// Guerrilla Mode town flags - one physical FlagCarrier pole per CITY zone,
// flying the owning side's flag so a town's allegiance is readable in-world
// (the map half of the same feature is the zone marker: init.sqs creates it
// as a "Flag" icon and ZoneRegistry::UpdateMarkers side-colors it).  The
// pole is placed OFF the road network (GRoadNet probe) and biased toward
// high ground on the town outskirts, so the flag reads from outside the
// town.  Texture resolution: faction descriptor `flag` key first, then a
// built-in per-side default (verified against the Classic 1.99 Flags.pbo),
// then the generic white flag.  Inactive whenever the ZoneRegistry is
// inactive, so ordinary missions are unaffected.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class EntityAI;

namespace Guerrilla
{

// One placement candidate for the pure spot picker.  The engine path fills
// these from the live world (terrain height + road net); unit tests inject
// values directly.
struct FlagSpotSample
{
    float x = 0;              // easting
    float z = 0;              // northing
    float height = 0;         // terrain Y (higher = seen from farther away)
    float distFromCenter = 0; // m from the zone center (outskirt bias)
    bool onRoad = false;      // hard reject - never plant a flag in the road
    bool underwater = false;  // hard reject - coastal towns reach the sea
};

class TownFlags : public SerializeClass
{
  public:
    TownFlags() = default;

    // engine-wide instance (used by the World hooks)
    static TownFlags& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset (drops pole refs only, never objects)
    void InitMission(); // Clear; the flags have no config of their own

    // queries ---------------------------------------------------------------
    bool IsActive() const;                // mirrors ZoneRegistry::IsActive
    bool IsPlaced(int zoneIndex) const;   // a flag spot has been chosen
    Vector3 FlagPos(int zoneIndex) const; // VZero while unplaced
    // test aid: force a zone's placement without world access, so the
    // save/load bookkeeping (placed rows, name matching) is unit-testable
    void MarkPlacedForTest(int zoneIndex, Vector3Par pos);

    // pure logic (unit-tested; no world access) ------------------------------
    // score = height + OutskirtWeight * distFromCenter, so ~25 m of outskirt
    // distance trades against 1 m of elevation; road/underwater samples are
    // never picked.  Returns the winning index, -1 when nothing qualifies.
    static constexpr float OutskirtWeight = 0.04f;
    static int PickFlagSpot(const AutoArray<FlagSpotSample>& samples);
    // faction `flag` key > per-side default (WEST/EAST/GUER, textures shipped
    // by the Classic 1.99 data) > the generic white flag.  Never empty.
    static RString ResolveFlagTexture(const char* owner, const char* factionFlag);

    // tuning constants (engine facts, not island data)
    static constexpr float TickInterval = 5.0f;  // s between flag passes
    static constexpr float RoadClearance = 6.0f; // m, GRoadNet::IsOnRoad probe size

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to TickInterval
    void Simulate(float deltaT);

    // save/load; placement + pole refs keyed by zone name (the FlagCarrier
    // objects themselves ride the world's building serializer)
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct FlagState
    {
        bool placed = false; // pos computed and valid (serialized)
        Vector3 pos = VZero; // chosen spot; kept stable across saves
        // OLink semantics: a deleted pole reads back as null -> recreated
        LLink<EntityAI> pole;
        // transient: last texture pushed to the pole; cleared on load so the
        // first tick re-applies (trust-but-verify - heals saves regardless
        // of whether FlagCarrier serialized its texture)
        RString appliedTexture;
        bool warned = false; // one-shot placement/spawn failure log
    };

    // one savegame row per placed zone
    struct FlagSaveState
    {
        RString name;
        Vector3 pos = VZero;
        LLink<EntityAI> pole;

        LSError Serialize(ParamArchive& ar);
    };

    void SyncStates(); // keep _states index-aligned with the ZoneRegistry
    // deterministic sampling (center + outskirt rings) -> PickFlagSpot;
    // false when no off-road dry candidate exists near the town
    bool ComputeSpot(Vector3Par center, float zoneArea, Vector3& out) const;
    EntityAI* CreatePole(Vector3Par pos) const; // null when the class is absent
    void ApplyPendingLoad();

    AutoArray<FlagState> _states; // index-aligned with ZoneRegistry zones
    float _accum = 0;
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<FlagSaveState> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
