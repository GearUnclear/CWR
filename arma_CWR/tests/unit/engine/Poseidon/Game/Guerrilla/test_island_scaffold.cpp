// Island-template scaffolding (issue #54 C2): the pure half of
// `PoseidonTools guerrilla scaffold`. Everything here runs against synthetic
// terrain - a flat plateau with one lake, one straight road and a few house
// clusters - so the placement rules are pinned without a .wrp or a data package.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
#include <Poseidon/Game/Guerrilla/IslandScaffold.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string.h>
#include <string>
#include <system_error>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// 5 km square: 20 m of flat ground, a lake centred on (4000, 4000) and a ridge
// running north-south at x = 2000 so "flattest nearby ground" has something to
// reject.
struct FakeTerrain final : HeightField
{
    float Height(float x, float z) const override
    {
        float lake = sqrtf((x - 4000) * (x - 4000) + (z - 4000) * (z - 4000));
        if (lake < 300.0f)
        {
            return -5.0f;
        }
        if (fabsf(x - 2000.0f) < 60.0f)
        {
            return 20.0f + (60.0f - fabsf(x - 2000.0f)) * 0.5f; // a 30 m ridge
        }
        return 20.0f;
    }
};

struct Parsed
{
    ParamFile file;
    explicit Parsed(const std::string& text)
    {
        QIStream in(text.c_str(), text.size());
        file.Parse(in);
    }
};

// ParamFile::Parse(QIStream&) is the RAW parser: comments and #defines are the
// preprocessor's job, and the preprocessor only runs on the file-name overload.
// A generated description.ext is full of comments, so it has to go through a
// file to be parsed the way the engine parses it.
struct ParsedFile
{
    ParamFile file;
    std::filesystem::path path;
    ParsedFile(const std::string& text, const char* name)
    {
        path = std::filesystem::temp_directory_path() / name;
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out.write(text.data(), (std::streamsize)text.size());
        }
        file.Parse(path.string().c_str());
    }
    ~ParsedFile()
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }
};

std::string JoinWarnings(const ScaffoldResult& out)
{
    std::string s;
    for (const std::string& w : out.warnings)
    {
        s += "\n  - " + w;
    }
    return s;
}

std::vector<ScaffoldPoint> StraightRoad()
{
    // one east-west road across the middle of the map, a piece every 25 m
    std::vector<ScaffoldPoint> road;
    for (float x = 200.0f; x <= 4800.0f; x += 25.0f)
    {
        ScaffoldPoint p;
        p.x = x;
        p.z = 2500.0f;
        road.push_back(p);
    }
    return road;
}

void AddCluster(std::vector<ScaffoldPoint>& out, float cx, float cz, int n)
{
    for (int i = 0; i < n; i++)
    {
        ScaffoldPoint p;
        p.x = cx + (float)(i % 3) * 30.0f;
        p.z = cz + (float)(i / 3) * 30.0f;
        out.push_back(p);
    }
}

// TownA and TownC are real settlements, TownB sits 130 m from TownA (dedup),
// Lake is in the water, Empty has no houses.
const char* kNames = "class Names\n"
                     "{\n"
                     "    class TownA { name=\"Town A\"; position[]={700.0, 700.0, 50.0}; };\n"
                     "    class TownB { name=\"Town B\"; position[]={800.0, 780.0, 50.0}; };\n"
                     "    class Lake  { name=\"Lake\";   position[]={4000.0, 4000.0, 50.0}; };\n"
                     "    class Empty { name=\"Empty\";  position[]={2500.0, 4500.0, 50.0}; };\n"
                     "    class TownC { name=\"Town C\"; position[]={2500.0, 700.0, 50.0}; };\n"
                     "    class Rocks { name=\"Rocks\"; type=\"RockArea\"; position[]={3000.0, 3000.0}; };\n"
                     "};\n";

FakeTerrain gTerrain;

ScaffoldInputs MakeInputs(std::vector<ScaffoldPoint>& roads, std::vector<ScaffoldPoint>& buildings,
                          const ParamEntry* names)
{
    ScaffoldInputs in;
    in.worldClass = "Fakeland";
    in.worldDisplay = "Fakeland";
    in.names = names;
    in.height = &gTerrain;
    in.roads = roads;
    in.buildings = buildings;
    in.worldSize = 5000.0f;
    return in;
}

