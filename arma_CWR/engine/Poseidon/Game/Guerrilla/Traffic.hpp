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
// ordinary missions are unaffected.  Patrol/convoy traffic feeds the
// AlertMachine two ways: a violent end (destroyed / crewDead) queues a
// TrafficAmbush the alert tick drains into a per-zone knowledge floor,
// and a live traffic crew under fire between zones is attributed to the
// nearest occupier zone within trafficAmbushRadius (IsTrafficGroup).

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
class EntityAI;
class AmmoType;

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
    // civ danger response: drivers react to nearby gunfire/explosions and to
    // player-caused wrecks.  dangerRadius <= 0 disables the tier entirely
    // (and disarms the frozen-core shot/blast hooks' fast gate).
    float dangerRadius = 200.0f;     // trafficDangerRadius: reaction band at severity 1 (m)
    float dangerCloseRadius = 60.0f; // trafficDangerCloseRadius: the point-blank band (m, clamped <= dangerRadius)
    float dangerCooldown = 45.0f;    // trafficDangerCooldown: s of one-reaction-per-episode latch per entry
    float dangerTtl = 20.0f;         // trafficDangerTtl: s a ring-buffer danger episode stays live
};

enum TrafficKind
{
    TKCiv,
    TKPatrol,
    TKConvoy,
    NTrafficKinds
};

// One queued violent traffic end: a patrol or convoy that despawned
// "destroyed" or "crewDead".  Drained by the AlertMachine tick
// (ConsumeAmbushes), which floors the attributed zone's acted-on knowsAbout
// for trafficAmbushWindow seconds - the wiped-patrol alert.  Queued rows
// survive a save, same contract as the UndercoverSystem's pending
// compromises.
struct TrafficAmbush
{
    Vector3 pos = VZero;         // wreck position (engine axes)
    TrafficKind kind = TKPatrol; // never TKCiv
    // the entry's origin zone name: the attribution fallback, honoured
    // only while the occupier still holds that zone
    RString originZone;

