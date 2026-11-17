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
//     Re-establishing MOVES the HQ: the cache contents travel with it
//     (changing layout replaces the holder), the garage ring relocates
//     (vehicles left behind are released), and the move is counted for the
//     script-side debit.
//   * Siting: the best enterable building inside the zone (Paths LOD, at
//     least hqMinPos AI positions, not destroyed; most positions wins, then
//     nearest the zone centre) holds a map desk and single crate at validated
//     floor positions; the garage ring sits beside it. A zone without room
//     falls back to the EDGE OF TOWN: an off-road, dry, free spot on the
//     outer rings of the zone area, with an open tent and camouflaged crate
//     stack beside the garage. Both cache layouts start empty.
//   * Garage: any Transport inside garageRadius of the garage spot can be
//     locked (beep-beep) - lock state and invulnerability are re-asserted
//     every tick for the live rows, so neither needs its own serialization;
//     a garaged vehicle that ends up far outside the ring is released.
//   * One HQ model (OCCUPIER-FORCES.md 9.1): with no START TOWN picked (the
//     "(camp)" default or a direct launch) the first active tick of a
//     campaign (campaignEnd on) establishes the HQ at the first CAMP zone
//     through Establish, without moving the player.  An old save with no HQ
//     gets the same on load unless its HQ was destroyed.
//   * HQ loss (D13, D14, D22, D24): the HQ meter (hqCapture, 0..100) at
//     HqPos rises only while a deliberate HQ_ASSAULT force stands within
//     HqRadius and no resistance HOLD presence does; at 100 Destroy() empties
//     the cache, wrecks the garaged vehicles, removes the ring and queues the
//     hqLost zone event.  The zone around the HQ is never touched.  A new HQ
//     needs an establish action (free: it is not a move); _hqEpoch counts
//     destructions so scripts can tell one HQ from the next.
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
class ZoneRegistry;
struct ZoneTuning;

// optional keys inside class CfgGuerrillaZones
struct BaseTuning
{
    int hqMinPos = 4;               // hqMinPos: min AI positions for an HQ building
    float garageRadius = 100.0f;    // garageRadius: m around the garage spot
    bool garageInvulnerable = true; // garageInvulnerable: locked vehicles take no damage
};