float Dist(float ax, float az, float bx, float bz)
{
    return sqrtf((ax - bx) * (ax - bx) + (az - bz) * (az - bz));
}

float NearestRoad(const std::vector<ScaffoldPoint>& roads, float x, float z)
{
    float best = 1e9f;
    for (const ScaffoldPoint& p : roads)
    {
        best = std::min(best, Dist(x, z, p.x, p.z));
    }
    return best;
}

TemplateInfo MakeInfo()
{
    TemplateInfo info;
    info.worldClass = "Fakeland";
    info.worldDisplay = "Fakeland";
    info.date = "2026-09-02";
    info.wrpPath = "worlds\\fakeland.wrp";
    info.randomSeed = 12340000;
    info.civClasses = {RString("Civilian"), RString("Civilian2")};
    info.civVehicles = {RString("Skoda")};
    return info;
}

} // namespace

TEST_CASE("model-path heuristics recognise the stock and @LoBo road/house vocabularies", "[game][guerrilla][scaffold]")
{
    // Classic 1.99 abel.wrp
    REQUIRE(ModelIsRoad("data3d\\cesta10 100.p3d"));
    REQUIRE(ModelIsRoad("data3d\\silnice25.p3d"));
    REQUIRE(ModelIsRoad("data3d\\asfaltka12.p3d"));
    REQUIRE(ModelIsRoad("data3d\\asfatlka10 25.p3d")); // the stock typo
    REQUIRE(ModelIsRoad("data3d\\kr_silnice_asfaltka_t.p3d"));
    // @LoBo sinai.wrp
    REQUIRE(ModelIsRoad("o\\road\\ces25.p3d"));
    REQUIRE(ModelIsRoad("lobo_ob\\roads\\asp10 100.p3d"));
    REQUIRE(ModelIsRoad("lobobloc\\croads\\sil25.p3d"));
    REQUIRE(ModelIsRoad("o\\road\\kr_new_kos_sil_t.p3d"));
    // not roads
    REQUIRE(!ModelIsRoad("data3d\\str borovice.p3d"));
    REQUIRE(!ModelIsRoad("data3d\\cihlovej_dum.p3d"));
    REQUIRE(!ModelIsRoad("lobo_ob\\plants\\palms.p3d"));

    REQUIRE(ModelIsBuilding("data3d\\cihlovej_dum_mini.p3d"));
    REQUIRE(ModelIsBuilding("data3d\\budova1.p3d"));
    REQUIRE(ModelIsBuilding("data3d\\kostelik.p3d"));
    REQUIRE(ModelIsBuilding("data3d\\statek_kulna.p3d"));
    REQUIRE(ModelIsBuilding("lobo_ob\\buildings\\lobo_building_06.p3d"));
    REQUIRE(ModelIsBuilding("lobo_ob\\blocs\\lobo_house_bloc12.p3d"));
    REQUIRE(ModelIsBuilding("lobo_ob\\city\\lobo_apartment2.p3d"));
    REQUIRE(ModelIsBuilding("o\\hous\\bouda_plech.p3d"));
    REQUIRE(ModelIsBuilding("ags_inds\\whouse1.p3d"));
    REQUIRE(ModelIsBuilding("baracken\\brown_baracke01.p3d"));
    // fences, signs, poles and roads are not houses, even in a house directory
    REQUIRE(!ModelIsBuilding("data3d\\plot_green_draty.p3d"));
    REQUIRE(!ModelIsBuilding("data3d\\znacka_stop.p3d"));
    REQUIRE(!ModelIsBuilding("data3d\\statek_plot.p3d"));
    REQUIRE(!ModelIsBuilding("baracken\\zaun5.p3d"));
    REQUIRE(!ModelIsBuilding("lobobloc\\croads\\sil25.p3d"));
    REQUIRE(!ModelIsBuilding("lobo_ob\\gw\\tp\\lobo_polemain.p3d"));
    REQUIRE(!ModelIsBuilding("data3d\\str_fikovnik.p3d"));
}

