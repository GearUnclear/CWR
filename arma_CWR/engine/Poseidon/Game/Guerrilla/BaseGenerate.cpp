#include <Poseidon/Game/Guerrilla/BaseGenerate.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>  // HashKey / Roll (the seeded jitter)
#include <Poseidon/Game/Guerrilla/ScriptVars.hpp>  // gmSelStartTown (the HQ keepout)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Core/Global.hpp> // Glob.header.worldname
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Vehicles/House.hpp> // Building (the quiet settlement check)
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/Roads.hpp>
#include <Poseidon/AI/Path/AITypes.hpp> // GeographyInfo
#include <Poseidon/World/MapTypes.hpp>  // MapTree .. (BaseSiteObjectClass)

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp> // CheckSize
#include <Poseidon/Foundation/Modules/Modules.hpp>       // INIT_MODULE

#include <Random/randomGen.hpp> // GRandGen: the one draw of a generation without a pinned seed

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

namespace Poseidon::Guerrilla
{

// ---------------------------------------------------------------------------
// the kind catalogue
// ---------------------------------------------------------------------------

const GenBaseKindInfo& GenBaseKindOf(GenBaseKind kind)
{
    // type, noun, garrisonMax, reserve, countsForWin, footprint, roadMax, placed
    static const GenBaseKindInfo kOutpost = {"OUTPOST", "Outpost", 6, 6, true, 1, 250.0f, true};
    static const GenBaseKindInfo kCheckpoint = {"CHECKPOINT", "Checkpoint", 5, 5, false, 0, 100.0f, true};
    static const GenBaseKindInfo kCoastal = {"COASTAL", "Coastal Post", 5, 5, false, 0, 250.0f, true};
    static const GenBaseKindInfo kHelipad = {"HELIPAD", "Helipad", 6, 6, false, 1, 250.0f, false};
    switch (kind)
    {
        case GenBaseKind::Checkpoint:
            return kCheckpoint;
        case GenBaseKind::Coastal:
            return kCoastal;
        case GenBaseKind::Helipad:
            return kHelipad;
        case GenBaseKind::Outpost:
        default:
            return kOutpost;
    }
}

bool GenBaseKindFromType(const char* type, GenBaseKind& out)
{
    if (!type)
    {
        return false;
    }
    static const GenBaseKind kAll[] = {GenBaseKind::Outpost, GenBaseKind::Checkpoint, GenBaseKind::Coastal,
                                       GenBaseKind::Helipad};
    for (GenBaseKind k : kAll)
    {
        if (stricmp(type, GenBaseKindOf(k).type) == 0)
        {
            out = k;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// pure helpers
// ---------------------------------------------------------------------------

int BaseGenFloor(int K)
{
    return K <= 0 ? 0 : (K + 2) / 3;
}

int BaseGenCeiling(int K, int maxTotal)
{
    const int half = K <= 0 ? 0 : (K + 1) / 2;
    return half < maxTotal ? half : maxTotal;
}

float BaseGenBearingDeg(float fromX, float fromZ, float toX, float toZ)
{
    float deg = atan2f(toX - fromX, toZ - fromZ) * (180.0f / 3.14159265f);
    if (deg < 0)
    {
        deg += 360.0f;
    }
    return deg >= 360.0f ? deg - 360.0f : deg;
}

float BaseGenBearingGap(float a, float b)
{
    float d = fabsf(a - b);
    while (d > 360.0f)
    {
        d -= 360.0f;
    }
    return d > 180.0f ? 360.0f - d : d;
}

bool RoadModelIsTrack(const char* modelPath)
{
    if (!modelPath || !*modelPath)
    {
        return false;
    }
    std::string path(modelPath);
    for (char& c : path)
    {
        c = (char)tolower((unsigned char)c);
        if (c == '/')
        {
            c = '\\';
        }
    }
    const size_t slash = path.find_last_of('\\');
    const std::string base = slash == std::string::npos ? path : path.substr(slash + 1);
    const std::string dir = slash == std::string::npos ? std::string() : path.substr(0, slash);
    // Czech "cesta" is a path: the stock dirt-track family (data3d cesta10 25,
    // cesta 12 konec); the Resistance three-letter stem "ces" counts with a
    // digit or a space after it, like ModelIsRoad's short stems
    if (base.compare(0, 5, "cesta") == 0 || dir.find("cesta") != std::string::npos)
    {
        return true;
    }
    if (base.compare(0, 3, "ces") == 0 && base.size() > 3 && (isdigit((unsigned char)base[3]) || base[3] == ' '))
    {
        return true;
    }
    // generic mod vocabulary
    return base.find("track") != std::string::npos || base.find("dirt") != std::string::npos ||
           base.find("polni") != std::string::npos;
}

namespace
{

constexpr float kNone = 1e9f;

float Dist2D(float ax, float az, float bx, float bz)
{
    const float dx = ax - bx;
    const float dz = az - bz;
    return sqrtf(dx * dx + dz * dz);
}

// the walkable landmass labels over the cells: the rule of RoadRoute::Landmass
// (a cell a soldier may stand on: not deep water, or a road or track across
// it; 8 neighbours; the same scan order, so the same ids on a real landscape)
std::vector<int> LabelLandmasses(const BaseGenInputs& in)
{
    const int nx = in.cellsX;
    const int nz = in.cellsZ;
    std::vector<int> label((size_t)nx * nz, -1);
    auto land = [&](int x, int z) {
        const GenCell& c = in.cells[(size_t)z * nx + x];
        return c.road || c.track || !c.deep;
    };
    std::vector<int> stack;
    int next = 0;
    for (int z0 = 0; z0 < nz; z0++)
    {
        for (int x0 = 0; x0 < nx; x0++)
        {
            const size_t i0 = (size_t)z0 * nx + x0;
            if (label[i0] >= 0 || !land(x0, z0))
            {
                continue;
            }
            const int id = next++;
            label[i0] = id;
            stack.clear();
            stack.push_back((int)i0);
            while (!stack.empty())
            {
                const int i = stack.back();
                stack.pop_back();
                const int x = i % nx;
                const int z = i / nx;
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        const int ax = x + dx;
                        const int az = z + dz;
                        if ((dx == 0 && dz == 0) || ax < 0 || az < 0 || ax >= nx || az >= nz)
                        {
                            continue;
                        }
                        const size_t ni = (size_t)az * nx + ax;
                        if (label[ni] >= 0 || !land(ax, az))
                        {
                            continue;
                        }
                        label[ni] = id;
                        stack.push_back((int)ni);
                    }
                }
            }
        }
    }
    return label;
}

// the landmass under a point: its cell, then rings out to 2 cells (a quay)
int LandmassAt(const BaseGenInputs& in, const std::vector<int>& label, float x, float z)
{
    const int cx = (int)floorf(x / in.cellSize);
    const int cz = (int)floorf(z / in.cellSize);
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
                const int ax = cx + dx;
                const int az = cz + dz;
                if (ax < 0 || az < 0 || ax >= in.cellsX || az >= in.cellsZ)
                {
                    continue;
                }
                const int id = label[(size_t)az * in.cellsX + ax];
                if (id >= 0)
                {
                    return id;
                }
            }
        }
    }
    return -1;
}

bool SameLand(int a, int b)
{
    return a < 0 || b < 0 || a == b;
}

// a bucket grid over the road points
struct RoadGrid
{
    float size = 100.0f;
    int n = 1;
    std::vector<std::vector<int>> buckets;
    const std::vector<GenRoadPoint>* pts = nullptr;

