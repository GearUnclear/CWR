#include <Poseidon/Game/Guerrilla/IslandScaffold.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Poseidon::Guerrilla
{

// ---------------------------------------------------------------------------
// object classification
// ---------------------------------------------------------------------------
//
// GROUND TRUTH for these token lists: the model histograms of the two data sets
// Guerrilla Mode targets, read with `PoseidonTools terrain objects <wrp>`.
//
//   Classic 1.99 abel.wrp (248 distinct models) puts everything in data3d\ and
//   names it in Czech: roads are cesta* (dirt), silnice* (road), asfaltka* and
//   the shipped typo asfatlka* (asphalt), plus kr_* crossing pieces; houses are
//   dum*/domek (house), budova* (building), deutshe*, cihlovej_dum (brick
//   house), hruzdum, bouda*/plechbud (shed), stodola/statek (barn/farm),
//   kostel*/kaple/zvonice (church/chapel/belfry), garaz, hangar, majak
//   (lighthouse), fortress*, fuelstation*, repair_center.
//
//   @LoBo sinai.wrp (655 distinct models) is Resistance-era and DIRECTORY-keyed:
//   o\road\ (ces*/sil*/asf*/kos*), lobo_ob\roads\ (asp*), lobobloc\croads\,
//   o\hous\, lobo_ob\buildings\, lobo_ob\blocs\, lobo_ob\city\, baracken\,
//   ags_inds\whouse*.
//
// So both a DIRECTORY test and a BASENAME test are needed; either one accepting
// is enough.
//
// LIMITS, stated plainly:
//   * This is a name guess, not the engine's answer. The engine calls a road
//     anything whose p3d resolves to Object type Network and a house anything
//     that dyn_casts to Building with a bounding sphere over 4 m; neither is
//     knowable without loading the model.
//   * A mod that names its houses in a private vocabulary inside a directory
//     that says nothing ("lobo_ob\gw\tp\...") is invisible here. The failure is
//     benign in one direction and not in the other: missed houses can only
//     REJECT a town (fewer CITY zones than the running game will seed, and the
//     mission-time classifier adds them back because the scaffolded template
//     leaves seedCities absent); missed roads make the camp/outpost placement
//     fall back to a plain grid sweep, which is reported as a warning.
//   * Bridges (most*) count as road: they carry traffic and a camp does not
//     belong on one.
//   * Ruins (dum_zboreny, dumruina) count as buildings. A bombed-out village is
//     still a village for the purpose of "is this Names entry a settlement".

namespace
{

std::string LowerPath(const char* modelPath)
{
    std::string s(modelPath ? modelPath : "");
    for (char& c : s)
    {
        if (c == '/')
        {
            c = '\\'; // mods author both separators; normalise before matching
        }
        else
        {
            c = (char)tolower((unsigned char)c);
        }
    }
    return s;
}

bool Contains(const std::string& hay, const char* needle)
{
    return hay.find(needle) != std::string::npos;
}

// the file name without directory or extension
std::string BaseName(const std::string& path)
{
    size_t slash = path.rfind('\\');
    std::string base = slash == std::string::npos ? path : path.substr(slash + 1);
    size_t dot = base.rfind('.');
    if (dot != std::string::npos)
    {
        base = base.substr(0, dot);
    }
    return base;
}

std::string DirName(const std::string& path)
{
    size_t slash = path.rfind('\\');
    return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
}

bool StartsWithAny(const std::string& base, const char* const* prefixes, int n)
{
    for (int i = 0; i < n; i++)
    {
        size_t len = strlen(prefixes[i]);
        if (base.size() >= len && base.compare(0, len, prefixes[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

bool ContainsAny(const std::string& s, const char* const* tokens, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (Contains(s, tokens[i]))
        {
            return true;
        }
    }
    return false;
}

float Dist2D(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return sqrtf(dx * dx + dz * dz);
}

float Dist2DSqF(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return dx * dx + dz * dz;
}

// A uniform bucket grid over a point cloud. Both the nearest-road query and the
// houses-within-300 m count run tens of thousands of times over clouds of tens
// of thousands of points, so the naive scan is the difference between a second
// and a minute.
class PointGrid
{
  public:
    void Build(const std::vector<ScaffoldPoint>& pts, float cell)
    {
        _pts = &pts;
        _cell = cell > 1.0f ? cell : 1.0f;
        _cells.clear();
        _nx = _nz = 0;
        if (pts.empty())
        {
            return;
        }
        _minX = _minZ = 1e30f;
        float maxX = -1e30f, maxZ = -1e30f;
        for (const ScaffoldPoint& p : pts)
        {
            _minX = std::min(_minX, p.x);
            _minZ = std::min(_minZ, p.z);
            maxX = std::max(maxX, p.x);
            maxZ = std::max(maxZ, p.z);
        }
        _nx = (int)((maxX - _minX) / _cell) + 1;
        _nz = (int)((maxZ - _minZ) / _cell) + 1;
        _cells.resize((size_t)_nx * _nz);
        for (size_t i = 0; i < pts.size(); i++)
        {
            _cells[Index(pts[i].x, pts[i].z)].push_back((int)i);
        }
    }

    bool Empty() const { return _pts == nullptr || _pts->empty(); }

    // Distance to the nearest point, searching cell rings outward. Returns
    // maxDist when nothing is nearer than that (callers only ever compare
    // against a band, so an exact answer beyond the cap buys nothing).
    float NearestDist(float x, float z, float maxDist) const
    {
        if (Empty())
        {
            return maxDist;
        }
        int cx = CellX(x), cz = CellZ(z);
        int maxRing = (int)(maxDist / _cell) + 1;
        float best = maxDist;
        for (int ring = 0; ring <= maxRing; ring++)
        {
            // once the closest possible point of this ring is further than the
            // best hit so far, no further ring can improve it
            if (ring > 0 && (float)(ring - 1) * _cell > best)
            {
                break;
            }
            for (int iz = cz - ring; iz <= cz + ring; iz++)
            {
                for (int ix = cx - ring; ix <= cx + ring; ix++)
                {
                    bool onRing = (abs(ix - cx) == ring || abs(iz - cz) == ring);
                    if (!onRing || ix < 0 || iz < 0 || ix >= _nx || iz >= _nz)
                    {
                        continue;
                    }
                    for (int idx : _cells[(size_t)iz * _nx + ix])
                    {
                        const ScaffoldPoint& p = (*_pts)[idx];
                        best = std::min(best, Dist2D(x, z, p.x, p.z));
                    }
                }
            }
        }
        return best;
    }

    int CountWithin(float x, float z, float r) const
    {
        if (Empty())
        {
            return 0;
        }
        int x0 = CellX(x - r), x1 = CellX(x + r);
        int z0 = CellZ(z - r), z1 = CellZ(z + r);
        float r2 = r * r;
        int count = 0;
        for (int iz = std::max(0, z0); iz <= std::min(_nz - 1, z1); iz++)
        {
            for (int ix = std::max(0, x0); ix <= std::min(_nx - 1, x1); ix++)
            {
                for (int idx : _cells[(size_t)iz * _nx + ix])
                {
                    const ScaffoldPoint& p = (*_pts)[idx];
                    if (Dist2DSqF(x, z, p.x, p.z) <= r2)
                    {
                        count++;
                    }
                }
            }
        }
        return count;
    }

  private:
    int CellX(float x) const { return std::clamp((int)floorf((x - _minX) / _cell), -1, _nx); }
    int CellZ(float z) const { return std::clamp((int)floorf((z - _minZ) / _cell), -1, _nz); }
    size_t Index(float x, float z) const
    {
        int ix = std::clamp((int)floorf((x - _minX) / _cell), 0, _nx - 1);
        int iz = std::clamp((int)floorf((z - _minZ) / _cell), 0, _nz - 1);
        return (size_t)iz * _nx + ix;
    }

    const std::vector<ScaffoldPoint>* _pts = nullptr;
    float _cell = 100.0f;
    float _minX = 0, _minZ = 0;
    int _nx = 0, _nz = 0;
    std::vector<std::vector<int>> _cells;
};

std::string Fmt(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return std::string(buf);
}

} // namespace

bool ModelIsRoad(const char* modelPath)
{
    std::string path = LowerPath(modelPath);
    if (path.empty())
    {
        return false;
    }
    std::string dir = DirName(path);
    // Directory vocabulary: "o/road/", "lobo_ob/roads/", "lobobloc/croads/".
    // NOTE: no comment in this file may END in a backslash. Line splicing runs
    // before comment processing, so a trailing backslash swallows the next line
    // of code, and this build suppresses -Wcomment along with ~50 other legacy
    // warnings, so nothing tells you.
    if (Contains(dir, "road") || Contains(dir, "silnic") || Contains(dir, "cesta"))
    {
        return true;
    }
    std::string base = BaseName(path);
    // OFP (data3d\) and Resistance (o\road\) road-piece stems. "kr_" is the
    // crossing family (kr_silnice_asfaltka_t, kr_new_kos_sil_t).
    static const char* kRoadStems[] = {"cesta", "silnice", "asfaltka",
                                       // the stock data set really does ship this typo
                                       "asfatlka", "ces", "sil", "asf", "asp", "kos", "kr_", "most", "runway",
                                       "letiste"};
    if (StartsWithAny(base, kRoadStems, (int)(sizeof(kRoadStems) / sizeof(*kRoadStems))))
    {
        // "ces"/"sil"/"asf"/"asp"/"kos" are three-letter Resistance stems and
        // would swallow unrelated names, so they only count with a road-shaped
        // suffix: a digit (piece length) or "konec" (end cap).
        static const char* kShort[] = {"ces", "sil", "asf", "asp", "kos"};
        if (StartsWithAny(base, kShort, 5) && !StartsWithAny(base, kRoadStems, 4))
        {
            bool hasDigit = false;
            for (char c : base)
            {
                hasDigit = hasDigit || (c >= '0' && c <= '9');
            }
            return hasDigit || Contains(base, "konec");
        }
        return true;
    }
    return false;
}

bool ModelIsBuilding(const char* modelPath)
{
    std::string path = LowerPath(modelPath);
    if (path.empty())
    {
        return false;
    }
    // A road piece under a house-shaped directory (lobobloc\croads\sil25.p3d)
    // must not count as a house: roads are never buildings.
    if (ModelIsRoad(modelPath))
    {
        return false;
    }
    std::string base = BaseName(path);
    // Fences, walls, poles and signs sit inside house directories and would
    // otherwise pass on the directory test alone. Reject them first.
    static const char* kNotBuilding[] = {"plot",   "pletivo", "zaun",  "zed_",     "wall",   "fence", "barbedwire",
                                         "znacka", "lampa",   "pole",  "hlaska",   "sloup",  "vrata", "brana",
                                         "bran",   "hrobec",  "kasna", "paletyc",  "pytle",  "stoh",  "kopa_",
                                         "molo",   "podesta", "jezek", "prolejza", "podlejz"};
    if (ContainsAny(base, kNotBuilding, (int)(sizeof(kNotBuilding) / sizeof(*kNotBuilding))))
    {
        return false;
    }
    std::string dir = DirName(path);
    static const char* kBuildingDirs[] = {"hous",  "build",  "bloc",  "\\city",   "town", "villa",
                                          "barac", "hangar", "chr\\", "hospital", "\\ind"};
    if (ContainsAny(dir, kBuildingDirs, (int)(sizeof(kBuildingDirs) / sizeof(*kBuildingDirs))))
    {
        return true;
    }
    static const char* kBuildingStems[] = {
        "dum",      "dom",         "budova",        "house",   "hous",     "build",   "bloc",     "apartment",
        "flats",    "hotel",       "shop",          "villa",   "kostel",   "kaple",   "zvonice",  "mosque",
        "church",   "bouda",       "plechbud",      "barak",   "baracke",  "stodola", "statek",   "garaz",
        "hangar",   "tovarna",     "factory",       "skola",   "school",   "hosp",    "medic",    "majak",
        "fortress", "fuelstation", "repair_center", "deutshe", "cihlovej", "hruzdum", "ryb_dom",  "nam_",
        "afdum",    "ruiny",       "vysilac",       "vez",     "strazni",  "whouse",  "warehouse"};
    return ContainsAny(base, kBuildingStems, (int)(sizeof(kBuildingStems) / sizeof(*kBuildingStems)));
}

// ---------------------------------------------------------------------------
// helpers shared by the placement pass
// ---------------------------------------------------------------------------

RString SanitizeClassName(const char* raw, const char* fallback)
{
    std::string out;
    for (const char* p = raw ? raw : ""; *p; p++)
    {
        unsigned char c = (unsigned char)*p;
        if (isalnum(c) || c == '_')
        {
            out.push_back((char)c);
        }
    }
    if (out.empty())
    {
        return RString(fallback);
    }
    if (isdigit((unsigned char)out[0]))
    {
        out.insert(out.begin(), 'Z'); // a config class name may not start with a digit
    }
    return RString(out.c_str());
}

namespace
{

RString UniqueClassName(const std::vector<ScaffoldZone>& zones, RString candidate)
{
    std::string base((const char*)candidate);
    std::string tryName = base;
    for (int suffix = 2; suffix < 100; suffix++)
    {
        bool clash = false;
        for (const ScaffoldZone& z : zones)
        {
            clash = clash || stricmp((const char*)z.className, tryName.c_str()) == 0;
        }
        if (!clash)
        {
            return RString(tryName.c_str());
        }
        tryName = base + std::to_string(suffix);
    }
    return RString(tryName.c_str());
}

struct Candidate
{
    float x = 0, z = 0, y = 0;
    float roadDist = 0;
    float flatness = 0;
};

// Relief across a 40 m cross centred on the point: the cheap stand-in for
// "would a camp sit on a slope". Four samples, not a full patch, because the
// candidate set runs to tens of thousands.
float Flatness(const HeightField& h, float x, float z, float y)
{
    const float kArm = 20.0f;
    float lo = y, hi = y;
    const float offs[4][2] = {{kArm, 0}, {-kArm, 0}, {0, kArm}, {0, -kArm}};
    for (const auto& o : offs)
    {
        float s = h.Height(x + o[0], z + o[1]);
        lo = std::min(lo, s);
        hi = std::max(hi, s);
    }
    return hi - lo;
}

// Ring samples around a subsampled set of road anchors: off-road points that
// are still road-reachable, which is what both the camp and the outposts want.
// No randomness anywhere - the same world always scaffolds to the same zones,
// so a re-run is a diff and not a reshuffle.
void GenerateCandidates(const ScaffoldInputs& in, const ScaffoldOptions& opt, const PointGrid& roadGrid,
                        std::vector<Candidate>& out)
{
    out.clear();
    const HeightField& h = *in.height;
    const int kMaxAnchors = 600;
    const float kRadii[] = {80.0f, 140.0f, 200.0f, 260.0f, 320.0f, 380.0f};
    const int kBearings = 12;

    std::vector<ScaffoldPoint> anchors;
    if (!in.roads.empty())
    {
        int stride = std::max<int>(1, (int)(in.roads.size() / kMaxAnchors));
        for (size_t i = 0; i < in.roads.size(); i += stride)
        {
            anchors.push_back(in.roads[i]);
        }
    }
    else
    {
        // No road net was recognised: sweep the map on a 200 m lattice instead.
        // The road-distance band is meaningless then and the caller skips it.
        for (float z = 200.0f; z < in.worldSize; z += 200.0f)
        {
            for (float x = 200.0f; x < in.worldSize; x += 200.0f)
            {
                ScaffoldPoint p;
                p.x = x;
                p.z = z;
                anchors.push_back(p);
            }
        }
    }

    for (const ScaffoldPoint& a : anchors)
    {
        int nRadii = in.roads.empty() ? 1 : (int)(sizeof(kRadii) / sizeof(*kRadii));
        for (int r = 0; r < nRadii; r++)
        {
            int bearings = in.roads.empty() ? 1 : kBearings;
            for (int b = 0; b < bearings; b++)
            {
                Candidate c;
                if (in.roads.empty())
                {
                    c.x = a.x;
                    c.z = a.z;
                }
                else
                {
                    float angle = 2.0f * 3.14159265f * (float)b / (float)kBearings;
                    c.x = a.x + kRadii[r] * cosf(angle);
                    c.z = a.z + kRadii[r] * sinf(angle);
                }
                if (c.x < 100.0f || c.z < 100.0f || c.x > in.worldSize - 100.0f || c.z > in.worldSize - 100.0f)
                {
                    continue; // the map border is water or the engine's out-of-map height
                }
                c.y = h.Height(c.x, c.z);
                if (c.y < opt.minDryHeight)
                {
                    continue;
                }
                c.roadDist = roadGrid.Empty() ? 1e9f : roadGrid.NearestDist(c.x, c.z, 600.0f);
                c.flatness = Flatness(h, c.x, c.z, c.y);
                out.push_back(c);
            }
        }
    }
}

float MinDistToZones(const std::vector<ScaffoldZone>& zones, float x, float z, const char* typeFilter)
{
    float best = 1e9f;
    for (const ScaffoldZone& zn : zones)
    {
        if (typeFilter && stricmp((const char*)zn.type, typeFilter) != 0)
        {
            continue;
        }
        best = std::min(best, Dist2D(x, z, zn.x, zn.z));
    }
    return best;
}

} // namespace

// ---------------------------------------------------------------------------
// town selection
// ---------------------------------------------------------------------------

void SelectTownZones(const ScaffoldInputs& in, const ScaffoldOptions& opt, ScaffoldResult& out)
{
    if (!in.names || !in.height)
    {
        return;
    }
    PointGrid buildingGrid;
    buildingGrid.Build(in.buildings, 100.0f);

    for (int i = 0; i < in.names->GetEntryCount(); i++)
    {
        const ParamEntry& e = in.names->GetEntry(i);
        RString name;
        Vector3 pos;
        // The same filter the running engine uses, so a scaffolded CITY set and
        // the mission-time auto-seed cannot disagree about what counts as a town.
        if (!ZoneRegistry::NamesEntryIsTown(e, name, pos))
        {
            continue;
        }
        float x = pos.X();
        float z = pos.Z();
        // pos.Y() is position[2] of the Names entry, which in an OFP-era Names
        // block is the map-LABEL size, not an elevation (Houdan ships 100 and
        // sits at 72 m). Always sample the terrain.
        float y = in.height->Height(x, z);
        if (y <= opt.minTownHeight)
        {
            out.warnings.push_back(
                Fmt("town '%s' skipped: sampled height %.1f m, it is in the water", (const char*)name, y));
            continue;
        }
        int houses = buildingGrid.CountWithin(x, z, opt.townBuildingRadius);
        if (houses < opt.townMinBuildings)
        {
            out.warnings.push_back(Fmt("town '%s' skipped: %d building(s) within %d m, a town needs %d",
                                       (const char*)name, houses, (int)opt.townBuildingRadius, opt.townMinBuildings));
            continue;
        }
        float dedup = MinDistToZones(out.zones, x, z, nullptr);
        if (dedup < opt.townDedupDist)
        {
            out.warnings.push_back(Fmt("town '%s' skipped: %.0f m from an already-seeded zone (dedup radius %d m)",
                                       (const char*)name, dedup, (int)opt.townDedupDist));
            continue;
        }

        ScaffoldZone zone;
        zone.className = UniqueClassName(out.zones, SanitizeClassName(e.GetName(), "Town"));
        zone.name = name;
        zone.type = "CITY";
        zone.owner = "NEUTRAL";
        zone.x = x;
        zone.z = z;
        zone.y = y;
        zone.support = opt.seedCitySupport;
        zone.note =
            RString(Fmt("CfgWorlds >> %s >> Names >> %s; %d building(s) within %d m", (const char*)in.worldClass,
                        (const char*)e.GetName(), houses, (int)opt.townBuildingRadius)
                        .c_str());
        out.zones.push_back(zone);
        out.cityCount++;
    }
}

// ---------------------------------------------------------------------------
// the whole pass
// ---------------------------------------------------------------------------

bool BuildZones(const ScaffoldInputs& in, const ScaffoldOptions& opt, ScaffoldResult& out)
{
    out = ScaffoldResult();
    if (in.worldClass.GetLength() == 0)
    {
        out.error = "no world class";
        return false;
    }
    if (!in.height)
    {
        out.error = "no height sampler";
        return false;
    }

    SelectTownZones(in, opt, out);
    std::vector<ScaffoldZone> towns = out.zones; // CITY zones, kept for ordering
    out.zones.clear();
    if (towns.empty())
    {
        // Stock Cain (Kolgujev) is the live example: its CfgWorlds Names class
        // is present but EMPTY. The template still boots, but it has no towns to
        // fight over, so the author has to add CITY zones by hand.
        out.warnings.push_back("no CITY zone could be derived: the world's Names block yields no settlement. The "
                               "template will have no towns until you add CITY zones by hand");
    }

    PointGrid roadGrid;
    roadGrid.Build(in.roads, 100.0f);
    if (in.roads.empty())
    {
        out.warnings.push_back("no road pieces recognised in the world: the camp and outposts fall back to a 200 m "
                               "lattice sweep and the off-road distance rules are not enforced");
    }

    std::vector<Candidate> candidates;
    GenerateCandidates(in, opt, roadGrid, candidates);
    if (candidates.empty())
    {
        out.error = "no dry candidate ground found for the camp";
        return false;
    }
    const bool haveRoads = !in.roads.empty();

    // Fallback anchor for a world that names no towns: the map centre. With no
    // settlements there is no "edge of the settled area" to sit on, and the
    // alternative (maximise distance from something) walks the camp into the
    // map corner, which on an island world is an offshore rock.
    float centroidX = in.worldSize * 0.5f, centroidZ = in.worldSize * 0.5f;

    // --- CAMP -------------------------------------------------------------
    // Relaxation ladder: the ideal is far from every town and in the road band.
    // A small or road-poor island can satisfy neither, and refusing to emit a
    // template at all would be the worst possible answer, so each rung is tried
    // in turn and the rung that fired is reported as a warning.
    const float kCityRelax[] = {1.0f, 0.66f, 0.33f, 0.0f};
    int bestIdx = -1;
    int usedRung = 0;
    for (int rung = 0; rung < 4 && bestIdx < 0; rung++)
    {
        float minCity = opt.campMinCityDist * kCityRelax[rung];
        std::vector<int> valid;
        for (size_t i = 0; i < candidates.size(); i++)
        {
            const Candidate& c = candidates[i];
            if (haveRoads && (c.roadDist < opt.campMinRoadDist || c.roadDist > opt.campMaxRoadDist))
            {
                continue;
            }
            if (!towns.empty() && MinDistToZones(towns, c.x, c.z, nullptr) < minCity)
            {
                continue;
            }
            valid.push_back((int)i);
        }
        if (valid.empty())
        {
            continue;
        }
        // prefer the flat ones; if nothing is flat enough, rank by flatness
        std::vector<int> flat;
        for (int i : valid)
        {
            if (candidates[i].flatness <= opt.campFlatTolerance)
            {
                flat.push_back(i);
            }
        }
        const std::vector<int>& pool = flat.empty() ? valid : flat;
        float bestScore = -1e30f;
        for (int i : pool)
        {
            const Candidate& c = candidates[i];
            // With towns: the CLOSEST spot that still clears the minimum town
            // distance, i.e. the rim of the settled area. Maximising the town
            // distance instead looks right on paper and puts the camp on the
            // furthest offshore islet on any archipelago world.
            // Without towns: the flattest ground nearest the map centre.
            float score = towns.empty() ? -(c.flatness * 1000.0f + Dist2D(c.x, c.z, centroidX, centroidZ))
                                        : -(MinDistToZones(towns, c.x, c.z, nullptr) + c.flatness * 10.0f);
            if (score > bestScore)
            {
                bestScore = score;
                bestIdx = i;
            }
        }
        usedRung = rung;
    }
    if (bestIdx < 0)
    {
        out.error = "no camp spot satisfied the placement rules, even fully relaxed";
        return false;
    }
    if (usedRung > 0)
    {
        out.warnings.push_back(Fmt("camp placed with the town-distance rule relaxed to %.0f m (the island has no dry "
                                   "off-road ground %.0f m clear of its towns)",
                                   opt.campMinCityDist * kCityRelax[usedRung], opt.campMinCityDist));
    }

    const Candidate& camp = candidates[bestIdx];
    {
        ScaffoldZone zone;
        zone.className = "Camp";
        zone.name = "Camp";
        zone.type = "CAMP";
        zone.owner = "RESISTANCE";
        zone.x = camp.x;
        zone.z = camp.z;
        zone.y = camp.y;
        zone.garrison = 0;
        zone.support = 100;
        zone.income = 0;
        zone.note = RString(Fmt("%.0f m to the nearest road, %.0f m to the nearest town, %.1f m of relief across 40 m",
                                haveRoads ? camp.roadDist : 0.0f,
                                towns.empty() ? 0.0f : MinDistToZones(towns, camp.x, camp.z, nullptr), camp.flatness)
                                .c_str());
        out.zones.push_back(zone);
        out.campIndex = 0;
    }

    // --- OUTPOSTS ---------------------------------------------------------
    // Greedy farthest-point over the candidate set, seeded with the camp and
    // the towns, so the occupier's posts end up spread over the island instead
    // of clustered on whichever road happened to sample well.
    std::vector<ScaffoldZone> placed = out.zones;
    placed.insert(placed.end(), towns.begin(), towns.end());
    bool nearTownsOnly = !towns.empty();
    for (int n = 0; n < opt.outposts; n++)
    {
        int pick = -1;
        float pickScore = -1.0f;
        for (int attempt = 0; attempt < 2 && pick < 0; attempt++)
        {
            // First pass keeps the posts inside the inhabited belt; second pass
            // drops that and takes anything legal, so a thinly settled world
            // still gets its posts (reported once, below).
            bool nearTowns = nearTownsOnly && attempt == 0;
            for (size_t i = 0; i < candidates.size(); i++)
            {
                const Candidate& c = candidates[i];
                if (haveRoads && (c.roadDist < opt.outpostMinRoadDist || c.roadDist > opt.outpostMaxRoadDist))
                {
                    continue;
                }
                if (nearTowns && MinDistToZones(towns, c.x, c.z, nullptr) > opt.outpostMaxCityDist)
                {
                    continue;
                }
                float d = MinDistToZones(placed, c.x, c.z, nullptr);
                if (d < opt.zoneMinSpacing)
                {
                    continue;
                }
                if (d > pickScore)
                {
                    pickScore = d;
                    pick = (int)i;
                }
            }
            if (pick < 0 && nearTowns)
            {
                out.warnings.push_back(
                    Fmt("outpost %d placed outside the inhabited belt: nothing within %.0f m of a town was still "
                        "%.0f m clear of every other zone",
                        n + 1, opt.outpostMaxCityDist, opt.zoneMinSpacing));
            }
        }
        if (pick < 0)
        {
            out.warnings.push_back(Fmt("only %d of %d outposts placed: no remaining candidate is %.0f m clear of every "
                                       "other zone and %.0f-%.0f m off a road",
                                       n, opt.outposts, opt.zoneMinSpacing, opt.outpostMinRoadDist,
                                       opt.outpostMaxRoadDist));
            break;
        }
        const Candidate& c = candidates[pick];
        ScaffoldZone zone;
        zone.className = UniqueClassName(out.zones, RString(Fmt("Outpost%d", n + 1).c_str()));
        zone.name = RString(Fmt("Outpost %d", n + 1).c_str());
        zone.type = "OUTPOST";
        zone.owner = "OCCUPIER";
        zone.x = c.x;
        zone.z = c.z;
        zone.y = c.y;
        zone.garrison = 6;
        zone.income = 25;
        zone.note =
            RString(Fmt("%.0f m to the nearest road, %.0f m to the nearest other zone", c.roadDist, pickScore).c_str());
        out.zones.push_back(zone);
        placed.push_back(zone);
        out.outpostCount++;
    }

    // towns last so the marker numbering runs CAMP, OUTPOSTs, CITYs
    for (const ScaffoldZone& t : towns)
    {
        out.zones.push_back(t);
    }
    for (size_t i = 0; i < out.zones.size(); i++)
    {
        out.zones[i].marker = RString(Fmt("gmZoneMarker_%d", (int)i).c_str());
    }
    if (out.zones.size() > (size_t)ZoneRegistry::MaxZones)
    {
        out.warnings.push_back(Fmt("%d zones exceeds the engine cap of %d; the tail will be dropped at mission load",
                                   (int)out.zones.size(), ZoneRegistry::MaxZones));
    }

    // The player starts a few metres off the camp centre: GuerrillaBase claims
    // the centre for the HQ/garage/cache ring, and a body standing exactly there
    // is one more object its free-position nudge has to work around.
    out.playerX = camp.x + 5.0f;
    out.playerZ = camp.z + 5.0f;
    out.playerY = in.height->Height(out.playerX, out.playerZ);
    out.ok = true;
    return true;
}

// ---------------------------------------------------------------------------
// --keep-zones: brace-matched splice of an existing class Zones block
// ---------------------------------------------------------------------------

namespace
{

// Advances past a // comment, a /* */ comment or a "string" starting at i.
// Returns true when it consumed something.
bool SkipNonCode(const std::string& s, size_t& i)
{
    if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/')
    {
        while (i < s.size() && s[i] != '\n')
        {
            i++;
        }
        return true;
    }
    if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '*')
    {
        i += 2;
        while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/'))
        {
            i++;
        }
        i = std::min(s.size(), i + 2);
        return true;
    }
    if (s[i] == '"')
    {
        i++;
        while (i < s.size() && s[i] != '"')
        {
            i++;
        }
        i = std::min(s.size(), i + 1);
        return true;
    }
    return false;
}

bool IsIdentChar(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

} // namespace

bool ExtractZonesBlock(const std::string& text, std::string& block)
{
    block.clear();
    for (size_t i = 0; i < text.size();)
    {
        if (SkipNonCode(text, i))
        {
            continue;
        }
        bool wordStart = (i == 0) || !IsIdentChar(text[i - 1]);
        if (!wordStart || text.compare(i, 5, "class") != 0 || (i + 5 < text.size() && IsIdentChar(text[i + 5])))
        {
            i++;
            continue;
        }
        size_t j = i + 5;
        while (j < text.size() && isspace((unsigned char)text[j]))
        {
            j++;
        }
        size_t identStart = j;
        while (j < text.size() && IsIdentChar(text[j]))
        {
            j++;
        }
        if (text.compare(identStart, j - identStart, "Zones") != 0)
        {
            i += 5;
            continue;
        }
        while (j < text.size() && isspace((unsigned char)text[j]))
        {
            j++;
        }
        if (j >= text.size() || text[j] != '{')
        {
            i += 5;
            continue;
        }
        int depth = 0;
        size_t k = j;
        while (k < text.size())
        {
            if (SkipNonCode(text, k))
            {
                continue;
            }
            if (text[k] == '{')
            {
                depth++;
            }
            else if (text[k] == '}')
            {
                depth--;
                if (depth == 0)
                {
                    k++;
                    // swallow the class terminator so the splice is complete
                    while (k < text.size() && isspace((unsigned char)text[k]))
                    {
                        k++;
                    }
                    if (k < text.size() && text[k] == ';')
                    {
                        k++;
                    }
                    block = text.substr(i, k - i);
                    return true;
                }
            }
            k++;
        }
        return false; // unbalanced
    }
    return false;
}

// ---------------------------------------------------------------------------
// template text
// ---------------------------------------------------------------------------

const char* InitSqsText()
{
    // Byte-for-byte guerrilla-mode/mission/Guerrilla.Abel/init.sqs (issue #54
    // B1): the shared managers live once at <GameDir>\gmcore and every template
    // is a two-line bootstrap into them.
    return "; Guerrilla Mode bootstrap: the shared script core lives at <GameDir>\\gmcore, installed from "
           "guerrilla-mode/core by guerrilla-mode/install-missions.ps1 (the leading backslash resolves against the "
           "game data root, not this mission folder).\n"
           "[] exec \"\\gmcore\\init.sqs\"\n";
}

namespace
{

void AppendZones(std::string& s, const ScaffoldResult& zones)
{
    s += "    class Zones\n    {\n";
    s += "        // COORDINATE ORDER (load-bearing): position[] is authored in\n";
    s += "        // script/getPos order [easting, northing, elevation] - NOT the\n";
    s += "        // mission.sqm order [easting, ELEVATION, northing]. The engine's\n";
    s += "        // ZoneRegistry::LoadZones documents and enforces this contract.\n";
    s += "        // Elevations are real sampled ground heights from the world's .wrp.\n";
    for (const ScaffoldZone& z : zones.zones)
    {
        s += Fmt("        class %s\n        {\n", (const char*)z.className);
        if (z.note.GetLength() > 0)
        {
            s += Fmt("            // %s\n", (const char*)z.note);
        }
        s += Fmt("            name = \"%s\";\n", (const char*)z.name);
        s += Fmt("            type = \"%s\";\n", (const char*)z.type);
        s += Fmt("            owner = \"%s\";\n", (const char*)z.owner);
        if (stricmp((const char*)z.type, "CITY") != 0)
        {
            s += Fmt("            garrison = %d;\n", z.garrison);
            s += Fmt("            income = %d;\n", z.income);
        }
        if (z.support > 0)
        {
            s += Fmt("            support = %d;\n", z.support);
        }
        s += Fmt("            marker = \"%s\";\n", (const char*)z.marker);
        s += Fmt("            position[] = {%.0f, %.0f, %.1f};\n", z.x, z.z, z.y);
        s += "        };\n";
    }
    s += "    };\n";
}

} // namespace

std::string RenderDescriptionExt(const TemplateInfo& info, const ScaffoldResult& zones, const ScaffoldOptions& opt,
                                 const std::string& keepZones)
{
    std::string s;
    s += Fmt("// Guerrilla Mode on %s (CfgWorlds >> %s).\n", (const char*)info.worldDisplay,
             (const char*)info.worldClass);
    s += Fmt("// GENERATED by `%s` on %s - a STARTING POINT, not a finished\n", (const char*)info.toolBanner,
             (const char*)info.date);
    s += "// campaign. Re-running the tool overwrites this file; pass --keep-zones to\n";
    s += "// preserve a hand-edited class Zones block across a re-run.\n";
    s += "//\n";
    s += Fmt("// Zone geometry was derived from %s: %d road piece(s) and %d\n", (const char*)info.wrpPath,
             info.roadPoints, info.buildingPoints);
    s += "// building(s) were identified by model-path heuristics (Guerrilla::ModelIsRoad\n";
    s += "// / ModelIsBuilding), and every elevation below is a real sampled ground\n";
    s += "// height, not a guess. CITY zones are the world's own Names entries that sit\n";
    s += Fmt("// on dry land with at least %d building(s) within %d m - the same test the\n", opt.townMinBuildings,
             (int)opt.townBuildingRadius);
    s += "// running engine applies (ZoneRegistry::LandscapeSettlementProbe).\n";
    s += "//\n";
    s += "// BEFORE YOU SHIP THIS, by hand:\n";
    s += "//   1. defaultOccupier / defaultResistance below are PLACEHOLDERS. Set them to\n";
    s += "//      CfgGuerrillaFactions class names your data set actually ships (the global\n";
    s += "//      library is guerrilla-mode/config/guerrilla-factions.hpp; a mod may add\n";
    s += "//      its own). playerSide must be the side the mission.sqm player is welded\n";
    s += "//      to - the engine counts resistance presence through that side's center.\n";
    s += "//   2. Walk the Camp and the Outposts in the editor. The placement rules know\n";
    s += "//      about roads, water and slope; they know nothing about fences, compounds\n";
    s += "//      or minefields, so a spot can be legal and still be walled in.\n";
    s += "//   3. Rename the Outposts to real places and tune garrison / income.\n";
    s += "//   4. Add a CfgGuerrillaMarket block if this island should have dealers.\n";
    s += "\n";
    s += Fmt("onLoadMission = \"Guerrilla Mode - %s\";\n", (const char*)info.worldDisplay);
    s += Fmt("onLoadIntro  = \"Liberate %s, one zone at a time.\";\n", (const char*)info.worldDisplay);
    s += "respawn = 0;\n";
    s += "joinInProgress = 0;\n";
    s += "\n";
    s += "class CfgGuerrillaZones\n{\n";
    s += "    // PLACEHOLDERS - see note 1 in the header. These are the direct-launch\n";
    s += "    // fallbacks used when the GUERRILLA new-game UI published no selection.\n";
    s += "    defaultOccupier = \"EAST\";\n";
    s += "    defaultResistance = \"GUER\";\n";
    s += Fmt("    playerSide = \"%s\";\n", (const char*)info.playerSide);
    s += "    // seedCities is deliberately ABSENT (= Auto): the mission-time settlement\n";
    s += "    // classifier runs over the loaded landscape and adds any town this offline\n";
    s += "    // pass missed, deduped against the explicit CITY zones below at 300 m.\n";
    s += Fmt("    seedCitySupport = %d;\n", opt.seedCitySupport);
    s += "\n";
    if (!keepZones.empty())
    {
        s += "    // KEPT VERBATIM from the previous description.ext (--keep-zones): the\n";
        s += "    // generated block was discarded, so hand edits below are authoritative.\n";
        s += "    " + keepZones + "\n";
    }
    else
    {
        AppendZones(s, zones);
    }
    s += "};\n\n";

    s += "// The ISLAND half of the faction table (issue #54 A4): only CIV lives here.\n";
    s += "// The war rosters are NOT island facts and belong in the global library\n";
    s += "// (guerrilla-mode/config/guerrilla-factions.hpp for vanilla, a mod-level\n";
    s += "// config for a mod's own factions); the engine unions the two tables\n";
    s += "// (Game/Guerrilla/FactionSources.*), this file winning on a name clash.\n";
    s += "class CfgGuerrillaFactions\n{\n";
    s += "    class CIV // civilian population descriptor (issue #8)\n    {\n";
    s += "        // side MUST be exactly \"CIV\": GM_fnSideFromString matches the engine's\n";
    s += "        // rendered side name, and the civilian side renders as \"CIV\".\n";
    s += "        side = \"CIV\";\n";
    s += "        // The probe only knows the stock Civilian* / car classnames. If your\n";
    s += "        // data set ships its own civilians (a mod's LoBo_Civ_* and vans, say),\n";
    s += "        // put them here instead: nothing in a config marks a class as\n";
    s += "        // \"civilian population\" or \"traffic hull\", so this cannot be derived.\n";
    if (info.civClasses.empty())
    {
        s += "        // No Civilian* class resolved in this package. An empty civClassCount\n";
        s += "        // is a valid answer: the managers read it as \"no CIV layer\" and exit.\n";
        s += "        civClassCount = 0;\n";
    }
    else
    {
        s += "        // Numbered keys + a count, NOT an array: gmFactionValue returns raw\n";
        s += "        // config text and skips array entries, so the scripts read\n";
        s += "        // civClassCount and then civClass1..civClassN. Every class below was\n";
        s += "        // probed against the mounted package at scaffold time.\n";
        s += Fmt("        civClassCount = %d;\n", (int)info.civClasses.size());
        for (size_t i = 0; i < info.civClasses.size(); i++)
        {
            s += Fmt("        civClass%d = \"%s\";\n", (int)i + 1, (const char*)info.civClasses[i]);
        }
    }
    s += "        // Ambient road-traffic hulls (native Traffic service). A real array:\n";
    s += "        // unresolvable hulls are dropped at load, an empty result just leaves\n";
    s += "        // civilian traffic inert.\n";
    if (info.civVehicles.empty())
    {
        s += "        civVehicles[] = {};\n";
    }
    else
    {
        s += "        civVehicles[] = {";
        for (size_t i = 0; i < info.civVehicles.size(); i++)
        {
            s += Fmt("%s\"%s\"", i ? ", " : "", (const char*)info.civVehicles[i]);
        }
        s += "};\n";
    }
    s += "    };\n};\n";
    return s;
}

std::string RenderMissionSqm(const TemplateInfo& info, const ScaffoldResult& zones)
{
    std::string s;
    s += "version=11;\n";
    s += "class Mission\n{\n";
    if (!info.addOns.empty())
    {
        s += "\t// The world and the player's own pbo, and nothing else: since issue #54\n";
        s += "\t// C1 the engine derives the TRANSITIVE addon closure itself (the placed\n";
        s += "\t// player's weapons/magazines owners at launch, everything the faction\n";
        s += "\t// descriptors name at registry load). Base-game classes carry an empty\n";
        s += "\t// owner and are always visible, so they never appear here.\n";
        s += "\taddOns[]=\n\t{\n";
        for (size_t i = 0; i < info.addOns.size(); i++)
        {
            s += Fmt("\t\t\"%s\"%s\n", (const char*)info.addOns[i], i + 1 < info.addOns.size() ? "," : "");
        }
        s += "\t};\n\taddOnsAuto[]=\n\t{\n";
        for (size_t i = 0; i < info.addOns.size(); i++)
        {
            s += Fmt("\t\t\"%s\"%s\n", (const char*)info.addOns[i], i + 1 < info.addOns.size() ? "," : "");
        }
        s += "\t};\n";
    }
    s += Fmt("\trandomSeed=%d;\n", info.randomSeed);
    s += "\tclass Intel\n\t{\n\t};\n";
    s += "\tclass Groups\n\t{\n\t\titems=1;\n\t\tclass Item0\n\t\t{\n";
    s += Fmt("\t\t\tside=\"%s\";\n", (const char*)info.playerSide);
    s += "\t\t\tclass Vehicles\n\t\t\t{\n\t\t\t\titems=1;\n\t\t\t\tclass Item0\n\t\t\t\t{\n";
    s += "\t\t\t\t\t// Player: the lone guerrilla at the Camp zone. SQM coordinate\n";
    s += "\t\t\t\t\t// order is [easting, ELEVATION, northing] - the zone config's\n";
    s += "\t\t\t\t\t// position[] is getPos order [easting, northing, elevation].\n";
    s += "\t\t\t\t\t// Offset a few metres off the zone centre so the body is not\n";
    s += "\t\t\t\t\t// standing in the spot GuerrillaBase wants for the HQ.\n";
    s += Fmt("\t\t\t\t\tposition[]={%.6f,%.6f,%.6f};\n", zones.playerX, zones.playerY, zones.playerZ);
    s += "\t\t\t\t\tid=0;\n";
    s += Fmt("\t\t\t\t\tside=\"%s\";\n", (const char*)info.playerSide);
    s += Fmt("\t\t\t\t\tvehicle=\"%s\";\n", (const char*)info.playerClass);
    s += "\t\t\t\t\tplayer=\"PLAYER COMMANDER\";\n";
    s += "\t\t\t\t\tleader=1;\n";
    s += "\t\t\t\t\tskill=0.600000;\n";
    s += "\t\t\t\t};\n\t\t\t};\n\t\t};\n\t};\n};\n";
    s += Fmt("class Intro\n{\n\trandomSeed=%d;\n\tclass Intel\n\t{\n\t};\n};\n", info.randomSeed + 1);
    s += Fmt("class OutroWin\n{\n\trandomSeed=%d;\n\tclass Intel\n\t{\n\t};\n};\n", info.randomSeed + 2);
    s += Fmt("class OutroLoose\n{\n\trandomSeed=%d;\n\tclass Intel\n\t{\n\t};\n};\n", info.randomSeed + 3);
    return s;
}

} // namespace Poseidon::Guerrilla
