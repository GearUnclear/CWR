#pragma once

// Occupier air (guerrilla-mode/OCCUPIER-FORCES.md 3.1 "Air births", 3.2
// "Helicopter insertion" and "Jet or attack-helicopter strike", D3, D4, D17,
// D20; the phase 6 opening spike, findings in guerrilla-mode/AIR-SPIKE.md).
//
// A self-contained native module phase 6 wires into CounterAttack's order
// flow.  It owns two kinds of live air record:
//
//   INSERTION  a transport helicopter born AIRBORNE (native creation plus
//              MakeAirborne at the config pilot height), its crew one group,
//              its infantry a SECOND group seated as cargo with the InCargo
//              state.  The pilots get Command::Move through IssueCommand
//              (never an arcade waypoint) to an LZ picked natively 300..500 m
//              from the target (object-free, dry, gentle), marked by an
//              invisible helipad (HeliHEmpty).  Close to the pad the helicopter
//              is ordered to land (Transport::LandStarted, LMLand, re-issued every
//              LandRetry s until down); once down
//              (height <= 5 m, slow) the cargo group gets out (the phase 3
//              truck dismount: OrderGetIn false, AllowGetIn false, DoGetOut,
//              plus a per-unit UnassignVehicle); when every cargo body is out
//              the landing order is ended (LandFinished, the pilot to Wait)
//              and the crew gets a fresh Move to its home point, and the airframe is deleted CREW FIRST, then the
//              hull, where the altitude-aware gate says nobody can see it.
//              A landing more than 100 m from the LZ, or a state that outlasts
//              its limit, is a FAILED insertion (phase 6 re-plans it).
//   SORTIE     a jet (or attack helicopter) born airborne, never on the
//              ground: Move to the target, an orbit of Moves around it in
//              COMBAT / open fire for sortieTime (search and destroy), then
//              home and deleted the same way.  Reveal (AIGroup::AddTarget) is
//              limited to sightings younger than 60 s with accuracy < 1.
//
// The rules (state machines, the LZ choice, the altitude-aware exposure) are
// pure and unit-tested through AirOps / AirLzProbe / AirViewer fakes; the
// engine implementations live in OccupierAir.cpp.
//
// World hooks: World.cpp calls Simulate every frame (idle with no records;
// the call also pulls this module and its command TU into the link) and
// WorldInit.cpp calls Clear at mission start.  Not wired into CounterAttack
// or the director yet: phase 6 proper calls SpawnInsertion / SpawnSortie.
// Records are transient (not serialized).
//
// Script surface (OccupierAirCommands.cpp): gmAirInsert, gmAirSortie,
// gmOccupierAir, gmAirExposure, gmAirUnseen, gmAirAbort, gmAirReveal,
// gmAirTune.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // CounterAttackGate, CounterAttackExposure tiers

namespace Poseidon
{
class AIGroup;
class Entity;
} // namespace Poseidon

