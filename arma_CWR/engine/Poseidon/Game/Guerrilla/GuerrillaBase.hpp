#pragma once

// Guerrilla Mode headquarters (issue #16 M1/M4 + issue #28) - the player's
// elected base: one HQ per campaign, sited in a zone of the player's
// choosing, with a persistent WEAPON CACHE (a keep-when-empty WeaponHolder,
// tracked by the StashRegistry) and a GARAGE ring where vehicles can be
// locked so they persist safely (invulnerable while locked, by default).
//
//   * Election: either the new-game START TOWN cycler (gmSelStartTown -> the
//     first Simulate tick of a fresh campaign establishes the HQ there and
//     relocates the player beside it) or the in-mission "Establish /
//     Move headquarters here" action (scripts/market.sqs -> gmHqEstablish).
//     Re-establishing MOVES the HQ: the cache object is moved with its
//     contents, the garage ring relocates (vehicles left behind are
//     released), and the move is counted for the script-side debit.
//   * Siting: the best enterable building inside the zone (Paths LOD, at
//     least hqMinPos AI positions, not destroyed; most positions wins, then
//     nearest the zone centre) holds the cache indoors and the garage ring
//     sits beside it.  A zone without such a building (a CAMP, a hamlet)
//     falls back to the EDGE OF TOWN: an off-road, dry, free spot on the
//     outer rings of the zone area, cache and garage together.
//   * Garage: any Transport inside garageRadius of the garage spot can be
//     locked (beep-beep) - lock state and invulnerability are re-asserted
//     every tick for the live rows, so neither needs its own serialization;
//     a garaged vehicle that ends up far outside the ring is released.
//
// Native core in the TownFlags / StashRegistry mold: pure, unit-testable
// pickers under a thin world layer, Serialize + a *Commands.cpp script
// surface (gmHq* / gmGarage*).  Inactive whenever the ZoneRegistry is
// inactive, so ordinary missions are unaffected.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>   // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/Foundation/Types/Pointers.hpp> // Ref (the beep wave)
#include <Poseidon/Audio/IAudioSystem.hpp>        // IWave (complete: Ref<IWave> lives in the class)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class ParamEntry;
class EntityAI;

namespace Guerrilla
{

// optional keys inside class CfgGuerrillaZones
struct BaseTuning
{
    int hqMinPos = 4;               // hqMinPos: min AI positions for an HQ building
    float garageRadius = 100.0f;    // garageRadius: m around the garage spot
    bool garageInvulnerable = true; // garageInvulnerable: locked vehicles take no damage
};

// One enterable-building candidate for the pure HQ picker.  The engine path
// fills these from GWorld's building table; unit tests inject values.
struct HqCandidate
{
    int index = -1; // caller's handle (world building index)
    int nPos = 0;   // AI positions inside (Paths LOD)
    float dist = 0; // m from the zone centre
};

// One outdoor placement candidate for the pure spot picker (garage ring /
// edge-of-town fallback).
struct HqSpotSample
{
    float x = 0;
    float z = 0;
    float height = 0;         // terrain Y
    float distFromAnchor = 0; // m from the anchor (building or zone centre)
    bool onRoad = false;      // hard reject
    bool underwater = false;  // hard reject
    bool free = true;         // hard reject when false (occupied by an object)
};

class GuerrillaBase : public SerializeClass
{
  public:
    GuerrillaBase() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static GuerrillaBase& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset (drops refs only, never objects)
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    void LoadFromConfig();
    // testable tuning parse; null leaves the defaults
    void LoadFromParams(const ParamEntry* zonesCfg);

    // queries ---------------------------------------------------------------
    bool IsActive() const; // mirrors ZoneRegistry::IsActive
    const BaseTuning& Tuning() const { return _tuning; }
    bool IsEstablished() const { return _established; }
    bool IsIndoors() const { return _established && _indoors; }
    RString ZoneName() const { return _established ? _zone : RString(); }
    Vector3 HqPos() const { return _established ? _hqPos : VZero; }
    Vector3 GaragePos() const { return _established ? _garagePos : VZero; }
    Vector3 CachePos() const { return _established ? _cachePos : VZero; }
    EntityAI* Building() const;
    EntityAI* Cache() const;
    int MoveCount() const { return _moveCount; }
    // the zone (any type) whose area contains pos; -1 when none
    int ZoneAt(Vector3Par pos) const;
    bool CanEstablish(Vector3Par pos) const { return IsActive() && ZoneAt(pos) >= 0; }
    // inside garageRadius (2D) of the garage spot; false while unestablished
    bool InGarageRange(Vector3Par pos) const;

