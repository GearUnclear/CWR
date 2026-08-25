#pragma once

// Guerrilla Mode ambient road traffic - the island's road network carries
// life between the zones: civilian cars driving town-to-town, occupier
// patrol vehicles between occupier-held zones and the occasional occupier
// supply convoy.  Native C++ in the GarrisonCache / TownFlags mold: a pure,
// unit-testable decision core (spawn rolls, time-of-day/alert/curfew/weather
// modulation, route picking, spawn-point selection, the commandeer predicate,
// despawn/stall rules) under a thin world layer (group + vehicle creation,
// seating, doMove-style route legs, the commandeer sequence, cleanup) with
// Serialize and a *Commands.cpp script surface.  Script stays a thin handler
// layer (init.sqs registers event handlers; civilians.sqs consumes the
// driverKilled ledger tuples).
//
// Commandeer: a civilian car that meets the real player standing in its
// lane ahead, or a player pointing a weapon at it, stops; the driver bails
// and flees on foot; the hull is released to the world (the player takes
// it, undercover keeps treating a civilian-typed car as anonymous).
// Road murders feed the civilian kill ledger through the same killed-EH
// expression civilians.sqs attaches to town civilians.
//
// Inactive whenever the ZoneRegistry is inactive or trafficEnabled=0, so
// ordinary missions are unaffected.  Accepted gap (see guerrilla-mode/
// STATUS.md): patrol traffic feeds no AlertMachine input.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
class AIGroup;