namespace Poseidon::Guerrilla
{

// insertion states (gmOccupierAir column 0)
enum AirInsertionState
{
    AISInbound,   // flying to the LZ
    AISLanding,   // ordered down on the pad
    AISUnloading, // on the ground, cargo getting out
    AISDeparting, // empty, flying home
    AISGone,      // crew and hull deleted
    AISFailed,    // landed off the LZ, lost, or timed out (still removed unseen)
    NAirInsertionStates
};

// sortie states
enum AirSortieState
{
    ASSInbound, // flying to the target
    ASSLoiter,  // orbiting the target, weapons free, for sortieTime
    ASSHome,    // flying home
    ASSGone,
    ASSFailed,
    NAirSortieStates
};

struct AirTuning
{
    float lzMin = 300.0f;          // heli LZ ring around the target (16)
    float lzMax = 500.0f;
    int lzTries = 48;              // candidates tried before PickLz gives up
    float lzClear = 22.0f;         // m radius for the slope samples (objects: the LZ's own terrain cell)
    float lzMaxGradient = 0.25f;   // rise over run across the clear radius (engine landing limit, see AIR-SPIKE.md)
    float lzMinDry = 1.0f;         // m of land above sea level
    float landTolerance = 100.0f;  // a landing farther from the LZ is a failed insertion
    float landOrderDist = 450.0f;  // 2D distance to the LZ where the landing is ordered (FindStopPosition sees pads within 500 m only)
    float landedHeight = 5.0f;     // TR UNLOAD's limit: no get-out above this (m above ground)
    float landedSpeed = 3.0f;      // m/s: "stopped" for the unload
    float departHeight = 20.0f;    // m above ground: lifted off
    float inboundTimeout = 360.0f; // s per state before the record fails
    float landingTimeout = 150.0f;
    float unloadTimeout = 90.0f;
    float departTimeout = 900.0f;  // s flying home before removal ignores the gate (it is far by then)
    float removeHold = 2.0f;       // s the airframe must stay unseen before it is deleted
    float homeReach = 400.0f;      // m from home where a departing airframe counts as home
    float airAudible = 800.0f;     // the air audible floor: a rotor or a jet is heard well past 300 m
    float revealMaxAge = 60.0f;    // reveal only sightings younger than this
    float revealMaxAccuracy = 1.0f; // ... and vaguer than an exact fix (accuracy < this)
    float orbitRadius = 900.0f;    // jets circle the target this far out while they search
    float orbitLeg = 20.0f;        // s between orbit Moves
    float sortieInboundTimeout = 300.0f;
    float jetMinHeight = 60.0f;    // m above ground below which a jet in flight is "on the ground" (failed)
    float trace = 0;               // s between per-record trace lines (0 = off; gmAirTune "trace", trials only)
};

// one LZ candidate as the probe sees it (the engine reads the terrain and
// the object grid; unit tests inject values)
struct AirLzFacts
{
    bool inMap = true;     // inside the terrain grid
    float landY = 0;       // surface height above sea level at the point
    float gradient = 0;    // max rise over run across the clear radius
    int objects = 0;       // objects (trees, buildings, walls) within the clear radius
    // the probe may move the candidate (the engine snaps it to the centre of
    // its terrain cell, the unit the engine's own landing test rates); the
    // pick then re-checks the ring with the moved point
    bool moved = false;
    float x = 0, z = 0;
};

class AirLzProbe
{
  public:
    virtual ~AirLzProbe() = default;
    virtual AirLzFacts Probe(Vector3Par pos, float clearRadius) const = 0;
};

// the altitude-aware viewer (the issue #53 gate lifted off the ground): the
// camera, the band, the terrain ray to a point at height.  Also a
// CounterAttackGate, whose Exposure(pos) treats pos.Y as altitude above sea
// level (OccupierAir::ExposureAt).
class AirViewer : public CounterAttackGate
{
  public:
    virtual bool PlayerValid() const = 0;
    virtual Vector3 PlayerPos() const = 0;
    virtual bool HasCamera() const = 0;
    virtual Vector3 CamPos() const = 0; // engine axes, Y = height above sea level
    virtual bool InFrustum(Vector3Par pos) const = 0;
    // terrain (or sea) ray from the camera to the exact point posAsl
    virtual bool LosBlocked(Vector3Par posAsl) const = 0;
    virtual float GroundY(float x, float z) const = 0; // surface height (sea = 0)
    virtual float MinSpawn() const = 0;  // the band's imperceptibility bound (object cull + margin)
    virtual float Audible() const = 0;   // the air audible floor (m, 2D from the player)
    int Exposure(Vector3Par pos) const override;
};

// what the world reports about one airframe each step
struct AirFacts
{
    bool hullAlive = false;     // the link lives and the hull is not destroyed
    bool hullExists = false;    // the link lives (wreck included)
    int crewAlive = 0;          // living crew seated in the hull
    bool pilotAlive = false;    // a living driver at the controls
    int cargoAlive = 0;         // living bodies of the cargo group
    int cargoSeated = 0;        // ... of them still seated in the hull
    Vector3 pos = VZero;        // engine axes
    float heightAgl = 0;        // m above ground (above the sea surface over water)
    float speed = 0;            // m/s
    bool unseen = false;        // the airframe could vanish unnoticed now (AirViewer, at its altitude)
    bool beyondBand = false;    // past the band's bound in 3D from the camera (nothing is drawn there)
};

// what the pure step asks the world to do (bits)
enum AirAction
{
    AANone = 0,
    AAMoveLz = 1,      // Move the pilots to the LZ (approach)
    AALand = 2,        // order the landing on the pad
    AAUnload = 4,      // the cargo group gets out
    AAMoveHome = 8,    // a fresh Move home (lift off)
    AARemoveCrew = 16, // delete the crew (FIRST)
    AARemoveHull = 32, // then the hull
    AARemovePad = 64,  // the helipad goes with the record
    AAOrbit = 128,     // a sortie's next orbit Move
    AAWeaponsFree = 256 // a sortie arrived: COMBAT, open fire
};

// the pure core of one insertion (unit-tested)
struct AirInsertionCore
{
    int state = AISInbound;
    float stateTime = 0;
    float age = 0;
    Vector3 lz = VZero;
    Vector3 home = VZero;
    Vector3 landedAt = VZero;
    bool landed = false;    // touched down at least once
    float landedOffset = -1; // 2D m from the LZ at touchdown (-1 = not yet)
    int landings = 0;       // touchdowns (an uncommanded second landing counts here)
    float unseenFor = 0;
    float touchFor = 0;     // s the airframe has sat low and slow (touchdown needs TouchHold)
    float liftedOffAt = -1; // age at lift-off
    float unloadAt = -1;    // age when the unload was ordered
    float landOrderAt = -1; // age when the landing was last ordered
    int landOrders = 0;     // landing orders issued (the first plus re-issues)
    bool unloadOrdered = false;
    bool crewRemoved = false;
    bool hullRemoved = false;
    int failReason = 0;     // AirFailReason
};

enum AirFailReason
{
    AFRNone,
    AFROffLz,      // landed > landTolerance from the LZ
    AFRTimeout,    // a state outlasted its limit
    AFRHullLost,   // destroyed or deleted under it
    AFRCrewLost,   // no pilot
    NAirFailReasons
};

// the pure core of one sortie
struct AirSortieCore
{
    int state = ASSInbound;
    float stateTime = 0;
    float age = 0;
    Vector3 target = VZero;
    Vector3 home = VZero;
    float sortieTime = 300.0f;
    float orbitClock = 0;
    int orbitIndex = 0;
    float unseenFor = 0;
    float minHeightSeen = 1e9f; // lowest height above ground seen in flight (taxi / landing evidence)
    bool touchedGround = false; // ever below 3 m above ground (a forced landing or taxi)
    bool crewRemoved = false;
    bool hullRemoved = false;
    int failReason = 0;
};

// plan for one insertion
struct AirInsertionPlan
{
    RString heliClass;
    int cargo = 8;
    Vector3 birthPos = VZero; // engine axes, Y ignored
    float birthAlt = 0;       // m above ground at birth (0 = the config pilot height)
    Vector3 lz = VZero;
    float lzRadius = 0;       // informational
    Vector3 target = VZero;
    RString side;             // "EAST" / "WEST" / "GUER"
};

// a live insertion
struct AirInsertion
{
    int id = 0;
    RString heliClass;
    RString side;
    LLink<Entity> hull;
    LLink<AIGroup> crew;
    LLink<AIGroup> cargo;
    LLink<Entity> pad;
    int cargoBorn = 0;
    Vector3 birthPos = VZero;
    Vector3 target = VZero;
    float crewGoneAt = -1; // age when the crew was deleted
    float hullGoneAt = -1; // age when the hull was deleted
    int removeSeq = 0;     // 1 = crew first then hull (the rule), 2 = hull first (the gmAirAbort probe)
    AirInsertionCore core;
};

// a live sortie
struct AirSortie
{
    int id = 0;
    RString planeClass;
    RString side;
    LLink<Entity> hull;
    LLink<AIGroup> crew;
    Vector3 birthPos = VZero;
    float crewGoneAt = -1;
    float hullGoneAt = -1;
    int reveals = 0; // AddTarget calls that passed RevealAllowed
    AirSortieCore core;
};

// the world-touching half (OccupierAir.cpp: EngineAirOps); unit tests fake it
class AirOps
{
  public:
    virtual ~AirOps() = default;
    virtual void Facts(Entity* hull, AIGroup* crew, AIGroup* cargo, const AirViewer& viewer, AirFacts& out) = 0;
    virtual void MoveTo(AIGroup* crew, Vector3Par target) = 0;
    virtual void Land(Entity* hull, AIGroup* crew, Entity* pad, Vector3Par lz) = 0;
    // end the landing order before the Move home: the touchdown is declared
    // while the pilot may still be Stopping with LMLand, and a landing order
    // left standing lands him again at the nearest free spot after lift-off
    virtual void CancelLand(Entity* hull) = 0;
    // the cargo group gets out (the phase 3 dismount), then moves on target on foot
    virtual void Unload(AIGroup* cargo, Entity* hull, Vector3Par target) = 0;
    virtual void WeaponsFree(AIGroup* crew) = 0;
    // delete the crew, and any cargo body still seated in the hull (FIRST)
    virtual void RemoveCrew(AIGroup* crew, AIGroup* cargo, Entity* hull) = 0;
    virtual void RemoveHull(Entity* hull) = 0;
    virtual void RemovePad(Entity* pad) = 0;
};

class OccupierAir
{
  public:
    static OccupierAir& Instance();

