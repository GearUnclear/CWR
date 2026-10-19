// Road-graph route search (RoadRoute.hpp) and its two diagnostic commands:
//
//   gmRoadRoute [from, to, joinRadius?]
//       -> [found, points, length m (link to link, the join leg included),
//           stage-1 searches run, index of the stage-1 start that reached
//           the town (-1 none), links expanded, m from `from` to the first
//           route link, start candidates, goal candidates, joined (stage 2),
//           m of the cross-country join leg, [x, z] where it leaves the
//           pocket road, [x, z] of the town network's link it reaches,
//           [[x, z], ...] the route's points in order]
//       the search the counter-attack service runs; joinRadius defaults to
//       the service's (RoadRouteServiceJoinRadius), 0 = road all the way only
//   gmRoadComponents [pos, radius]
//       -> [[component size (capped at 20000), its links within radius,
//            m to its nearest link, [x, z] of that link, its unconnected
//            ends, m from its nearest unconnected end to another component's
//            link end (-1 none within 80 m), [x, z] of that end], ...]
//       one row per road component touching the circle, nearest first
//   gmLandmass [pos]
//       -> the walkable landmass id of pos (RoadRoute::Landmass), -1 at sea
//   gmRoadQuote [from, to]
//       -> [length m or -1, lookups, hits, plans, over budget, recorded,
//           detours, entries, plan ms in total, the last plan's ms]: the
//          director's road quote of the pair (RoadQuoteCache) and its counters
//   gmForcesEtaNotes
//       -> [[words, package, source zone index, seconds, kind], ...]: the
//          arrival words ("road 1794 s", "line x1.5 2733 s") of the
//          director's last INFO lines, oldest first
//   gmGeoGrid [pos, halfCells, step?]
//       -> the strategic planner's geography cells around pos (every
//          step-th cell), one string per row north to south ('~' deep
//          water, '#' full, 'R' road, 'F' forest, else the gradient 0..7,
//          7 too steep on foot)
//
// Registered from this TU's INIT_MODULE at stage 3 (the TrafficCommands.cpp
// pattern); the TU is in the link through RoadRoute::Search.

#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>

#include <Poseidon/Core/Global.hpp>                  // Glob.time (the road quote's clock)
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // RoadIsReasonable (the service's detour rule)
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp> // gmForcesEtaNotes

