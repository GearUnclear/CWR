#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace Poseidon::Guerrilla
{

// ---------------------------------------------------------------------------
// GROUND TRUTH for the vocabulary below: `PoseidonTools terrain objects` over
// the Remaster's abel.wrp and eden.wrp (2026-10-14).
//
//   Harbours.  Malden's Le Port quay is data3d molo_beton (a concrete pier) and
//   molo_krychle (a concrete quay block) side by side at [8476, 3754]; Everon
//   has the same pair at [7392, 6835].  Every other molo on either island is a
//   wooden fishing jetty (molo_drevo, molo_drevo_bs, molo_drevo_end), alone or
//   in pairs on open coast.  So: concrete pier, quay block, crane and dock
//   structures weigh 1, a wooden jetty, mooring or boat 0.5, and a seaport
//   needs at least one structure of weight 1 to be seeded.  Two jetties make a
//   fishing landing, not a harbour.
//
//   Military.  OFP's base buildings are budova1..budova5 (barracks and
//   offices), hlaska (watchtower), strazni_vez (guard tower), fortress1/2,
//   garaz_s_tankem / garaz_bez_tanku (vehicle sheds), hangar, repair_center
//   and fuelstation_army.  Malden's Southeast Compound is 4 garages, 4
//   hlaska and a strazni_vez inside 160 x 150 m; Everon's three garrisoned
//   bases are budova clusters with a hlaska each.  The plain garaz (a village
//   garage) and vez (a church tower) are deliberately NOT military.
//
//   Mods.  @LoBo ships its harbour set as AGS_port (directory ags_port) and
//   its barracks as Baracken; both directory names are accepted.
//
// LIMITS: like ModelIsBuilding this is a name guess.  A mod pier named
// something unrelated in a directory that says nothing is invisible, and the
// water-edge geometry test only ever ADDS to a cluster of named objects
// (a coastal town's waterfront houses must never become a harbour on their
// own).
// ---------------------------------------------------------------------------

namespace
{

std::string LowerPath(const char* modelPath)
{
    std::string s(modelPath ? modelPath : "");
    for (char& c : s)
    {
        c = c == '/' ? '\\' : (char)tolower((unsigned char)c);
    }
    return s;
}

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

bool Has(const std::string& s, const char* token)
{
    return s.find(token) != std::string::npos;
}

bool Starts(const std::string& s, const char* prefix)
{
    size_t n = strlen(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}

bool HasAny(const std::string& s, const char* const* tokens, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (Has(s, tokens[i]))
        {
            return true;
        }
    }
    return false;
}

float Dist(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return sqrtf(dx * dx + dz * dz);
}

std::string Fmt(const char* fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return std::string(buf);
}

bool IsMilitaryType(const char* type)
{
    return stricmp(type, "AIRFIELD") == 0 || stricmp(type, "SEAPORT") == 0 || stricmp(type, "OUTPOST") == 0;
}

// the garrison's spawn square fits on dry ground here
bool SpotIsDry(const HeightField& h, const BaseDetectOptions& opt, float x, float z)
{
    if (h.Height(x, z) < opt.minDryHeight)
    {
        return false;
    }
    const float s = opt.spawnHalfSize;
    const float corners[4][2] = {{-s, -s}, {s, -s}, {-s, s}, {s, s}};
    for (const auto& c : corners)
    {
        if (h.Height(x + c[0], z + c[1]) < opt.minCornerHeight)
        {
            return false;
        }
    }
    return true;
}

// The raw centre when it is dry, else the first dry spot on rings of 20 m
// steps out to maxInlandWalk, 16 bearings each starting due north and turning
// clockwise.  No randomness: the same input always lands on the same spot, so
// a reload (the runtime seed is config-rebuilt) reproduces the zone.
bool FindDrySpot(const HeightField& h, const BaseDetectOptions& opt, float x, float z, float& outX, float& outZ)
{
    if (SpotIsDry(h, opt, x, z))
    {
        outX = x;
        outZ = z;
        return true;
    }
    const float kPi = 3.14159265f;
    for (float r = 20.0f; r <= opt.maxInlandWalk + 0.5f; r += 20.0f)
    {
        for (int b = 0; b < 16; b++)
        {
            float a = (float)b * (2.0f * kPi / 16.0f);
            float px = x + r * sinf(a);
            float pz = z + r * cosf(a);
            if (SpotIsDry(h, opt, px, pz))
            {
                outX = px;
                outZ = pz;
                return true;
            }
        }
    }
    return false;
}

// water (ground below sea level) at the point or on 8 bearings at 10 / 20 /
// 30 m (up to `reach`)
bool WaterNear(const HeightField& h, float x, float z, float reach)
{
    if (h.Height(x, z) < 0.0f)
    {
        return true;
    }
    const float kPi = 3.14159265f;
    for (float r = 10.0f; r <= reach + 0.5f; r += 10.0f)
    {
        for (int b = 0; b < 8; b++)
        {
            float a = (float)b * (kPi / 4.0f);
            if (h.Height(x + r * sinf(a), z + r * cosf(a)) < 0.0f)
            {
                return true;
            }
        }
    }
    return false;
}

// a building whose footprint (its bounding radius, 8 m when unknown) touches
// water: the centre or one of 4 points on the rim is below sea level
bool FootprintTouchesWater(const HeightField& h, const DetectObject& o)
{
    float r = o.radius > 0 ? o.radius : 8.0f;
    if (h.Height(o.x, o.z) < 0.0f)
    {
        return true;
    }
    return h.Height(o.x + r, o.z) < 0.0f || h.Height(o.x - r, o.z) < 0.0f || h.Height(o.x, o.z + r) < 0.0f ||
           h.Height(o.x, o.z - r) < 0.0f;
}

// single-link clusters over the listed object indices: two objects within
// linkDist of each other share a cluster.  Clusters come back in the order of
// their first member, members in index order.
std::vector<std::vector<int>> Cluster(const std::vector<DetectObject>& objs, const std::vector<int>& idx,
                                      float linkDist)
{
    std::vector<int> label(idx.size(), -1);
    std::vector<std::vector<int>> out;
    for (size_t i = 0; i < idx.size(); i++)
    {
        if (label[i] >= 0)
        {
            continue;
        }
        int id = (int)out.size();
        out.emplace_back();
        std::vector<size_t> stack{i};
        label[i] = id;
        while (!stack.empty())
        {
            size_t a = stack.back();
            stack.pop_back();
            out[id].push_back(idx[a]);
            for (size_t b = 0; b < idx.size(); b++)
            {
                if (label[b] < 0 &&
                    Dist(objs[idx[a]].x, objs[idx[a]].z, objs[idx[b]].x, objs[idx[b]].z) <= linkDist)
                {
                    label[b] = id;
                    stack.push_back(b);
                }
            }
        }
        std::sort(out[id].begin(), out[id].end());
    }
    return out;
}

void AddObjectEvidence(BaseCandidate& c, const std::vector<DetectObject>& objs, const std::vector<int>& members,
                       const char* prefix)
{
    const size_t kMax = 10;
    for (size_t i = 0; i < members.size() && i < kMax; i++)
    {
        const DetectObject& o = objs[members[i]];
        c.evidence.push_back(Fmt("%s%s at [%.0f, %.0f]", prefix, (const char*)o.model, o.x, o.z));
    }
    if (members.size() > kMax)
    {
        c.evidence.push_back(Fmt("%s... and %d more", prefix, (int)(members.size() - kMax)));
    }
}

void ReadPairs(const ParamEntry* arr, std::vector<ScaffoldPoint>& out)
{
    out.clear();
    if (!arr || !arr->IsArray())
    {
        return;
    }
    for (int i = 0; i + 1 < arr->GetSize(); i += 2)
    {
        ScaffoldPoint p;
        p.x = (float)(*arr)[i];
        p.z = (float)(*arr)[i + 1];
        out.push_back(p);
    }
}

bool ReadIlsClass(const ParamEntry& cls, const std::string& source, IlsEntry& e)
{
    if (!cls.IsClass())
    {
        return false;
    }
    const ParamEntry* pos = cls.FindEntry("ilsPosition");
    if (!pos || !pos->IsArray() || pos->GetSize() < 2)
    {
        return false;
    }
    e = IlsEntry();
    e.x = (float)(*pos)[0];
    e.z = (float)(*pos)[1];
    const ParamEntry* dir = cls.FindEntry("ilsDirection");
    if (dir && dir->IsArray() && dir->GetSize() >= 3)
    {
        e.dirX = (float)(*dir)[0];
        e.dirZ = (float)(*dir)[2];
    }
    ReadPairs(cls.FindEntry("ilsTaxiIn"), e.taxiIn);
    ReadPairs(cls.FindEntry("ilsTaxiOff"), e.taxiOff);
    e.source = RString(source.c_str());
    const ParamClass* pc = cls.GetClassInterface();
    e.inherited = pc && !pc->FindEntryNoInheritance("ilsPosition");
    return true;
}

bool InRegion(const BaseDetectInputs& in, float x, float z)
{
    return !in.hasRegion ||
           (x >= in.regionMinX && x <= in.regionMaxX && z >= in.regionMinZ && z <= in.regionMaxZ);
}

bool NameTaken(const RString& name, const BaseDetectInputs& in, const std::vector<BaseCandidate>& done,
               bool checkAuthored, const char* adoptedLabel)
{
    for (const BaseCandidate& c : done)
    {
        if (c.name.GetLength() > 0 && stricmp(c.name, name) == 0)
        {
            return true;
        }
    }
    // a Names town of the same name would be dropped by the city seed's
    // name dedup, so a base never takes a town's name either
    for (const DetectPlace& t : in.towns)
    {
        if (stricmp(t.name, name) == 0 && !(adoptedLabel && stricmp(adoptedLabel, name) == 0))
        {
            return true;
        }
    }
    if (checkAuthored)
    {
        for (const DetectAuthoredZone& a : in.authored)
        {
            if (stricmp(a.name, name) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

// adoptedLabel: the Names label an airfield took its name from; that one
// town name is not a clash (the airfield replaces the label)
RString UniqueName(const std::string& rawStem, const BaseDetectInputs& in, const std::vector<BaseCandidate>& done,
                   bool checkAuthored, const char* adoptedLabel = nullptr)
{
    // Names labels carry stray spaces ("Ras Sudr "): one space between words
    std::string stem;
    for (char ch : rawStem)
    {
        if (ch == ' ' && (stem.empty() || stem.back() == ' '))
        {
            continue;
        }
        stem += ch;
    }
    while (!stem.empty() && stem.back() == ' ')
    {
        stem.pop_back();
    }
    RString name(stem.c_str());
    for (int n = 2; NameTaken(name, in, done, checkAuthored, adoptedLabel) && n < 100; n++)
    {
        name = RString(Fmt("%s %d", stem.c_str(), n).c_str());
    }
    return name;
}

bool LooksLikeAirfieldLabel(const char* name)
{
    std::string s = LowerPath(name);
    static const char* kTokens[] = {"airbase", "airfield", "airport", "airstrip", "air base", "air field",
                                    "letiste", "aerodrom", "runway", "flugplatz"};
    return HasAny(s, kTokens, (int)(sizeof(kTokens) / sizeof(*kTokens)));
}

// A Names entry a base may be named after: a settlement by the engine's own
// rule (LandscapeSettlementProbe: dry, at least 3 objects of the input set,
// which are house-sized buildings plus the classified ones, within 300 m) and
// not an airfield label.  A bare label (an island in the sea, a border, an
// airbase) names nothing: "Tiran Is Harbour" was the Sharm port.
bool NamesSettlement(const BaseDetectInputs& in, const DetectPlace& t)
{
    if (LooksLikeAirfieldLabel(t.name) || in.height->Height(t.x, t.z) < 0.1f)
    {
        return false;
    }
    int n = 0;
    for (const DetectObject& o : in.objects)
    {
        if (Dist(o.x, o.z, t.x, t.z) <= 300.0f && ++n >= 3)
        {
            return true;
        }
    }
    return false;
}

// the nearest Names SETTLEMENT within maxDist (see NamesSettlement)
const DetectPlace* NearestTown(const BaseDetectInputs& in, float x, float z, float maxDist)
{
    const DetectPlace* best = nullptr;
    float bestD = maxDist;
    for (const DetectPlace& t : in.towns)
    {
        if (!NamesSettlement(in, t))
        {
            continue;
        }
        float d = Dist(x, z, t.x, t.z);
        if (d <= bestD)
        {
            bestD = d;
            best = &t;
        }
    }
    return best;
}

} // namespace

// ---------------------------------------------------------------------------
// classification
// ---------------------------------------------------------------------------

BaseModelClass ClassifyBaseModel(const char* modelPath)
{
    BaseModelClass c;
    std::string path = LowerPath(modelPath);
    if (path.empty())
    {
        return c;
    }
    std::string base = BaseName(path);
    std::string dir = DirName(path);
    // road signs name places ("znacka_spierre" is a St. Pierre sign, not a
    // pier), and fences and walls stand in every compound
    static const char* kNever[] = {"znacka", "plot", "pletivo", "zed_", "fence", "wall", "sign"};
    if (HasAny(base, kNever, (int)(sizeof(kNever) / sizeof(*kNever))))
    {
        return c;
    }

    // harbour: weight 1 for a structure a harbour is built of
    static const char* kHarbourStrong[] = {"molo_beton", "molo_krychle", "crane",   "jerab", "dock",
                                           "quay",       "harbour",      "harbor",  "pristav", "wharf"};
    // weight 0.5: a jetty, a quay wall (nabrezi also lines stock rivers), a
    // mooring, a boat
    static const char* kHarbourWeak[] = {"molo", "pier", "jetty", "nabrezi", "kotva", "mooring", "lodka", "boat"};
    const bool portSet = Has(dir, "ags_port\\") || Has(dir, "\\port\\") || Has(dir, "harbour\\") ||
                         Has(dir, "harbor\\");
    if (Has(base, "jerabina"))
    {
        // "str jerabina" is a rowan tree, not a crane (jerab)
    }
    else if (portSet && (Has(base, "signal") || Has(base, "buoy") || Has(base, "bojka")))
    {
        // AGS_port's channel buoys mark the open gulfs, not a harbour
    }
    else if (HasAny(base, kHarbourStrong, (int)(sizeof(kHarbourStrong) / sizeof(*kHarbourStrong))))
    {
        c.harbour = 1.0f;
    }
    else if (portSet && Has(base, "crate"))
    {
        c.harbour = 0.25f; // a port's crates back a quay, they never make one
    }
    else if (portSet)
    {
        // a mod's harbour set (AGS_port in @LoBo: piers, prefabs, breakwater blocks)
        c.harbour = 1.0f;
    }
    else if (HasAny(base, kHarbourWeak, (int)(sizeof(kHarbourWeak) / sizeof(*kHarbourWeak))) || base == "lod" ||
             Starts(base, "lod_"))
    {
        c.harbour = 0.5f;
    }

    // aviation: hangars and aircraft shelters (RKSL spells it "hanger"),
    // control towers at half weight
    static const char* kAviation[] = {"hangar", "hanger", "hasnato", "lobo_has", "aircraft_shelter", "shelter_air"};
    if (HasAny(base, kAviation, (int)(sizeof(kAviation) / sizeof(*kAviation))))
    {
        c.aviation = 1.0f;
    }
    else if (Has(base, "start_turm") || Has(base, "control_tower") || Has(base, "controltower") ||
             Has(base, "airport_tower"))
    {
        c.aviation = 0.5f;
    }

    // military
    static const char* kMilitary[] = {"fortress",        "bunker",       "bunkr",        "hlaska",   "strazni_vez",
                                      "watchtower",      "viewtower",    "view_tower",   "guardtower", "barrack",
                                      "baracke",         "kasarn",       "garaz_s_tank", "garaz_bez_tank",
                                      "repair_center",   "fuelstation_army", "military", "milbase", "hlidac_budka"};
    if (c.aviation > 0 || HasAny(base, kMilitary, (int)(sizeof(kMilitary) / sizeof(*kMilitary))) ||
        Starts(base, "mil_") || (Starts(base, "budova") && base.size() > 6 && isdigit((unsigned char)base[6])) ||
        Has(dir, "baracken\\") || Has(dir, "milobj"))
    {
        c.military = 1.0f;
    }
    return c;
}

// ---------------------------------------------------------------------------
// ILS
// ---------------------------------------------------------------------------

int ReadIlsEntries(const ParamEntry& worldEntry, std::vector<IlsEntry>& out)
{
    int read = 0;
    std::string world = std::string("CfgWorlds >> ") + (const char*)worldEntry.GetName();
    IlsEntry e;
    if (ReadIlsClass(worldEntry, world, e))
    {
        out.push_back(e);
        read++;
    }
    const ParamEntry* secondary = worldEntry.IsClass() ? worldEntry.FindEntry("SecondaryAirports") : nullptr;
    if (secondary && secondary->IsClass())
    {
        for (int i = 0; i < secondary->GetEntryCount(); i++)
        {
            const ParamEntry& s = secondary->GetEntry(i);
            if (ReadIlsClass(s, world + " >> SecondaryAirports >> " + (const char*)s.GetName(), e))
            {
                // a secondary airport's keys are its own: inheritance inside
                // SecondaryAirports is from a sibling strip, not a base world
                e.inherited = false;
                out.push_back(e);
                read++;
            }
        }
    }
    return read;
}

const char* BaseStatusName(BaseStatus s)
{
    switch (s)
    {
    case BaseStatus::Seed:
        return "seed";
    case BaseStatus::Offline:
        return "offline";
    case BaseStatus::Covered:
        return "covered";
    case BaseStatus::OutsideRegion:
        return "outside-region";
    case BaseStatus::LowConfidence:
        return "low-confidence";
    }
    return "?";
}

RString BaseWorldName(const char* worldClass, const char* worldDisplay)
{
    if (worldDisplay && *worldDisplay && *worldDisplay != '$')
    {
        return RString(worldDisplay);
    }
    return RString(worldClass ? worldClass : "");
}

// ---------------------------------------------------------------------------
// detection
// ---------------------------------------------------------------------------

void DetectBases(const BaseDetectInputs& in, const BaseDetectOptions& opt, BaseDetectResult& out)
{
    out = BaseDetectResult();
    if (!in.height)
    {
        out.notes.push_back("no height sampler: nothing detected");
        return;
    }
    const HeightField& h = *in.height;
    std::vector<BaseCandidate> airfields, seaports, outposts;

    // ---- AIRFIELD: one per ILS entry ----
    for (const IlsEntry& e : in.ils)
    {
        std::string src = (const char*)e.source;
        if (fabsf(e.x) < 1.0f && fabsf(e.z) < 1.0f)
        {
            out.notes.push_back(Fmt("%s: ilsPosition is the map origin, no airfield", src.c_str()));
            continue;
        }
        if (e.x < 0 || e.z < 0 || e.x > in.worldSize || e.z > in.worldSize)
        {
            out.notes.push_back(Fmt("%s: ilsPosition [%.0f, %.0f] is outside the %.0f m map, no airfield",
                                    src.c_str(), e.x, e.z, in.worldSize));
            continue;
        }
        // the runway: the touchdown point plus the taxi-off path, which in
        // every stock world runs the length of the strip; without one, the
        // strip is taken as 860 m (Malden's and Everon's) away from the
        // approach direction
        std::vector<ScaffoldPoint> pts;
        pts.push_back(ScaffoldPoint{e.x, e.z});
        float endX = e.x, endZ = e.z, far = 0;
        for (const ScaffoldPoint& p : e.taxiOff)
        {
            float d = Dist(e.x, e.z, p.x, p.z);
            if (d > far)
            {
                far = d;
                endX = p.x;
                endZ = p.z;
            }
        }
        if (far < 1.0f)
        {
            float len = sqrtf(e.dirX * e.dirX + e.dirZ * e.dirZ);
            if (len > 0.01f)
            {
                endX = e.x - e.dirX / len * 860.0f;
                endZ = e.z - e.dirZ / len * 860.0f;
                far = 860.0f;
            }
        }
        pts.push_back(ScaffoldPoint{endX, endZ});
        pts.insert(pts.end(), e.taxiOff.begin(), e.taxiOff.end());
        pts.insert(pts.end(), e.taxiIn.begin(), e.taxiIn.end());
        float midRunX = (e.x + endX) * 0.5f;
        float midRunZ = (e.z + endZ) * 0.5f;
        if (h.Height(e.x, e.z) < 0.5f || h.Height(endX, endZ) < 0.5f || h.Height(midRunX, midRunZ) < 0.5f)
        {
            out.notes.push_back(Fmt("%s: the runway [%.0f, %.0f]..[%.0f, %.0f] is not on dry land, no airfield",
                                    src.c_str(), e.x, e.z, endX, endZ));
            continue;
        }
        float minX = pts[0].x, maxX = pts[0].x, minZ = pts[0].z, maxZ = pts[0].z;
        for (const ScaffoldPoint& p : pts)
        {
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minZ = std::min(minZ, p.z);
            maxZ = std::max(maxZ, p.z);
        }
        float midX = (minX + maxX) * 0.5f;
        float midZ = (minZ + maxZ) * 0.5f;
        BaseCandidate c;
        c.type = "AIRFIELD";
        c.radius = opt.coverAnyDist;
        for (const ScaffoldPoint& p : pts)
        {
            c.radius = std::max(c.radius, Dist(midX, midZ, p.x, p.z));
        }
        // centre: the apron (where ilsTaxiIn starts: the parking end of the
        // taxiway) when it is dry, else the footprint's midpoint, else the
        // nearest dry ground to it
        bool placed = false;
        const char* where = "";
        if (!e.taxiIn.empty() && SpotIsDry(h, opt, e.taxiIn[0].x, e.taxiIn[0].z))
        {
            c.x = e.taxiIn[0].x;
            c.z = e.taxiIn[0].z;
            placed = true;
            where = "the apron (first ilsTaxiIn point)";
        }
        else if (FindDrySpot(h, opt, midX, midZ, c.x, c.z))
        {
            placed = true;
            where = "the runway footprint's midpoint";
        }
        if (!placed)
        {
            out.notes.push_back(Fmt("%s: no dry ground for the zone centre near [%.0f, %.0f], no airfield",
                                    src.c_str(), midX, midZ));
            continue;
        }
        c.y = h.Height(c.x, c.z);
        c.garrison = opt.airfieldGarrison;
        c.income = opt.income;
        bool taxi = e.taxiIn.size() + e.taxiOff.size() >= 2;
        c.confidence = e.inherited ? 0.3f : (taxi ? 0.9f : 0.7f);
        c.evidence.push_back(Fmt("ILS %s: ilsPosition [%.0f, %.0f], ilsTaxiIn %d point(s), ilsTaxiOff %d point(s)",
                                 src.c_str(), e.x, e.z, (int)e.taxiIn.size(), (int)e.taxiOff.size()));
        c.evidence.push_back(Fmt("runway [%.0f, %.0f]..[%.0f, %.0f], %.0f m; centre on %s", e.x, e.z, endX, endZ,
                                 far, where));
        if (e.inherited)
        {
            c.evidence.push_back("ilsPosition is inherited from a base class, not this world's own entry");
        }
        int mil = 0;
        for (const DetectObject& o : in.objects)
        {
            if (ClassifyBaseModel(o.model).military > 0 && Dist(midX, midZ, o.x, o.z) <= c.radius)
            {
                mil++;
            }
        }
        if (mil > 0)
        {
            c.evidence.push_back(Fmt("%d military object(s) inside the footprint", mil));
        }
        bool dup = false;
        for (const BaseCandidate& a : airfields)
        {
            dup = dup || Dist(a.x, a.z, c.x, c.z) < opt.airstripMergeDist;
        }
        if (dup)
        {
            out.notes.push_back(Fmt("%s: within %.0f m of an airfield already found, merged", src.c_str(),
                                    opt.airstripMergeDist));
            continue;
        }
        airfields.push_back(c);
    }
    const size_t ilsAirfields = airfields.size();

    // ---- AIRFIELD from the map's objects: hangar and shelter clusters ----
    std::vector<int> avIdx;
    for (int i = 0; i < (int)in.objects.size(); i++)
    {
        if (ClassifyBaseModel(in.objects[i].model).aviation > 0)
        {
            avIdx.push_back(i);
        }
    }
    std::vector<BaseCandidate> strips;
    for (const std::vector<int>& members : Cluster(in.objects, avIdx, opt.airstripClusterDist))
    {
        int hangars = 0;
        float cx = 0, cz = 0;
        for (int m : members)
        {
            hangars += ClassifyBaseModel(in.objects[m].model).aviation >= 1.0f ? 1 : 0;
            cx += in.objects[m].x;
            cz += in.objects[m].z;
        }
        cx /= (float)members.size();
        cz /= (float)members.size();
        if ((int)members.size() < opt.airstripMinObjects || hangars < 2)
        {
            continue;
        }
        bool merged = false;
        for (const BaseCandidate& a : airfields)
        {
            merged = merged || Dist(a.x, a.z, cx, cz) < std::max(opt.airstripMergeDist, a.radius);
        }
        if (merged)
        {
            out.notes.push_back(Fmt("%d aviation object(s) near [%.0f, %.0f] belong to an airfield already found",
                                    (int)members.size(), cx, cz));
            continue;
        }
        BaseCandidate c;
        c.type = "AIRFIELD";
        c.radius = opt.coverAnyDist;
        for (int m : members)
        {
            c.radius = std::max(c.radius, Dist(cx, cz, in.objects[m].x, in.objects[m].z) + 50.0f);
        }
        if (!FindDrySpot(h, opt, cx, cz, c.x, c.z))
        {
            out.notes.push_back(Fmt("aviation cluster near [%.0f, %.0f]: no dry ground, no airfield", cx, cz));
            continue;
        }
        c.y = h.Height(c.x, c.z);
        c.garrison = opt.airfieldGarrison;
        c.income = opt.income;
        c.confidence = std::min(0.85f, 0.5f + 0.05f * (float)members.size());
        c.evidence.push_back(Fmt("%d aviation object(s), %d of them hangars or shelters, centre [%.0f, %.0f]; no ILS "
                                 "entry: an airstrip read from the map's objects",
                                 (int)members.size(), hangars, cx, cz));
        AddObjectEvidence(c, in.objects, members, "");
        strips.push_back(c);
    }
    // strongest first, then position: the runtime and the tool see the
    // objects in different orders, the names must not depend on it
    std::stable_sort(strips.begin(), strips.end(), [](const BaseCandidate& a, const BaseCandidate& b) {
        if (a.confidence != b.confidence)
        {
            return a.confidence > b.confidence;
        }
        return a.x != b.x ? a.x < b.x : a.z < b.z;
    });
    // one base, several clusters: an airbase spread over a kilometre and more
    // (@LoBo's Egyptian Airbase: hangar rows 1 km apart) is ONE airfield.  A
    // weaker strip within airstripMergeDist of a stronger one folds into it,
    // and the stronger one's footprint grows to cover it.
    std::vector<BaseCandidate> keptStrips;
    std::vector<std::vector<size_t>> groups; // per kept strip: the strips folded into it (itself first)
    for (size_t si = 0; si < strips.size(); si++)
    {
        const BaseCandidate& st = strips[si];
        bool folded = false;
        for (size_t k = 0; k < keptStrips.size() && !folded; k++)
        {
            float d = Dist(keptStrips[k].x, keptStrips[k].z, st.x, st.z);
            if (d < opt.airstripMergeDist)
            {
                keptStrips[k].evidence.push_back(
                    Fmt("merged: a second aviation cluster %.0f m away at [%.0f, %.0f]", d, st.x, st.z));
                groups[k].push_back(si);
                folded = true;
            }
        }
        if (!folded)
        {
            keptStrips.push_back(st);
            groups.push_back({si});
        }
    }
    // a merged airbase centres on the middle of its clusters (the base, not
    // its biggest hangar row), its footprint covering every one of them
    for (size_t k = 0; k < keptStrips.size(); k++)
    {
        if (groups[k].size() < 2)
        {
            continue;
        }
        float minX = 1e30f, maxX = -1e30f, minZ = 1e30f, maxZ = -1e30f;
        for (size_t si : groups[k])
        {
            minX = std::min(minX, strips[si].x);
            maxX = std::max(maxX, strips[si].x);
            minZ = std::min(minZ, strips[si].z);
            maxZ = std::max(maxZ, strips[si].z);
        }
        BaseCandidate& kc = keptStrips[k];
        float nx, nz;
        if (FindDrySpot(h, opt, (minX + maxX) * 0.5f, (minZ + maxZ) * 0.5f, nx, nz))
        {
            kc.x = nx;
            kc.z = nz;
            kc.y = h.Height(nx, nz);
        }
        for (size_t si : groups[k])
        {
            kc.radius = std::max(kc.radius, Dist(kc.x, kc.z, strips[si].x, strips[si].z) + strips[si].radius);
        }
        kc.evidence.push_back(Fmt("centre on the middle of the %d merged clusters", (int)groups[k].size()));
    }
    airfields.insert(airfields.end(), keptStrips.begin(), keptStrips.end());

    // ---- SEAPORT: clusters of harbour structures on the water ----
    std::vector<int> harbourIdx;
    for (int i = 0; i < (int)in.objects.size(); i++)
    {
        if (ClassifyBaseModel(in.objects[i].model).harbour > 0)
        {
            harbourIdx.push_back(i);
        }
    }
    for (const std::vector<int>& members : Cluster(in.objects, harbourIdx, opt.seaportClusterDist))
    {
        float score = 0, strong = 0, wx = 0, wz = 0, wsum = 0;
        bool coastal = false;
        for (int i : members)
        {
            const DetectObject& o = in.objects[i];
            float w = ClassifyBaseModel(o.model).harbour;
            score += w;
            strong = std::max(strong, w);
            wx += o.x * w;
            wz += o.z * w;
            wsum += w;
            coastal = coastal || WaterNear(h, o.x, o.z, opt.seaportWaterProbe);
        }
        float cx = wx / wsum, cz = wz / wsum;
        if (!coastal)
        {
            out.notes.push_back(Fmt("harbour-named objects near [%.0f, %.0f] stand away from the water, no seaport",
                                    cx, cz));
            continue;
        }
        // geometry: buildings whose footprint touches water beside the
        // cluster add a quarter each, at most half in all
        std::vector<int> waterside;
        for (int i = 0; i < (int)in.objects.size(); i++)
        {
            const DetectObject& o = in.objects[i];
            BaseModelClass k = ClassifyBaseModel(o.model);
            if (k.harbour > 0)
            {
                continue;
            }
            bool near = false;
            for (int m : members)
            {
                near = near || Dist(o.x, o.z, in.objects[m].x, in.objects[m].z) <= opt.seaportClusterDist;
            }
            if (near && FootprintTouchesWater(h, o))
            {
                waterside.push_back(i);
            }
        }
        float geo = std::min(0.5f, 0.25f * (float)waterside.size());
        int count = (int)(members.size() + waterside.size());
        if (count < opt.seaportMinObjects || score + geo < opt.seaportMinScore)
        {
            out.notes.push_back(Fmt("%d harbour object(s) near [%.0f, %.0f] score %.2f, below a seaport (%d objects, "
                                    "score %.2f)",
                                    count, cx, cz, score + geo, opt.seaportMinObjects, opt.seaportMinScore));
            continue;
        }
        BaseCandidate c;
        c.type = "SEAPORT";
        c.radius = opt.coverAnyDist;
        for (int m : members)
        {
            c.radius = std::max(c.radius, Dist(cx, cz, in.objects[m].x, in.objects[m].z) + 50.0f);
        }
        if (!FindDrySpot(h, opt, cx, cz, c.x, c.z))
        {
            out.notes.push_back(Fmt("harbour near [%.0f, %.0f]: no dry ground within %.0f m, no seaport", cx, cz,
                                    opt.maxInlandWalk));
            continue;
        }
        c.y = h.Height(c.x, c.z);
        c.garrison = opt.seaportGarrison;
        c.income = opt.income;
        c.confidence = std::min(0.95f, 0.3f + 0.2f * (score + geo));
        if (strong < 1.0f)
        {
            // jetties and boats only: a fishing landing, reported but never seeded
            c.confidence = std::min(c.confidence, 0.4f);
        }
        c.evidence.push_back(Fmt("%d harbour object(s), score %.2f, cluster centre [%.0f, %.0f], zone centre %.0f m "
                                 "inland of it",
                                 (int)members.size(), score + geo, cx, cz, Dist(cx, cz, c.x, c.z)));
        AddObjectEvidence(c, in.objects, members, "");
        AddObjectEvidence(c, in.objects, waterside, "waterside building ");
        seaports.push_back(c);
    }

    // ---- OUTPOST: clusters of military buildings outside the other two ----
    std::vector<int> milIdx;
    for (int i = 0; i < (int)in.objects.size(); i++)
    {
        const DetectObject& o = in.objects[i];
        if (ClassifyBaseModel(o.model).military <= 0)
        {
            continue;
        }
        bool inside = false;
        for (const BaseCandidate& a : airfields)
        {
            inside = inside || Dist(a.x, a.z, o.x, o.z) <= a.radius;
        }
        for (const BaseCandidate& s : seaports)
        {
            inside = inside || Dist(s.x, s.z, o.x, o.z) <= s.radius;
        }
        if (!inside)
        {
            milIdx.push_back(i);
        }
    }
    for (const std::vector<int>& members : Cluster(in.objects, milIdx, opt.outpostClusterDist))
    {
        if ((int)members.size() < opt.outpostMinObjects)
        {
            continue;
        }
        float cx = 0, cz = 0;
        for (int m : members)
        {
            cx += in.objects[m].x;
            cz += in.objects[m].z;
        }
        cx /= (float)members.size();
        cz /= (float)members.size();
        BaseCandidate c;
        c.type = "OUTPOST";
        c.radius = opt.coverAnyDist;
        for (int m : members)
        {
            c.radius = std::max(c.radius, Dist(cx, cz, in.objects[m].x, in.objects[m].z) + 30.0f);
        }
        if (!FindDrySpot(h, opt, cx, cz, c.x, c.z))
        {
            out.notes.push_back(Fmt("military cluster near [%.0f, %.0f]: no dry ground, no outpost", cx, cz));
            continue;
        }
        c.y = h.Height(c.x, c.z);
        c.garrison = opt.outpostGarrison;
        c.income = opt.income;
        c.confidence = std::min(0.9f, 0.4f + 0.1f * (float)members.size());
        c.evidence.push_back(Fmt("%d military object(s) within %.0f m links, centre [%.0f, %.0f]",
                                 (int)members.size(), opt.outpostClusterDist, cx, cz));
        AddObjectEvidence(c, in.objects, members, "");
        outposts.push_back(c);
    }

    // stable order inside the object-derived groups: strongest first, then
    // position, so a re-run (and a reload) yields the same names
    auto byStrength = [](const BaseCandidate& a, const BaseCandidate& b) {
        if (a.confidence != b.confidence)
        {
            return a.confidence > b.confidence;
        }
        if (a.x != b.x)
        {
            return a.x < b.x;
        }
        return a.z < b.z;
    };
    std::stable_sort(seaports.begin(), seaports.end(), byStrength);
    std::stable_sort(outposts.begin(), outposts.end(), byStrength);

    std::vector<BaseCandidate> all;
    all.insert(all.end(), airfields.begin(), airfields.end());
    all.insert(all.end(), seaports.begin(), seaports.end());
    all.insert(all.end(), outposts.begin(), outposts.end());

    // ---- status: region, then authored precedence, then confidence ----
    for (BaseCandidate& c : all)
    {
        bool outpost = stricmp(c.type, "OUTPOST") == 0;
        c.status = outpost ? BaseStatus::Offline : BaseStatus::Seed;
        if (!InRegion(in, c.x, c.z))
        {
            c.status = BaseStatus::OutsideRegion;
            continue;
        }
        float bestD = 1e30f;
        const DetectAuthoredZone* by = nullptr;
        for (const DetectAuthoredZone& a : in.authored)
        {
            float d = Dist(c.x, c.z, a.x, a.z);
            float reach = IsMilitaryType(a.type) ? std::max(opt.coverMilitaryDist, c.radius) : opt.coverAnyDist;
            if (d <= reach && d < bestD)
            {
                bestD = d;
                by = &a;
            }
        }
        if (by)
        {
            c.status = BaseStatus::Covered;
            c.coveredBy = by->name;
            c.evidence.push_back(Fmt("covered by the authored %s '%s' %.0f m away", (const char*)by->type,
                                     (const char*)by->name, bestD));
            continue;
        }
        if (c.confidence < opt.seedConfidence)
        {
            c.status = BaseStatus::LowConfidence;
        }
    }

    // ---- names ----
    RString world = BaseWorldName(in.worldClass, in.worldDisplay);
    for (size_t i = 0; i < all.size(); i++)
    {
        BaseCandidate& c = all[i];
        std::string stem;
        const char* label = nullptr;
        if (stricmp(c.type, "AIRFIELD") == 0)
        {
            // a Names label that reads like an airfield, inside the footprint
            // or within the city seed's 300 m of its rim, names it (and is
            // thereby kept out of the CITY seed by its name dedup)
            float bestD = c.radius + 300.0f;
            for (const DetectPlace& t : in.towns)
            {
                float d = Dist(c.x, c.z, t.x, t.z);
                if (d <= bestD && LooksLikeAirfieldLabel(t.name))
                {
                    bestD = d;
                    label = t.name;
                }
            }
            if (label)
            {
                stem = label;
            }
            else if (i < ilsAirfields)
            {
                stem = std::string((const char*)world) + " Airfield";
            }
            else
            {
                const DetectPlace* t = NearestTown(in, c.x, c.z, opt.nameTownDist);
                stem = std::string(t ? (const char*)t->name : (const char*)world) + " Airfield";
            }
        }
        else
        {
            const DetectPlace* t = NearestTown(in, c.x, c.z, opt.nameTownDist);
            stem = std::string(t ? (const char*)t->name : (const char*)world) +
                   (stricmp(c.type, "SEAPORT") == 0 ? " Harbour" : " Post");
        }
        std::vector<BaseCandidate> done(all.begin(), all.begin() + (std::ptrdiff_t)i);
        // a covered candidate never becomes a zone, so its name may repeat an
        // authored one; every other name must be unique against the zone table
        c.name = UniqueName(stem, in, done, c.status != BaseStatus::Covered, label);
        if (label && stricmp(c.name, label) == 0)
        {
            c.evidence.push_back(Fmt("named after the Names label '%s'", label));
        }
    }
    out.candidates = std::move(all);
}

} // namespace Poseidon::Guerrilla
