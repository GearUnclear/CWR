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
//      network (Malden's Southeast Compound: its gate tracks and the roads
//      east of it are separate components, the nearest link of the island's
//      net 1.3 km away).  The route then follows one of the source's pocket
//      roads (a link within snapRadius, so a walled base leaves by a gate),
//      leaves it at the link that minimises
//          pocket road so far + joinWeight x cross-country gap + straight
//          remainder to the town,
//      crosses to the town network's link at the end of that gap (within
//      joinRadius of the source), and follows the town's road from there.
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
};

} // namespace Poseidon::Guerrilla