    // pure rules (unit-tested) ------------------------------------------------
    // the altitude-aware exposure tier (CounterAttackExposure) of a point at
    // altitude (pos.Y above sea level).  The audible floor is the AIR floor;
    // the sea is not special (off-map renders, 3.1); past the band's bound in
    // 3D, or behind terrain at that height = hidden; outside the cone = cone-only.
    static int ExposureAt(const AirViewer& viewer, Vector3Par posWithAlt);
    // the despawn rule at altitude (Traffic::CanExposeDespawn with the air
    // audible floor): outside the view cone AND (past the bound or
    // terrain-hidden), never inside the audible floor
    static bool UnseenAt(const AirViewer& viewer, Vector3Par posWithAlt);
    // the LZ choice: candidates on the lzMin..lzMax ring, the side away from
    // awayFrom first (when given), every candidate checked by the probe (in
    // the map, dry, gradient within lzMaxGradient, no objects within lzClear)
    static bool PickLzPure(Vector3Par target, const Vector3* awayFrom, const AirTuning& t, const AirLzProbe& probe,
                           Vector3& lz, RString& why, int* tried = nullptr);
    // the order the ring candidates are tried in (deterministic; index i of
    // lzTries): the angle offset from the away direction and the radius
    static void LzCandidate(int i, const AirTuning& t, float& angleOffset, float& radius);
    // one step of an insertion: returns AirAction bits for the world.  The
    // removal bits are returned crew first: AARemoveCrew in one step,
    // AARemoveHull in the next (so the order is observable), AARemovePad with
    // the hull.
    static int StepInsertionPure(AirInsertionCore& c, const AirFacts& f, float dt, const AirTuning& t);
    static int StepSortiePure(AirSortieCore& c, const AirFacts& f, float dt, const AirTuning& t);
    // the orbit point i around the target
    static Vector3 OrbitPoint(Vector3Par target, float radius, int i);
    // limited reveal (3.2, D9): a sighting younger than revealMaxAge with an
    // accuracy below revealMaxAccuracy (never an exact fix)
    static bool RevealAllowed(float ageSeconds, float accuracy, const AirTuning& t);
    static const char* StateName(int state);
    static const char* SortieStateName(int state);
    static const char* FailName(int reason);

