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
    // civ arrival park-vs-despawn roll.  Parked cars remain live entries and
    // count against maxCiv, so a town accumulates at most maxCiv parked cars
    // (a separate trafficMaxParked cap is deferred).  0 restores the pre-park
    // behaviour exactly.
    float parkChance = 0.6f;     // trafficParkChance
    float parkDwellMin = 60.0f;  // trafficParkDwellMin (s)
    float parkDwellMax = 180.0f; // trafficParkDwellMax (s)
    // perception gate (view-distance aware spawn/despawn, issue #53).  The
    // live object-cull distance pushes the band out (EffectiveBand); the
    // radius/minSpawnDist/despawnHysteresis keys above are floors, not caps.
    float exposeMargin = 150.0f;   // trafficExposeMargin: m past the cull that counts as imperceptible
    float despawnDeferMax = 90.0f; // trafficDespawnDeferMax: s a perception-blocked despawn may defer (0 = no defer)
    bool scaleCaps = false; // trafficScaleCaps: scale maxCiv/maxPatrols with the widened band (ScaleCap; identity
                            // whenever the band is not widened, i.e. daytime at the default view distance - but the
                            // night light bound widens the band at ANY setting, so 1 raises night densities; kept
                            // off for the pre-#53 densities)
    // convoy discipline under fire.  The combat gate freezes the trip ladder
    // (stall accrual, arrival/stall endings, re-legs) while a patrol/convoy
    // group is fighting or freshly disclosed - the native AI halts, dismounts
    // cargo and fights, and no traffic order may interfere - bounded by the
    // hold budget so a pathologically hot group cannot leak forever.
    float combatStaleAfter = 120.0f; // trafficCombatStaleAfter (s): disclosure older than this is stale
    float combatHoldMax = 300.0f;    // trafficCombatHoldMax (s): per-episode hold budget (0 = gate off)
    float bailCombatWindow = 60.0f;  // trafficBailCombatWindow (s): escort lost within this of the last
                                     // disclosure = the truck crew bails (0 = never)
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
    TSStopping,  // commandeer: Stop issued, waiting out commandeerStopDelay
    TSExiting,   // commandeer: driver bailed (transient, one tick)
    TSArrived,   // at the destination, waiting for the player to be far enough
    TSStalled,   // stall timer expired, waiting for the player to be far enough
    TSParking,   // civ arrived + park roll won: Stop issued, waiting out the brake delay
    TSDwelling,  // driver on foot near the parked car, dwell timer running
    TSDeparting, // driver ordered back in; waiting for DriverBrain()==unit
    TSLingering, // observed trip end (issue #53): stopped in place, crew seated, despawn once unobserved
    NTrafficStates
};

// what to do with an entry whose trip has ended: arrival with a losing (or
// no) park roll, or an expired stall.  Pure decision over the perception
// verdict; the world layer supplies despawnSafe (DespawnSafe) and executes.
enum TrafficEndAction
{
    TEndDespawn, // unobserved: delete outright (the pre-#53 semantics)
    TEndReLeg,   // observed with legs left: drive on to another zone
    TEndAbandon, // observed civ stall: driver dismounts and walks off, hull released
    TEndLinger,  // observed, no continuation: stop where it is, crew stays seated
};

enum TrafficEventType
{
    TESpawned,      // "spawned"      [veh, kind, originIdx, destIdx]
    TEDespawned,    // "despawned"    [kind, reason]
    TECommandeered, // "commandeered" [veh, driver]
    TEArrived,      // "arrived"      [veh, kind, destIdx]
    TEDriverKilled, // "driverKilled" - killed-EH EXPRESSION attached to every civ driver
    TEParked,       // "parked"   [veh, kind, destIdx]
    TEDeparted,     // "departed" [veh, kind, destIdx] (destIdx = the NEW destination)
    TEBailed,       // "bailed"   [veh, kind, destIdx] - escort lost under fire, the truck crew
                    // abandons the load (the loot moment)
    NTrafficEventTypes
};