    int Bucket(float v) const
    {
        int b = (int)floorf(v / size);
        return b < 0 ? 0 : (b >= n ? n - 1 : b);
    }
    void Build(const std::vector<GenRoadPoint>& points, float extent)
    {
        pts = &points;
        n = std::max(1, (int)ceilf(extent / size));
        buckets.assign((size_t)n * n, std::vector<int>());
        for (int i = 0; i < (int)points.size(); i++)
        {
            buckets[(size_t)Bucket(points[i].z) * n + Bucket(points[i].x)].push_back(i);
        }
    }
    // the nearest point passing pred within radius (kNone when none); idx
    // gets its index
    template <class Pred> float Nearest(float x, float z, float radius, Pred pred, int* idx = nullptr) const
    {
        float best = kNone;
        int bestI = -1;
        const int r = (int)ceilf(radius / size);
        const int bx = Bucket(x);
        const int bz = Bucket(z);
        for (int zz = std::max(0, bz - r); zz <= std::min(n - 1, bz + r); zz++)
        {
            for (int xx = std::max(0, bx - r); xx <= std::min(n - 1, bx + r); xx++)
            {
                for (int i : buckets[(size_t)zz * n + xx])
                {
                    const GenRoadPoint& p = (*pts)[i];
                    if (!pred(p))
                    {
                        continue;
                    }
                    const float d = Dist2D(x, z, p.x, p.z);
                    if (d <= radius && (d < best || (d == best && i < bestI)))
                    {
                        best = d;
                        bestI = i;
                    }
                }
            }
        }
        if (idx)
        {
            *idx = bestI;
        }
        return best;
    }
};

struct Site
{
    float x = 0;
    float z = 0;
    float y = 0;
    int landmass = -1;
    float relief = 0;
    float roadDist = 0;
    float trunkDist = kNone;
    float heading = 0;
    bool track = false;
    bool detected = false;
    RString name; // detected candidates only
    unsigned long long key = 0; // the jitter key: the cell index or the detected name
    bool used = false;
    // the kind it can take: an outpost (the 3 x 3 footprint passed), else a
    // minor kind (only the centre cell passed): Coastal near deep water,
    // else Checkpoint
    GenBaseKind kind = GenBaseKind::Outpost;
    bool minor = false;
    int solids = 0; // map objects in the 3 x 3 (GenCell::solids / trees)
    int trees = 0;
};

struct Source
{
    RString name;
    float x = 0;
    float z = 0;
    int landmass = -1;
};

struct TownState
{
    RString name;
    float x = 0;
    float z = 0;
    int landmass = -1;
    // the two nearest sources on its landmass
    float d1 = kNone;
    float d2 = kNone;
    int s1 = -1;
    int s2 = -1;
};

void NearestTwo(std::vector<TownState>& towns, const std::vector<Source>& sources)
{
    for (TownState& t : towns)
    {
        t.d1 = t.d2 = kNone;
        t.s1 = t.s2 = -1;
        for (int i = 0; i < (int)sources.size(); i++)
        {
            const Source& s = sources[i];
            if (!SameLand(t.landmass, s.landmass))
            {
                continue;
            }
            const float d = Dist2D(t.x, t.z, s.x, s.z);
            if (d < t.d1)
            {
                t.d2 = t.d1;
                t.s2 = t.s1;
                t.d1 = d;
                t.s1 = i;
            }
            else if (d < t.d2)
            {
                t.d2 = d;
                t.s2 = i;
            }
        }
    }
}

// bearing violation of one town given its two nearest (positions)
bool BearingViolation(const TownState& t, float ax, float az, float bx, float bz, float minDeg)
{
    const float a = BaseGenBearingDeg(t.x, t.z, ax, az);
    const float b = BaseGenBearingDeg(t.x, t.z, bx, bz);
    return BaseGenBearingGap(a, b) < minDeg;
}

RString UniqueName(const std::string& base, const std::vector<RString>& taken)
{
    auto isTaken = [&](const std::string& n) {
        for (const RString& t : taken)
        {
            if (stricmp(t, n.c_str()) == 0)
            {
                return true;
            }
        }
        return false;
    };
    if (!isTaken(base))
    {
        return RString(base.c_str());
    }
    for (int i = 2; i < 100; i++)
    {
        const std::string n = base + " " + std::to_string(i);
        if (!isTaken(n))
        {
            return RString(n.c_str());
        }
    }
    return RString(base.c_str());
}

enum RejectReason
{
    RjRegion,
    RjEdge,
    RjLandmass,
    RjWater,
    RjForest,
    RjObjects,
    RjSlope,
    RjLow,
    RjKeepout,
    RjZone,
    RjTown,
    RjSpacing,
    RjOnRoad,
    RjNoRoad,
    RjFar,
    NRejects
};
const char* const kRejectNames[NRejects] = {"region", "edge",    "landmass", "water",  "forest",
                                            "objects", "slope",  "low",      "keepout", "zone",
                                            "town",    "spacing", "on-road", "no-network-road", "far"};
// the order the common rules report their first failure in (a site's "why")
const int kCommonOrder[] = {RjRegion, RjEdge,    RjLandmass, RjLow,  RjKeepout, RjZone,
                            RjTown,   RjSpacing, RjFar,      RjOnRoad, RjNoRoad};

int FirstReason(unsigned mask)
{
    if (mask & (1u << RjWater))
    {
        return RjWater;
    }
    if (mask & (1u << RjObjects))
    {
        return RjObjects;
    }
    if (mask & (1u << RjForest))
    {
        return RjForest;
    }
    if (mask & (1u << RjSlope))
    {
        return RjSlope;
    }
    for (int r : kCommonOrder)
    {
        if (mask & (1u << r))
        {
            return r;
        }
    }
    return -1;
}

int PopCount(unsigned m)
{
    int n = 0;
    for (; m; m &= m - 1)
    {
        n++;
    }
    return n;
}

// the terrain-cell rules of one cell as a site centre: footprint 1 = the
// 3 x 3 (an outpost), 0 = the centre cell (a minor kind; the water rule
// still covers the 3 x 3, the garrison's spawn square).  Returns the failed
// rules as a bit mask (all of them, not the first) and the relief.
unsigned TerrainMask(const BaseGenInputs& in, const BaseGenOptions& opt, int cx, int cz, int footprint, float& relief)
{
    unsigned mask = 0;
    float lo = kNone, hi = -kNone;
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            const GenCell& n = in.cells[(size_t)(cz + dz) * in.cellsX + (cx + dx)];
            lo = std::min(lo, n.height);
            hi = std::max(hi, n.height);
            if (n.water)
            {
                mask |= 1u << RjWater;
            }
            const bool inFootprint = footprint >= 1 || (dx == 0 && dz == 0);
            if (!inFootprint)
            {
                continue;
            }
            if (n.full)
            {
                mask |= 1u << RjObjects;
            }
            if (n.forestInner)
            {
                mask |= 1u << RjForest;
            }
            if (n.gradient >= opt.maxGradientBand)
            {
                mask |= 1u << RjSlope;
            }
        }
    }
    const GenCell& c = in.cells[(size_t)cz * in.cellsX + cx];
    if (c.forestOuter)
    {
        mask |= 1u << RjForest;
    }
    if (c.road)
    {
        mask |= 1u << RjOnRoad;
    }
    if (c.hardObjects > 0 || c.objects > 1)
    {
        mask |= 1u << RjObjects;
    }
    relief = hi - lo;
    return mask;
}

} // namespace