    // engine (OccupierAir.cpp) -------------------------------------------------
    bool PickLz(Vector3Par target, const Vector3* awayFrom, float minR, float maxR, Vector3& lz, RString& why);
    bool SpawnInsertion(const AirInsertionPlan& plan, int& id, RString& why);
    bool SpawnSortie(const RString& planeClass, Vector3Par birthPos, float birthAlt, Vector3Par target,
                     float sortieTime, const RString& side, int& id, RString& why);
    // reveal a target to a sortie's crew (limited, RevealAllowed)
    bool Reveal(int sortieId, Entity* target, float ageSeconds, float accuracy);
    // delete now: crew first then hull (hullFirst = the engine-fact probe:
    // the hull first, which ejects the crew); returns crew bodies left alive
    // in the world afterwards
    int Abort(int id, bool hullFirst);
    // step every record with the engine viewer and ops
    void Step(float dt, const AirViewer& viewer, AirOps& ops);
    // engine hook: per frame from World.cpp, throttled to TickInterval
    void Simulate(float deltaT);
    // the engine viewer refreshed for the current camera (gmAirExposure)
    static const AirViewer& EngineViewer();
    void Clear();

    int InsertionCount() const { return _insertions.Size(); }
    const AirInsertion* GetInsertion(int i) const
    {
        return i >= 0 && i < _insertions.Size() ? &_insertions[i] : nullptr;
    }
    int SortieCount() const { return _sorties.Size(); }
    const AirSortie* GetSortie(int i) const { return i >= 0 && i < _sorties.Size() ? &_sorties[i] : nullptr; }
    // live airframes (insertions and sorties not yet gone) and the groups they hold
    int LiveAirframes() const;
    int LiveGroups() const;
    AirTuning& TuningMutable() { return _tuning; }
    const AirTuning& Tuning() const { return _tuning; }

    static constexpr float TickInterval = 0.5f;
    static constexpr float TouchHold = 1.0f;    // s low and slow before a touchdown counts
    static constexpr float UnloadRetry = 3.0f;  // s before a still-seated cargo is ordered out again
    // s between landing orders while not yet down: the approach Move can end
    // after LandStarted and take the pilot out of Stopping (it then climbs
    // back to its flying height and hovers); LandStarted is a no-op while the
    // pilot is still Stopping, so the re-issue only bites after such a wave-off
    static constexpr float LandRetry = 5.0f;
    static constexpr float LiftRetry = 45.0f;   // s on the ground after the Move home before it is re-issued
    static constexpr int MaxRecords = 64;       // finished records kept for gmOccupierAir

  private:
    AirTuning _tuning;
    AutoArray<AirInsertion> _insertions;
    AutoArray<AirSortie> _sorties;
    int _nextId = 1;
    float _accum = 0;
    float _traceClock = 0;
};

} // namespace Poseidon::Guerrilla
