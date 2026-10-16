#pragma once

// Generated occupier bases (OCCUPIER-FORCES.md 5.7, step 1: zones only, no
// props).  An island whose authored and seeded bases leave towns without an
// occupier base in reach gets extra OUTPOST zones at new game, placed by the
// owner's density rule (2026-10-16):
//   * coverage: every town has a ground base on its own landmass within
//     coverDist (2.5 km, straight line);
//   * redundancy: every landmass that carries towns has at least minSources
//     (3) ground bases;
//   * total: the occupier's military zones number between ceil(K / 3) and
//     ceil(K / 2) (K = the towns), never more than maxTotal (10); generation
//     only ADDS until coverage, redundancy and the floor hold, the ceiling
//     caps it;
//   * spacing: a generated base stands at least baseSpacing (1.6 km) from
//     every occupier base and from the HQ, the Camp and the player's start
//     (keepouts), zoneMinDist (300 m) from any zone and townMinDist (500 m)
//     from any town;
//   * road: the centre stands roadMin..roadMax (60..250 m) from a road that
//     joins the island network and never on a road; a third of the
//     generated bases (rounded up) sit on a track trackTrunkDist (400 m) or
//     more from the trunk road;
//   * bearing: each town's two nearest sources should be bearingMinDeg (60
//     degrees) apart as seen from the town (a scoring term: it shapes the
//     choice, it never adds a base).
// The OUTPOST candidates BaseDetect finds (real props on the map) are
// preferred sites: they pass the same rules except the terrain-cell tests
// (their props are the objects those tests refuse).
//
// PURE, like BaseDetect: the caller injects the landscape's per-cell facts
// (one cell per geography square, 50 m on every stock world), the road
// points, the towns, the existing occupier bases, the zones, the keepouts
// and the detected outposts, and gets back the picks.  The engine half
// (BaseSiteProbe below, LandscapeBaseSiteProbe) reads GLandscape and
// GRoadNet; the seeding entry (ZoneRegistry::GenerateBaseZones, defined in
// BaseGenerate.cpp) turns the picks into zone rows.
//
// Persistence: the picks are generated ONCE, at new game, from one seed
// (generateBasesSeed, else one GRandGen draw) and STORED in the zone save
// block (GeneratedBaseRow, version GeneratedBasesVersion).  A reload
// re-injects the stored rows unchanged; nothing is ever re-derived, so a
// later build that tunes the scorer cannot move a running campaign's bases.
// A save written before the feature carries no rows and generates nothing.
//
// NOTE: no comment in this file may END in a backslash (see IslandScaffold.cpp).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Game/Guerrilla/IslandScaffold.hpp> // ScaffoldPoint, HeightField
#include <Poseidon/IO/Serialization/SerializeClass.hpp> // LSError, ParamArchive

#include <string>
#include <vector>