    LSError Serialize(ParamArchive& ar);
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
    TSPanicked,  // danger cower: braked in place, crew seated, resumes once the shooting stops
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

// blocked-reaction tier: what a still-driving car that stopped progressing
// mid-leg should try BEFORE the stallTimeout ladder gives up on it.  Stopped
// hulls lock their road links, and both the road A* and VerifyPath respect
// locks, so a re-issued leg detours around a blockage whenever any way
// around exists; the U-turn covers the road home when none does.
enum TrafficBlockedAction
{
    TBlockNone,     // below the staged threshold, still rolling, or retries spent
    TBlockRetryLeg, // re-issue the same leg: the fresh plan sees the locked links
    TBlockUTurn,    // no way through: swap the endpoints and drive back
};

// danger-reaction tier (civ danger response): what a civilian entry does
// about nearby gunfire, an explosion or a fresh player-caused wreck.  One
// reaction per cooldown episode; the commandeer sub-tick always wins over a
// reaction (it runs first and parks the entry in TSStopping).
enum TrafficDangerReaction
{
    TDRNone,  // out of band, wrong kind/state, or the cooldown latch holds
    TDRCower, // brake and duck: TSPanicked until the neighbourhood quiets
    TDRUTurn, // swap the endpoints and flee for home at full speed
    TDRRush,  // keep the leg, floor it (accelerate past the trouble)
    TDRBail,  // the driver abandons the car and runs from the danger
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
    TEPanicked,     // "panicked" [veh, kind, reaction] (reaction = DangerReactionName)
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

// one coalesced danger episode in the transient ring buffer (never
// serialized; events landing near a live episode merge into it, so automatic
// fire stays one episode instead of flooding the ring)
struct TrafficDangerEvent
{
    Vector3 pos = VZero;   // engine axes
    float severity = 1.0f; // dimensionless; ~1 = a rifle shot (audibleFire / DangerRifleAudibleFire)
    bool playerCaused = false;
    float age = 0; // s since the newest coalesced constituent
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
    // drain the queued violent patrol/convoy ends into out (cleared first);
    // called by the AlertMachine tick
    void ConsumeAmbushes(AutoArray<TrafficAmbush>& out);
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
    // blocked recovery, staged INSIDE the stall window so the anti-leak bound
    // is unchanged: retry the leg at stallTimeout/3, U-turn at 2/3, and the
    // expiry ladder still fires at stallTimeout for a car that defeats both.
    // nearStopped separates genuinely blocked (speed ~0) from merely slow
    // (an off-road crawl or a queued car accrues stall time too); retries
    // reset with the stall clock the moment the car actually moves
    static constexpr int MaxBlockedRetries = 2;
    static constexpr float BlockedSpeedEpsilon = 0.5f; // m/s that still counts as standing
    static TrafficBlockedAction DecideBlocked(float stalledSeconds, int retries, bool nearStopped,
                                              const TrafficTuning& tuning);
    // danger response (civ danger response) ---------------------------------
    // ring-buffer ops over the transient episode buffer: coalesce within
    // DangerCoalesceRadius (max severity, OR-ed playerCaused, age refreshed),
    // a full ring evicts the oldest episode, AgeDangerEvents expires at ttl
    static constexpr int MaxDangerEvents = 8;
    static constexpr float DangerCoalesceRadius = 50.0f; // m
    static void AddDangerEvent(AutoArray<TrafficDangerEvent>& buf, Vector3Par pos, float severity, bool playerCaused);
    static void AgeDangerEvents(AutoArray<TrafficDangerEvent>& buf, float dt, float ttl);
    // index of the episode nearest pos (2D), -1 when the buffer is empty;
    // outDist gets the winner's distance (-1 when none)
    static int NearestDanger(const AutoArray<TrafficDangerEvent>& buf, Vector3Par pos, float& outDist);
    // the reaction decision.  severity scales both bands through
    // sqrt(severity) clamped to [DangerScaleMin, DangerScaleMax]; the roll
    // splits each band's outcomes; cooldownLeft > 0 is the per-entry
    // one-reaction-per-episode latch.  Only TKCiv reacts (patrols/convoys
    // keep their native combat AI); TSStopping/TSExiting stay the
    // commandeer's, TSPanicked already reacted; the parked family and the
    // ended states (stalled/lingering) always bail - there is no live leg
    // left to drive
    static constexpr float DangerScaleMin = 0.5f;
    static constexpr float DangerScaleMax = 1.5f;
    static constexpr float DangerRifleAudibleFire = 8.0f;   // audibleFire that reads as severity 1
    static constexpr float DangerExplosionSeverity = 2.25f; // any real blast maxes the band (DangerScaleMax squared)
    static constexpr float DangerBlastPowerMin = 4.0f;      // indirectHit * indirectHitRange below this is no blast
    static constexpr float WreckDangerSeverity = 0.5f;      // a player-caused wreck: a narrowed band
    static constexpr float WreckDangerTtlScale = 3.0f;      // wrecks radiate danger for this many dangerTtl
    static constexpr float DangerCowerHold = 20.0f;         // s a cowering car holds before re-checking the ring
    // severity mappings for the two engine feeds, pure so the post-merge
    // in-game probe can recalibrate a single constant each.
    // DangerSeverityFromBlast returns 0 for anything that is not a real
    // blast - Landscape::ExplosionDammage runs for EVERY projectile impact
    // (ShotShell ground and object hits included), so plain bullet impacts
    // must map to no episode
    static float DangerSeverityFromAudible(float audibleFire);
    static float DangerSeverityFromBlast(bool explosive, float indirectHit, float indirectHitRange);
    // how long a fresh player-caused wreck radiates danger before it is set
    // dressing (without the cutoff every civ parking near an old ambush
    // site would bail forever, each bail adding another hull)
    static bool WreckDangerLive(float wreckAge, const TrafficTuning& tuning);
    static constexpr float DangerCloseCowerBand = 0.5f; // close band: roll < this = cower
    static constexpr float DangerCloseBailBand = 0.8f;  // ... < this = bail; the rest U-turns
    static constexpr float DangerFarUTurnBand = 0.5f;   // far band: roll < this = U-turn
    static constexpr float DangerFarRushBand = 0.75f;   // ... < this = accelerate past (probe-gated: set equal to
                                                        // DangerFarUTurnBand to disable the rush branch); rest cowers
    static TrafficDangerReaction DecideDangerReaction(float distance, float severity, int kind, TrafficState state,
                                                      float cooldownLeft, float roll, const TrafficTuning& tuning);
    static const char* DangerReactionName(int reaction); // "cower"/"uturn"/"rush"/"bail" ("none" otherwise)
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
    // danger feed (the frozen-core hooks; see TrafficNotifyShotFast below).
    // NotifyShot runs per ROUND from every firing entity while armed, so the
    // cheap early-outs come before any math
    void NotifyShot(EntityAI* shooter, float audibleFire);
    void NotifyExplosion(AIUnit* ownerUnit, Vector3Par pos, const AmmoType* type);
    void NotifyDanger(Vector3Par pos, float severity, bool playerCaused);
    // script/test aid: spawn one entry of a kind from a zone, bypassing the
    // chance roll and the caps (NOT the road placement).  Null on failure.
    Transport* ForceSpawn(int kind, int zoneIndex);
    // registry half of a commandeer: the entry leaves the live table and
    // the hull becomes a released world object.  False when not tracked.
    bool Release(Transport* veh);
    // test aid: a bookkeeping-only entry (no world objects) so the
    // save/load rows are unit-testable
    void MarkEntryForTest(TrafficKind kind, const char* originZone, const char* destZone, int legs);
    // test aid: queue an ambush stimulus without world objects (the engine
    // path queues from DespawnEntry's violent ends)
    void QueueAmbushForTest(Vector3Par pos, TrafficKind kind, const char* originZone);
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
        float stallTime = 0;      // s of no movement
        int blockedRetries = 0;   // recovery attempts this stall episode (reset with stallTime)
        float stateTime = 0;      // s in the current state (commandeer/park timing)
        float dwellDuration = 0;  // s; transient like stateTime, re-rolled on load
        float exposeDefer = 0;    // s a wanted despawn has been perception-blocked; transient, NOT serialized
        float combatHold = 0;     // s the combat gate has held this episode; transient, NOT serialized
                                  // (the gate re-derives from the group's serialized disclosure on load)
        float dangerCooldown = 0; // s left of the one-reaction-per-episode danger latch; transient, NOT serialized
        RString lingerReason;     // TSLingering: the despawn reason once unobserved ("arrived"/"stalled")
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
        // s since release: the wreck danger-source cutoff clock (see
        // WreckDangerLive).  Transient, NOT serialized - a loaded wreck
        // restarts its (bounded) radiating window, which is acceptable
        float wreckAge = 0;

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
    // memory, and TEBailed marks the loot moment for scripts.  With fleeFrom
    // (the danger bail) the crew RUNS from the danger point instead, and
    // playerCaused marks the released hull's memory (a bail the player's
    // fire forced)
    void AbandonEntry(int index, const char* reason, AutoArray<TrafficEventRecord>& fired, bool panic = false,
                      const Vector3* fleeFrom = nullptr, bool playerCaused = false);
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
    // per-pass danger snapshot: the live ring episodes plus the player-caused
    // wrecks the released table already tracks (a fresh wreck needs no
    // engine hook)
    void BuildDangerSources();

