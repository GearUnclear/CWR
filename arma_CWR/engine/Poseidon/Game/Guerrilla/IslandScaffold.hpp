#pragma once

// Island-template scaffolding (issue #54 C2): turn a world that has a CfgWorlds
// entry with a Names block and a road net into a playable Guerrilla template
// without hand-sampled elevations or hand-listed addOns[].
//
// Everything here is PURE: the caller injects a height sampler, a road-point
// cloud, a building-point cloud and the world's Names ParamEntry, and gets back
// a zone table plus the text of description.ext / mission.sqm. File I/O, the
// package mount and the .wrp read live in the tool command
// (apps/tools/Tools/commands/GuerrillaScaffoldCommand.cpp), so the placement
// rules can be unit-tested against synthetic terrain.
//
// WHY a separate placement pass at all, when the engine already auto-seeds CITY
// zones at mission time (ZoneRegistry::SeedCityZones + LandscapeSettlementProbe)?
// Because the seed pass only ever produces towns. A campaign also needs a
// resistance CAMP the player can start in and occupier OUTPOSTs to take, and
// those have to be placed against terrain the mission author would otherwise
// have to survey by hand. The town half deliberately mirrors the engine rules
// (NamesEntryIsTown, the 300 m dedup, the >= 3 houses within 300 m test) so a
// scaffolded template and the runtime classifier agree about what a town is.