TEST_CASE("towns are the Names entries on dry land with houses around them", "[game][guerrilla][scaffold]")
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);   // Town A
    AddCluster(buildings, 780, 760, 4);   // Town B sits in the same cluster
    AddCluster(buildings, 2500, 700, 5);  // Town C
    AddCluster(buildings, 4000, 4000, 8); // houses in the lake: still wet
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));

    ScaffoldOptions opt;
    ScaffoldResult out;
    SelectTownZones(in, opt, out);

    REQUIRE(out.cityCount == 2);
    REQUIRE(Str(out.zones[0].name) == "Town A");
    REQUIRE(Str(out.zones[1].name) == "Town C");
    REQUIRE(Str(out.zones[0].type) == "CITY");
    REQUIRE(Str(out.zones[0].owner) == "NEUTRAL");
    REQUIRE(Str(out.zones[0].className) == "TownA"); // the Names class name
    REQUIRE(out.zones[0].y > 0.0f);
    REQUIRE(out.zones[0].support == opt.seedCitySupport);
    // Town B deduped, Lake wet, Empty houseless, Rocks filtered by type
    REQUIRE(out.warnings.size() == 3);
}

TEST_CASE("camp and outposts hold their placement constraints", "[game][guerrilla][scaffold]")
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    AddCluster(buildings, 2500, 700, 5);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));

    ScaffoldOptions opt;
    ScaffoldResult out;
    REQUIRE(BuildZones(in, opt, out));
    REQUIRE(out.campIndex == 0);
    REQUIRE(out.outpostCount == 3);
    REQUIRE(out.cityCount == 2);
    REQUIRE(out.zones.size() == 6);
    REQUIRE(Str(out.zones[0].type) == "CAMP");
    REQUIRE(Str(out.zones[0].owner) == "RESISTANCE");
    REQUIRE(Str(out.zones[1].type) == "OUTPOST");
    REQUIRE(Str(out.zones[1].owner) == "OCCUPIER");
    REQUIRE(Str(out.zones[5].type) == "CITY");
    // markers are renumbered over the final order
    REQUIRE(Str(out.zones[0].marker) == "gmZoneMarker_0");
    REQUIRE(Str(out.zones[5].marker) == "gmZoneMarker_5");

    const ScaffoldZone& camp = out.zones[0];
    REQUIRE(camp.y >= opt.minDryHeight);
    float campRoad = NearestRoad(roads, camp.x, camp.z);
    REQUIRE(campRoad >= opt.campMinRoadDist);
    REQUIRE(campRoad <= opt.campMaxRoadDist);
    REQUIRE(Dist(camp.x, camp.z, 700, 700) >= opt.campMinCityDist);
    REQUIRE(Dist(camp.x, camp.z, 2500, 700) >= opt.campMinCityDist);
    // the only warnings are the town-skip notes: no relaxation rung fired and
    // every outpost was placed
    INFO("warnings:" << JoinWarnings(out));
    REQUIRE(JoinWarnings(out).find("relaxed") == std::string::npos);
    REQUIRE(JoinWarnings(out).find("outposts placed") == std::string::npos);
    REQUIRE(JoinWarnings(out).find("no road pieces") == std::string::npos);
    // the player stands next to the camp, not on it, and on dry ground
    REQUIRE(Dist(out.playerX, out.playerZ, camp.x, camp.z) > 0.0f);
    REQUIRE(Dist(out.playerX, out.playerZ, camp.x, camp.z) < 20.0f);
    REQUIRE(out.playerY > 0.0f);

    for (size_t i = 1; i <= 3; i++)
    {
        const ScaffoldZone& post = out.zones[i];
        REQUIRE(post.y >= opt.minDryHeight);
        float d = NearestRoad(roads, post.x, post.z);
        REQUIRE(d >= opt.outpostMinRoadDist);
        REQUIRE(d <= opt.outpostMaxRoadDist);
        REQUIRE(post.garrison == 6);
        for (size_t j = 0; j < out.zones.size(); j++)
        {
            if (j == i)
            {
                continue;
            }
            REQUIRE(Dist(post.x, post.z, out.zones[j].x, out.zones[j].z) >= opt.zoneMinSpacing);
        }
    }
}

TEST_CASE("a world with no recognised roads still scaffolds, with a warning", "[game][guerrilla][scaffold]")
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads; // nothing recognised
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));

    ScaffoldOptions opt;
    ScaffoldResult out;
    REQUIRE(BuildZones(in, opt, out));
    REQUIRE(out.campIndex == 0);
    REQUIRE(out.zones[0].y >= opt.minDryHeight);
    bool warned = false;
    for (const std::string& w : out.warnings)
    {
        warned = warned || w.find("no road pieces recognised") != std::string::npos;
    }
    REQUIRE(warned);
}