    TrafficTuning _tuning;
    AutoArray<TrafficEntry> _entries;
    AutoArray<ReleasedEntry> _released;
    AutoArray<FleeingDriver> _fleeing;
    RString _handlers[NTrafficEventTypes];
    float _accum = 0;
    float _subAccum = 0;
    Perception _percept; // transient per-pass snapshot
    // violent patrol/convoy ends awaiting the AlertMachine drain
    AutoArray<TrafficAmbush> _ambushes;
    // danger ring: shot/blast episodes since ~dangerTtl (transient, never
    // serialized), and the per-pass source snapshot (_danger + wrecks)
    AutoArray<TrafficDangerEvent> _danger;
    AutoArray<TrafficDangerEvent> _dangerNow;
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<TrafficEntry> _pending;
};

// ---------------------------------------------------------------------------
// frozen-core danger hooks (EntityAI::FireWeaponEffects and
// Landscape::ExplosionDammage call these).  Global-bool fast gate in the
// GUndercoverActive mold: armed only while the traffic service is live,
// tracks entries and has the danger response enabled, so the per-round call
// in the 99.9% case costs one bool read and no math.  Kept in sync by
// Traffic::Simulate and Traffic::Clear.
extern bool GTrafficDangerArmed;

inline void TrafficNotifyShotFast(EntityAI* shooter, float audibleFire)
{
    if (!GTrafficDangerArmed)
    {
        return;
    }
    Traffic::Instance().NotifyShot(shooter, audibleFire);
}

inline void TrafficNotifyExplosionFast(AIUnit* ownerUnit, Vector3Par pos, const AmmoType* type)
{
    if (!GTrafficDangerArmed)
    {
        return;
    }
    Traffic::Instance().NotifyExplosion(ownerUnit, pos, type);
}

} // namespace Guerrilla
} // namespace Poseidon