#include <Poseidon/AI/Path/AITypes.hpp> // GeographyInfo (gmGeoGrid)
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>   // InvLandGrid / InRange
#include <Poseidon/World/Terrain/Roads.hpp>       // GRoadNet
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toIntFloor / toIntCeil
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Poseidon::Guerrilla
{

namespace
{

typedef std::unordered_set<const RoadLink*> LinkSet;

float RrDist2DSq(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return dx * dx + dz * dz;
}

float RrDist2D(Vector3Par a, Vector3Par b)
{
    return sqrtf(RrDist2DSq(a, b));
}

// the nearest end of `link` to pos, squared (2D)
float NearestEndSq(const RoadLink* link, Vector3Par pos)
{
    float best = 1e20f;
    for (int k = 0; k < link->NConnections(); k++)
    {
        const float d2 = RrDist2DSq(link->PosConnections()[k], pos);
        if (d2 < best)
        {
            best = d2;
        }
    }
    return best;
}

// the component of start over the engine's connections, at most cap links
void Flood(const RoadLink* start, int cap, std::vector<const RoadLink*>& out)
{
    out.clear();
    if (!start)
    {
        return;
    }
    LinkSet seen;
    std::vector<const RoadLink*> stack;
    seen.insert(start);
    stack.push_back(start);
    while (!stack.empty() && (int)out.size() < cap)
    {
        const RoadLink* link = stack.back();
        stack.pop_back();
        out.push_back(link);
        for (int k = 0; k < link->NConnections(); k++)
        {
            const RoadLink* next = link->Connections()[k];
            if (next && seen.insert(next).second)
            {
                stack.push_back(next);
            }
        }
    }
}

// A* from start to any link of goals (heuristic toward goalC), expanding at
// most budget - expanded links.  path holds start..goal on success; on a
// failure with the open list empty, exhausted is set and reached holds every
// link the search touched (the start's whole component).
bool AStar(const RoadLink* start, const LinkSet& goals, Vector3Par goalC, int budget, int& expanded,
           std::vector<const RoadLink*>& path, bool& exhausted, std::vector<const RoadLink*>& reached)
{
    path.clear();
    reached.clear();
    exhausted = false;
    struct Node
    {
        const RoadLink* link;
        int parent;
        float g;
        bool closed;
    };
    std::vector<Node> nodes;
    std::unordered_map<const RoadLink*, int> index;
    typedef std::pair<float, int> Entry;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
    nodes.push_back(Node{start, -1, 0.0f, false});
    index[start] = 0;
    open.push(Entry(RrDist2D(start->GetCenter(), goalC), 0));
    int found = -1;
    while (!open.empty() && expanded < budget)
    {
        const int cur = open.top().second;
        open.pop();
        if (nodes[cur].closed)
        {
            continue;
        }
        nodes[cur].closed = true;
        expanded++;
        const RoadLink* link = nodes[cur].link;
        if (goals.count(link) > 0)
        {
            found = cur;
            break;
        }
        const Vector3 c = link->GetCenter();
        for (int k = 0; k < link->NConnections(); k++)
        {
            const RoadLink* next = link->Connections()[k];
            if (!next)
            {
                continue;
            }
            const Vector3 nc = next->GetCenter();
            const float g = nodes[cur].g + RrDist2D(c, nc);
            int ni;
            auto it = index.find(next);
            if (it == index.end())
            {
                ni = (int)nodes.size();
                nodes.push_back(Node{next, cur, g, false});
                index[next] = ni;
            }
            else
            {
                ni = it->second;
                if (nodes[ni].closed || g >= nodes[ni].g)
                {
                    continue;
                }
                nodes[ni].g = g;
                nodes[ni].parent = cur;
            }
            open.push(Entry(g + RrDist2D(nc, goalC), ni));
        }
    }
    if (found < 0)
    {
        if (open.empty())
        {
            exhausted = true;
            for (size_t i = 0; i < nodes.size(); i++)
            {
                reached.push_back(nodes[i].link);
            }
        }
        return false;
    }
    for (int i = found; i >= 0; i = nodes[i].parent)
    {
        path.push_back(nodes[i].link);
    }
    std::reverse(path.begin(), path.end());
    return true;
}

// a pocket road is followed at most this many links (a base inside a big
// unjoined net still finds its way out within the budget)
constexpr int MaxPocketLinks = 2000;

} // namespace

void RoadRoute::LinksNear(Vector3Par pos, float radius, std::vector<const RoadLink*>& out)
{
    out.clear();
    if (!GRoadNet || radius <= 0)
    {
        return;
    }
    std::vector<std::pair<float, const RoadLink*>> found;
    const float r2 = radius * radius;
    const int cells = toIntCeil(radius * InvLandGrid);
    const int cx = toIntFloor(pos.X() * InvLandGrid);
    const int cz = toIntFloor(pos.Z() * InvLandGrid);
    for (int zz = cz - cells; zz <= cz + cells; zz++)
    {
        for (int xx = cx - cells; xx <= cx + cells; xx++)
        {
            if (!InRange(zz, xx))
            {
                continue;
            }
            const RoadList& list = GRoadNet->GetRoadList(xx, zz);
            for (int i = 0; i < list.Size(); i++)
            {
                const RoadLink* link = list[i];
                if (!link)
                {
                    continue;
                }
                const float d2 = RrDist2DSq(link->GetCenter(), pos);
                if (d2 < r2)
                {
                    found.push_back(std::make_pair(d2, link));
                }
            }
        }
    }
    std::sort(found.begin(), found.end(),
              [](const std::pair<float, const RoadLink*>& a, const std::pair<float, const RoadLink*>& b)
              { return a.first < b.first; });
    for (size_t i = 0; i < found.size(); i++)
    {
        if (out.empty() || out.back() != found[i].second)
        {
            out.push_back(found[i].second);
        }
    }
}

int RoadRoute::ComponentSize(const RoadLink* start, int cap)
{
    std::vector<const RoadLink*> comp;
    Flood(start, cap, comp);
    return (int)comp.size();
}

namespace
{

// a geography cell a soldier may stand on: the cost rule of
// Soldier::GetCost (deep water is closed unless a road or track crosses it)
bool LandCell(int x, int z)
{
    const GeographyInfo g = GLandscape->GetGeography(x, z);
    return g.u.road || g.u.track || g.u.waterDepth < 2;
}

struct LandmassLabels
{
    const void* landscape = nullptr;
    int range = 0;
    unsigned sample = 0;
    std::vector<int> label; // z * range + x -> landmass id, -1 sea
};

// a cheap fingerprint of the loaded terrain: a new world on the same
// landscape object relabels
unsigned GeographySample(int range)
{
    unsigned h = 2166136261u;
    const int stride = std::max(range / 32, 1);
    for (int z = 0; z < range; z += stride)
    {
        for (int x = 0; x < range; x += stride)
        {
            h = (h ^ GLandscape->GetGeography(x, z).packed) * 16777619u;
        }
    }
    return h;
}

const LandmassLabels* Labels()
{
    static LandmassLabels cache;
    if (!GLandscape)
    {
        return nullptr;
    }
    const int range = LandRange;
    if (range <= 0)
    {
        return nullptr;
    }
    const unsigned sample = GeographySample(range);
    if (cache.landscape == GLandscape && cache.range == range && cache.sample == sample)
    {
        return &cache;
    }
    cache.landscape = GLandscape;
    cache.range = range;
    cache.sample = sample;
    cache.label.assign((size_t)range * range, -1);
    std::vector<int> stack;
    int next = 0;
    for (int z0 = 0; z0 < range; z0++)
    {
        for (int x0 = 0; x0 < range; x0++)
        {
            const size_t i0 = (size_t)z0 * range + x0;
            if (cache.label[i0] >= 0 || !LandCell(x0, z0))
            {
                continue;
            }
            const int id = next++;
            cache.label[i0] = id;
            stack.clear();
            stack.push_back((int)i0);
            while (!stack.empty())
            {
                const int i = stack.back();
                stack.pop_back();
                const int x = i % range;
                const int z = i / range;
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        const int nx = x + dx;
                        const int nz = z + dz;
                        if ((dx == 0 && dz == 0) || nx < 0 || nz < 0 || nx >= range || nz >= range)
                        {
                            continue;
                        }
                        const size_t ni = (size_t)nz * range + nx;
                        if (cache.label[ni] >= 0 || !LandCell(nx, nz))
                        {
                            continue;
                        }
                        cache.label[ni] = id;
                        stack.push_back((int)ni);
                    }
                }
            }
        }
    }
    return &cache;
}

} // namespace