#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <string>
#include <vector>

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{

// A map point in engine axes: X is easting, Z is northing. Y is never carried
// here - ground height always comes from the injected sampler, never from the
// .wrp object record (an object's Y is where the ARTIST put it, which for a
// bridge or a rooftop prop is metres off the terrain).
struct ScaffoldPoint
{
    float x = 0;
    float z = 0;
};

// Ground height in metres above sea level at a world point. The tool
// implements this over the .wrp heightmap using the same triangle
// interpolation as Landscape::SurfaceY, so a scaffolded elevation matches what
// the running engine reports for the same spot.
struct HeightField
{
    virtual ~HeightField() = default;
    virtual float Height(float x, float z) const = 0;
};

// ---------------------------------------------------------------------------
// object classification (name heuristics)
// ---------------------------------------------------------------------------
//
// A headless .wrp read gives model PATHS, not shapes. The engine classifies a
// placed object by loading its p3d (Object::GetType() == Network for roads,
// dyn_cast<Building> plus a bounding sphere > 4 m for houses), which needs the
// model subsystem and the whole bank set. These two predicates are the
// name-only stand-in. Their limits are real and documented in the .cpp: they
// know the OFP/Resistance model vocabulary (data3d\, o\road\, o\hous\) plus the
// generic directory tokens mods use, and they will miss a mod that names its
// houses something unrelated in a directory that says nothing.
bool ModelIsRoad(const char* modelPath);
bool ModelIsBuilding(const char* modelPath);

// ---------------------------------------------------------------------------
// zone placement
// ---------------------------------------------------------------------------

struct ScaffoldZone
{
    RString className; // config class name, a valid identifier and unique
    RString name;      // display name; the savegame + dedup key (ZoneRegistry)
    RString type;      // CAMP | OUTPOST | CITY
    RString owner;     // RESISTANCE | OCCUPIER | NEUTRAL
    float x = 0;       // easting
    float z = 0;       // northing
    float y = 0;       // sampled ground height
    int garrison = 0;
    int income = 0;
    int support = 0;
    RString marker;
    RString note; // one-line provenance comment emitted above the zone
};

struct ScaffoldOptions
{
    int outposts = 3;         // OUTPOST zones to place
    int seedCitySupport = 20; // starting support of a seeded town

    // Town rules - deliberately the engine's own numbers so the scaffolded
    // CITY set and the mission-time auto-seed agree (ZoneRegistry.cpp
    // SeedDedupDistSq, LandscapeSettlementProbe::kRadius / kMinBuildings).
    float townDedupDist = 300.0f;
    float townBuildingRadius = 300.0f;
    int townMinBuildings = 3;

    // Camp rules.
    // Out of reach of the occupier's town garrisons, but only just: the camp is
    // placed at the SMALLEST town distance that still clears this, which puts it
    // on the rim of the settled area rather than on the far edge of the map.
    float campMinCityDist = 1500.0f;
    float campMinRoadDist = 60.0f;  // off-road: a camp on the tarmac is a target
    float campMaxRoadDist = 400.0f; // but still reachable on foot from a road
    float campFlatTolerance = 2.0f; // m of relief across a 40 m span

    // Outpost rules.
    float outpostMinRoadDist = 150.0f;
    float outpostMaxRoadDist = 400.0f;
    // An occupier post guards something. Without this cap the farthest-point
    // spread walks the posts onto whichever offshore islet is furthest from
    // everything, which is where nobody ever goes.
    float outpostMaxCityDist = 2500.0f;

    float zoneMinSpacing = 800.0f; // between any two placed (non-town) zones
    float minDryHeight = 1.0f;     // placed zones: above the wash, not just above 0
    float minTownHeight = 0.0f;    // towns: the engine only asks for dry land
};

struct ScaffoldResult
{
    std::vector<ScaffoldZone> zones; // CAMP first, then OUTPOSTs, then CITYs
    int campIndex = -1;
    int cityCount = 0;
    int outpostCount = 0;
    float playerX = 0; // camp centre plus a small offset (mission.sqm)
    float playerZ = 0;
    float playerY = 0;
    std::vector<std::string> warnings;
    bool ok = false;
    std::string error;
};

struct ScaffoldInputs
{
    RString worldClass;
    RString worldDisplay;
    const ParamEntry* names = nullptr; // CfgWorlds >> <class> >> Names
    const HeightField* height = nullptr;
    std::vector<ScaffoldPoint> roads;
    std::vector<ScaffoldPoint> buildings;
    float worldSize = 12800.0f; // grid cells * landGrid, for candidate clamping
};

// Selects the towns, places the camp and the outposts. Fails (ok = false, with
// error set) only when the inputs cannot yield a template at all: no world
// class, no height sampler, or no dry ground for the camp.
bool BuildZones(const ScaffoldInputs& in, const ScaffoldOptions& opt, ScaffoldResult& out);

// The town half on its own (BuildZones runs it first). Appends CITY zones in
// Names-block order, deduped at opt.townDedupDist against each other.
void SelectTownZones(const ScaffoldInputs& in, const ScaffoldOptions& opt, ScaffoldResult& out);

// ---------------------------------------------------------------------------
// template text
// ---------------------------------------------------------------------------

struct TemplateInfo
{
    RString worldClass;
    RString worldDisplay;
    RString playerClass = "SoldierGB";
    RString playerSide = "GUER";
    std::vector<RString> addOns;      // CfgPatches owners; empty entries dropped
    std::vector<RString> civClasses;  // CIV descriptor civClass1..N
    std::vector<RString> civVehicles; // CIV descriptor civVehicles[]
    RString toolBanner = "PoseidonTools guerrilla scaffold";
    RString date;       // YYYY-MM-DD, injected so the tests stay deterministic
    int randomSeed = 0; // mission.sqm randomSeed base
    int outposts = 3;   // echoed into the header comment
    RString wrpPath;    // echoed into the header comment
    int roadPoints = 0; // echoed into the header comment
    int buildingPoints = 0;
};

// keepZones, when non-empty, is spliced in verbatim in place of the generated
// "class Zones { ... };" block (--keep-zones). It must be the whole block,
// including the class keyword and the trailing semicolon, as ExtractZonesBlock
// returns it.
std::string RenderDescriptionExt(const TemplateInfo& info, const ScaffoldResult& zones, const ScaffoldOptions& opt,
                                 const std::string& keepZones = std::string());
std::string RenderMissionSqm(const TemplateInfo& info, const ScaffoldResult& zones);
// Byte-for-byte the bootstrap of guerrilla-mode/mission/Guerrilla.Abel/init.sqs.
const char* InitSqsText();

// Cuts the "class Zones { ... };" block out of an existing description.ext by
// brace matching (comments and string literals are skipped, so a brace inside
// either does not confuse the scan). Returns false when there is no such block.
bool ExtractZonesBlock(const std::string& descriptionExt, std::string& block);

// Config-identifier scrub for a zone class name: keeps [A-Za-z0-9_], drops the
// rest, prefixes a leading digit, falls back to `fallback` when nothing is left.
RString SanitizeClassName(const char* raw, const char* fallback);

} // namespace Guerrilla
} // namespace Poseidon