TEST_CASE("the generated description.ext parses back with the zones it was given", "[game][guerrilla][scaffold]")
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    AddCluster(buildings, 2500, 700, 5);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));

    ScaffoldOptions opt;
    ScaffoldResult out;
    REQUIRE(BuildZones(in, opt, out));

    std::string text = RenderDescriptionExt(MakeInfo(), out, opt);
    ParsedFile back(text, "ud_scaffold_desc.ext");
    const ParamEntry* cfg = back.file.FindEntry("CfgGuerrillaZones");
    REQUIRE(cfg != nullptr);
    // seedCities must stay ABSENT so the mission-time classifier runs
    REQUIRE(cfg->FindEntry("seedCities") == nullptr);
    REQUIRE(cfg->ReadValue("seedCitySupport", 0.0f) == (float)opt.seedCitySupport);
    const ParamEntry* zones = cfg->FindEntry("Zones");
    REQUIRE(zones != nullptr);
    REQUIRE(zones->GetEntryCount() == (int)out.zones.size());

    // the zone table the engine would build out of it matches what we placed
    ZoneRegistry reg;
    reg.LoadFromParams(cfg, back.file.FindEntry("CfgGuerrillaFactions"));
    REQUIRE(reg.NZones() == (int)out.zones.size());
    REQUIRE(reg.FindZoneIndex("Camp") == 0);
    REQUIRE(reg.FindZoneIndex("Town A") >= 0);
    const ZoneRecord* camp = reg.GetZone(0);
    REQUIRE(fabsf(camp->pos.X() - out.zones[0].x) < 1.0f);
    REQUIRE(fabsf(camp->pos.Z() - out.zones[0].z) < 1.0f);

    // and the CIV descriptor survives the round trip
    const ParamEntry* civ = back.file.FindEntry("CfgGuerrillaFactions");
    REQUIRE(civ != nullptr);
    const ParamEntry* civClass = civ->FindEntry("CIV");
    REQUIRE(civClass != nullptr);
    REQUIRE(civClass->ReadValue("civClassCount", 0.0f) == 2.0f);
    REQUIRE(Str(civClass->ReadValue("civClass1", RString())) == "Civilian");

    std::string sqm = RenderMissionSqm(MakeInfo(), out);
    ParsedFile sqmBack(sqm, "ud_scaffold_mission.sqm");
    const ParamEntry* mission = sqmBack.file.FindEntry("Mission");
    REQUIRE(mission != nullptr);
    REQUIRE(mission->FindEntry("Groups") != nullptr);
}

TEST_CASE("--keep-zones splices an edited Zones block through a re-run", "[game][guerrilla][scaffold]")
{
    const std::string edited = "class CfgGuerrillaZones\n"
                               "{\n"
                               "    playerSide = \"GUER\";\n"
                               "    class Zones\n"
                               "    {\n"
                               "        // a hand comment with a brace } in it\n"
                               "        class Camp\n"
                               "        {\n"
                               "            name = \"Base Camp\"; // renamed by hand\n"
                               "            type = \"CAMP\";\n"
                               "            owner = \"RESISTANCE\";\n"
                               "            marker = \"gmZoneMarker_0\";\n"
                               "            position[] = {1234, 5678, 42.5};\n"
                               "        };\n"
                               "    };\n"
                               "};\n";
    std::string block;
    REQUIRE(ExtractZonesBlock(edited, block));
    REQUIRE(block.find("Base Camp") != std::string::npos);
    REQUIRE(block.find("brace } in it") != std::string::npos);
    REQUIRE(block.rfind(';') == block.size() - 1);
    // exactly one class Zones block, closed at the right brace
    REQUIRE(block.find("CfgGuerrillaZones") == std::string::npos);

    // the splice survives a full re-render and still parses
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));
    ScaffoldOptions opt;
    ScaffoldResult out;
    REQUIRE(BuildZones(in, opt, out));

    std::string text = RenderDescriptionExt(MakeInfo(), out, opt, block);
    REQUIRE(text.find("Base Camp") != std::string::npos);
    ParsedFile back(text, "ud_scaffold_keep.ext");
    const ParamEntry* zones = back.file.FindEntry("CfgGuerrillaZones")->FindEntry("Zones");
    REQUIRE(zones != nullptr);
    REQUIRE(zones->GetEntryCount() == 1); // the hand block, not the generated one
    ZoneRegistry reg;
    reg.LoadFromParams(back.file.FindEntry("CfgGuerrillaZones"), nullptr);
    REQUIRE(reg.NZones() == 1);
    REQUIRE(Str(reg.GetZone(0)->name) == "Base Camp");

    // a file with no Zones block is reported, not silently spliced
    std::string none;
    REQUIRE(!ExtractZonesBlock("class CfgGuerrillaZones { playerSide = \"GUER\"; };", none));
}