int RoadRoute::Landmass(Vector3Par pos)
{
    const LandmassLabels* labels = Labels();
    if (!labels)
    {
        return -1;
    }
    const int range = labels->range;
    const int cx = toIntFloor(pos.X() * InvLandGrid);
    const int cz = toIntFloor(pos.Z() * InvLandGrid);
    // the cell itself, then its rings out to 2 cells (a quay or a pier)
    for (int ring = 0; ring <= 2; ring++)
    {
        for (int dz = -ring; dz <= ring; dz++)
        {
            for (int dx = -ring; dx <= ring; dx++)
            {
                if (std::max(std::abs(dx), std::abs(dz)) != ring)
                {
                    continue;
                }
                const int x = cx + dx;
                const int z = cz + dz;
                if (x < 0 || z < 0 || x >= range || z >= range)
                {
                    continue;
                }
                const int id = labels->label[(size_t)z * range + x];
                if (id >= 0)
                {
                    return id;
                }
            }
        }
    }
    return -1;
}

bool RoadRoute::SameLandmass(Vector3Par a, Vector3Par b)
{
    const int la = Landmass(a);
    const int lb = Landmass(b);
    return la < 0 || lb < 0 || la == lb;
}

RoadRouteParams RoadRoute::ServiceParams()
{
    RoadRouteParams p;
    p.joinRadius = RoadRouteServiceJoinRadius;
    return p;
}

bool RoadRoute::IsServiceParams(const RoadRouteParams& p)
{
    const RoadRouteParams s = ServiceParams();
    return p.snapRadius == s.snapRadius && p.startWide == s.startWide && p.maxStarts == s.maxStarts &&
           p.maxNodes == s.maxNodes && p.joinRadius == s.joinRadius && p.joinWeight == s.joinWeight;
}

float RoadRoute::ServiceLength(const AutoArray<Vector3>& centres, Vector3Par to)
{
    if (centres.Size() == 0)
    {
        return 0;
    }
    float length = 0;
    for (int i = 1; i < centres.Size(); i++)
    {
        length += RrDist2D(centres[i - 1], centres[i]);
    }
    return length + RrDist2D(centres[centres.Size() - 1], to);
}

bool RoadRoute::Search(Vector3Par from, Vector3Par to, const RoadRouteParams& params, AutoArray<Vector3>& centres,
                       RoadRouteStats* stats)
{
    RoadRouteStats local;
    RoadRouteStats& st = stats ? *stats : local;
    const bool found = SearchCore(from, to, params, centres, st);
    // the service's search: its length is the road quote of the pair
    // (RoadQuoteCache), so a route CounterAttack planned is quoted as it is
    if (GRoadNet && IsServiceParams(params))
    {
        RoadQuoteCache::Instance().Record(from, to, found, found ? ServiceLength(centres, to) : 0.0f,
                                          RoadQuoteCache::WorldNow());
    }
    return found;
}