namespace Poseidon
{
class ParamEntry;
class Transport;
class Person;
class AIUnit;

namespace Guerrilla
{

// optional keys inside class CfgGuerrillaZones (all prefixed traffic*)
struct TrafficTuning
{
    bool enabled = true;              // trafficEnabled
    float interval = 5.0f;            // s between traffic passes
    float radius = 1500.0f;           // player band: live traffic lives inside this
    float minSpawnDist = 300.0f;      // ... but never spawns/despawns-on-arrival closer than this
    float despawnHysteresis = 300.0f; // radius + this = the far-despawn edge
    int maxCiv = 3;
    int maxPatrols = 1;
    int maxConvoys = 1;
    float civChance = 0.5f; // per-pass spawn probability per kind
    float patrolChance = 0.25f;
    float convoyChance = 0.04f;
    float convoyWarScale = 0.5f;          // convoy chance grows with (warLevel - 1)
    float civNightScale = 0.1f;           // civ chance factor outside the day window
    float dayStart = 6.0f / 24.0f;        // day-fraction the civ day window opens...
    float dayEnd = 21.0f / 24.0f;         // ... and closes
    float alertPatrolBoost = 0.5f;        // patrol chance +this on a YELLOW/RED origin
    float curfewWarLevel = 3.0f;          // curfew arms at this war level
    float curfewPatrolBoost = 2.0f;       // patrol chance factor under curfew
    float rainCivFade = 0.6f;             // civ chance -this at full rain
    float stallTimeout = 90.0f;           // s of < 5 m movement before a stalled car is torn down
    float arriveRadius = 60.0f;           // m to the destination road point
    int maxLegs = 3;                      // re-dispatches while the player is still near
    float commandeerRadius = 25.0f;       // m, player-to-car trigger radius
    float commandeerLaneHalfWidth = 4.0f; // m, lateral offset for "in the lane"
    float commandeerStopDelay = 2.5f;     // s between Stop and the driver bailing
    float fleeDist = 150.0f;              // m, the bailed driver's flee point
};

enum TrafficKind
{
    TKCiv,
    TKPatrol,
    TKConvoy,
    NTrafficKinds
};

enum TrafficState
{
    TSDriving,
    TSStopping, // commandeer: Stop issued, waiting out commandeerStopDelay
    TSExiting,  // commandeer: driver bailed (transient, one tick)
    TSArrived,  // at the destination, waiting for the player to be far enough
    TSStalled,  // stall timer expired, waiting for the player to be far enough
    NTrafficStates
};

enum TrafficEventType
{
    TESpawned,      // "spawned"      [veh, kind, originIdx, destIdx]
    TEDespawned,    // "despawned"    [kind, reason]
    TECommandeered, // "commandeered" [veh, driver]
    TEArrived,      // "arrived"      [veh, kind, destIdx]
    TEDriverKilled, // "driverKilled" - killed-EH EXPRESSION attached to every civ driver
    NTrafficEventTypes
};

// Per-pass observations for the pure spawn decision.  The engine path fills
// this from the live world; unit tests inject values directly.
struct TrafficDecisionInput
{
    bool enabled = true;
    bool playerValid = false; // real player exists and is alive
    int liveCiv = 0;
    int livePatrols = 0;
    int liveConvoys = 0;
    bool hasCivRoute = false; // a CITY -> CITY route exists near the player
    bool hasPatrolRoute = false;
    bool hasConvoyRoute = false;
    float warLevel = 1.0f;
    float roll = 0.0f;     // uniform [0,1)
    float civScale = 1.0f; // ModulationFactors outputs (1 = neutral)
    float patrolScale = 1.0f;
};

// Per-pass world observations for the modulation pre-stage.  Defaults are
// the neutral case (noon, clear, GREEN, war 1): ModulationFactors leaves
// both scales at 1 and DecideSpawn behaves exactly as unmodulated.
struct TrafficModulationInput
{
    float dayFraction = 0.5f; // wall clock, [0,1)
    float nightEffect = 0.0f; // sun darkness, 0 day .. 1 night
    float rain = 0.0f;        // [0,1]
    float warLevel = 1.0f;
    int originAlertCiv = 0;      // civ route origin's AlertState (0 = ASGreen)
    bool originOccupied = false; // ... and whether the occupier owns it
};

// One zone as the pure route picker sees it.
struct TrafficZoneCandidate
{
    int index = -1; // ZoneRegistry index
    float x = 0;    // easting
    float z = 0;    // northing
    bool isCity = false;
    bool occupierOwned = false; // owner == the campaign's occupier side
};

// One observation for the pure commandeer predicate (engine axes; only X/Z
// are used).
struct CommandeerObs
{
    Vector3 carPos = VZero;
    Vector3 carDir = VForward;
    Vector3 playerPos = VZero;
    Vector3 playerDir = VForward;
    bool weaponInHands = false;
};

class Traffic : public SerializeClass
{
  public:
    Traffic() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static Traffic& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset, including event handlers (drops refs only)
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    void LoadFromConfig();
    // testable tuning parse; null leaves the defaults
    void LoadFromParams(const ParamEntry* zonesCfg);

    // queries ---------------------------------------------------------------
    bool IsActive() const; // ZoneRegistry active AND trafficEnabled
    const TrafficTuning& Tuning() const { return _tuning; }
    int Count(int kind) const; // live entries of a kind; kind < 0 = all
    int NEntries() const { return _entries.Size(); }
    Transport* EntryVehicle(int i) const; // may be null (dead link)
    // registry lookup by vehicle: false when the vehicle is not live traffic
    bool FindEntry(const Transport* veh, TrafficKind& kind, int& originIndex, int& destIndex,
                   TrafficState& state) const;
    // is this group the crew of a live traffic entry of the given kind
    // (the AlertMachine attribution hook; kind < 0 = any)
    bool IsTrafficGroup(const AIGroup* grp, int kind = -1) const;
    static const char* KindName(int kind);     // "civ" / "patrol" / "convoy"
    static int KindFromName(const char* name); // -1 unknown; "all" -> -1 too

    // events ----------------------------------------------------------------
    void SetEventHandler(TrafficEventType type, RString handler);
    RString GetEventHandler(TrafficEventType type) const;
    static int EventTypeFromName(const char* name); // -1 when unknown