TEST_CASE("class names are scrubbed to config identifiers", "[game][guerrilla][scaffold]")
{
    REQUIRE(Str(SanitizeClassName("Saint Phillippe", "Town")) == "SaintPhillippe");
    REQUIRE(Str(SanitizeClassName("La-Riviere", "Town")) == "LaRiviere");
    REQUIRE(Str(SanitizeClassName("12Bravo", "Town")) == "Z12Bravo");
    REQUIRE(Str(SanitizeClassName("", "Town")) == "Town");
    REQUIRE(Str(SanitizeClassName("...", "Town")) == "Town");
}

// ---------------------------------------------------------------------------
// detected bases (issue #76)
// ---------------------------------------------------------------------------

namespace
{

// One ILS entry on the plateau and a concrete pier + quay block on the lake
// shore: the smallest world that yields one AIRFIELD and one SEAPORT.
void AddBaseInputs(BaseDetectInputs& in)
{
    in.worldClass = "Fakeland";
    in.worldDisplay = "Fakeland";
    in.height = &gTerrain;
    in.worldSize = 5000.0f;
    IlsEntry ils;
    ils.x = 3000.0f;
    ils.z = 1200.0f;
    ils.dirX = 1.0f;
    ils.dirZ = 0.0f;
    ils.taxiIn.push_back(ScaffoldPoint{3050.0f, 1300.0f});
    ils.taxiOff.push_back(ScaffoldPoint{2200.0f, 1200.0f});
    ils.source = "CfgWorlds >> Fakeland";
    in.ils.push_back(ils);
    DetectObject pier;
    pier.x = 4000.0f;
    pier.z = 3690.0f;
    pier.model = "data3d\\molo_beton.p3d";
    in.objects.push_back(pier);
    DetectObject quay;
    quay.x = 4030.0f;
    quay.z = 3690.0f;
    quay.model = "data3d\\molo_krychle.p3d";
    in.objects.push_back(quay);
    DetectPlace town;
    town.name = "Town C";
    town.x = 2500.0f;
    town.z = 700.0f;
    in.towns.push_back(town);
}

std::string ScaffoldWithBases(ScaffoldResult& out, BaseDetectResult& bases)
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    AddCluster(buildings, 2500, 700, 5);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));
    ScaffoldOptions opt;
    opt.outposts = 0;
    ScaffoldResult first;
    REQUIRE(BuildZones(in, opt, first));

    BaseDetectInputs detectIn;
    AddBaseInputs(detectIn);
    for (const ScaffoldZone& z : first.zones)
    {
        DetectAuthoredZone a;
        a.name = z.name;
        a.type = z.type;
        a.x = z.x;
        a.z = z.z;
        detectIn.authored.push_back(a);
    }
    DetectBases(detectIn, BaseDetectOptions(), bases);

    const int budget = 2;
    REQUIRE(DetectedOutpostsToEmit(bases, budget) == 0); // no military cluster in this world
    for (const BaseCandidate& c : bases.candidates)
    {
        in.reserved.push_back(ScaffoldPoint{c.x, c.z});
    }
    opt.outposts = budget;
    REQUIRE(BuildZones(in, opt, out));
    MergeBaseCandidates(bases, budget, out);
    return RenderDescriptionExt(MakeInfo(), out, opt);
}

} // namespace