bool RoadRoute::SearchCore(Vector3Par from, Vector3Par to, const RoadRouteParams& params, AutoArray<Vector3>& centres,
                           RoadRouteStats& st)
{
    centres.Clear();
    st = RoadRouteStats();

    std::vector<const RoadLink*> starts;
    std::vector<const RoadLink*> goals;
    LinksNear(from, params.snapRadius, starts);
    const size_t nNear = starts.size();
    if (!starts.empty())
    {
        // then the links farther out: a source whose whole nearby road is a
        // pocket reaches the island's net there
        std::vector<const RoadLink*> wide;
        LinksNear(from, params.startWide, wide);
        LinkSet have(starts.begin(), starts.end());
        for (size_t i = 0; i < wide.size(); i++)
        {
            if (have.count(wide[i]) == 0)
            {
                starts.push_back(wide[i]);
            }
        }
    }
    LinksNear(to, params.snapRadius, goals);
    st.startCandidates = (int)starts.size();
    st.goalCandidates = (int)goals.size();
    if (goals.empty())
    {
        return false;
    }
    const LinkSet goalSet(goals.begin(), goals.end());
    const Vector3 goalC = goals[0]->GetCenter();
    std::vector<const RoadLink*> path;
    std::vector<const RoadLink*> reached;
    bool exhausted = false;

    // ---- stage 1: road all the way ------------------------------------------
    LinkSet dead;
    int expanded = 0;
    for (size_t s = 0; s < starts.size() && st.startsSearched < params.maxStarts && expanded < params.maxNodes; s++)
    {
        const RoadLink* start = starts[s];
        if (dead.count(start) > 0)
        {
            continue;
        }
        st.startsSearched++;
        if (!AStar(start, goalSet, goalC, params.maxNodes, expanded, path, exhausted, reached))
        {
            // the whole component of this start is not joined to the town's
            // roads: skip its other links
            dead.insert(reached.begin(), reached.end());
            continue;
        }
        st.linksSearched = expanded;
        for (size_t i = 0; i < path.size(); i++)
        {
            centres.Add(path[i]->GetCenter());
        }
        st.found = true;
        st.startIndex = (int)s;
        st.startOffset = RrDist2D(start->GetCenter(), from);
        return true;
    }
    st.linksSearched = expanded;
    if (params.joinRadius <= 0)
    {
        return false;
    }

    // ---- stage 2: the source's pocket road, across, the town's road ----------
    std::vector<const RoadLink*> townNet;
    Flood(goals[0], params.maxNodes, townNet);
    const LinkSet townSet(townNet.begin(), townNet.end());
    std::vector<const RoadLink*> landing;
    const float jr2 = params.joinRadius * params.joinRadius;
    // the cross-country leg is walked: only the town network's links on the
    // source's own landmass are landings (no leg across the sea)
    const int fromLand = Landmass(from);
    for (size_t i = 0; i < townNet.size(); i++)
    {
        if (RrDist2DSq(townNet[i]->GetCenter(), from) > jr2)
        {
            continue;
        }
        const int linkLand = Landmass(townNet[i]->GetCenter());
        if (fromLand >= 0 && linkLand >= 0 && linkLand != fromLand)
        {
            continue;
        }
        landing.push_back(townNet[i]);
    }
    if (landing.empty())
    {
        return false;
    }
    // the best leg from a point already `base` m of travel along: the landing
    // link minimising joinWeight x gap + the straight remainder
    auto bestLanding = [&](Vector3Par at, float base, float& cost, const RoadLink*& land)
    {
        for (size_t m = 0; m < landing.size(); m++)
        {
            const Vector3 mc = landing[m]->GetCenter();
            const float c = base + params.joinWeight * RrDist2D(at, mc) + RrDist2D(mc, to);
            if (!land || c < cost)
            {
                cost = c;
                land = landing[m];
            }
        }
    };
    // candidate 0: straight from the source (no pocket road)
    float bestCost = 0;
    const RoadLink* bestLand = nullptr;
    bestLanding(from, 0, bestCost, bestLand);
    std::vector<const RoadLink*> bestPocket; // s0..L along the pocket road
    // the pocket components touching snapRadius, each from its link nearest the source
    LinkSet seenComp;
    for (size_t s = 0; s < nNear; s++)
    {
        const RoadLink* s0 = starts[s];
        if (seenComp.count(s0) > 0 || townSet.count(s0) > 0)
        {
            continue;
        }
        std::vector<const RoadLink*> comp;
        Flood(s0, MaxPocketLinks, comp);
        seenComp.insert(comp.begin(), comp.end());
        // Dijkstra along the pocket road from s0
        std::unordered_map<const RoadLink*, std::pair<float, const RoadLink*>> dist; // g, parent
        typedef std::pair<float, const RoadLink*> Entry;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
        const float g0 = RrDist2D(from, s0->GetCenter());
        dist[s0] = std::make_pair(g0, (const RoadLink*)nullptr);
        open.push(Entry(g0, s0));
        LinkSet closed;
        while (!open.empty())
        {
            const Entry e = open.top();
            open.pop();
            if (!closed.insert(e.second).second)
            {
                continue;
            }
            st.linksSearched++;
            const RoadLink* link = e.second;
            float cost = 0;
            const RoadLink* land = nullptr;
            bestLanding(link->GetCenter(), e.first, cost, land);
            if (land && cost < bestCost)
            {
                bestCost = cost;
                bestLand = land;
                bestPocket.clear();
                for (const RoadLink* p = link; p; p = dist[p].second)
                {
                    bestPocket.push_back(p);
                }
                std::reverse(bestPocket.begin(), bestPocket.end());
            }
            if ((int)closed.size() >= MaxPocketLinks)
            {
                break;
            }
            for (int k = 0; k < link->NConnections(); k++)
            {
                const RoadLink* next = link->Connections()[k];
                if (!next)
                {
                    continue;
                }
                const float g = e.first + RrDist2D(link->GetCenter(), next->GetCenter());
                auto it = dist.find(next);
                if (it == dist.end() || g < it->second.first)
                {
                    dist[next] = std::make_pair(g, link);
                    open.push(Entry(g, next));
                }
            }
        }
    }
    if (!bestLand)
    {
        return false;
    }
    int townExpanded = 0;
    if (!AStar(bestLand, goalSet, goalC, params.maxNodes, townExpanded, path, exhausted, reached))
    {
        st.linksSearched += townExpanded;
        return false;
    }
    st.linksSearched += townExpanded;
    // a joined route begins at the source itself: the route's first point is
    // where a force is born, and a walled base's gate (its pocket road's
    // nearest link) is no place to stand a squad up; from its yard the field
    // planner walks it out to the pocket road
    centres.Add(Vector3(from));
    for (size_t i = 0; i < bestPocket.size(); i++)
    {
        centres.Add(bestPocket[i]->GetCenter());
    }
    for (size_t i = 0; i < path.size(); i++)
    {
        centres.Add(path[i]->GetCenter());
    }
    st.found = true;
    st.joined = true;
    st.joinFrom = bestPocket.empty() ? Vector3(from) : bestPocket.back()->GetCenter();
    st.joinTo = bestLand->GetCenter();
    st.joinGap = RrDist2D(st.joinFrom, st.joinTo);
    st.startOffset = 0;
    return true;
}