// combat gate verdict for one patrol/convoy entry per pass (convoy
// discipline under fire).  The world layer accrues the entry's held time on
// TCGHold, resets it on TCGClear, and lets the ladder resume on TCGExhausted
// WITHOUT resetting - the budget stays spent until the episode really clears,
// so an exhausted gate can never flip back to holding.
enum TrafficCombatGate
{
    TCGClear,     // no combat, disclosure stale: the ladder runs, budget resets
    TCGHold,      // fighting or freshly disclosed: freeze stall accrual, no trip endings, no new orders
    TCGExhausted, // still hot but the budget ran out: the ladder resumes (bounded escape)
};

// what DespawnEntry(keepHull) does with one crew member
enum TrafficCrewDisposal
{
    TCDBody,         // dead: filed with the hull, deleted with it
    TCDFlee,         // living on foot: the fleeing table, which respects life
    TCDDismountFlee, // living but seated: step out first, then the fleeing table
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

// The band the spawn/despawn rules actually use: the config band pushed out
// past the live object-cull distance so no creation or deletion ever happens
// inside draw range.  Config values are floors; a pushed-out floor only
// narrows the band, and the radius widens only once less than a usable
// width (min(config width, SpawnScanRadius)) remains above the floor - so
// the band never degenerates to empty, yet the radius and despawn edge stay
// at their config values at the daytime default view distance (the spec's
// defaults-preserve-behaviour claim).  Traffic::ConfigBand is the identity
// case (force spawns and the no-camera fallback).
struct TrafficEffectiveBand
{
    float minSpawn = 300.0f;     // spawn distance floor (softened by the LOS/frustum escapes), also the
                                 // imperceptibility bound: beyond it nothing is drawn, so a teardown is invisible
    float radius = 1500.0f;      // spawn band cap, also the PickRoute origin gate
    float despawnEdge = 1800.0f; // far-despawn beyond this
    float closeHold = 300.0f;    // the config minSpawnDist verbatim: never tear down inside this even
                                 // hidden - an idling engine is audible where the mesh is not visible
};

// Per-point perception observations, filled by the world layer (camera cone
// + terrain ray); unit tests inject values.  The defaults are the
// no-information case: nothing hidden, everything in view, so only the
// distance legs of the CanExpose predicates apply.
struct TrafficExposeObs
{
    bool losBlocked = false; // terrain hides the point from the camera
    bool inFrustum = true;   // within the generous view cone of the camera heading
    float camDist2 = -1.0f;  // squared 2D distance from the CAMERA (the actual viewer: a scripted
                             // camera can sit far from the player); < 0 = unknown, the distance
                             // legs fall back to the player distance
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
    // the entry's current destination road point; false when not live traffic
    bool EntryDest(const Transport* veh, Vector3& out) const;
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
    // ... and against a live effective band (beyond band.despawnEdge)
    static bool ShouldDespawn(float playerDistSq, const TrafficEffectiveBand& band);
    // perception gate (issue #53).  EffectiveBand derives the live band from
    // the engine object-cull distance: anything past cull + exposeMargin is
    // imperceptible by construction (vehicles already pop at the cull).
    // lightsOn raises the cull to the night light bound - light sources draw
    // out to horizontZ + NightLightCullMargin, farther than the mesh, and AI
    // cars auto-light headlights at night.
    static constexpr float NightLightCullMargin = 500.0f; // the Scene light-enumeration bound past horizontZ
    static constexpr float HeadlightNightEffect = 0.2f;   // darkest randomized AI headlight threshold (TransportCore)
    static TrafficEffectiveBand EffectiveBand(const TrafficTuning& tuning, float objectsZ, bool lightsOn,
                                              float horizontZ);
    // the pre-perception band: the config values verbatim
    static TrafficEffectiveBand ConfigBand(const TrafficTuning& tuning);
    // a spawn at dist2 is imperceptible when ANY leg passes: beyond the
    // effective floor, terrain-hidden, or outside the view cone
    static bool CanExposeSpawn(float dist2, bool losBlocked, bool inFrustum, const TrafficEffectiveBand& band);
    // a despawn is imperceptible only OUTSIDE the view cone AND (beyond the
    // imperceptibility bound (band.minSpawn: past the cull nothing is drawn)
    // OR terrain-hidden), and NEVER inside band.closeHold (audible even
    // unseen - the pre-#53 close-range hold); the world layer defers a
    // blocked despawn, hard-bounded by trafficDespawnDeferMax
    static bool CanExposeDespawn(float dist2, bool losBlocked, bool inFrustum, const TrafficEffectiveBand& band);
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
    // band widens the origin gate along with the spawn band (null = the
    // config radius) - all three radius consumers must move together or a
    // pushed-out band leaves no origins in reach
    static bool PickRoute(int kind, const AutoArray<TrafficZoneCandidate>& zones, float playerX, float playerZ,
                          const TrafficTuning& tuning, float roll, int& originIndex, int& destIndex,
                          int originZone = -1, const TrafficEffectiveBand* band = nullptr);
    // spawn point: the candidate road point farthest from the player inside
    // the band [minSpawnDist, radius]; returns the index, -1 when none
    static int SelectSpawnPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, const TrafficTuning& tuning);
    // ... perception-aware variant: the config minSpawnDist stays a HARD
    // floor (a close spawn is audible even unseen), the cap is band.radius,
    // and every candidate must pass CanExposeSpawn.  obs runs parallel to
    // roadPts (null = the no-information defaults, i.e. distance-only).
    // Scoring is tiered (alibi spawns, issue #53 T2): a point that survives
    // an immediate turn-around (beyond the exposure floor or terrain-hidden)
    // beats one hidden only by the view cone; when preferOrigin (patrols and
    // convoys pulling out of their base), a candidate inside AlibiOriginRadius
    // of originPos beats everything - a car appearing inside its own base is
    // a plausible birth even when the base is watched.  Farthest from the
    // player breaks ties within a tier.
    static int SelectSpawnPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, const TrafficTuning& tuning,
                                const TrafficEffectiveBand& band, const AutoArray<TrafficExposeObs>* obs,
                                const Vector3* originPos = nullptr, bool preferOrigin = false);
    // alibi fallback (issue #53 T2): no imperceptible driving spawn exists,
    // so pick a roadside point INSIDE the origin zone to spawn the car
    // PARKED and pull out via the depart machinery (a car leaving the curb
    // is a plausible birth even directly observed).  The config minSpawnDist
    // stays the hard floor; hidden candidates (terrain or cone) are
    // preferred, then the farthest from the player.  -1 when none.
    static constexpr float AlibiOriginRadius = 300.0f; // m of the zone centre that counts as "inside the town"
    static int SelectAlibiPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, Vector3Par originPos,
                                const TrafficTuning& tuning, const AutoArray<TrafficExposeObs>* obs);
    // density scaling (issue #53 T6, trafficScaleCaps): traffic lives on
    // roads, so the caps scale LINEARLY with the widened band (rounded),
    // never below the configured value; identity when the band is not
    // widened.  Convoys are deliberately unscaled (rare by design).
    static int ScaleCap(int cap, float effRadius, float configRadius);
    // commandeer trigger: inside commandeerRadius AND (player ahead in the
    // lane: within 20 deg of the car's heading and commandeerLaneHalfWidth
    // of its line, OR weapon in hands aimed within 15 deg of the car)
    static bool CommandeerTriggered(const CommandeerObs& obs, const TrafficTuning& tuning);
    // stall rule: < StallMoveEpsilon m of movement per pass accumulates
    static constexpr float StallMoveEpsilon = 5.0f;
    static bool StallExpired(float stalledSeconds, const TrafficTuning& tuning);
    // civ arrival park roll: roll < parkChance
    static bool DecidePark(float roll, const TrafficTuning& tuning);
    // load downgrade for the three park states, keyed on whether the saved
    // driver came back seated (transient flags never survive the load, so an
    // on-foot departer re-dwells and re-issues the get-in from scratch)
    static TrafficState LoadedParkState(TrafficState saved, bool driverSeated);
    // observed endings (issue #53).  A trip that wants to end while the
    // player can perceive the car never hard-despawns: arrivals re-leg while
    // legs remain and then linger; stalls hand a civ driver to the walk-off
    // (the flee machinery at walking pace) and keep other crews seated
    static TrafficEndAction ArrivedEndAction(bool despawnSafe, int legs, int maxLegs);
    static TrafficEndAction StalledEndAction(bool despawnSafe, int kind);
    // convoy discipline under fire --------------------------------------------
    // combat gate for the patrol/convoy trip ladder.  inCombat is
    // GetCombatModeMinor() >= CMCombat (the exact threshold Car::IsCautious
    // uses to drop out of convoy-follow); sinceDisclosed is seconds since the
    // group's last AIGroup::Disclose (never disclosed reads as huge = stale)
    static TrafficCombatGate CombatGateAction(bool inCombat, float sinceDisclosed, float heldTime,
                                              const TrafficTuning& tuning);
    // escort dead/destroyed while the fight is recent: the unescorted truck
    // crew bails (AbandonEntry panic flavor) instead of driving on
    static bool ConvoyBailTriggered(bool escortExisted, bool escortDead, float sinceDisclosed,
                                    const TrafficTuning& tuning);
    // DespawnEntry(keepHull) crew filing: only the actual dead ride
    // ReleasedEntry::bodies (which CleanupReleased deletes wholesale)
    static TrafficCrewDisposal CrewDisposal(bool personDead, bool seated);

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
    // test/debug aid (gmTrafficPercept): take a fresh perception snapshot
    // and report the verdict for one point - whether a camera exists, the
    // per-point observations and the live effective band the next pass's
    // spawn/despawn decisions will use
    void PerceptProbe(Vector3Par pos, bool& hasCamera, bool& losBlocked, bool& inFrustum, TrafficEffectiveBand& band);

    // save/load; live rows (vehicle/group refs + zone names), released
    // hulls, fleeing drivers, handlers
    LSError Serialize(ParamArchive& ar) override;

    // constants (engine facts, not island data)
    static constexpr float CommandeerSubTick = 0.5f;       // s
    static constexpr float CommandeerWatchRadius = 60.0f;  // m, sub-tick arms inside this
    static constexpr float SpawnScanRadius = 700.0f;       // m of road net scanned around the origin
    static constexpr float FleeDeleteDist = 300.0f;        // m, bailed driver deleted beyond this...
    static constexpr float FleeDeleteAfter = 60.0f;        // ... or after this many seconds
    static constexpr float ParkWanderRadius = 40.0f;       // m, dwell stroll cap
    static constexpr int ParkWanderOdds = 6;               // 1-in-6 per main tick
    static constexpr float DepartTimeout = 45.0f;          // s in TSDeparting before fallback
    static constexpr float FrustumCosHalfAngle = 0.5f;     // generous view cone: +-60 deg of the camera heading
    static constexpr float LosTargetHeight = 2.5f;         // m above the road the terrain ray aims (a car roof)
    static constexpr int MaxLosProbes = 8;                 // terrain rays per spawn attempt (farthest first)
    static constexpr float PlayerCausedLingerScale = 2.0f; // released despawn-edge multiplier for player-caused remains

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
        float stallTime = 0;     // s of no movement
        float stateTime = 0;     // s in the current state (commandeer/park timing)
        float dwellDuration = 0; // s; transient like stateTime, re-rolled on load
        float exposeDefer = 0;   // s a wanted despawn has been perception-blocked; transient, NOT serialized
        float combatHold = 0;    // s the combat gate has held this episode; transient, NOT serialized
                                 // (the gate re-derives from the group's serialized disclosure on load)
        RString lingerReason;    // TSLingering: the despawn reason once unobserved ("arrived"/"stalled")
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
        // issue #53 T4: the player remembers where he ambushed a patrol -
        // wrecks and corpses he caused keep a PlayerCausedLingerScale longer
        // despawn edge than ambient set dressing.  Set on the violent ends
        // (destroyed/crewDead hulls, the commandeer bail); the cheap proxy
        // for "player-caused" - ambient traffic has no other enemies
        bool playerCaused = false;

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
    // forced (gmTrafficForceSpawn) bypasses the perception gate along with
    // the chance roll: the tests spawn deliberately close to the player
    bool SpawnEntry(int kind, int originIndex, int destIndex, Vector3Par playerPos, Transport*& outVeh,
                    AutoArray<TrafficEventRecord>& fired, bool forced = false);
    void IssueRoute(TrafficEntry& e, Vector3Par dest, int combatMode, int speedMode, bool column);
    void UpdateEntries(Vector3Par playerPos, bool playerValid, AutoArray<TrafficEventRecord>& fired);
    // handles one entry in TSParking/TSDwelling/TSDeparting; may despawn/release it
    void UpdateParked(int index, Vector3Par playerPos, bool playerValid, AutoArray<TrafficZoneCandidate>& zones,
                      AutoArray<TrafficEventRecord>& fired);
    void UpdateCommandeer(float dt);
    void CleanupReleased(Vector3Par playerPos, bool playerValid);
    void CleanupFleeing(Vector3Par playerPos, bool playerValid, float dt);
    void DespawnEntry(int index, const char* reason, bool keepHull, AutoArray<TrafficEventRecord>& fired);
    // observed endings (issue #53) -------------------------------------------
    // observed civ stall: the living crew dismounts and walks off (the
    // fleeing table at walking pace), the hull joins the released set
    // dressing; both are deleted by the perception-gated cleanups once
    // unobserved.  panic is the under-fire flavor (escort lost): the crew
    // RUNS from the player at full speed, the remains keep the player-caused
    // memory, and TEBailed marks the loot moment for scripts
    void AbandonEntry(int index, const char* reason, AutoArray<TrafficEventRecord>& fired, bool panic = false);
    // observed trip end for a crew that stays seated: brake to a stop and
    // hold in TSLingering until DespawnSafe says nobody is watching
    void EnterLinger(TrafficEntry& e, const char* reason);
    void DeleteCrew(AIGroup* grp) const;
    // perception (world layer; refreshed once per traffic pass) --------------
    // no camera (dedicated server) = no perception: the config band and the
    // pre-perception distance rules apply verbatim
    struct Perception
    {
        bool hasCamera = false;
        Vector3 camPos = VZero;
        bool camDirValid = false;  // false = near-vertical camera: no usable 2D heading, the
                                   // cone test degrades to "everything in frustum" (conservative)
        Vector3 camDir = VForward; // 2D unit heading (meaningless while !camDirValid)
        TrafficEffectiveBand band; // ConfigBand when no camera
    };
    void RefreshPerception();
    bool PointInFrustum(Vector3Par pos) const;  // generous 2D cone test
    bool PointLosBlocked(Vector3Par pos) const; // terrain ray from the camera
    // per-candidate spawn observations (parallel to pts): the cheap cone test
    // for every point, terrain rays budgeted MaxLosProbes farthest-first
    // among the in-cone points inside the exposure floor (every other point
    // already passes CanExposeSpawn without a ray).  No camera = defaults.
    void BuildExposeObs(const AutoArray<Vector3>& pts, Vector3Par playerPos, AutoArray<TrafficExposeObs>& obs) const;
    // is tearing something down at pos imperceptible right now; legacyEdge is
    // the pre-perception distance rule for the no-camera fallback.  band
    // overrides the pass snapshot's (the player-caused released rows use a
    // PlayerCausedLingerScale-widened edge); null = _percept.band
    bool DespawnSafe(Vector3Par pos, float playerD2, float legacyEdge,
                     const TrafficEffectiveBand* band = nullptr) const;
    // gate one wanted despawn: true = go ahead (safe, or the defer timer ran
    // out); false accrues the entry's defer timer
    bool GateDespawn(TrafficEntry& e, Vector3Par pos, float playerD2, float legacyEdge);
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
    Perception _percept; // transient per-pass snapshot
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<TrafficEntry> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