void GenerateBases(const BaseGenInputs& in, const BaseGenOptions& opt, BaseGenResult& out)
{
    out = BaseGenResult();
    const bool gridOk = in.cellsX > 0 && in.cellsZ > 0 && in.cellSize > 0 &&
                        (int)in.cells.size() == in.cellsX * in.cellsZ;
    if (!gridOk)
    {
        out.notes.push_back("no terrain grid: nothing generated");
        return;
    }
    const std::vector<int> label = LabelLandmasses(in);
    const float extentX = in.cellsX * in.cellSize;
    const float extentZ = in.cellsZ * in.cellSize;

    // towns, sources, their landmasses
    std::vector<TownState> towns;
    std::unordered_set<int> townLands;
    for (const GenPlace& p : in.towns)
    {
        TownState t;
        t.name = p.name;
        t.x = p.x;
        t.z = p.z;
        t.landmass = LandmassAt(in, label, p.x, p.z);
        towns.push_back(t);
        townLands.insert(t.landmass);
    }
    std::vector<Source> sources;
    for (const GenPlace& p : in.sources)
    {
        Source s;
        s.name = p.name;
        s.x = p.x;
        s.z = p.z;
        s.landmass = LandmassAt(in, label, p.x, p.z);
        sources.push_back(s);
    }
    out.K = (int)towns.size();
    out.existing = (int)sources.size();
    out.floor = BaseGenFloor(out.K);
    out.ceiling = BaseGenCeiling(out.K, opt.maxTotal);
    auto uncoveredCount = [&]() {
        NearestTwo(towns, sources);
        int n = 0;
        for (const TownState& t : towns)
        {
            n += t.d1 > opt.coverDist ? 1 : 0;
        }
        return n;
    };
    out.uncoveredBefore = uncoveredCount();

    int room = out.ceiling - out.existing;
    if (room > in.zoneRoom)
    {
        room = in.zoneRoom;
    }

    RoadGrid roads;
    roads.Build(in.roads, std::max(extentX, extentZ));
    auto anyRoad = [](const GenRoadPoint&) { return true; };
    auto netRoad = [](const GenRoadPoint& p) { return p.network; };
    auto trunkRoad = [](const GenRoadPoint& p) { return !p.track; };

    auto heightAt = [&](float x, float z) {
        if (in.height)
        {
            return in.height->Height(x, z);
        }
        int cx = (int)floorf(x / in.cellSize);
        int cz = (int)floorf(z / in.cellSize);
        cx = cx < 0 ? 0 : (cx >= in.cellsX ? in.cellsX - 1 : cx);
        cz = cz < 0 ? 0 : (cz >= in.cellsZ ? in.cellsZ - 1 : cz);
        return in.cells[(size_t)cz * in.cellsX + cx].height;
    };
    auto inRegion = [&](float x, float z) {
        return !in.hasRegion || (x >= in.regionMinX && x <= in.regionMaxX && z >= in.regionMinZ && z <= in.regionMaxZ);
    };

    int rejects[NRejects] = {};
    // per cell: the reason it was refused (-1 an outpost site, -3 a site only
    // a minor kind fits, -2 not scanned), for the open-town diagnostics
    std::vector<signed char> cellWhy((size_t)in.cellsX * in.cellsZ, (signed char)-2);
    // deep water within coastDist of a point (a coastal post's site)
    auto nearSea = [&](float x, float z) {
        const int r = (int)ceilf(opt.coastDist / in.cellSize);
        const int cx = (int)floorf(x / in.cellSize);
        const int cz = (int)floorf(z / in.cellSize);
        for (int zz = std::max(0, cz - r); zz <= std::min(in.cellsZ - 1, cz + r); zz++)
        {
            for (int xx = std::max(0, cx - r); xx <= std::min(in.cellsX - 1, cx + r); xx++)
            {
                if (in.cells[(size_t)zz * in.cellsX + xx].deep &&
                    Dist2D(x, z, ((float)xx + 0.5f) * in.cellSize, ((float)zz + 0.5f) * in.cellSize) <= opt.coastDist)
                {
                    return true;
                }
            }
        }
        return false;
    };
    // the map objects within the layout's reach of a centre (a bucket grid
    // over the object list, one bucket per cell)
    std::vector<std::vector<int>> objBuckets((size_t)in.cellsX * in.cellsZ);
    for (int i = 0; i < (int)in.objects.size(); i++)
    {
        const int bx = std::min(std::max((int)floorf(in.objects[i].x / in.cellSize), 0), in.cellsX - 1);
        const int bz = std::min(std::max((int)floorf(in.objects[i].z / in.cellSize), 0), in.cellsZ - 1);
        objBuckets[(size_t)bz * in.cellsX + bx].push_back(i);
    }
    auto census = [&](float x, float z, int& solids, int& trees) {
        solids = trees = 0;
        const int r = (int)ceilf((opt.layoutReach + 10.0f) / in.cellSize);
        const int cx = (int)floorf(x / in.cellSize);
        const int cz = (int)floorf(z / in.cellSize);
        for (int zz = std::max(0, cz - r); zz <= std::min(in.cellsZ - 1, cz + r); zz++)
        {
            for (int xx = std::max(0, cx - r); xx <= std::min(in.cellsX - 1, cx + r); xx++)
            {
                for (int i : objBuckets[(size_t)zz * in.cellsX + xx])
                {
                    const GenObject& o = in.objects[i];
                    if (Dist2D(x, z, o.x, o.z) - o.radius < opt.layoutReach)
                    {
                        solids += o.solid ? 1 : 0;
                        trees += o.solid ? 0 : 1;
                    }
                }
            }
        }
    };
    // the rules a site shares with a detected outpost (everything but the
    // terrain cells), EVERY one evaluated: the failed rules as a bit mask
    // (FirstReason names the one a refusal is counted under).  roadMax is
    // the kind's road band ceiling.  A site with a network road in its band
    // gets its road distance, heading and track flag.
    auto commonMask = [&](Site& s, float roadMax) {
        unsigned mask = 0;
        if (!inRegion(s.x, s.z))
        {
            mask |= 1u << RjRegion;
        }
        if (s.x < opt.edgeMargin || s.z < opt.edgeMargin || s.x > extentX - opt.edgeMargin ||
            s.z > extentZ - opt.edgeMargin)
        {
            mask |= 1u << RjEdge;
        }
        if (s.landmass < 0 || townLands.find(s.landmass) == townLands.end())
        {
            mask |= 1u << RjLandmass;
        }
        const float h = opt.spawnHalfSize;
        if (s.y < opt.minDryHeight || heightAt(s.x - h, s.z - h) < opt.minCornerHeight ||
            heightAt(s.x + h, s.z - h) < opt.minCornerHeight || heightAt(s.x - h, s.z + h) < opt.minCornerHeight ||
            heightAt(s.x + h, s.z + h) < opt.minCornerHeight)
        {
            mask |= 1u << RjLow;
        }
        for (const ScaffoldPoint& k : in.keepouts)
        {
            if (Dist2D(s.x, s.z, k.x, k.z) < opt.keepoutDist)
            {
                mask |= 1u << RjKeepout;
                break;
            }
        }
        for (const ScaffoldPoint& k : in.zones)
        {
            if (Dist2D(s.x, s.z, k.x, k.z) < opt.zoneMinDist)
            {
                mask |= 1u << RjZone;
                break;
            }
        }
        bool serves = false;
        for (const TownState& t : towns)
        {
            const float d = Dist2D(s.x, s.z, t.x, t.z);
            if (d < opt.townMinDist)
            {
                mask |= 1u << RjTown;
            }
            serves = serves || (d <= opt.coverDist && SameLand(t.landmass, s.landmass));
        }
        for (const Source& src : sources)
        {
            if (Dist2D(s.x, s.z, src.x, src.z) < opt.baseSpacing)
            {
                mask |= 1u << RjSpacing;
                break;
            }
        }
        if (!serves)
        {
            mask |= 1u << RjFar;
        }
        if (roads.Nearest(s.x, s.z, opt.roadMin, anyRoad) < opt.roadMin)
        {
            mask |= 1u << RjOnRoad;
        }
        int ri = -1;
        s.roadDist = roads.Nearest(s.x, s.z, roadMax, netRoad, &ri);
        if (ri < 0)
        {
            mask |= 1u << RjNoRoad;
        }
        else
        {
            const GenRoadPoint& r = in.roads[ri];
            s.heading = BaseGenBearingDeg(s.x, s.z, r.x, r.z);
            s.trunkDist = roads.Nearest(s.x, s.z, opt.trackTrunkDist, trunkRoad);
            s.track = r.track && s.trunkDist >= opt.trackTrunkDist;
        }
        return mask;
    };
    auto kindRoadMax = [&](GenBaseKind kind) { return std::min(opt.roadMax, GenBaseKindOf(kind).roadMax); };
    // one cell as a site: the outpost's 3 x 3 footprint first, then (when
    // that fails the terrain rules) the minor kind's centre cell.  Returns
    // the failure mask of the better of the two (0: s is a site).
    auto evalCell = [&](int cx, int cz, Site& s, bool allowMinor, bool full) {
        const GenCell& c = in.cells[(size_t)cz * in.cellsX + cx];
        s = Site();
        s.x = ((float)cx + 0.5f) * in.cellSize;
        s.z = ((float)cz + 0.5f) * in.cellSize;
        s.y = c.height;
        s.landmass = label[(size_t)cz * in.cellsX + cx];
        s.key = (unsigned long long)cz * (unsigned long long)in.cellsX + (unsigned long long)cx;
        float relief = 0;
        const unsigned t3 = TerrainMask(in, opt, cx, cz, 1, relief);
        s.relief = relief;
        if (t3 == 0)
        {
            return commonMask(s, kindRoadMax(GenBaseKind::Outpost)); // an outpost site, or no kind passes here
        }
        float r1 = 0;
        const unsigned t1 = allowMinor ? TerrainMask(in, opt, cx, cz, 0, r1) : ~0u;
        if (t1 != 0 && !full)
        {
            return t3; // no kind's terrain fits: the terrain refusal (the cheap path of the scan)
        }
        const unsigned m3 = full ? (t3 | commonMask(s, kindRoadMax(GenBaseKind::Outpost))) : t3;
        if (t1 == ~0u)
        {
            return m3;
        }
        Site m = s;
        m.minor = true;
        m.kind = nearSea(m.x, m.z) ? GenBaseKind::Coastal : GenBaseKind::Checkpoint;
        const unsigned m1 = t1 | commonMask(m, kindRoadMax(m.kind));
        if (m1 == 0 || PopCount(m1) < PopCount(m3))
        {
            s = m;
            return m1;
        }
        return m3;
    };

    std::vector<Site> sites;
    // the detected outposts first: preferred (their props stand there)
    for (const GenDetected& d : in.detected)
    {
        Site s;
        s.x = d.x;
        s.z = d.z;
        s.y = d.y > 0 ? d.y : heightAt(d.x, d.z);
        s.landmass = LandmassAt(in, label, d.x, d.z);
        s.detected = true;
        s.name = d.name;
        s.key = HashKey(d.name, in.seed);
        const unsigned m = commonMask(s, kindRoadMax(GenBaseKind::Outpost));
        if (m != 0)
        {
            char buf[200];
            snprintf(buf, sizeof(buf), "detected outpost '%s' at [%.0f, %.0f] not used: %s", (const char*)d.name,
                     (double)d.x, (double)d.z, kRejectNames[FirstReason(m)]);
            out.notes.push_back(buf);
            continue;
        }
        const int cx = std::min(std::max((int)floorf(d.x / in.cellSize), 1), in.cellsX - 2);
        const int cz = std::min(std::max((int)floorf(d.z / in.cellSize), 1), in.cellsZ - 2);
        float lo = kNone, hi = -kNone;
        for (int dz = -1; dz <= 1; dz++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                const float hh = in.cells[(size_t)(cz + dz) * in.cellsX + (cx + dx)].height;
                lo = std::min(lo, hh);
                hi = std::max(hi, hh);
            }
        }
        s.relief = hi - lo;
        sites.push_back(s);
    }
    // empty ground: every cell whose 3 x 3 footprint (150 m on a 50 m grid)
    // passes the terrain rules, else (a minor kind) whose centre cell does
    for (int cz = 1; cz < in.cellsZ - 1; cz++)
    {
        for (int cx = 1; cx < in.cellsX - 1; cx++)
        {
            Site s;
            const unsigned m = evalCell(cx, cz, s, opt.minorKinds, false);
            const size_t ci = (size_t)cz * in.cellsX + cx;
            if (m != 0)
            {
                const int why = FirstReason(m);
                rejects[why]++;
                cellWhy[ci] = (signed char)why;
                continue;
            }
            cellWhy[ci] = (signed char)(s.minor ? -3 : -1);
            out.minorCandidates += s.minor ? 1 : 0;
            census(s.x, s.z, s.solids, s.trees);
            sites.push_back(s);
        }
    }
    out.candidates = (int)sites.size();
    {
        std::string line = "sites: " + std::to_string(sites.size()) + " passed (" +
                           std::to_string(out.minorCandidates) + " only for a minor kind); rejected";
        for (int i = 0; i < NRejects; i++)
        {
            if (rejects[i] > 0)
            {
                line += " " + std::string(kRejectNames[i]) + " " + std::to_string(rejects[i]);
            }
        }
        out.notes.push_back(line);
    }

    std::vector<RString> taken = in.takenNames;
    enum Need
    {
        NeedCover,
        NeedRedundancy,
        NeedFloor,
        NNeeds
    };
    bool satisfied = false;
    for (int pick = 0; pick < room; pick++)
    {
        const int uncovered = uncoveredCount(); // also refreshes the nearest two
        std::unordered_map<int, int> landSources;
        for (const Source& s : sources)
        {
            landSources[s.landmass]++;
        }
        auto needsRedundancy = [&](int land) {
            return townLands.find(land) != townLands.end() && landSources[land] < opt.minSources;
        };
        bool anyRedundancy = false;
        for (int land : townLands)
        {
            anyRedundancy = anyRedundancy || (land >= 0 && needsRedundancy(land));
        }
        const bool wantFloor = (int)sources.size() < out.floor;
        if (uncovered == 0 && !anyRedundancy && !wantFloor)
        {
            satisfied = true;
            break; // every rule holds: nothing more to add
        }
        // still valid (spacing to this pass's picks), and the primary gain
        // under the most urgent need that some site can serve.  Within a
        // need the outposts go first (tier 0, with the detected outposts);
        // the minor kinds (tier 1) serve only a need no outpost site can.
        std::vector<int> gain(sites.size(), 0);
        int maxGain = 0;
        int need = NeedCover;
        int tier = 0;
        bool found = false;
        for (need = NeedCover; need < NNeeds && !found; need++)
        {
            if ((need == NeedCover && uncovered == 0) || (need == NeedRedundancy && !anyRedundancy) ||
                (need == NeedFloor && !wantFloor))
            {
                continue;
            }
            for (tier = 0; tier < 2 && !found; tier++)
            {
                maxGain = 0;
                for (int i = 0; i < (int)sites.size(); i++)
                {
                    Site& s = sites[i];
                    gain[i] = 0;
                    if (s.used || (s.minor ? 1 : 0) != tier)
                    {
                        continue;
                    }
                    bool spaced = true;
                    for (int k = out.existing; k < (int)sources.size() && spaced; k++)
                    {
                        spaced = Dist2D(s.x, s.z, sources[k].x, sources[k].z) >= opt.baseSpacing;
                    }
                    if (!spaced)
                    {
                        s.used = true; // a later pick never revives it
                        continue;
                    }
                    if (need == NeedCover)
                    {
                        for (const TownState& t : towns)
                        {
                            gain[i] += (t.d1 > opt.coverDist && SameLand(t.landmass, s.landmass) &&
                                        Dist2D(s.x, s.z, t.x, t.z) <= opt.coverDist)
                                           ? 1
                                           : 0;
                        }
                    }
                    else if (need == NeedRedundancy)
                    {
                        gain[i] = needsRedundancy(s.landmass) ? 1 : 0;
                    }
                    else
                    {
                        gain[i] = 1; // every site serves a town (RjFar)
                    }
                    maxGain = std::max(maxGain, gain[i]);
                }
                found = maxGain > 0;
            }
        }
        if (!found)
        {
            out.notes.push_back("no site left that serves an open need: generation stops at " +
                                std::to_string(out.bases.size()) + " base(s)");
            break;
        }
        need--; // the loops stepped once past the need and tier that served
        tier--;
        // the bearing violations now
        int violBefore = 0;
        for (const TownState& t : towns)
        {
            if (t.s1 >= 0 && t.s2 >= 0)
            {
                violBefore += BearingViolation(t, sources[t.s1].x, sources[t.s1].z, sources[t.s2].x, sources[t.s2].z,
                                               opt.bearingMinDeg)
                                  ? 1
                                  : 0;
            }
        }
        const int tracksNeeded = ((int)out.bases.size() + 1 + 2) / 3;
        const bool trackShort = out.tracks < tracksNeeded;
        int best = -1;
        float bestScore = -kNone;
        for (int i = 0; i < (int)sites.size(); i++)
        {
            const Site& s = sites[i];
            if (s.used || gain[i] <= 0 || gain[i] < maxGain - 1)
            {
                continue;
            }
            // bearing: the town's two nearest with this site added
            int violAfter = 0;
            for (const TownState& t : towns)
            {
                float ax = 0, az = 0, bx = 0, bz = 0;
                int n = 0;
                const float dc = SameLand(t.landmass, s.landmass) ? Dist2D(t.x, t.z, s.x, s.z) : kNone;
                // merge the site into the nearest two
                if (dc < t.d1)
                {
                    ax = s.x;
                    az = s.z;
                    n = 1;
                    if (t.s1 >= 0)
                    {
                        bx = sources[t.s1].x;
                        bz = sources[t.s1].z;
                        n = 2;
                    }
                }
                else if (t.s1 >= 0)
                {
                    ax = sources[t.s1].x;
                    az = sources[t.s1].z;
                    n = 1;
                    if (dc < t.d2)
                    {
                        bx = s.x;
                        bz = s.z;
                        n = 2;
                    }
                    else if (t.s2 >= 0)
                    {
                        bx = sources[t.s2].x;
                        bz = sources[t.s2].z;
                        n = 2;
                    }
                }
                if (n == 2)
                {
                    violAfter += BearingViolation(t, ax, az, bx, bz, opt.bearingMinDeg) ? 1 : 0;
                }
            }
            float bearing = opt.bearingWeight * (float)(violBefore - violAfter);
            bearing = std::max(-200.0f, std::min(200.0f, bearing));
            // quality: flat ground, the middle of the road band, spread, jitter
            float minSrc = kNone;
            for (const Source& src : sources)
            {
                minSrc = std::min(minSrc, Dist2D(s.x, s.z, src.x, src.z));
            }
            const float flat = std::max(0.0f, std::min(1.0f, 1.0f - s.relief / 10.0f)) * 100.0f;
            const float bandMax = s.detected ? opt.roadMax : kindRoadMax(s.kind);
            const float mid = 0.5f * (opt.roadMin + bandMax);
            const float band =
                std::max(0.0f, 1.0f - fabsf(s.roadDist - mid) / std::max(1.0f, bandMax - opt.roadMin)) * 100.0f;
            const float spread =
                std::max(0.0f, std::min(1.0f, (minSrc - opt.baseSpacing) / 2400.0f)) * 100.0f;
            const float jit = (float)Roll(s.key, 0x6E42A5u, 1000) * 0.001f * opt.jitter;
            const float quality = std::max(0.0f, std::min(300.0f, flat + band + spread + jit));
            // object-free footprints: a building, a wall, a fence or a gate
            // in the layout's reach costs nearly a town, trees a little; a
            // detected outpost's own props are the point of it
            float clutter = 0;
            if (!s.detected)
            {
                clutter = std::min(opt.clutterMax,
                                   (s.solids > 0 ? opt.solidPenalty : 0.0f) + opt.treePenalty * (float)s.trees);
            }
            float score = opt.townWeight * (float)gain[i] + bearing + quality - clutter;
            score += s.detected ? opt.detectedBonus : 0.0f;
            score += (trackShort && s.track) ? opt.trackBonus : 0.0f;
            if (score > bestScore)
            {
                bestScore = score;
                best = i;
            }
        }
        if (best < 0)
        {
            break;
        }
        Site& s = sites[best];
        s.used = true;
        GeneratedBase b;
        b.kind = s.detected ? GenBaseKind::Outpost : s.kind;
        b.x = s.x;
        b.z = s.z;
        b.y = s.y;
        b.heading = s.heading;
        b.detected = s.detected;
        b.track = s.track;
        b.roadDist = s.roadDist;
        b.trunkDist = s.trunkDist;
        b.covers = need == NeedCover ? gain[best] : 0;
        b.solids = s.solids;
        b.trees = s.trees;
        if (s.detected)
        {
            b.name = UniqueName(std::string((const char*)s.name), taken);
        }
        else
        {
            const char* noun = GenBaseKindOf(b.kind).noun;
            float nearD = kNone;
            for (const TownState& t : towns)
            {
                const float d = Dist2D(s.x, s.z, t.x, t.z);
                if (d <= opt.nameTownDist && d < nearD)
                {
                    nearD = d;
                    b.nearTown = t.name;
                }
            }
            std::string base;
            if (b.nearTown.GetLength() > 0)
            {
                base = std::string((const char*)b.nearTown) + " " + noun;
            }
            else
            {
                base = "Hill " + std::to_string((int)floorf(s.y + 0.5f)) + " " + noun;
            }
            b.name = UniqueName(base, taken);
        }
        if (tier == 1)
        {
            out.notes.push_back(std::string((const char*)b.name) + ": no outpost site serves the open need, so a " +
                                GenBaseKindOf(b.kind).noun + " (its centre cell passes the terrain rules) takes it");
        }
        taken.push_back(b.name);
        out.tracks += b.track ? 1 : 0;
        out.minor += GenBaseKindOf(b.kind).countsForWin ? 0 : 1;
        out.bases.push_back(b);
        Source src;
        src.name = b.name;
        src.x = b.x;
        src.z = b.z;
        src.landmass = s.landmass;
        sources.push_back(src);
    }
    if (!satisfied && (int)out.bases.size() >= room && room > 0)
    {
        out.notes.push_back("the ceiling (min(ceil(K / 2), " + std::to_string(opt.maxTotal) +
                            ") total, or the zone cap) stopped generation at " + std::to_string(out.bases.size()) +
                            " base(s)");
    }
    else if (room <= 0)
    {
        out.notes.push_back("no room under the ceiling: " + std::to_string(out.existing) + " base(s) for " +
                            std::to_string(out.K) + " town(s) already");
    }

    // the report after generation
    out.uncoveredAfter = uncoveredCount();
    out.trackNeed = ((int)out.bases.size() + 2) / 3;
    if (out.tracks < out.trackNeed)
    {
        out.notes.push_back("track share short: " + std::to_string(out.tracks) + " of " +
                            std::to_string(out.bases.size()) + " generated base(s) on a track, the rule asks " +
                            std::to_string(out.trackNeed) + " (no track site served an open need)");
    }
    std::string open;
    for (const TownState& t : towns)
    {
        BaseGenTownReport r;
        r.name = t.name;
        r.x = t.x;
        r.z = t.z;
        r.landmass = t.landmass;
        if (t.s1 >= 0)
        {
            r.nearest = sources[t.s1].name;
            r.nearestDist = t.d1;
        }
        if (t.s1 >= 0 && t.s2 >= 0)
        {
            r.bearingGap = BaseGenBearingGap(BaseGenBearingDeg(t.x, t.z, sources[t.s1].x, sources[t.s1].z),
                                             BaseGenBearingDeg(t.x, t.z, sources[t.s2].x, sources[t.s2].z));
        }
        if (t.d1 > opt.coverDist)
        {
            open += (open.empty() ? "" : ", ") + std::string((const char*)t.name);
            // why no site served it: the refusals of the cells in reach, and
            // the near misses (cells one rule short of a site, for an outpost
            // or a minor kind), each with the nearest such cell
            int why[NRejects] = {};
            int miss[NRejects] = {};
            float missD[NRejects];
            float missX[NRejects];
            float missZ[NRejects];
            for (int i = 0; i < NRejects; i++)
            {
                missD[i] = kNone;
                missX[i] = missZ[i] = 0;
            }
            int passed = 0;
            const int reach = (int)ceilf(opt.coverDist / in.cellSize);
            const int tx = (int)floorf(t.x / in.cellSize);
            const int tz = (int)floorf(t.z / in.cellSize);
            for (int cz = std::max(1, tz - reach); cz <= std::min(in.cellsZ - 2, tz + reach); cz++)
            {
                for (int cx = std::max(1, tx - reach); cx <= std::min(in.cellsX - 2, tx + reach); cx++)
                {
                    const float x = ((float)cx + 0.5f) * in.cellSize;
                    const float z = ((float)cz + 0.5f) * in.cellSize;
                    const float dt = Dist2D(x, z, t.x, t.z);
                    if (dt > opt.coverDist || !SameLand(label[(size_t)cz * in.cellsX + cx], t.landmass))
                    {
                        continue;
                    }
                    const int w = cellWhy[(size_t)cz * in.cellsX + cx];
                    if (w >= 0)
                    {
                        why[w]++;
                    }
                    passed += (w == -1 || w == -3) ? 1 : 0;
                    Site s;
                    const unsigned m = evalCell(cx, cz, s, opt.minorKinds, true);
                    if (PopCount(m) == 1)
                    {
                        const int r1 = FirstReason(m);
                        miss[r1]++;
                        if (dt < missD[r1])
                        {
                            missD[r1] = dt;
                            missX[r1] = x;
                            missZ[r1] = z;
                        }
                    }
                }
            }
            std::string line = std::string((const char*)t.name) + " is open: " + std::to_string(passed) +
                               " site(s) in reach (spaced out by the picks or none); cells refused";
            for (int i = 0; i < NRejects; i++)
            {
                if (why[i] > 0)
                {
                    line += " " + std::string(kRejectNames[i]) + " " + std::to_string(why[i]);
                }
            }
            out.notes.push_back(line);
            std::string near = std::string((const char*)t.name) + " near misses (one rule short of a site):";
            bool anyMiss = false;
            for (int i = 0; i < NRejects; i++)
            {
                if (miss[i] > 0)
                {
                    char buf[160];
                    snprintf(buf, sizeof(buf), " %s %d (nearest [%.0f, %.0f], %.0f m)", kRejectNames[i], miss[i],
                             (double)missX[i], (double)missZ[i], (double)missD[i]);
                    near += buf;
                    anyMiss = true;
                }
            }
            out.notes.push_back(anyMiss ? near : near + " none");
        }
        out.towns.push_back(r);
    }
    if (!open.empty())
    {
        out.notes.push_back("towns still beyond " + std::to_string((int)opt.coverDist) +
                            " m of a ground base: " + open);
    }
}