// ---------------------------------------------------------------------------
// the road quote cache (OCCUPIER-FORCES.md 6.5, 2026-10-16)
// ---------------------------------------------------------------------------

namespace
{

struct QuoteKey
{
    int fx, fz, tx, tz; // both ends to the metre
    bool operator==(const QuoteKey& o) const { return fx == o.fx && fz == o.fz && tx == o.tx && tz == o.tz; }
};

struct QuoteKeyHash
{
    size_t operator()(const QuoteKey& k) const
    {
        size_t h = 1469598103934665603ull;
        const int v[4] = {k.fx, k.fz, k.tx, k.tz};
        for (int i = 0; i < 4; i++)
        {
            h = (h ^ (size_t)(unsigned)v[i]) * 1099511628211ull;
        }
        return h;
    }
};

struct QuoteEntry
{
    bool found = false;
    float length = 0;
    float stamp = 0; // mission time it was planned or recorded
};

QuoteKey KeyOf(Vector3Par from, Vector3Par to)
{
    return QuoteKey{toIntFloor(from.X() + 0.5f), toIntFloor(from.Z() + 0.5f), toIntFloor(to.X() + 0.5f),
                    toIntFloor(to.Z() + 0.5f)};
}

bool RoadNetPlanner(Vector3Par from, Vector3Par to, float& length)
{
    AutoArray<Vector3> centres;
    RoadRouteStats st;
    if (!RoadRoute::Search(from, to, RoadRoute::ServiceParams(), centres, &st))
    {
        return false;
    }
    length = RoadRoute::ServiceLength(centres, to);
    return true;
}

// the road net (or the test planner) the entries belong to, and the clock:
// a new net, or a clock running backwards (a new mission), clears them
bool SameWorld(const void* net, const void*& seen, float now, float& lastNow)
{
    bool same = true;
    if (net != seen)
    {
        seen = net;
        same = false;
    }
    if (lastNow >= 0 && now + 1e-3f < lastNow)
    {
        same = false;
    }
    lastNow = now;
    return same;
}

} // namespace

