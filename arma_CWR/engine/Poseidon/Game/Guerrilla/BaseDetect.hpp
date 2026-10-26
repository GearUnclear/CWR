#pragma once

// Occupation-network auto-detection (issue #76): find the AIRFIELD, SEAPORT
// and OUTPOST hubs a world models, so a new island gets its military zones
// without a hand survey.
//
// PURE, like IslandScaffold: the caller injects the world's ILS table (read
// from CfgWorlds with ReadIlsEntries), the placed objects it cares about
// (model path + position), a height sampler, the Names towns and the zones
// the template authors, and gets back a candidate list.  Two callers:
//   * the runtime seed (ZoneRegistry, CfgGuerrillaZones >> seedBases): feeds
//     the loaded landscape and adds every SEED candidate of type AIRFIELD or
//     SEAPORT as a seeded occupier zone.  OUTPOST candidates are never seeded
//     by this pass; with generateBases on they are the preferred sites of the
//     generated bases (BaseGenerate.hpp, OCCUPIER-FORCES.md 5.7: the owner's
//     2026-10-16 reversal of the #76 rule), kept in DetectedOutposts().
//   * the offline tool (PoseidonTools guerrilla scaffold / lint): feeds the
//     .wrp and prints every candidate, OUTPOSTs included, with its evidence.
//
// Authored zones win: a candidate near an authored zone is reported as
// COVERED and never seeded (see BaseDetectOptions for the distances).
//
// The rules (BaseDetect.cpp has the detail and the ground truth they were
// tuned on):
//   AIRFIELD  one per ILS entry (ilsPosition[] / ilsDirection[] / ilsTaxiIn[]
//             / ilsTaxiOff[] on the world class, plus every subclass of an
//             optional class SecondaryAirports with the same keys).  The zone
//             centre is the apron (the first ilsTaxiIn point) when it is dry,
//             else the runway footprint's midpoint.  An entry at the map
//             origin, outside the map, or with the runway in the water is
//             rejected.  An ILS entry the world class only INHERITS (from a
//             base world) carries low confidence and is never seeded.
//             A world's config states ONE runway (World::GetILS); a map that
//             models more airfields (@LoBo's Sinai: the Wadi strip, the
//             Egyptian Airbase) is read from its objects too: a single-link
//             cluster (airstripClusterDist) of aviation objects (hangars,
//             aircraft shelters, control towers) with at least
//             airstripMinObjects members, two of them hangars or shelters,
//             and more than airstripMergeDist from an airfield already found
//             (a weaker strip that close to a stronger one folds into it).
//             An airfield takes the name of a Names label inside its footprint
//             that reads like one ("Egyptian Airbase"), so the CITY seed's
//             name dedup drops that label instead of seeding it as a town.
//   SEAPORT   a single-link cluster (seaportClusterDist) of harbour-named
//             objects (concrete piers, quay blocks, cranes, docks; a wooden
//             jetty counts half) with at least seaportMinObjects members, a
//             harbour score of at least seaportMinScore, and water beside at
//             least one member.  A building whose footprint touches water and
//             stands within seaportClusterDist of the cluster adds a quarter
//             (geometry evidence, never enough on its own).  The centre is
//             walked inland to dry ground the garrison's spawn square fits on.
//   OUTPOST   a single-link cluster (outpostClusterDist) of military-named
//             objects (fortress, bunker, watchtower, barracks, military
//             garage, hangar) with at least outpostMinObjects members, not
//             inside an AIRFIELD or SEAPORT candidate's footprint.
//
// NOTE: no comment in this file may END in a backslash (see IslandScaffold.cpp).

#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Game/Guerrilla/IslandScaffold.hpp> // ScaffoldPoint, HeightField