TEST_CASE("detected airfield and seaport are written into the zone block with their evidence",
          "[game][guerrilla][scaffold]")
{
    ScaffoldResult out;
    BaseDetectResult bases;
    std::string text = ScaffoldWithBases(out, bases);

    INFO(text);
    REQUIRE(bases.candidates.size() == 2);
    REQUIRE(Str(bases.candidates[0].type) == "AIRFIELD");
    REQUIRE(bases.candidates[0].status == BaseStatus::Seed);
    REQUIRE(Str(bases.candidates[1].type) == "SEAPORT");
    REQUIRE(bases.candidates[1].status == BaseStatus::Seed);

    // order: CAMP, AIRFIELD, SEAPORT, heuristic OUTPOSTs, CITYs
    REQUIRE(out.baseCount == 2);
    REQUIRE(Str(out.zones[0].type) == "CAMP");
    REQUIRE(Str(out.zones[1].type) == "AIRFIELD");
    REQUIRE(Str(out.zones[2].type) == "SEAPORT");
    REQUIRE(Str(out.zones[3].type) == "OUTPOST");
    REQUIRE(Str(out.zones[2].marker) == "gmZoneMarker_2");
    REQUIRE(out.outpostCount == 2);
    // the heuristic outposts kept clear of the detected bases
    for (const ScaffoldZone& z : out.zones)
    {
        if (Str(z.type) == "OUTPOST")
        {
            REQUIRE(Dist(z.x, z.z, out.zones[1].x, out.zones[1].z) >= 800.0f);
            REQUIRE(Dist(z.x, z.z, out.zones[2].x, out.zones[2].z) >= 800.0f);
        }
    }

    // the evidence is in the text, as comments above each zone class
    REQUIRE(text.find("// evidence: ILS CfgWorlds >> Fakeland") != std::string::npos);
    REQUIRE(text.find("molo_beton") != std::string::npos);
    REQUIRE(text.find("molo_krychle") != std::string::npos);
    // the block authors AIRFIELD / SEAPORT zones, so the header says the seed is off
    REQUIRE(text.find("runtime seed is OFF for this template") != std::string::npos);

    // parsed the way the engine parses it: the zones are real classes with the
    // detector's centre, garrison and the occupier as owner
    ParsedFile back(text, "ud_scaffold_bases.ext");
    const ParamEntry* cfg = back.file.FindEntry("CfgGuerrillaZones");
    REQUIRE(cfg != nullptr);
    REQUIRE(cfg->FindEntry("seedBases") == nullptr); // absent = Auto
    const ParamEntry* zones = cfg->FindEntry("Zones");
    REQUIRE(zones != nullptr);
    REQUIRE(zones->GetEntryCount() == (int)out.zones.size());
    const ParamEntry& air = zones->GetEntry(1);
    REQUIRE(Str(air.ReadValue("type", RString())) == "AIRFIELD");
    REQUIRE(Str(air.ReadValue("owner", RString())) == "OCCUPIER");
    REQUIRE(Str(air.ReadValue("name", RString())) == Str(bases.candidates[0].name));
    REQUIRE(air.ReadValue("garrison", 0.0f) == (float)bases.candidates[0].garrison);
    const ParamEntry* airPos = air.FindEntry("position");
    REQUIRE(airPos != nullptr);
    REQUIRE(fabsf((float)(*airPos)[0] - bases.candidates[0].x) < 1.0f);
    REQUIRE(fabsf((float)(*airPos)[1] - bases.candidates[0].z) < 1.0f);
    const ParamEntry& sea = zones->GetEntry(2);
    REQUIRE(Str(sea.ReadValue("type", RString())) == "SEAPORT");
    REQUIRE(Str(sea.ReadValue("name", RString())) == Str(bases.candidates[1].name));

    ZoneRegistry reg;
    reg.LoadFromParams(cfg, back.file.FindEntry("CfgGuerrillaFactions"));
    REQUIRE(reg.NZones() == (int)out.zones.size());
    REQUIRE(reg.FindZoneIndex(bases.candidates[0].name) == 1);

    // deterministic: the same inputs render the same bytes
    ScaffoldResult again;
    BaseDetectResult basesAgain;
    REQUIRE(ScaffoldWithBases(again, basesAgain) == text);
}