namespace Poseidon
{
namespace Guerrilla
{

// ---------------------------------------------------------------------------
// the kind catalogue (design catalogue, tmp/of-generated-bases-design.md 3)
// ---------------------------------------------------------------------------

enum class GenBaseKind
{
    Outpost,    // a town-retake source: the workhorse of step 1
    Checkpoint, // a minor post: a road tripwire
    Coastal,    // a minor post: a coastal tripwire
    Helipad,    // a minor post: an air annex
};

struct GenBaseKindInfo
{
    const char* type; // the zone type ("OUTPOST", "CHECKPOINT", "COASTAL", "HELIPAD")
    const char* noun; // the name's noun ("Vigny Outpost")
    int garrisonMax;  // the zone's garrisonMax
    int reserve;      // the stored garrison it starts with
    // minor posts (the owner, 2026-10-16) capture, source and alarm like any
    // base but are left out of the win roster (CampaignEnd::CountsForWin)
    bool countsForWin;
};
const GenBaseKindInfo& GenBaseKindOf(GenBaseKind kind);
// the kind of a zone type; false for any type that is not in the catalogue
bool GenBaseKindFromType(const char* type, GenBaseKind& out);

// ---------------------------------------------------------------------------
// inputs
// ---------------------------------------------------------------------------

// One landscape geography cell (GeographyInfo, AI/Path/AITypes.hpp, plus the
// sampled centre height).  The core labels the walkable landmasses from these
// cells itself, with RoadRoute::Landmass's rule.
struct GenCell
{
    float height = 0;   // ground height at the cell centre (sea level 0)
    bool water = false; // waterDepth > 0 (or the centre below sea level)
    bool deep = false;  // waterDepth >= 2: parts landmasses unless a road or track crosses it
    bool full = false;
    bool forestInner = false;
    bool forestOuter = false;
    bool road = false;
    bool track = false;
    int objects = 0;     // howManyObjects (0..3)
    int hardObjects = 0; // howManyHardObjects (0..3)
    int gradient = 0;    // the slope band 0..7: 1 from 0.05, 2 from 0.10, 3 from 0.15 rise over run
};

// A point on a road (a link centre or a connection end).
struct GenRoadPoint
{
    float x = 0;
    float z = 0;
    bool network = false; // its link joins the island network (not a pocket)
    bool track = false;   // a dirt track, not the trunk road
};

struct GenPlace
{
    RString name;
    float x = 0;
    float z = 0;
};

// An OUTPOST candidate BaseDetect found (status Offline), with its evidence.
struct GenDetected
{
    RString name;
    float x = 0;
    float z = 0;
    float y = 0;
};

struct BaseGenInputs
{
    // the cell grid, row-major (index z * cellsX + x); cell (x, z) covers
    // [x * cellSize, (x + 1) * cellSize) easting and likewise northing
    int cellsX = 0;
    int cellsZ = 0;
    float cellSize = 50.0f;
    std::vector<GenCell> cells;
    // optional finer height sampler for the spawn-square corners; null =
    // the nearest cell centre's height
    const HeightField* height = nullptr;
    std::vector<GenRoadPoint> roads;
    std::vector<GenPlace> towns;   // the K towns
    std::vector<GenPlace> sources; // the occupier's military zones now (authored and seeded)
    std::vector<ScaffoldPoint> zones;    // every zone centre (the zoneMinDist exclusion)
    std::vector<ScaffoldPoint> keepouts; // the Camp, the HQ or start town, the player
    std::vector<GenDetected> detected;
    std::vector<RString> takenNames; // every zone name (the generated names stay unique)
    bool hasRegion = false;
    float regionMinX = 0, regionMinZ = 0, regionMaxX = 0, regionMaxZ = 0;
    unsigned long long seed = 1;
    // zone rows left under ZoneRegistry::MaxZones
    int zoneRoom = 64;
};

struct BaseGenOptions
{
    float coverDist = 2500.0f;
    int minSources = 3;
    float baseSpacing = 1600.0f;
    float keepoutDist = 1600.0f;
    float zoneMinDist = 300.0f;
    float townMinDist = 500.0f;
    float roadMin = 60.0f;
    float roadMax = 250.0f;
    float trackTrunkDist = 400.0f;
    float bearingMinDeg = 60.0f;
    int maxTotal = 10;
    // terrain: every cell of the 3 x 3 footprint below this slope band (2 =
    // 0.10 rise over run), the centre at least minDryHeight up, the spawn
    // square's corners (+-spawnHalfSize) at least minCornerHeight up
    int maxGradientBand = 2;
    float minDryHeight = 2.0f;
    float minCornerHeight = 0.5f;
    float spawnHalfSize = 25.0f;
    float edgeMargin = 200.0f;
    // naming: "<town> <noun>" after the nearest town within this distance,
    // else "Hill <elevation> <noun>"
    float nameTownDist = 2500.0f;
    // score weights: a covered town is worth townWeight; the bonuses are
    // below one town so they never cost an extra base
    float townWeight = 1000.0f;
    float detectedBonus = 600.0f;
    float trackBonus = 600.0f;
    float bearingWeight = 100.0f; // per town whose bearing rule a pick fixes (capped at +-200)
    float jitter = 50.0f;         // seeded jitter, 0..jitter
};

// ---------------------------------------------------------------------------
// output
// ---------------------------------------------------------------------------

struct GeneratedBase
{
    RString name;
    GenBaseKind kind = GenBaseKind::Outpost;
    float x = 0;
    float z = 0;
    float y = 0;
    float heading = 0;     // degrees clockwise from north, centre towards the nearest network road
    bool detected = false; // a BaseDetect OUTPOST candidate, not empty ground
    bool track = false;    // on a track trackTrunkDist or more from the trunk road
    float roadDist = 0;    // to the nearest network road point
    float trunkDist = 0;   // to the nearest trunk (non-track) road point, 1e9 when none
    int covers = 0;        // towns this pick newly covered
    RString nearTown;      // the town it is named after (empty for a "Hill" name)
};

struct BaseGenTownReport
{
    RString name;
    float x = 0;
    float z = 0;
    int landmass = -1;
    RString nearest; // the nearest ground source after generation ("" none)
    float nearestDist = 1e9f;
    float bearingGap = 360.0f; // between its two nearest sources, 360 with fewer than two
};

struct BaseGenResult
{
    std::vector<GeneratedBase> bases;
    int K = 0;
    int existing = 0;
    int floor = 0;   // ceil(K / 3)
    int ceiling = 0; // min(ceil(K / 2), maxTotal)
    int uncoveredBefore = 0;
    int uncoveredAfter = 0;
    int tracks = 0;    // generated bases on a track
    int trackNeed = 0; // ceil(generated / 3)
    int candidates = 0; // sites that passed every hard rule
    std::vector<BaseGenTownReport> towns;
    std::vector<std::string> notes;
};

// The placement.  Deterministic: the same inputs and seed give the same picks.
void GenerateBases(const BaseGenInputs& in, const BaseGenOptions& opt, BaseGenResult& out);

// pure helpers (unit-tested)
int BaseGenFloor(int K);                  // ceil(K / 3)
int BaseGenCeiling(int K, int maxTotal);  // min(ceil(K / 2), maxTotal)
float BaseGenBearingDeg(float fromX, float fromZ, float toX, float toZ); // 0..360, clockwise from north
float BaseGenBearingGap(float a, float b);                             // 0..180
// a road model that is a dirt track ("data3d\cesta10 25.p3d"), not a paved road
bool RoadModelIsTrack(const char* modelPath);

// ---------------------------------------------------------------------------
// the stored row (ZoneRegistry::Serialize, block GeneratedBases)
// ---------------------------------------------------------------------------

// bumped when a row's meaning changes; a save without the block (version 0)
// is from before the feature and generates nothing
constexpr int GeneratedBasesVersion = 1;

struct GeneratedBaseRow
{
    RString name;
    RString type;
    float x = 0;
    float y = 0;
    float z = 0;
    int garrisonMax = 0;
    int reserve = 0;
    bool countsForWin = true;
    float heading = 0;
    bool detected = false;
    bool track = false;
    // the step 2 layout's seed (props), derived from the campaign seed and
    // the name; stored so a later layout cannot drift either
    int layoutSeed = 0;