// ---------------------------------------------------------------------------
// the stored row
// ---------------------------------------------------------------------------

LSError GeneratedBaseRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    PARAM_CHECK(ar.Serialize("type", type, 1, RString("OUTPOST")))
    PARAM_CHECK(ar.Serialize("x", x, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("y", y, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("z", z, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("garrisonMax", garrisonMax, 1, 0))
    PARAM_CHECK(ar.Serialize("reserve", reserve, 1, 0))
    PARAM_CHECK(ar.Serialize("countsForWin", countsForWin, 1, true))
    PARAM_CHECK(ar.Serialize("heading", heading, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("detected", detected, 1, false))
    PARAM_CHECK(ar.Serialize("track", track, 1, false))
    PARAM_CHECK(ar.Serialize("layoutSeed", layoutSeed, 1, 0))
    return LSOK;
}

// ---------------------------------------------------------------------------
// the engine probe
// ---------------------------------------------------------------------------

float LandscapeBaseSiteProbe::SurfaceHeight::Height(float x, float z) const
{
    return GLandscape ? GLandscape->SurfaceY(x, z) : 0.0f;
}

int BaseSiteObjectClass(const Object* obj)
{
    // map objects only (roads are Network, runtime entities are vehicles),
    // with geometry: what a layout prop would stand on or against
    if (!obj || obj->GetType() != Primary || !obj->HasGeometry())
    {
        return 0;
    }
    const LODShapeWithShadow* shape = obj->GetShape();
    if (!shape)
    {
        return 0;
    }
    switch (shape->GetMapType())
    {
        case MapTree:
        case MapSmallTree:
        case MapBush:
        case MapForestBorder:
        case MapForestTriangle:
        case MapForestSquare:
            return 1;
        default:
            break;
    }
    if (obj->GetDestructType() == DestructTree)
    {
        return 1;
    }
    // a sign or a stone never blocks a prop row
    return shape->BoundingSphere() < 0.75f ? 0 : 2;
}

void BaseSiteObjectsNear(float x, float z, float r, std::vector<BaseSiteObject>& out)
{
    out.clear();
    if (!GLandscape)
    {
        return;
    }
    const float grid = GLandscape->GetLandGrid();
    const int range = GLandscape->GetLandRange();
    const int x0 = std::max(0, (int)floorf((x - r) / grid));
    const int x1 = std::min(range - 1, (int)floorf((x + r) / grid));
    const int z0 = std::max(0, (int)floorf((z - r) / grid));
    const int z1 = std::min(range - 1, (int)floorf((z + r) / grid));
    for (int zz = z0; zz <= z1; zz++)
    {
        for (int xx = x0; xx <= x1; xx++)
        {
            const ObjectList& list = GLandscape->GetObjects(zz, xx);
            for (int i = 0; i < list.Size(); i++)
            {
                const Object* obj = list[i];
                const int kind = BaseSiteObjectClass(obj);
                if (kind == 0 || Dist2D(obj->Position().X(), obj->Position().Z(), x, z) > r)
                {
                    continue;
                }
                BaseSiteObject o;
                const char* name = obj->GetShape()->Name();
                const char* slash = name ? strrchr(name, '\\') : nullptr;
                o.model = slash ? slash + 1 : (name ? name : "");
                o.x = obj->Position().X();
                o.z = obj->Position().Z();
                o.radius = obj->GetShape()->BoundingSphere();
                o.solid = kind == 2;
                out.push_back(o);
            }
        }
    }
}

namespace
{
// the last generation's notes (gmBaseGenNotes; transient diagnostics)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
std::vector<std::string>& LastNotes()
{
    static std::vector<std::string> notes;
    return notes;
}
#pragma clang diagnostic pop
} // namespace

const std::vector<std::string>& BaseGenLastNotes()
{
    return LastNotes();
}

bool LandscapeBaseSiteProbe::Gather(BaseGenInputs& in) const
{
    if (!GLandscape || !GWorld)
    {
        return false;
    }
    const int range = GLandscape->GetLandRange();
    const float grid = GLandscape->GetLandGrid();
    if (range <= 0 || grid <= 0)
    {
        return false;
    }
    in.cellsX = range;
    in.cellsZ = range;
    in.cellSize = grid;
    in.cells.assign((size_t)range * range, GenCell());
    in.height = &height;
    for (int z = 0; z < range; z++)
    {
        for (int x = 0; x < range; x++)
        {
            const GeographyInfo g = GLandscape->GetGeography(x, z);
            GenCell& c = in.cells[(size_t)z * range + x];
            c.height = GLandscape->SurfaceY(((float)x + 0.5f) * grid, ((float)z + 0.5f) * grid);
            c.water = g.u.waterDepth > 0 || c.height < 0.0f;
            c.deep = g.u.waterDepth >= 2;
            c.full = g.u.full != 0;
            c.forestInner = g.u.forestInner != 0;
            c.forestOuter = g.u.forestOuter != 0;
            c.road = g.u.road != 0;
            c.track = g.u.track != 0;
            c.objects = (int)g.u.howManyObjects;
            c.hardObjects = (int)g.u.howManyHardObjects;
            c.gradient = (int)g.u.gradient;
            // the object grid, as the layout props see it (step 3)
            const ObjectList& list = GLandscape->GetObjects(z, x);
            for (int i = 0; i < list.Size(); i++)
            {
                const Object* obj = list[i];
                const int kind = BaseSiteObjectClass(obj);
                if (kind == 0)
                {
                    continue;
                }
                GenObject o;
                o.x = obj->Position().X();
                o.z = obj->Position().Z();
                o.radius = obj->GetShape()->BoundingSphere();
                o.solid = kind == 2;
                in.objects.push_back(o);
            }
        }
    }
    // every road link once, labelled by component (the island network: a
    // component with at least a quarter of the largest one's links)
    if (GRoadNet)
    {
        std::vector<const RoadLink*> links;
        std::unordered_set<const RoadLink*> seen;
        for (int zz = 0; zz < range; zz++)
        {
            for (int xx = 0; xx < range; xx++)
            {
                const RoadList& list = GRoadNet->GetRoadList(xx, zz);
                for (int i = 0; i < list.Size(); i++)
                {
                    const RoadLink* link = list[i];
                    if (link && seen.insert(link).second)
                    {
                        links.push_back(link);
                    }
                }
            }
        }
        std::unordered_map<const RoadLink*, int> comp;
        std::vector<int> compSize;
        std::vector<const RoadLink*> stack;
        for (const RoadLink* start : links)
        {
            if (comp.find(start) != comp.end())
            {
                continue;
            }
            const int id = (int)compSize.size();
            compSize.push_back(0);
            comp[start] = id;
            stack.clear();
            stack.push_back(start);
            while (!stack.empty())
            {
                const RoadLink* l = stack.back();
                stack.pop_back();
                compSize[id]++;
                for (int k = 0; k < l->NConnections(); k++)
                {
                    const RoadLink* n = l->Connections()[k];
                    if (n && seen.count(n) && comp.find(n) == comp.end())
                    {
                        comp[n] = id;
                        stack.push_back(n);
                    }
                }
            }
        }
        int largest = 0;
        for (int s : compSize)
        {
            largest = std::max(largest, s);
        }
        const int networkMin = std::max(8, largest / 4);
        for (const RoadLink* l : links)
        {
            GenRoadPoint p;
            p.network = compSize[comp[l]] >= networkMin;
            const Object* obj = l->GetObject();
            const LODShapeWithShadow* shape = obj ? obj->GetShape() : nullptr;
            p.track = shape && RoadModelIsTrack(shape->Name());
            const Vector3 c = l->GetCenter();
            p.x = c.X();
            p.z = c.Z();
            in.roads.push_back(p);
            for (int k = 0; k < l->NConnections(); k++)
            {
                GenRoadPoint e = p;
                e.x = l->PosConnections()[k].X();
                e.z = l->PosConnections()[k].Z();
                in.roads.push_back(e);
            }
        }
    }
    if (Person* player = GWorld->GetRealPlayer())
    {
        ScaffoldPoint k;
        k.x = player->Position().X();
        k.z = player->Position().Z();
        in.keepouts.push_back(k);
    }
    return true;
}

bool LandscapeBaseSiteProbe::IsSettlement(const char* /*name*/, float x, float z) const
{
    if (!GLandscape || !GWorld)
    {
        return true;
    }
    if (GLandscape->SurfaceY(x, z) < 0.1f)
    {
        return false;
    }
    int buildings = 0;
    const float r = LandscapeSettlementProbe::kRadius;
    for (int i = 0; i < GWorld->NBuildings() && buildings < LandscapeSettlementProbe::kMinBuildings; i++)
    {
        Poseidon::Building* b = dyn_cast<Poseidon::Building>(GWorld->GetBuilding(i));
        if (!b)
        {
            continue;
        }
        LODShapeWithShadow* shape = b->GetShape();
        if (!shape || shape->BoundingSphere() <= LandscapeSettlementProbe::kMinBuildingRadius)
        {
            continue;
        }
        if (Dist2D(b->Position().X(), b->Position().Z(), x, z) <= r)
        {
            buildings++;
        }
    }
    return buildings >= LandscapeSettlementProbe::kMinBuildings;
}

// ---------------------------------------------------------------------------
// the seeding entry (ZoneRegistry::LoadFromParams, after SeedCityZones)
// ---------------------------------------------------------------------------

bool ZoneRegistry::ReadGenerateBases(const ParamEntry* zonesCfg)
{
    return zonesCfg && zonesCfg->FindEntry("generateBases") && zonesCfg->ReadValue("generateBases", 0.0f) != 0.0f;
}

void ZoneRegistry::InjectGeneratedZones()
{
    for (int i = 0; i < _generated.Size(); i++)
    {
        const GeneratedBaseRow& row = _generated[i];
        if (FindZoneIndex(row.name) >= 0)
        {
            LOG_INFO(Core, "ZoneRegistry: generated base '{}' not injected - a zone of that name exists",
                     (const char*)row.name);
            continue;
        }
        if (_zones.Size() >= MaxZones)
        {
            LOG_WARN(Core, "ZoneRegistry: zone cap ({}) reached injecting generated bases - the rest skipped",
                     MaxZones);
            break;
        }
        ZoneRecord z;
        z.name = row.name;
        z.type = row.type;
        z.ownerConfig = "OCCUPIER";
        z.owner = ResolveOwnerToken(z.ownerConfig);
        z.garrison = row.reserve > 0 ? (float)row.reserve : 0.0f;
        z.garrisonMax = row.garrisonMax > 0 ? (float)row.garrisonMax : 0.0f;
        z.income = _tuning.defaultIncome;
        z.seeded = true;
        z.generated = true;
        z.countsForWin = row.countsForWin;
        char marker[32];
        snprintf(marker, sizeof(marker), "gmZoneGen_%d", i);
        z.marker = marker;
        z.pos = Vector3(row.x, row.y, row.z);
        _zones.Add(z);
    }
}

void ZoneRegistry::GenerateBaseZones(const ParamEntry& zonesCfg, const SettlementProbe* /*settlement*/)
{
    _tuning.generateBases = ReadGenerateBases(&zonesCfg);
    if (_genDecided)
    {
        // a load (the stored rows), or this session already decided: never
        // re-derived
        if (_genOldSave && _tuning.generateBases)
        {
            LOG_INFO(Core, "ZoneRegistry: this campaign was saved before generated bases existed - none generated "
                           "(generateBases applies to new games)");
        }
        _genOldSave = false;
        InjectGeneratedZones();
        return;
    }
    _generated.Clear();
    if (!_tuning.generateBases)
    {
        _genDecided = true;
        return;
    }
    const auto start = std::chrono::steady_clock::now();
    static LandscapeBaseSiteProbe engineProbe;
    const BaseSiteProbe* probe = _siteProbeOverride ? _siteProbeOverride : &engineProbe;
    BaseGenInputs in;
    if (!probe->Gather(in))
    {
        return; // no world: nothing decided, nothing generated
    }
    _genDecided = true;
    // the seed: pinned by generateBasesSeed, else the one draw
    unsigned long long seed = 0;
    if (zonesCfg.FindEntry("generateBasesSeed"))
    {
        seed = (unsigned long long)(unsigned)toInt(zonesCfg.ReadValue("generateBasesSeed", 0.0f));
    }
    if (seed == 0)
    {
        seed = (unsigned long long)(unsigned)(toInt(GRandGen.RandomValue() * 1073741823.0f) | 1);
    }
    in.seed = seed;
    // the registry's half: towns, sources, zones, keepouts, names, region
    const RString startTown = ScriptVars::String("gmselstarttown");
    for (int i = 0; i < _zones.Size(); i++)
    {
        const ZoneRecord& z = _zones[i];
        ScaffoldPoint p;
        p.x = z.pos.X();
        p.z = z.pos.Z();
        in.zones.push_back(p);
        in.takenNames.push_back(z.name);
        if (stricmp(z.type, "CAMP") == 0 || (startTown.GetLength() > 0 && stricmp(z.name, startTown) == 0))
        {
            in.keepouts.push_back(p);
        }
        GenPlace place;
        place.name = z.name;
        place.x = p.x;
        place.z = p.z;
        if (stricmp(z.type, "CITY") == 0)
        {
            // seedCities = 1 seeds every label: only a settlement is a town
            // the density rule serves
            if (z.seeded && _tuning.seedCities == ZoneTuning::SeedCities::All &&
                !probe->IsSettlement(z.name, p.x, p.z))
            {
                continue;
            }
            in.towns.push_back(place);
        }
        else if (CampaignEnd::IsMilitaryType(z.type) && stricmp(z.owner, _occupierSide) == 0)
        {
            in.sources.push_back(place);
        }
    }
    for (const BaseCandidate& c : _detectedOutposts)
    {
        if (c.status != BaseStatus::Offline)
        {
            continue;
        }
        GenDetected d;
        d.name = c.name;
        d.x = c.x;
        d.z = c.z;
        d.y = c.y;
        in.detected.push_back(d);
    }
    if (_tuning.playRegion.valid)
    {
        in.hasRegion = true;
        in.regionMinX = _tuning.playRegion.minX;
        in.regionMinZ = _tuning.playRegion.minZ;
        in.regionMaxX = _tuning.playRegion.maxX;
        in.regionMaxZ = _tuning.playRegion.maxZ;
    }
    in.zoneRoom = MaxZones - _zones.Size();
    BaseGenOptions opt;
    // generateBasesMinor = 0: no minor kinds (an island author's switch;
    // absent = 1, the step 3 rule)
    if (zonesCfg.FindEntry("generateBasesMinor"))
    {
        opt.minorKinds = zonesCfg.ReadValue("generateBasesMinor", 1.0f) != 0.0f;
    }
    BaseGenResult result;
    GenerateBases(in, opt, result);
    for (const GeneratedBase& b : result.bases)
    {
        const GenBaseKindInfo& kind = GenBaseKindOf(b.kind);
        GeneratedBaseRow row;
        row.name = b.name;
        row.type = kind.type;
        row.x = b.x;
        row.y = b.y;
        row.z = b.z;
        row.garrisonMax = kind.garrisonMax;
        row.reserve = kind.reserve;
        row.countsForWin = kind.countsForWin;
        row.heading = b.heading;
        row.detected = b.detected;
        row.track = b.track;
        row.layoutSeed = (int)(HashKey(b.name, seed) & 0x7fffffffull);
        _generated.Add(row);
    }
    InjectGeneratedZones();
    for (const std::string& note : result.notes)
    {
        LOG_INFO(Core, "ZoneRegistry: base generation: {}", note);
    }
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    LOG_INFO(Core,
             "ZoneRegistry: generated {} base(s), {} of a minor kind, on '{}' (K {}, existing {}, floor {}, ceiling "
             "{}; towns beyond {} m: {} before, {} after; {} on a track of {} asked) from seed {} in {:.1f} ms",
             (int)result.bases.size(), result.minor, (const char*)Glob.header.worldname, result.K, result.existing,
             result.floor,
             result.ceiling, (int)opt.coverDist, result.uncoveredBefore, result.uncoveredAfter, result.tracks,
             result.trackNeed, (unsigned long long)seed, ms);
    std::vector<BaseSiteObject> objs;
    for (const GeneratedBase& b : result.bases)
    {
        // the solids in the layout's reach, by model (an empty-ground pick
        // should have none: the clutter penalty)
        std::string solids;
        if (!b.detected && !_siteProbeOverride)
        {
            BaseSiteObjectsNear(b.x, b.z, opt.layoutReach, objs);
            for (const BaseSiteObject& o : objs)
            {
                if (o.solid)
                {
                    solids += (solids.empty() ? "" : ", ") + o.model;
                }
            }
        }
        LOG_INFO(Core,
                 "ZoneRegistry: generated {} '{}' at [{:.0f}, {:.0f}] ({}, road {:.0f} m, trunk {}, heading {:.0f}, "
                 "covers {} town(s); within {:.0f} m {} solid and {} tree object(s){}{})",
                 GenBaseKindOf(b.kind).type, (const char*)b.name, (double)b.x, (double)b.z,
                 b.detected ? "detected outpost" : (b.track ? "on a track" : "empty ground"), (double)b.roadDist,
                 b.trunkDist >= 1e8f ? std::string("none in reach") : std::to_string((int)b.trunkDist) + " m",
                 (double)b.heading, b.covers, (double)opt.layoutReach, b.solids, b.trees, solids.empty() ? "" : ": ", solids);
    }
    LastNotes() = result.notes;
}

} // namespace Poseidon::Guerrilla

// ---------------------------------------------------------------------------
// script surface (step 3 diagnostics)
// ---------------------------------------------------------------------------

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

// gmBaseGenNotes -> the last generation's notes, one string each (the
// "ZoneRegistry: base generation:" INFO lines: the refusal counts, an open
// town's refusals and near misses, a minor kind taking a need)
static GameValue GmBaseGenNotes(const GameState* state)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    for (const std::string& note : BaseGenLastNotes())
    {
        array.Add(GameValue(GameStringType(RString(note.c_str()))));
    }
    return value;
}

// gmBaseSiteObjects [easting, northing, radius] -> the map objects the site
// scorer and the layout props count there: [[model, [easting, northing],
// radius, solid], ...] (solid false = a tree, a bush or a forest piece)
static GameValue GmBaseSiteObjects(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 3))
    {
        return value;
    }
    std::vector<BaseSiteObject> objs;
    BaseSiteObjectsNear((float)array[0], (float)array[1], (float)array[2], objs);
    for (const BaseSiteObject& o : objs)
    {
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(4);
        row[0] = GameStringType(RString(o.model.c_str()));
        GameValue posValue = state->CreateGameValue(GameArray);
        GameArrayType& pos = posValue;
        pos.Resize(2);
        pos[0] = o.x;
        pos[1] = o.z;
        row[1] = posValue;
        row[2] = o.radius;
        row[3] = o.solid;
        out.Add(rowValue);
    }
    return value;
}

INIT_MODULE(GuerrillaBaseGenerate, 3)
{
    GGameState.NewNularOp(GameNular(GameArray, "gmBaseGenNotes", GmBaseGenNotes));
    GGameState.NewFunction(GameFunction(GameArray, "gmBaseSiteObjects", GmBaseSiteObjects, GameArray));
}