struct RoadQuoteCache::Impl
{
    std::unordered_map<QuoteKey, QuoteEntry, QuoteKeyHash> entries;
};

RoadQuoteCache::RoadQuoteCache() : _impl(new Impl)
{
}

RoadQuoteCache& RoadQuoteCache::Instance()
{
    static RoadQuoteCache cache;
    return cache;
}

float RoadQuoteCache::WorldNow()
{
    return Glob.time - Foundation::Time(0);
}

void RoadQuoteCache::Clear()
{
    _impl->entries.clear();
    _tickNow = -1;
    _tickPlans = 0;
    _lastNow = -1;
}

void RoadQuoteCache::SetPlanner(Planner planner)
{
    _planner = planner;
    Clear();
    _net = nullptr;
}

void RoadQuoteCache::SetBudget(int plansPerTick, float ttl)
{
    _plansPerTick = plansPerTick > 0 ? plansPerTick : 0;
    _ttl = ttl > 0 ? ttl : 0;
}

int RoadQuoteCache::Size() const
{
    return (int)_impl->entries.size();
}

const void* RoadQuoteCache::NetId() const
{
    return _planner ? reinterpret_cast<const void*>(_planner) : static_cast<const void*>(GRoadNet);
}

void RoadQuoteCache::Record(Vector3Par from, Vector3Par to, bool found, float length, float now)
{
    const void* net = NetId();
    if (!net)
    {
        return;
    }
    if (!SameWorld(net, _net, now, _lastNow))
    {
        _impl->entries.clear();
        _tickNow = -1;
        _tickPlans = 0;
    }
    QuoteEntry& e = _impl->entries[KeyOf(from, to)];
    e.found = found && length > 0;
    e.length = e.found ? length : 0;
    e.stamp = now;
    _stats.recorded++;
}

float RoadQuoteCache::Length(Vector3Par from, Vector3Par to)
{
    return Length(from, to, WorldNow());
}

float RoadQuoteCache::Length(Vector3Par from, Vector3Par to, float now)
{
    _stats.lookups++;
    const void* net = NetId();
    if (!net)
    {
        return -1; // no road net: the straight line x forcesRoadFactor
    }
    if (!SameWorld(net, _net, now, _lastNow))
    {
        _impl->entries.clear();
        _tickNow = -1;
        _tickPlans = 0;
    }
    if (now != _tickNow)
    {
        _tickNow = now; // a new tick: a fresh plan budget
        _tickPlans = 0;
    }
    const QuoteKey key = KeyOf(from, to);
    auto it = _impl->entries.find(key);
    const bool budget = _tickPlans < _plansPerTick;
    QuoteEntry e;
    if (it != _impl->entries.end() && (now - it->second.stamp <= _ttl || !budget))
    {
        // fresh, or expired with no budget left to refresh it (the road net
        // has not changed)
        _stats.hits++;
        e = it->second;
    }
    else if (!budget)
    {
        _stats.overBudget++;
        return -1;
    }
    else
    {
        _tickPlans++;
        _stats.plans++;
        const auto t0 = std::chrono::steady_clock::now();
        float length = 0;
        const bool found = (_planner ? _planner : RoadNetPlanner)(from, to, length);
        const auto t1 = std::chrono::steady_clock::now();
        _stats.lastPlanMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
        _stats.planMs += _stats.lastPlanMs;
        e.found = found && length > 0;
        e.length = e.found ? length : 0;
        e.stamp = now;
        _impl->entries[key] = e;
    }
    if (!e.found)
    {
        return -1;
    }
    // the service's detour rule: a road this far round is not marched
    if (!CounterAttack::RoadIsReasonable(e.length, RrDist2D(from, to)))
    {
        _stats.detours++;
        return -1;
    }
    return e.length;
}

} // namespace Poseidon::Guerrilla

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

GameValue PosXZ(const GameState* state, Vector3Par pos)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Add(GameValue(pos.X()));
    array.Add(GameValue(pos.Z()));
    return value;
}

} // namespace