// One enterable-building candidate for the pure HQ picker.  The engine path
// fills these from terrain cells and GWorld's building table; tests inject values.
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
    // established and not destroyed (Destroy clears _established, so the
    // two always agree; HasHq is the name the occupier-forces plan uses)
    bool IsEstablished() const { return _established; }
    bool HasHq() const { return _established && !_hqDestroyed; }
    // the HQ fell and no new one was established since
    bool HqDestroyed() const { return _hqDestroyed; }
    // the campaign has had an HQ at some point (CampaignEnd's lose guard)
    bool HqEver() const { return _established || _hqDestroyed || _hqEpoch > 0; }
    // HQ destructions so far (0 for the first HQ); unchanged by re-establishing
    int HqEpoch() const { return _hqEpoch; }
    // the HQ meter, 0..100 (D24)
    float HqCapture() const { return _hqCapture; }
    bool IsIndoors() const { return _established && _indoors; }
    RString ZoneName() const { return _established ? _zone : RString(); }
    Vector3 HqPos() const { return _established ? _hqPos : VZero; }
    Vector3 GaragePos() const { return _established ? _garagePos : VZero; }
    Vector3 CachePos() const { return _established ? _cachePos : VZero; }
    EntityAI* Building() const;
    EntityAI* Cache() const;
    EntityAI* Prop() const;
    int MoveCount() const { return _moveCount; }
    // the zone (any type) whose area contains pos; -1 when none
    int ZoneAt(Vector3Par pos) const;
    // the same against an explicit registry (unit tests)
    static int ZoneAt(const ZoneRegistry& registry, Vector3Par pos);
    // The zone the HQ stands in (OCCUPIER-FORCES.md 9.1): the zone whose
    // area (zoneArea) contains HqPos, nearest first; an edge-of-town HQ whose
    // spot lies just outside the area falls back to the zone it was
    // established in (ZoneName).  -1 while unestablished.  The legacy
    // counter-attack dispatch never targets it (N4a).
    int HqZone() const;
    int HqZone(const ZoneRegistry& registry) const;
    bool CanEstablish(Vector3Par pos) const { return IsActive() && ZoneAt(pos) >= 0; }
    // inside garageRadius (2D) of the garage spot; false while unestablished
    bool InGarageRange(Vector3Par pos) const;

    // election --------------------------------------------------------------
    // Establish (or move) the HQ in the zone containing pos.  World path:
    // picks the building / edge spot, creates or moves the cache holder,
    // relocates the garage ring.  False when no zone contains pos or the
    // world is not up.  After a destruction it is a first establishment
    // (not a move): fresh empty cache, fresh ring, _hqDestroyed cleared.
    bool Establish(Vector3Par pos);
    // the first CAMP zone (registry order, case-insensitive type); -1 none
    static int FirstCampZone(const ZoneRegistry& registry);
    // the default-HQ rule (9.1): a campaign (campaignEnd on) with no HQ that
    // was never destroyed gets one at the Camp; a destroyed HQ never comes
    // back by itself (a retaken Camp is not an HQ again)
    bool WantsDefaultHq(bool campaignEnabled) const { return campaignEnabled && !_established && !_hqDestroyed; }

    // HQ meter and destruction (OCCUPIER-FORCES.md 9.1) ---------------------
    static constexpr float HqRadius = 150.0f; // m around HqPos, both sides
    // the pure meter step: rises by captureRate x min(assault, captureCrewCap)
    // with assault units and no hold presence, frozen with both, decays by
    // captureDecayAbandoned with no assault units; clamped to 0..100
    static float NextHqCapture(float meter, int assault, int hold, const ZoneTuning& tuning);
    // presence seams: bodies within radius (2D) of pos.  The defaults are the
    // engine's: CounterAttack::HqAssaultPresence (HQ_ASSAULT units only, live
    // or abstract) and the resistance HOLD count (the player under
    // PlayerHoldsPresence, plus every living resistance unit).  Unit tests
    // inject fakes; null restores the engine default.
    using HqPresenceFn = int (*)(Vector3Par pos, float radius);
    static void SetHqPresenceSources(HqPresenceFn assault, HqPresenceFn hold);
    // one meter tick at HqPos (the zone tick cadence); Destroy at 100.
    // No-op without an HQ.
    void TickHqMeter(const ZoneTuning& tuning);
    // the gmHqForceCapture lever: sets the meter (clamped 0..100); 100 with
    // an HQ standing destroys it at once
    void ForceHqCapture(float value);
    // D13 / D22: the HQ falls.  Empties the cache now and queues the holder
    // and the HQ prop for deletion out of the player's perception, wrecks
    // every garaged vehicle (damage 1, left where it stands) and drops the
    // ring, sets _hqDestroyed, clears _established, bumps _hqEpoch, logs one
    // INFO line and queues ZEHqLost with the HQ zone (or -1).  Returns the
    // garage rows it took (the destroy list, null links included); -1 when
    // there was no HQ.
    int Destroy();
    // ruins (emptied holder, prop) still waiting for an unseen moment
    int PendingRuins() const { return _ruins.Size(); }

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
    // the one-shot start-town flag as a save carries it (old-save tests)
    bool AutoTried() const { return _autoTried; }
    void SetAutoTriedForTest(bool tried) { _autoTried = tried; }

    // simulation ------------------------------------------------------------
    // per-frame engine hook; the garage pass throttles to TickInterval, the
    // beep FSM and the one-shot start-town / Camp election run per frame,
    // the HQ meter at the zone tick interval
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

    // an object of a fallen HQ waiting to be deleted unseen (the emptied
    // cache holder, the desk or tent); rides the world's building serializer
    struct RuinRow
    {
        LLink<EntityAI> obj;

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
    bool PickBuilding(int zoneIndex, EntityAI*& outBuilding, Vector3& outInterior, Vector3& outDesk) const;
    bool ComputeOutdoorSpot(Vector3Par anchor, const float* ringRadii, int nRings, Vector3& out) const;
    EntityAI* CreateCache(Vector3Par where, bool indoors) const;
    void MoveCache(EntityAI* cache, Vector3Par where, bool indoors) const;
    EntityAI* CreateProp(Vector3Par where, bool indoors) const;
    void SyncVisuals();
    void ReleaseRow(int i);
    void AssertRow(GarageRow& row);
    void Beep(EntityAI* veh);
    void SimulateBeep(float deltaT);
    void TryAutoElection();
    // the start-town tail: move the player beside the HQ, exempt the town
    void OnStartTownEstablished(int zoneIndex, const RString& town);
    void RelocatePlayer() const;
    // the shared state commit of Establish and MarkEstablishedForTest; true
    // when it was a move (an HQ already stood)
    bool CommitEstablished(const RString& zone, Vector3Par hqPos, Vector3Par garagePos, Vector3Par cachePos,
                           bool indoors);
    // delete the ruins nobody can see right now (Traffic's despawn rule
    // through CounterAttack::EngineGate)
    void SweepRuins();

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
    LLink<EntityAI> _prop;
    Vector3 _propPos = VZero;
    bool _layoutPlaced = false; // absent in older saves; upgraded on the first tick
    AutoArray<GarageRow> _rows;
    int _moveCount = 0;
    bool _autoTried = false; // start-town election attempted (serialized)
    bool _campTried = false; // default Camp HQ attempted this session (transient)
    bool _hqDestroyed = false; // the HQ fell and none stands (serialized)
    int _hqEpoch = 0;          // destructions so far (serialized)
    float _hqCapture = 0;      // the HQ meter (serialized)
    float _hqAccum = 0;        // meter cadence (transient)
    AutoArray<RuinRow> _ruins; // serialized (second-pass refs)
    bool _cacheWarned = false;
    float _accum = 0;
    BeepState _beep;
};

} // namespace Guerrilla
} // namespace Poseidon