    // election --------------------------------------------------------------
    // Establish (or move) the HQ in the zone containing pos.  World path:
    // picks the building / edge spot, creates or moves the cache holder,
    // relocates the garage ring.  False when no zone contains pos or the
    // world is not up.
    bool Establish(Vector3Par pos);

    // garage ------------------------------------------------------------------
    // lock: the vehicle must be a Transport inside the ring and not carry
    // the real player; unlock: any tracked row.  False when refused.
    bool GarageLock(EntityAI* veh, bool lock);
    int GarageCount() const { return _rows.Size(); }
    EntityAI* GarageVehicle(int i) const; // null out of range / dead link
    bool GarageHas(const EntityAI* veh) const;

    // pure logic (unit-tested; no world access) ------------------------------
    // most AI positions wins, nearer the centre breaks ties; -1 when empty
    static int PickHqBuilding(const AutoArray<HqCandidate>& candidates);
    // first acceptable sample in order (callers order the rings nearest
    // first): off-road, dry, free; -1 when nothing qualifies
    static int PickSpot(const AutoArray<HqSpotSample>& samples);
    static bool InRange2D(Vector3Par a, Vector3Par b, float radius);
    // beep cue timeline (s): burst 1 = [0, BeepOn), burst 2 = [BeepGap,
    // BeepGap + BeepOn); the FSM below walks it
    static constexpr float BeepOn = 0.15f;
    static constexpr float BeepGap = 0.35f;

    // tuning constants (engine facts, not island data)
    static constexpr float TickInterval = 2.0f;  // s between garage passes
    static constexpr float RoadClearance = 6.0f; // m, GRoadNet::IsOnRoad probe size
    static constexpr float ReleaseFactor = 1.5f; // rows beyond garageRadius * this are released
    static constexpr float CacheAside = 4.0f;    // m, outdoor cache offset from the garage spot
    static constexpr float PlayerAside = 6.0f;   // m, start-town relocation offset

    // test aids -------------------------------------------------------------
    // force the established state without world access (save/load rows)
    void MarkEstablishedForTest(const char* zone, Vector3Par hqPos, Vector3Par garagePos, Vector3Par cachePos,
                                bool indoors);
    // a garage row with a null vehicle link (pruned on the load pass)
    void AddGarageRowForTest();

    // simulation ------------------------------------------------------------
    // per-frame engine hook; the garage pass throttles to TickInterval, the
    // beep FSM and the one-shot start-town election run per frame
    void Simulate(float deltaT);

    // save/load; the cache holder and the garaged vehicles ride the world's
    // building / vehicle serializers - this block is the election state
    // plus the tracking rows (SerializeRef)
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct GarageRow
    {
        // OLink semantics: a deleted vehicle reads back as null -> row dropped
        LLink<EntityAI> veh;

        LSError Serialize(ParamArchive& ar);
    };

    // beep-beep lock cue, transient (never serialized)
    struct BeepState
    {
        LLink<EntityAI> veh;
        Ref<IWave> wave;
        int stage = -1; // -1 idle, 0 first burst, 1 gap, 2 second burst
        float t = 0;
    };

    // world-touching internals (engine path only)
    bool PickBuilding(int zoneIndex, EntityAI*& outBuilding, Vector3& outInterior) const;
    bool ComputeOutdoorSpot(Vector3Par anchor, const float* ringRadii, int nRings, Vector3& out) const;
    EntityAI* CreateCache(Vector3Par where, bool indoors) const;
    void MoveCache(EntityAI* cache, Vector3Par where, bool indoors) const;
    void ReleaseRow(int i);
    void AssertRow(GarageRow& row);
    void Beep(EntityAI* veh);
    void SimulateBeep(float deltaT);
    void TryAutoElection();
    void RelocatePlayer() const;

    BaseTuning _tuning;
    // tuning read for the current mission (cleared by Clear; a save written
    // without our block leaves it false and the first tick reloads)
    bool _configLoaded = false;
    bool _established = false;
    bool _indoors = false;
    RString _zone;
    Vector3 _hqPos = VZero;     // zone-side anchor (building or edge spot)
    Vector3 _garagePos = VZero; // centre of the garage ring
    Vector3 _cachePos = VZero;  // where the holder sits (interior point or outdoors)
    LLink<EntityAI> _building;  // null for an outdoor HQ
    LLink<EntityAI> _cache;
    AutoArray<GarageRow> _rows;
    int _moveCount = 0;
    bool _autoTried = false; // start-town election attempted (serialized)
    bool _cacheWarned = false;
    float _accum = 0;
    BeepState _beep;
};

} // namespace Guerrilla
} // namespace Poseidon