    LSError Serialize(ParamArchive& ar);
};

// ---------------------------------------------------------------------------
// the world seam
// ---------------------------------------------------------------------------

// Fills the world half of the inputs (cells, roads, height, the player's
// position as a keepout).  False = no world to place on (headless): nothing
// is generated.  The registry adds the towns, sources, zones, keepouts,
// detected outposts, names, region and seed itself.
struct BaseSiteProbe
{
    virtual ~BaseSiteProbe() = default;
    virtual bool Gather(BaseGenInputs& in) const = 0;
    // a seeded town (seedCities = 1 seeds every label) that has no
    // settlement under it is not a town the density rule serves
    virtual bool IsSettlement(const char* /*name*/, float /*x*/, float /*z*/) const { return true; }
};

// The engine's BaseSiteProbe: GLandscape's geography cells and SurfaceY,
// RoadRoute::Landmass, every GRoadNet link (centre and connection ends;
// network = its component holds at least a quarter of the links of the
// largest component on its landmass; track = RoadModelIsTrack on its
// model), the real player as a keepout.  IsSettlement is the quiet twin of
// LandscapeSettlementProbe (the pool logs its own verdicts later).
struct LandscapeBaseSiteProbe final : BaseSiteProbe
{
    struct SurfaceHeight final : HeightField
    {
        float Height(float x, float z) const override;
    };
    SurfaceHeight height;
    bool Gather(BaseGenInputs& in) const override;
    bool IsSettlement(const char* name, float x, float z) const override;
};

} // namespace Guerrilla
} // namespace Poseidon