    // pure logic (unit-tested; no world access) ------------------------------
    // One roll, rarest kind first: every eligible kind (cap not reached, a
    // route exists) gets its nominal chance; the roll is shifted past each
    // refused band.  Returns the TrafficKind to spawn, -1 for none.
    static int DecideSpawn(const TrafficDecisionInput& in, const TrafficTuning& tuning);
    // far-despawn edge: beyond radius + despawnHysteresis
    static bool ShouldDespawn(float playerDistSq, const TrafficTuning& tuning);
    // convoy chance grows with the war level, clamped to ConvoyChanceCap
    static constexpr float ConvoyChanceCap = 0.3f;
    static float ConvoyChance(const TrafficTuning& tuning, float warLevel);
    // per-kind chance scale factors from time of day, alert, curfew and
    // weather, composed multiplicatively; civScale ends in [0,1], patrolScale
    // in [0,inf) (DecideSpawn clamps the resulting chance).  Neutral inputs
    // leave both at 1.
    static constexpr float DayRampFraction = 2.0f / 24.0f; // trapezoid edge ramp
    static constexpr float AlertYellowCivScale = 0.4f;
    static constexpr float CurfewNightEffect = 0.5f; // darkness gate, agrees with the headlight system
    static void ModulationFactors(const TrafficModulationInput& in, const TrafficTuning& tuning, float& civScale,
                                  float& patrolScale);
    // route endpoints from the candidate table.  civ: CITY within radius of
    // the player -> another CITY 800..5000 m away (fallback any other CITY);
    // patrol: occupier-owned zone within radius -> another occupier-owned
    // zone; convoy: occupier-owned NON-CITY zone within radius -> another
    // occupier-owned zone.  roll (uniform [0,1)) picks among the candidates.
    // originZone >= 0 pins the origin (gmTrafficForceSpawn), skipping the
    // player-range gate.  False when no route exists.
    static constexpr float CivRouteMinDist = 800.0f;
    static constexpr float CivRouteMaxDist = 5000.0f;
    static bool PickRoute(int kind, const AutoArray<TrafficZoneCandidate>& zones, float playerX, float playerZ,
                          const TrafficTuning& tuning, float roll, int& originIndex, int& destIndex,
                          int originZone = -1);
    // spawn point: the candidate road point farthest from the player inside
    // the band [minSpawnDist, radius]; returns the index, -1 when none
    static int SelectSpawnPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, const TrafficTuning& tuning);
    // commandeer trigger: inside commandeerRadius AND (player ahead in the
    // lane: within 20 deg of the car's heading and commandeerLaneHalfWidth
    // of its line, OR weapon in hands aimed within 15 deg of the car)
    static bool CommandeerTriggered(const CommandeerObs& obs, const TrafficTuning& tuning);
    // stall rule: < StallMoveEpsilon m of movement per pass accumulates
    static constexpr float StallMoveEpsilon = 5.0f;
    static bool StallExpired(float stalledSeconds, const TrafficTuning& tuning);

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to interval (+ a 0.5 s
    // commandeer sub-tick while a civ car is near the player)
    void Simulate(float deltaT);
    // script/test aid: spawn one entry of a kind from a zone, bypassing the
    // chance roll and the caps (NOT the road placement).  Null on failure.
    Transport* ForceSpawn(int kind, int zoneIndex);
    // registry half of a commandeer: the entry leaves the live table and
    // the hull becomes a released world object.  False when not tracked.
    bool Release(Transport* veh);
    // test aid: a bookkeeping-only entry (no world objects) so the
    // save/load rows are unit-testable
    void MarkEntryForTest(TrafficKind kind, const char* originZone, const char* destZone, int legs);

    // save/load; live rows (vehicle/group refs + zone names), released
    // hulls, fleeing drivers, handlers
    LSError Serialize(ParamArchive& ar) override;

    // constants (engine facts, not island data)
    static constexpr float CommandeerSubTick = 0.5f;      // s
    static constexpr float CommandeerWatchRadius = 60.0f; // m, sub-tick arms inside this
    static constexpr float SpawnScanRadius = 700.0f;      // m of road net scanned around the origin
    static constexpr float FleeDeleteDist = 300.0f;       // m, bailed driver deleted beyond this...
    static constexpr float FleeDeleteAfter = 60.0f;       // ... or after this many seconds

  private:
    struct TrafficEntry
    {
        TrafficKind kind = TKCiv;
        TrafficState state = TSDriving;
        // OLink semantics: deleted objects read back as null and are pruned
        LLink<Transport> vehicle;
        LLink<Transport> escort; // convoy escort hull (null otherwise)
        LLink<AIGroup> group;
        LLink<Person> driver;
        RString originZone; // zone names (re-resolved to indices by name on load)
        RString destZone;
        int originIndex = -1; // transient
        int destIndex = -1;   // transient
        Vector3 dest = VZero; // destination road point (engine axes)
        int legs = 0;
        float stallTime = 0; // s of no movement
        float stateTime = 0; // s in the current state (commandeer timing)
        Vector3 lastPos = VZero;

        LSError Serialize(ParamArchive& ar);
    };

    // a hull that left the live table: commandeered, driver killed, or
    // destroyed.  Deleted when the player is beyond the despawn edge unless
    // somebody boarded it (then it is the player's - an ordinary object).
    struct ReleasedEntry
    {
        LLink<Transport> vehicle;
        LLinkArray<Person> bodies; // dead crew deleted with the hull
        LLink<AIGroup> group;
        bool boarded = false;

        LSError Serialize(ParamArchive& ar);
    };

    // a bailed civ driver running from the player
    struct FleeingDriver
    {
        LLink<Person> person;
        LLink<AIGroup> group;
        float age = 0;

        LSError Serialize(ParamArchive& ar);
    };

    struct TrafficEventRecord
    {
        TrafficEventType type;
        int kind;
        int originIndex;
        int destIndex;
        RString reason;
        LLink<Transport> vehicle;
        LLink<Person> driver;
    };

    // world-touching internals (engine path only)
    void BuildZoneCandidates(AutoArray<TrafficZoneCandidate>& out) const;
    bool CollectRoadSpots(Vector3Par center, AutoArray<Vector3>& pts, AutoArray<Vector3>& dirs) const;
    Transport* CreateTrafficVehicle(RString type, Vector3Par pos, Vector3Par dir) const;
    AIGroup* CreateTrafficGroup(const char* sideName) const;
    Person* CreateCrewman(AIGroup* grp, RString type, Vector3Par near, Transport* veh, int position) const;
    bool SpawnEntry(int kind, int originIndex, int destIndex, Vector3Par playerPos, Transport*& outVeh,
                    AutoArray<TrafficEventRecord>& fired);
    void IssueRoute(TrafficEntry& e, Vector3Par dest, int combatMode, int speedMode, bool column);
    void UpdateEntries(Vector3Par playerPos, bool playerValid, AutoArray<TrafficEventRecord>& fired);
    void UpdateCommandeer(float dt);
    void CleanupReleased(Vector3Par playerPos, bool playerValid);
    void CleanupFleeing(Vector3Par playerPos, bool playerValid, float dt);
    void DespawnEntry(int index, const char* reason, bool keepHull, AutoArray<TrafficEventRecord>& fired);
    void DeleteCrew(AIGroup* grp) const;
    void DispatchEvents(const AutoArray<TrafficEventRecord>& fired);
    void ApplyPendingLoad();
    int DestForOrigin(int kind, const AutoArray<TrafficZoneCandidate>& zones, int originIndex, float roll) const;

    TrafficTuning _tuning;
    AutoArray<TrafficEntry> _entries;
    AutoArray<ReleasedEntry> _released;
    AutoArray<FleeingDriver> _fleeing;
    RString _handlers[NTrafficEventTypes];
    float _accum = 0;
    float _subAccum = 0;
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<TrafficEntry> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
