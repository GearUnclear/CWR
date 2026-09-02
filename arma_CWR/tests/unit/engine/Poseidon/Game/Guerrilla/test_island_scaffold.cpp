// Island-template scaffolding (issue #54 C2): the pure half of
// `PoseidonTools guerrilla scaffold`. Everything here runs against synthetic
// terrain - a flat plateau with one lake, one straight road and a few house
// clusters - so the placement rules are pinned without a .wrp or a data package.

#include <catch2/catch_test_macros.hpp>

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
