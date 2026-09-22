#pragma once

// Guerrilla Mode enemy Legends (Change 3): where the three named enemy
// commanders stand, and what each of them IS.
//
// Two halves, deliberately split so the interesting judgement is testable
// without a world:
//   * PURE - PickLegendSpots ranks injected candidate stands; ResolveLegendRoles
//     turns a faction's capability into three boss profiles;
//     LegendPlacementPrecheck answers "can this island seat three commanders?"
//     analytically, with no landscape at all.
//   * ENGINE - BuildLegendSamples fills the candidate stands from the live
//     terrain (Landscape surface Y, road net, world place names), and
//     ReadLegendCapability reads the ALREADY-RESOLVED faction descriptor
//     (the plan-15 pass has run) plus one type-bank probe for the tank hull.
//
// PLACEMENT GEOMETRY IS THE REGISTRY'S OWN, NOT THE ZONE'S.  Stands sit on a
// 350..950 m annulus around a military zone, with a 400 m floor between any two
// stands and a 400 m floor off the player's Camp.  The inner ring is well
// outside the default 150 m zone presence radius (ZoneTuning::zoneArea), which
// is the decision this file encodes: a commander is a SEPARATE hunt, he does not
// garrison his zone, his existence never blocks that zone's capture and never
// pins its alert state.  zoneArea is mission-configurable, so a template that
// raises it past Rings[0] reverses that reading - LegendPlacementPrecheck says
// so in its diagnostic when it is told the template's zoneArea.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
namespace Guerrilla
{

// ---------------------------------------------------------------------------
// placement
// ---------------------------------------------------------------------------

// One zone the placement pass may hang a commander off.  Built from the zone
// table (name/type/pos) plus "does the occupier own it right now".
struct LegendZoneCandidate
{
    RString name;
    RString type; // "CAMP" | "OUTPOST" | "AIRFIELD" | "SEAPORT" | "CITY"
    bool occupied = false;
    float x = 0; // easting
    float z = 0; // northing
};

// One candidate stand.  The engine path fills these from the live world; unit
// tests inject them directly, which is the whole point of the split.
struct LegendSpotSample
{
    int zone = -1;           // index into the candidate zone array
    float x = 0, z = 0;      // easting / northing
    float height = 0;        // terrain Y at the stand
    float distFromZone = 0;  // m from the zone centre (the ring radius)
    float distFromCamp = 0;  // m from the player's Camp, PER SAMPLE
    bool onRoad = false;     // hard reject
    bool underwater = false; // hard reject
    bool named = false;      // a world place name stands near this spot
};

struct LegendPlacementConstants
{
    static constexpr int NRings = 4;
    static constexpr float Rings[NRings] = {350.0f, 550.0f, 750.0f, 950.0f};
    static constexpr int Bearings = 12;             // 30 deg apart
    static constexpr float SameZoneFloor = 400.0f;  // min m between two stands off ONE zone
    static constexpr float CrossZoneFloor = 400.0f; // ... and between stands off two zones
    static constexpr float CampPreferred = 800.0f;  // preference, relaxes to CampFloor
    static constexpr float CampFloor = 400.0f;      // hard reject below this
    // Hard reject this close to ANY OTHER zone centre, the player's Camp and
    // the auto-seeded towns included.  Standing inside a town's presence
    // radius would make a commander count as occupier presence there: it
    // freezes that town's support at the floor for the whole campaign and can
    // pin its alert, which is exactly the coupling the ring geometry exists to
    // avoid.  A stand can never be this close to its OWN zone (the inner ring
    // is 350 m), so the rule needs no self-exclusion.
    static constexpr float ForeignZoneFloor = 300.0f;
    static constexpr float MinDryHeight = 0.1f;    // sea level is Y=0; keep off the wash
    static constexpr float RoadClearance = 6.0f;   // GRoadNet::IsOnRoad probe size
    static constexpr float NamedNearMin = 300.0f;  // a place name this far from the zone
    static constexpr float NamedNearMax = 1200.0f; // ... but no farther, can name a stand
    static constexpr int Wanted = 3;               // three commanders per campaign
};

// Why fewer than Wanted stands survived.  LPOk means all three were seated.
enum LegendPlacementReason
{
    LPOk = 0,
    LPNoZones,       // no candidate zone / no sample at all
    LPAllUnderwater, // every sample stands in the sea
    LPAllOnRoad,     // every dry sample stands on the road net
    LPTooCrowded     // terrain was fine; the distance floors refused the rest
};

struct LegendPlacementResult
{
    AutoArray<int> picked;    // indices into the sample array, in pick order
    int reason = LPOk;        // LegendPlacementReason
    bool campRelaxed = false; // the 800 m Camp preference had to fall to 400 m
};

// PURE.  Greedy, one stand at a time, hard rejects first (on the road, in the
// sea, inside CampFloor of the Camp, or inside the separation floor of a stand
// already picked), then a TOTAL preference order:
//   distinct zone > named > distFromCamp >= CampPreferred > larger spread from
//   the picked stands > larger Camp distance > lower sample index.
// The order ends at the sample index on purpose: no comparison ever leans on
// sort stability, so the same geometry yields the same stands on every compiler.
// `seed` is the campaign seed.  It is accepted (and persisted by the caller) so
// this call site never has to change if a future revision wants to jitter
// equally ranked candidates; the ranking as it stands is fully deterministic and
// does not consult it.
LegendPlacementResult PickLegendSpots(const AutoArray<LegendSpotSample>& samples, unsigned seed);

// PURE and analytic: no landscape, no roads, no place names.  Answers the
// question a template author needs answered at authoring time - does this zone
// table have a non-CITY non-CAMP zone at all, and do the annuli around the zones
// the ladder would use hold three stands that clear the separation and Camp
// floors?  Fills `diagnostic` with a sentence naming the problem and returns
// false when they do not; leaves it EMPTY on success, except that a template
// whose zoneArea reaches the inner ring gets a diagnostic (and still true)
// because such a template silently reverses the outside-the-presence-radius
// reading this file is built on.  Terrain can still refuse a stand the analytic
// pass accepted - this is a cheap pre-filter, not a promise.
//
// NO PRODUCTION CALLER TODAY, and deliberately not wired into the guerrilla
// scaffold: that command GENERATES the zone table it would be asked to check
// (and splices an authored one through verbatim under --keep-zones), so it
// would only ever check a table that passes by construction.  What ships as the
// spec's setup diagnostic is the runtime pass in LegendRegistry::
// ApplyBossResolution, which says the same two things with the landscape in
// hand: one LOG_WARN plus a player-facing journal line when fewer than three
// commanders could be placed, and one LOG_WARN when zoneArea reaches the inner
// ring.  This function stays because it is the same judgement without a world,
// it is what the unit suite runs over the five shipped zone tables, and an
// authoring-time delivery (alongside the offline descriptor audit in
// DescriptorLint, over a real parsed `class Zones` block) has somewhere to
// start.  Delete it only together with those cases.
bool LegendPlacementPrecheck(const AutoArray<LegendZoneCandidate>& zones, RString& diagnostic, float zoneArea = 150.0f);

// The zone ladder, in one place so the precheck and the live seeding pass cannot
// drift: (1) every non-CITY non-CAMP zone the occupier owns; (2) if that is
// fewer than Wanted, every non-CITY non-CAMP zone.  Four of the five shipped
// templates field fewer than three military zones, so several commanders sharing
// one zone is the NORMAL case, not the degenerate one.  The template's CAMP
// rides along as the LAST entry of `out` so the sampler, the picker and the
// caller all index one array; campIndex is its index in `out`, or -1 when the
// template has no CAMP.
void SelectLegendZones(const AutoArray<LegendZoneCandidate>& all, AutoArray<LegendZoneCandidate>& out, int& campIndex);

// The terrain questions the sample builder asks.  The live implementation reads
// GLOB_LAND / GRoadNet / the world Names block; an offline caller (the island
// scaffold, which already holds a HeightField and a road point set) can answer
// them from its own data and run the identical sampler.
struct LegendTerrainProvider
{
    virtual ~LegendTerrainProvider() = default;
    virtual float SurfaceY(float x, float z) const = 0;
    virtual bool IsOnRoad(float x, float y, float z, float radius) const = 0;
    // is there a world place name near this spot, close enough that a stand
    // there can be described by it?
    virtual bool NamedNear(float x, float z) const { return false; }
};

// PURE given a provider.  Rings x Bearings candidate stands per zone, in zone
// order; `out` is cleared first.  campIndex indexes `zones` (-1: the template
// has no Camp, and the Camp rules simply do not bind).  A null provider yields
// the analytic set: dry, off-road and unnamed everywhere, which is exactly what
// LegendPlacementPrecheck reasons over.  `avoid`, when given, is the WHOLE zone
// table (CITY zones and the Camp included): a stand within ForeignZoneFloor of
// any of those centres is never emitted, because it would count as occupier
// presence in a zone the commander was not placed to watch.
void BuildLegendSamplesWith(const LegendTerrainProvider* terrain, const AutoArray<LegendZoneCandidate>& zones,
                            int campIndex, AutoArray<LegendSpotSample>& out,
                            const AutoArray<LegendZoneCandidate>* avoid = nullptr);

// ENGINE.  BuildLegendSamplesWith over the live world.  False (and an empty
// `out`) when there is no landscape to sample, which is every headless run.
bool BuildLegendSamples(const AutoArray<LegendZoneCandidate>& zones, int campIndex, AutoArray<LegendSpotSample>& out,
                        const AutoArray<LegendZoneCandidate>* avoid = nullptr);

// ---------------------------------------------------------------------------
// roles
// ---------------------------------------------------------------------------

struct FactionRecord; // ZoneRegistry.hpp

enum LegendRole
{
    LRSniper = 0,
    LRElite = 1,
    LRTank = 2,
    NLegendRoles
};

// What one commander turned out to be.  requested records the ask, resolved what
// the faction could actually field: degradation is ONE step and ALWAYS to Elite.
struct LegendRoleResolution
{
    int requested = LRElite;
    int resolved = LRElite;
    RString bossClass, guardClass;
    int guardCount = 4;
    RString vehicleClass, crewClass;
    bool hasCommanderSeat = false, hasGunnerSeat = false;
};

// What the faction can field for a boss.  Every string is already
// package-resolved (the plan-15 pass blanked what cannot spawn), so the resolver
// only ever tests for emptiness.
struct LegendRoleCapability
{
    RString sniperClass, eliteClass, riflemanClass, tankClass, tankCrewClass;
    bool tankHasCommander = false, tankHasGunner = false;
};

// PURE.  Preference Sniper, Elite, Tank for commanders 0, 1, 2 (and cycling for
// any further count).  A role the faction cannot field degrades in ONE step to
// Elite - never to a third choice, never twice - so repeats are expected and
// intended.  Sniper takes one guard, Elite four, a tank commander takes the
// hull's remaining crew seats.
void ResolveLegendRoles(const LegendRoleCapability& cap, int count, AutoArray<LegendRoleResolution>& out);

// ENGINE.  Reads the resolved descriptor.  A boss is CAMPAIGN scale, not
// garrison scale: the sniper, the officer and the guards come off the faction's
// BEST rung, not the rung the current war level has unlocked (roles resolve once
// at seeding, where the war level is always 1, and every shipped roster puts its
// sniper on the top rung only - reading the war-level rung would mean no
// campaign ever fielded a sniper).  warLevel is still consulted for the tank
// crew fallback, which is a garrison-scale body.  The tank hull is the only key
// re-probed here (the descriptor pass cannot tell a tank from a truck): a
// missing world or a hull whose type will not construct simply leaves tankClass
// empty, which degrades that commander to Elite instead of faulting the seeding
// tick.
LegendRoleCapability ReadLegendCapability(const FactionRecord& faction, float warLevel);

// "Sniper" / "Commander" / "Tank Commander"; "Commander" for anything else, so a
// caller can never render an empty role.
const char* LegendRoleName(int role);

} // namespace Guerrilla
} // namespace Poseidon