TEST_CASE("covered and over-budget bases become commented-out notes", "[game][guerrilla][scaffold]")
{
    ScaffoldResult out;
    BaseCandidate covered;
    covered.type = "SEAPORT";
    covered.name = "Lake Harbour";
    covered.x = 4000;
    covered.z = 3600;
    covered.confidence = 0.8f;
    covered.status = BaseStatus::Covered;
    covered.coveredBy = "Old Pier";
    covered.evidence.push_back("data3d\\molo_beton.p3d at [4000, 3690]");
    BaseCandidate post1;
    post1.type = "OUTPOST";
    post1.name = "Ridge Post";
    post1.x = 2000;
    post1.z = 3000;
    post1.garrison = 6;
    post1.income = 25;
    post1.confidence = 0.8f;
    post1.status = BaseStatus::Offline;
    BaseCandidate post2 = post1;
    post2.name = "Lake Post";
    post2.x = 3000;
    BaseDetectResult bases;
    bases.candidates = {covered, post1, post2};

    REQUIRE(DetectedOutpostsToEmit(bases, 1) == 1);
    REQUIRE(DetectedOutpostsToEmit(bases, 5) == 2);
    REQUIRE(DetectedOutpostsToEmit(bases, 0) == 0);

    ScaffoldZone camp;
    camp.className = "Camp";
    camp.name = "Camp";
    camp.type = "CAMP";
    camp.owner = "RESISTANCE";
    out.zones.push_back(camp);
    out.campIndex = 0;
    MergeBaseCandidates(bases, 1, out);
    REQUIRE(out.zones.size() == 2);
    REQUIRE(Str(out.zones[1].name) == "Ridge Post");
    REQUIRE(Str(out.zones[1].className) == "RidgePost");
    REQUIRE(Str(out.zones[1].type) == "OUTPOST");
    REQUIRE(out.outpostCount == 1);
    REQUIRE(out.baseNotes.size() == 3); // covered + its evidence, over-budget post

    std::string text = RenderDescriptionExt(MakeInfo(), out, ScaffoldOptions());
    INFO(text);
    REQUIRE(text.find("// NOT EMITTED: covered SEAPORT \"Lake Harbour\"") != std::string::npos);
    REQUIRE(text.find("covered by \"Old Pier\"") != std::string::npos);
    REQUIRE(text.find("\"Lake Post\"") != std::string::npos);
    REQUIRE(text.find("over the --outposts budget of 1") != std::string::npos);
    // no AIRFIELD / SEAPORT class authored: the header says the seed runs
    REQUIRE(text.find("so the runtime seed is ON for this") != std::string::npos);
    ParsedFile back(text, "ud_scaffold_notes.ext");
    const ParamEntry* zones = back.file.FindEntry("CfgGuerrillaZones")->FindEntry("Zones");
    REQUIRE(zones != nullptr);
    REQUIRE(zones->GetEntryCount() == 2); // the notes are comments, not classes
}

TEST_CASE("playRegion keeps towns and placed zones inside the theatre", "[game][guerrilla][scaffold]")
{
    Parsed names(kNames);
    std::vector<ScaffoldPoint> roads = StraightRoad();
    std::vector<ScaffoldPoint> buildings;
    AddCluster(buildings, 700, 700, 6);
    AddCluster(buildings, 2500, 700, 5);
    ScaffoldInputs in = MakeInputs(roads, buildings, names.file.FindEntry("Names"));
    in.hasRegion = true;
    in.regionMinX = 1500;
    in.regionMinZ = 0;
    in.regionMaxX = 5000;
    in.regionMaxZ = 5000;
    ScaffoldOptions opt;
    ScaffoldResult out;
    REQUIRE(BuildZones(in, opt, out));
    REQUIRE(out.cityCount == 1); // Town A at x = 700 is outside
    for (const ScaffoldZone& z : out.zones)
    {
        REQUIRE(z.x >= 1500.0f);
        REQUIRE(Str(z.name) != "Town A");
    }

    TemplateInfo info = MakeInfo();
    info.hasRegion = true;
    info.regionMinX = 1500;
    info.regionMaxX = 5000;
    info.regionMaxZ = 5000;
    std::string text = RenderDescriptionExt(info, out, opt);
    ParsedFile back(text, "ud_scaffold_region.ext");
    const ParamEntry* region = back.file.FindEntry("CfgGuerrillaZones")->FindEntry("playRegion");
    REQUIRE(region != nullptr);
    REQUIRE(region->GetSize() == 4);
    REQUIRE((float)(*region)[0] == 1500.0f);
    REQUIRE((float)(*region)[3] == 5000.0f);
}
