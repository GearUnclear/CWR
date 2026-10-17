#pragma once

// Road-graph route search over the engine's RoadNet links, used by the
// occupier counter-attack service (CounterAttack.cpp BuildRoute,
// OCCUPIER-FORCES.md 3.2) and the road diagnostics (gmRoadRoute /
// gmRoadComponents, RoadRoute.cpp).
//
// RoadNet::SearchPath gives up after 100 nodes, too short for a base-to-town
// road, so this is a plain A* over link centres, in two stages:
//   1. Road all the way.  The start candidates are every link within
//      snapRadius of the source, then every link out to startWide, nearest
//      first.  A start whose component an earlier search exhausted is skipped
//      WITHOUT spending the start budget (maxStarts counts searches run).
//   2. Join (joinRadius > 0), when stage 1 finds nothing: the source's own
//      roads are a pocket the island's data never joined to the town's
//      network.  The route then follows one of the source's pocket
//      roads (a link within snapRadius, so a walled base leaves by a gate),
//      leaves it at the link that minimises
//          pocket road so far + joinWeight x cross-country gap + straight
//          remainder to the town,
//      crosses to the town network's link at the end of that gap (within
//      joinRadius of the source and on the source's own landmass: the gap is
//      walked, never swum), and follows the town's road from there.  A source
//      on another landmass than the town has no route (Malden's Southeast
//      Compound stands on an island of its own, 800 m of sea east of Le
//      Port; its roads are a pocket for that reason).
//      Starting from the source itself (no pocket road) is a candidate too.
//      A joined route's first point is the source itself (a force is born
//      at the route's first point: a base's yard, not its gate).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>

#include <vector>

namespace Poseidon
{
class RoadLink;
}

namespace Poseidon::Guerrilla
{

// m from the source the counter-attack service joins the town's network
// (stage 2): beyond it a pocket base marches the straight line
constexpr float RoadRouteServiceJoinRadius = 2000.0f;

struct RoadRouteParams
{
    float snapRadius = 300.0f; // m: links this near an end are candidates
    float startWide = 900.0f;  // m: further start candidates, after the near ones
    int maxStarts = 96;        // searches run (exhausted components cost nothing)
    int maxNodes = 60000;      // links expanded per stage
    float joinRadius = 0.0f;   // m from the source the town's net may be joined; 0 = no join stage
    float joinWeight = 2.0f;   // cost per m of the cross-country join leg
};

struct RoadRouteStats
{
    int startCandidates = 0; // links within startWide of the source
    int goalCandidates = 0;  // links within snapRadius of the target
    int startsSearched = 0;  // stage-1 searches run
    int startIndex = -1;     // stage 1: index (nearest first) of the start that reached a goal
    int linksSearched = 0;   // links expanded, both stages
    float startOffset = 0;   // m from the source to the first route link
    bool found = false;
    bool joined = false;      // stage 2 found it
    float joinGap = 0;        // m of the cross-country leg
    Vector3 joinFrom = VZero; // where the route leaves the pocket road (or the source)
    Vector3 joinTo = VZero;   // the town network's link it crosses to
};

class RoadRoute
{
  public:
    // the road links whose centres lie within radius of pos (2D), nearest
    // first; empty without a road net
    static void LinksNear(Vector3Par pos, float radius, std::vector<const RoadLink*>& out);

    // link centres from the source's road to a link near `to` (the target
    // itself is not appended); false and an empty array when no route
    // exists within the budget
    static bool Search(Vector3Par from, Vector3Par to, const RoadRouteParams& params, AutoArray<Vector3>& centres,
                       RoadRouteStats* stats = nullptr);

    // the links reachable from start over the engine's connections, at most
    // cap of them (cap reached = "the island's net")
    static int ComponentSize(const RoadLink* start, int cap);

    // the walkable landmass of pos: the landscape's geography cells joined
    // to each other over land (8 neighbours; deep water parts them, a road
    // or track across it is a bridge), labelled once per landscape.  A pos
    // on deep water takes the nearest land cell within 2 cells.  -1 without
    // a landscape, off the map or out at sea.  Malden's Southeast Compound
    // stands on an island of its own, 800 m of sea east of Le Port: no foot
    // or wheeled force crosses from it.
    static int Landmass(Vector3Par pos);

    // false only when both ends have a known landmass and they differ
    static bool SameLandmass(Vector3Par a, Vector3Par b);