// gmRoadRoute [from, to, joinRadius?]
static GameValue GmRoadRoute(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& args = oper1;
    if (args.Size() < 2)
    {
        CheckSize(state, args, 2);
        return value;
    }
    Vector3 from, to;
    if (!GetPos(state, from, args[0]) || !GetPos(state, to, args[1]))
    {
        return value;
    }
    RoadRouteParams params;
    params.joinRadius = RoadRouteServiceJoinRadius;
    if (args.Size() >= 3 && args[2].GetType() == GameScalar)
    {
        params.joinRadius = (float)args[2];
    }
    AutoArray<Vector3> centres;
    RoadRouteStats st;
    RoadRoute::Search(from, to, params, centres, &st);
    float length = 0;
    for (int i = 1; i < centres.Size(); i++)
    {
        length += RrDist2D(centres[i - 1], centres[i]);
    }
    out.Add(GameValue(st.found));
    out.Add(GameValue((float)centres.Size()));
    out.Add(GameValue(length));
    out.Add(GameValue((float)st.startsSearched));
    out.Add(GameValue((float)st.startIndex));
    out.Add(GameValue((float)st.linksSearched));
    out.Add(GameValue(st.startOffset));
    out.Add(GameValue((float)st.startCandidates));
    out.Add(GameValue((float)st.goalCandidates));
    out.Add(GameValue(st.joined));
    out.Add(GameValue(st.joinGap));
    out.Add(PosXZ(state, st.joinFrom));
    out.Add(PosXZ(state, st.joinTo));
    GameValue points = state->CreateGameValue(GameArray);
    GameArrayType& pts = points;
    for (int i = 0; i < centres.Size(); i++)
    {
        pts.Add(PosXZ(state, centres[i]));
    }
    out.Add(points);
    return value;
}

// gmRoadComponents [pos, radius]
static GameValue GmRoadComponents(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& args = oper1;
    if (!CheckSize(state, args, 2) || args[1].GetType() != GameScalar)
    {
        return value;
    }
    Vector3 pos;
    if (!GetPos(state, pos, args[0]))
    {
        return value;
    }
    const float radius = (float)args[1];
    const int cap = 20000;
    std::vector<const RoadLink*> near;
    RoadRoute::LinksNear(pos, radius, near);
    std::unordered_map<const RoadLink*, int> label; // link -> component row
    struct Row
    {
        std::vector<const RoadLink*> links;
        int inRadius;
        const RoadLink* nearest;
    };
    std::vector<Row> rows;
    for (size_t i = 0; i < near.size(); i++)
    {
        const RoadLink* link = near[i];
        auto it = label.find(link);
        if (it != label.end())
        {
            rows[it->second].inRadius++;
            continue;
        }
        const int row = (int)rows.size();
        rows.push_back(Row{{}, 1, link});
        Flood(link, cap, rows[row].links);
        for (size_t k = 0; k < rows[row].links.size(); k++)
        {
            label[rows[row].links[k]] = row;
        }
    }
    std::vector<const RoadLink*> cand;
    for (size_t r = 0; r < rows.size(); r++)
    {
        const Row& row = rows[r];
        int dangling = 0;
        float bestGap2 = -1;
        Vector3 bestEnd = VZero;
        const bool small = (int)row.links.size() < cap;
        for (size_t i = 0; i < row.links.size(); i++)
        {
            const RoadLink* l = row.links[i];
            for (int k = 0; k < l->NConnections(); k++)
            {
                if (l->Connections()[k])
                {
                    continue;
                }
                dangling++;
                if (!small)
                {
                    continue;
                }
                const Vector3 end = l->PosConnections()[k];
                RoadRoute::LinksNear(end, 140.0f, cand);
                for (size_t c = 0; c < cand.size(); c++)
                {
                    auto lt = label.find(cand[c]);
                    if (lt != label.end() && lt->second == (int)r)
                    {
                        continue;
                    }
                    // rows hold whole components, so an unlabelled link is foreign
                    const float d2 = NearestEndSq(cand[c], end);
                    if (d2 <= 80.0f * 80.0f && (bestGap2 < 0 || d2 < bestGap2))
                    {
                        bestGap2 = d2;
                        bestEnd = end;
                    }
                }
            }
        }
        GameValue item = state->CreateGameValue(GameArray);
        GameArrayType& a = item;
        a.Add(GameValue((float)row.links.size()));
        a.Add(GameValue((float)row.inRadius));
        a.Add(GameValue(RrDist2D(row.nearest->GetCenter(), pos)));
        a.Add(PosXZ(state, row.nearest->GetCenter()));
        a.Add(GameValue((float)dangling));
        a.Add(GameValue(bestGap2 < 0 ? -1.0f : sqrtf(bestGap2)));
        a.Add(PosXZ(state, bestEnd));
        out.Add(item);
    }
    return value;
}