#include <string>
#include <vector>

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{

// ---------------------------------------------------------------------------
// inputs
// ---------------------------------------------------------------------------

// One runway's ILS record as the world config states it (engine axes: x is
// easting, z is northing).  Read by ReadIlsEntries the way the engine reads
// it for World::GetILS (WorldInit.cpp: ilsPosition[0..1], the taxi arrays as
// flat x, z pairs, expressions such as "7922+69" evaluated).
struct IlsEntry
{
    float x = 0;
    float z = 0;
    float dirX = 0; // ilsDirection[0]
    float dirZ = 0; // ilsDirection[2]
    std::vector<ScaffoldPoint> taxiIn;
    std::vector<ScaffoldPoint> taxiOff;
    RString source;         // "CfgWorlds >> Abel" or "... >> SecondaryAirports >> Strip2"
    bool inherited = false; // ilsPosition comes from a base class, not this world's own entry
};

// Reads the main ILS entry of a CfgWorlds world class plus every subclass of
// its optional class SecondaryAirports.  An entry without ilsPosition[] (or
// with fewer than 2 values) is skipped.  Returns the number of entries read.
int ReadIlsEntries(const ParamEntry& worldEntry, std::vector<IlsEntry>& out);

// A placed object the detector may look at.  The callers pre-filter with
// ClassifyBaseModel (and keep house-sized buildings for the water-edge test).
struct DetectObject
{
    float x = 0;
    float z = 0;
    RString model;     // model path as the .wrp / the shape names it ("data3d\molo_beton.p3d")
    float radius = 0;  // bounding radius when known (runtime), 0 = unknown (offline: 8 m is assumed)
};

// A named place (a Names entry) used to name harbours and outposts.
struct DetectPlace
{
    RString name;
    float x = 0;
    float z = 0;
};

// A zone the template authors (CfgGuerrillaZones >> Zones).
struct DetectAuthoredZone
{
    RString name;
    RString type; // CAMP | AIRFIELD | SEAPORT | OUTPOST | CITY
    float x = 0;
    float z = 0;
};

// What a model path says about an object.  Every weight is 0 for anything
// else.  harbour: 1 for a concrete pier, quay block, crane, dock or harbour
// structure, 0.5 for a wooden jetty, quay wall, mooring or boat, 0.25 for a
// port's crates.  military: 1 for a fortress, bunker, watchtower, barracks,
// military garage, hangar, military repair or fuel point.
struct BaseModelClass
{
    float harbour = 0;
    float military = 0;
    // 1 for a hangar or an aircraft shelter, 0.5 for a control tower
    float aviation = 0;
};
BaseModelClass ClassifyBaseModel(const char* modelPath);

struct BaseDetectInputs
{
    RString worldClass;
    RString worldDisplay; // names the airfields ("Malden Airfield"); falls back to worldClass
    std::vector<IlsEntry> ils;
    std::vector<DetectObject> objects;
    const HeightField* height = nullptr; // required; sea level is 0
    float worldSize = 12800.0f;
    std::vector<DetectPlace> towns;
    std::vector<DetectAuthoredZone> authored;
    // playRegion[] = {minX, minZ, maxX, maxZ}: a candidate whose centre lies
    // outside is reported OUTSIDE_REGION.  hasRegion false = the whole map.
    bool hasRegion = false;
    float regionMinX = 0, regionMinZ = 0, regionMaxX = 0, regionMaxZ = 0;
};

struct BaseDetectOptions
{
    // garrison and income of a seeded or scaffolded zone, per type (Abel's
    // authored ladder: Airfield 12, Harbour 8, Compound 6).  Income: plan 13
    // yields are not implemented, so every base pays like an OUTPOST
    // (ZoneTuning::defaultIncome); the runtime passes its own value.
    int airfieldGarrison = 12;
    int seaportGarrison = 8;
    int outpostGarrison = 6;
    int income = 25;

    float seaportClusterDist = 150.0f;
    int seaportMinObjects = 2;
    float seaportMinScore = 1.5f;
    float seaportWaterProbe = 30.0f; // a member needs water within this many metres
    float outpostClusterDist = 120.0f;
    // 4, not 3: a mod world scatters guard huts and towers in threes along
    // every border (@LoBo's Sinai gave 70 three-object "posts")
    int outpostMinObjects = 4;
    float airstripClusterDist = 300.0f;
    int airstripMinObjects = 3;
    float airstripMergeDist = 1500.0f; // a second airfield this close to one already found is the same one

    // covered: within coverMilitaryDist (or the candidate's own footprint
    // radius, whichever is larger) of an authored AIRFIELD / SEAPORT /
    // OUTPOST, or within coverAnyDist (ZoneTuning::zoneArea) of any other
    // authored zone
    float coverMilitaryDist = 300.0f;
    float coverAnyDist = 150.0f;

    // dry-ground placement: the centre at least minDryHeight above sea level,
    // the garrison's spawn square (zoneArea / 3 = 50 m, so +-25 m) at least
    // minCornerHeight at every corner; the inland walk tries rings up to
    // maxInlandWalk metres from the raw centre
    float minDryHeight = 2.0f;
    float minCornerHeight = 0.5f;
    float spawnHalfSize = 25.0f;
    float maxInlandWalk = 300.0f;

    // a candidate below this confidence is LOW_CONFIDENCE (reported, never seeded)
    float seedConfidence = 0.5f;
    // naming: a harbour or outpost takes the nearest town within this
    // distance ("Le Port Harbour"), else the world's name
    float nameTownDist = 4000.0f;
};

// ---------------------------------------------------------------------------
// output
// ---------------------------------------------------------------------------

enum class BaseStatus
{
    Seed,          // passes every rule: the runtime seeds it (AIRFIELD / SEAPORT only)
    Offline,       // an OUTPOST candidate: reported by the tool; a generated-base site (generateBases)
    Covered,       // an authored zone already models it (coveredBy names it)
    OutsideRegion, // centre outside playRegion[]
    LowConfidence, // below seedConfidence
};
const char* BaseStatusName(BaseStatus s); // "seed" | "offline" | "covered" | "outside-region" | "low-confidence"

struct BaseCandidate
{
    RString type; // AIRFIELD | SEAPORT | OUTPOST
    RString name; // "Malden Airfield", "Le Port Harbour", "Larche Post"; unique among the result and the authored zones
    float x = 0;  // zone centre (dry ground, engine axes)
    float z = 0;
    float y = 0;          // sampled ground height at the centre
    float radius = 0;     // footprint: the runway or the object cluster, at least coverAnyDist
    float confidence = 0; // 0..1
    int garrison = 0;
    int income = 0;
    BaseStatus status = BaseStatus::Seed;
    RString coveredBy;                 // the authored zone's name when Covered
    std::vector<std::string> evidence; // one line each: the ILS entry, or each object (model at [x, z])
};

struct BaseDetectResult
{
    // AIRFIELDs (ILS order), then SEAPORTs, then OUTPOSTs, each group in a
    // stable order (cluster score, then position); every status included
    std::vector<BaseCandidate> candidates;
    std::vector<std::string> notes; // rejected ILS entries and the like
};

void DetectBases(const BaseDetectInputs& in, const BaseDetectOptions& opt, BaseDetectResult& out);

// The display name a world gives its airfields: worldDisplay unless it is
// empty or an unresolved "$STR" key, else the class name.
RString BaseWorldName(const char* worldClass, const char* worldDisplay);

} // namespace Guerrilla
} // namespace Poseidon