    // the search the counter-attack service runs (CounterAttack.cpp
    // BuildRoute, gmRoadRoute): snapRadius 300 m, the service's join radius,
    // the other defaults.  A Search with these parameters records its
    // length in RoadQuoteCache.
    static RoadRouteParams ServiceParams();
    static bool IsServiceParams(const RoadRouteParams& p);
    // the length the service marches a route Search returned: link to
    // link, then the last link to `to` (CounterAttack::RouteLength of the
    // centres with the target appended)
    static float ServiceLength(const AutoArray<Vector3>& centres, Vector3Par to);

  private:
    // Search without the cache record
    static bool SearchCore(Vector3Par from, Vector3Par to, const RoadRouteParams& params, AutoArray<Vector3>& centres,
                           RoadRouteStats& st);
};

// The road quote (OCCUPIER-FORCES.md 6.5, the arrival rule, 2026-10-16).
// The director quotes a ground package's arrival on the road route the
// counter-attack service will march (the leg CounterAttack::LegTime times),
// not the straight line.  This cache holds that route's length per
// (source, target) pair, keyed by both positions to the metre (a zone pair:
// zone positions are fixed; an HQ assault's target is the HQ position).
//   - A Search with the service's parameters (CounterAttack's BuildRoute,
//     gmRoadRoute) records its result, so a route the service already
//     planned is reused.
//   - Otherwise Length plans the pair itself, at most once per director
//     tick (a failure is cached too) and at most plansPerTick pairs per tick
//     (a tick is one value of the clock: every quote of one director tick
//     sees the same mission time).  Past that budget an unplanned pair
//     answers -1 and the director quotes the straight line x
//     forcesRoadFactor; an expired entry is served as it is (the road net
//     does not change) until a later tick refreshes it.
//   - An entry lives ttl s of mission time.  A clock running backwards (a
//     new mission) or a new road net clears the cache.
//   - The service's detour rule holds: a route longer than
//     CounterAttack::RoadIsReasonable allows is not marched (the force cuts
//     across), so it answers -1 too.
// No road net (unit tests, a world without roads) answers -1 without
// planning, unless a test planner is set.
struct RoadQuoteStats
{
    int lookups = 0;
    int hits = 0;       // answered from an entry (fresh or served expired)
    int plans = 0;      // pairs planned by Length
    int overBudget = 0; // unplanned pairs past the tick's budget
    int recorded = 0;   // results recorded by a service Search
    int detours = 0;    // routes the detour rule set aside
    float planMs = 0;   // wall time Length spent planning, in total
    float lastPlanMs = 0;
};

class RoadQuoteCache
{
  public:
    // a planner: true and the service's length (m) when a road route
    // exists.  The default searches the road net (RoadRoute::Search with
    // ServiceParams).
    typedef bool (*Planner)(Vector3Par from, Vector3Par to, float& length);
    static constexpr float DefaultTtl = 600.0f;   // s of mission time
    static constexpr int DefaultPlansPerTick = 6; // a tick's six sources of one target

    static RoadQuoteCache& Instance();

    // the road route's length (m) from `from` to `to` as the service
    // marches it, or -1: no route, the detour rule, no road net, or not
    // planned within this tick's budget.  `now` is the mission time (s);
    // the overload reads it from the world clock.
    float Length(Vector3Par from, Vector3Par to, float now);
    float Length(Vector3Par from, Vector3Par to);
    // a service Search's result
    void Record(Vector3Par from, Vector3Par to, bool found, float length, float now);
    void Clear();
    // nullptr = the road net
    void SetPlanner(Planner planner);
    void SetBudget(int plansPerTick, float ttl);
    int PlansPerTick() const { return _plansPerTick; }
    float Ttl() const { return _ttl; }
    int Size() const;
    const RoadQuoteStats& Stats() const { return _stats; }
    void ResetStats() { _stats = RoadQuoteStats(); }
    // the world clock Length(from, to) reads: mission time in s
    static float WorldNow();

  private:
    RoadQuoteCache();
    const void* NetId() const;
    struct Impl;
    Impl* _impl;
    Planner _planner = nullptr;
    int _plansPerTick = DefaultPlansPerTick;
    float _ttl = DefaultTtl;
    RoadQuoteStats _stats;
    // the tick the plan budget belongs to
    float _tickNow = -1;
    int _tickPlans = 0;
    float _lastNow = -1;
    const void* _net = nullptr;
};

} // namespace Poseidon::Guerrilla