// gmGeoGrid [pos, halfCells, step?]: the landscape's geography cells (the
// strategic planner's grid) around pos, one string per row from north to
// south, one character per cell from west to east (every step-th cell, so a
// coarse map of a whole island fits): '~' deep water, '#' full (no way
// through on foot), 'R' road or track, 'F' forest, else the gradient 0..7
// (7 = too steep on foot)
static GameValue GmGeoGrid(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& args = oper1;
    if (args.Size() < 2 || args[1].GetType() != GameScalar)
    {
        CheckSize(state, args, 2);
        return value;
    }
    Vector3 pos;
    if (!GetPos(state, pos, args[0]) || !GLandscape)
    {
        return value;
    }
    int step = 1;
    if (args.Size() >= 3 && args[2].GetType() == GameScalar)
    {
        step = std::min(std::max(toIntFloor((float)args[2]), 1), 16);
    }
    const int half = std::min(std::max(toIntFloor((float)args[1]), 0), 40);
    const int cx = toIntFloor(pos.X() * InvLandGrid);
    const int cz = toIntFloor(pos.Z() * InvLandGrid);
    for (int zr = half; zr >= -half; zr--)
    {
        const int zz = cz + zr * step;
        char row[96];
        int n = 0;
        for (int xr = -half; xr <= half; xr++)
        {
            const int xx = cx + xr * step;
            char c = '?';
            if (InRange(zz, xx))
            {
                const GeographyInfo g = GLandscape->GetGeography(xx, zz);
                const bool road = g.u.road || g.u.track;
                if (road)
                {
                    c = 'R';
                }
                else if (g.u.waterDepth >= 2)
                {
                    c = '~';
                }
                else if (g.u.full && !g.u.forestOuter)
                {
                    c = '#';
                }
                else if (g.u.forestOuter || g.u.forestInner)
                {
                    c = 'F';
                }
                else
                {
                    c = (char)('0' + g.u.gradient);
                }
            }
            row[n++] = c;
        }
        row[n] = 0;
        out.Add(GameValue(RString(row)));
    }
    return value;
}

// gmRoadQuote [from, to] -> [length m or -1, lookups, hits, plans, over
// budget, recorded, detours, entries, plan ms in total, the last plan's ms]:
// the director's road quote of the pair (RoadQuoteCache::Length, at the
// world clock), then the cache's counters
static GameValue GmRoadQuote(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& args = oper1;
    if (!CheckSize(state, args, 2))
    {
        return value;
    }
    Vector3 from, to;
    if (!GetPos(state, from, args[0]) || !GetPos(state, to, args[1]))
    {
        return value;
    }
    RoadQuoteCache& cache = RoadQuoteCache::Instance();
    const float length = cache.Length(from, to);
    const RoadQuoteStats& st = cache.Stats();
    out.Add(GameValue(length));
    out.Add(GameValue((float)st.lookups));
    out.Add(GameValue((float)st.hits));
    out.Add(GameValue((float)st.plans));
    out.Add(GameValue((float)st.overBudget));
    out.Add(GameValue((float)st.recorded));
    out.Add(GameValue((float)st.detours));
    out.Add(GameValue((float)cache.Size()));
    out.Add(GameValue(st.planMs));
    out.Add(GameValue(st.lastPlanMs));
    return value;
}

// gmForcesEtaNotes -> [[words, package, source zone index, seconds,
// "ROAD" / "LINE_FACTOR" / "LINE"], ...]: the arrival words of the
// director's last INFO lines, oldest first (ForcePackages::LogEtaNote)
static GameValue GmForcesEtaNotes(const GameState* state)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    for (int i = 0; i < ForcePackages::EtaNoteCount(); i++)
    {
        const ForcePackages::EtaNoteRow& n = ForcePackages::EtaNoteAt(i);
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Add(GameValue(n.text));
        row.Add(GameValue(RString(ForcePackages::PackageName(n.package))));
        row.Add(GameValue((float)n.source));
        row.Add(GameValue(n.seconds));
        row.Add(GameValue(RString(n.eta == FEQRoad ? "ROAD" : (n.eta == FEQLineFactor ? "LINE_FACTOR" : "LINE"))));
        out.Add(rowValue);
    }
    return value;
}

// gmLandmass [pos]: the walkable landmass id of pos (RoadRoute::Landmass),
// -1 at sea or without a landscape
static GameValue GmLandmass(const GameState* state, GameValuePar oper1)
{
    Vector3 pos;
    if (!GetPos(state, pos, oper1))
    {
        return GameValue(-1.0f);
    }
    return GameValue((float)RoadRoute::Landmass(pos));
}

INIT_MODULE(GuerrillaRoadRoute, 3)
{
    GGameState.NewFunction(GameFunction(GameScalar, "gmLandmass", GmLandmass, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmGeoGrid", GmGeoGrid, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmRoadRoute", GmRoadRoute, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmRoadComponents", GmRoadComponents, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmRoadQuote", GmRoadQuote, GameArray));
    GGameState.NewNularOp(GameNular(GameArray, "gmForcesEtaNotes", GmForcesEtaNotes));
}
