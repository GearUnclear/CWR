#include "GuerrillaScaffoldCommand.hpp"

#include "../PackageMount.hpp"

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>
#include <Poseidon/Game/Guerrilla/IslandScaffold.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/World/Terrain/WrpReader.hpp>

#include <CLI/App.hpp>
#include <CLI/Option.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// windows.h (pulled in transitively) macro-renames GetObject; WrpReader has one.
#ifdef GetObject
#undef GetObject
#endif

namespace PoseidonTools
{

namespace
{
using namespace Poseidon;
using namespace Poseidon::Guerrilla;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// height sampling
// ---------------------------------------------------------------------------

// The .wrp heightmap read through the same interpolation Landscape::SurfaceY
// uses, so a scaffolded elevation is the number the running engine reports for
// that spot rather than a bilinear approximation of it.
//
// Index mapping is not a guess: Landscape::LoadOptimized fills its grid with
// `SetData(x, z, heights[z * lx + x])` and the RVW path does the same, which is
// exactly WrpReader::GetHeight(x, z). World x maps to the first index, world z
// to the second, no flip.
class WrpHeightField final : public HeightField
{
  public:
    WrpHeightField(const WrpReader& reader, float landGrid) : _reader(&reader)
    {
        _gridX = reader.GetGridX();
        _gridZ = reader.GetGridZ();
        int terrainRange = reader.GetTerrainX() > 0 ? reader.GetTerrainX() : _gridX;
        // Landscape::SetLandGrid: terrainGrid = landGrid * landRange/terrainRange.
        // Every OFP-era world has terrainRange == landRange, so this is landGrid.
        _terrainGrid = landGrid * (float)_gridX / (float)terrainRange;
        _invTerrainGrid = _terrainGrid > 0.0f ? 1.0f / _terrainGrid : 0.02f;
    }

    float Height(float x, float z) const override
    {
        float xRel = x * _invTerrainGrid;
        float zRel = z * _invTerrainGrid;
        int xi = (int)floorf(xRel);
        int zi = (int)floorf(zRel);
        if (xi < 0 || zi < 0 || xi + 1 >= _gridX || zi + 1 >= _gridZ)
        {
            return kOutsideMap;
        }
        float xIn = xRel - (float)xi;
        float zIn = zRel - (float)zi;
        float y00 = _reader->GetHeight(xi, zi);
        float y01 = _reader->GetHeight(xi + 1, zi);
        float y10 = _reader->GetHeight(xi, zi + 1);
        float y11 = _reader->GetHeight(xi + 1, zi + 1);
        // the same two-triangle split as Landscape::SurfaceY
        if (xIn <= 1.0f - zIn)
        {
            return y00 + (y10 - y00) * zIn + (y01 - y00) * xIn;
        }
        float d1011 = y10 - y11;
        float d0111 = y01 - y11;
        return y10 + d0111 - d1011 * xIn - d0111 * zIn;
    }

    float TerrainGrid() const { return _terrainGrid; }
    float WorldSize() const { return (float)_gridX * _terrainGrid; }

  private:
    static constexpr float kOutsideMap = -100.0f; // Landscape's YOutsideMap
    const WrpReader* _reader;
    int _gridX = 0, _gridZ = 0;
    float _terrainGrid = 50.0f;
    float _invTerrainGrid = 0.02f;
};

// ---------------------------------------------------------------------------
// config probes
// ---------------------------------------------------------------------------

bool ClassExists(const char* bank, const char* className)
{
    const ParamEntry* cfg = Pars.FindEntry(bank);
    return cfg && cfg->FindEntry(className) != nullptr;
}

const ParamClass* FindVehicleClass(const char* className)
{
    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    const ParamEntry* e = vehicles ? vehicles->FindEntry(className) : nullptr;
    return e ? e->GetClassInterface() : nullptr;
}

// The player's unit class: the package's SoldierGB when it ships one, else the
// first Man-derived CfgVehicles class on side 2 (GUER), else the vanilla
// SoldierWB, which every OFP-derived data set carries.
RString PickPlayerClass(std::vector<std::string>& notes)
{
    if (ClassExists("CfgVehicles", "SoldierGB"))
    {
        return RString("SoldierGB");
    }
    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    const ParamClass* manClass = FindVehicleClass("Man");
    if (vehicles && manClass)
    {
        for (int i = 0; i < vehicles->GetEntryCount(); i++)
        {
            const ParamEntry& e = vehicles->GetEntry(i);
            if (!e.IsClass() || e.ReadValue("scope", 0.0f) < 2.0f)
            {
                continue;
            }
            const ParamClass* cls = e.GetClassInterface();
            if (e.ReadValue("side", -1.0f) != 2.0f || !cls || !cls->IsDerivedFrom(*manClass))
            {
                continue;
            }
            notes.push_back(std::string("no SoldierGB in this package; the player is the first GUER Man class, ") +
                            (const char*)e.GetName());
            return RString(e.GetName());
        }
    }
    notes.push_back("no SoldierGB and no GUER Man class in this package; falling back to SoldierWB");
    return RString("SoldierWB");
}

// The CfgPatches class that declares this world in its worlds[] - the name that
// belongs in mission.sqm addOns[]. Falls back to the world class's own owner
// (ParamEntry::GetOwner, the addon-visibility stamp), which is empty for a
// base-game world and therefore correctly contributes nothing.
RString WorldAddonName(const ParamEntry& worldEntry, const char* worldClass)
{
    const ParamEntry* patches = Pars.FindEntry("CfgPatches");
    if (patches)
    {
        for (int i = 0; i < patches->GetEntryCount(); i++)
        {
            const ParamEntry& patch = patches->GetEntry(i);
            const ParamEntry* worlds = patch.IsClass() ? patch.FindEntry("worlds") : nullptr;
            if (!worlds || !worlds->IsArray())
            {
                continue;
            }
            for (int k = 0; k < worlds->GetSize(); k++)
            {
                if (stricmp(RString((RStringB)(*worlds)[k]), worldClass) == 0)
                {
                    return RString(patch.GetName());
                }
            }
        }
    }
    return RString(worldEntry.GetOwner());
}

RString ClassOwner(const char* bank, const char* className)
{
    const ParamEntry* cfg = Pars.FindEntry(bank);
    const ParamEntry* cls = cfg ? cfg->FindEntry(className) : nullptr;
    return cls ? RString(cls->GetOwner()) : RString();
}

void AddUnique(std::vector<RString>& out, RString value)
{
    if (value.GetLength() == 0)
    {
        return;
    }
    for (const RString& v : out)
    {
        if (stricmp(v, value) == 0)
        {
            return;
        }
    }
    out.push_back(value);
}

std::string TodayIso()
{
    time_t now = time(nullptr);
    struct tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return buf;
}

// A stable seed per world, so re-running the tool never churns mission.sqm.
int SeedForWorld(const char* worldClass)
{
    unsigned h = 2166136261u;
    for (const char* p = worldClass; p && *p; p++)
    {
        h = (h ^ (unsigned char)tolower((unsigned char)*p)) * 16777619u;
    }
    return (int)(h % 90000000u) + 1000000;
}

bool WriteTextFile(const fs::path& path, const std::string& text, std::string& error)
{
    // binary: the templates are LF-terminated in the repo and must stay that way
    // through a Windows build of the tool.
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        error = "cannot write " + path.string();
        return false;
    }
    out.write(text.data(), (std::streamsize)text.size());
    if (!out)
    {
        error = "short write to " + path.string();
        return false;
    }
    return true;
}

std::string ReadTextFile(const fs::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return std::string();
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------------------
// the command
// ---------------------------------------------------------------------------

struct ScaffoldArgs
{
    std::string world;
    std::string dataDir;
    std::vector<std::string> mods;
    std::string outDir;
    int outposts = 3;
    bool keepZones = false;
    std::vector<float> playRegion; // empty or {minX, minZ, maxX, maxZ}
};

int CountAirSea(const std::vector<DetectAuthoredZone>& zones)
{
    int n = 0;
    for (const DetectAuthoredZone& z : zones)
    {
        if (stricmp((const char*)z.type, "AIRFIELD") == 0 || stricmp((const char*)z.type, "SEAPORT") == 0)
        {
            n++;
        }
    }
    return n;
}

int RunScaffold(const ScaffoldArgs& args)
{
    if (!args.playRegion.empty() && (args.playRegion.size() != 4 || args.playRegion[0] >= args.playRegion[2] ||
                                     args.playRegion[1] >= args.playRegion[3]))
    {
        std::cerr << "Error: --play-region takes minX minZ maxX maxZ with min < max\n";
        return 2;
    }
    const bool hasRegion = !args.playRegion.empty();

    // Resolve every path BEFORE mounting: PackageMount chdirs into the data
    // directory, so a relative --out would land inside the game install.
    std::error_code ec;
    fs::path outDir = fs::absolute(fs::path(args.outDir), ec);
    if (ec)
    {
        std::cerr << "Error: cannot resolve --out " << args.outDir << "\n";
        return 2;
    }
    std::string keptZones;
    if (args.keepZones)
    {
        std::string existing = ReadTextFile(outDir / "description.ext");
        if (existing.empty())
        {
            std::cout << "NOTE --keep-zones: no existing description.ext at " << outDir.string()
                      << ", generating a fresh one\n";
        }
        else if (!ExtractZonesBlock(existing, keptZones))
        {
            std::cerr << "Error: --keep-zones: the existing description.ext has no class Zones block to keep\n";
            return 2;
        }
        else
        {
            std::cout << "KEEP-ZONES splicing " << keptZones.size() << " bytes from the existing description.ext\n";
        }
    }

    PackageMount mount;
    std::string error;
    if (!mount.Mount(args.dataDir, args.mods, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

    // The kept block's zones are what the template authors, so they are what
    // the detector's covered rule must see (parsed with the preprocessor, the
    // way the engine reads description.ext).
    std::vector<DetectAuthoredZone> keptAuthored;
    if (!keptZones.empty())
    {
        ParamFile existingCfg;
        existingCfg.Parse((outDir / "description.ext").string().c_str());
        ReadTemplateZones(existingCfg.FindEntry("CfgGuerrillaZones"), keptAuthored);
    }

    ScaffoldWorld world;
    if (!world.Load(args.world, mount.DataDir(), error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }
    const ParamEntry& worldEntry = world.Entry();
    const ParamEntry* names = worldEntry.FindEntry("Names");
    if (!names)
    {
        std::cerr << "Error: CfgWorlds >> " << args.world
                  << " has no Names block, so it names no towns and cannot be scaffolded.\n";
        return 2;
    }
    RString worldClass = world.WorldClass();
    RString worldDisplay = world.WorldDisplay();
    std::cout << "WORLD " << (const char*)worldClass << " \"" << (const char*)worldDisplay << "\" " << world.WrpShown()
              << " (" << world.Summary() << ")\n";

    ScaffoldInputs in;
    in.worldClass = worldClass;
    in.worldDisplay = worldDisplay;
    in.names = names;
    in.height = &world.Height();
    in.worldSize = world.WorldSize();
    if (hasRegion)
    {
        in.hasRegion = true;
        in.regionMinX = args.playRegion[0];
        in.regionMinZ = args.playRegion[1];
        in.regionMaxX = args.playRegion[2];
        in.regionMaxZ = args.playRegion[3];
    }
    for (int i = 0; i < world.ObjectCount(); i++)
    {
        ScaffoldPoint p;
        p.x = world.ObjectX(i);
        p.z = world.ObjectZ(i);
        const char* model = world.ObjectModel(i);
        if (ModelIsRoad(model))
        {
            in.roads.push_back(p);
        }
        else if (ModelIsBuilding(model))
        {
            in.buildings.push_back(p);
        }
    }
    std::cout << "CLASSIFIED roads=" << in.roads.size() << " buildings=" << in.buildings.size() << "\n";

    // Pass 1: the camp and the towns alone. The camp does not depend on the
    // outposts, so these are the zones the second pass places again.
    ScaffoldOptions opt;
    opt.outposts = 0;
    ScaffoldResult firstPass;
    if (!BuildZones(in, opt, firstPass))
    {
        std::cerr << "Error: " << firstPass.error << "\n";
        return 2;
    }

    // Base detection (issue #76) against what the template will author: the
    // kept block, else the generated CAMP and CITY zones.
    BaseDetectInputs detectIn;
    world.FillBaseInputs(detectIn);
    if (hasRegion)
    {
        detectIn.hasRegion = true;
        detectIn.regionMinX = args.playRegion[0];
        detectIn.regionMinZ = args.playRegion[1];
        detectIn.regionMaxX = args.playRegion[2];
        detectIn.regionMaxZ = args.playRegion[3];
    }
    // An AIRFIELD that adopted a Names label ("Egyptian Airbase") makes that
    // label a base, not a town: the runtime CITY seed drops it by name, so
    // the generated CITY set drops it too. Probe with the camp alone, so the
    // town that is about to be dropped cannot cover the airfield.
    if (keptZones.empty())
    {
        BaseDetectInputs probeIn = detectIn;
        for (const ScaffoldZone& z : firstPass.zones)
        {
            if (stricmp((const char*)z.type, "CAMP") == 0)
            {
                probeIn.authored.push_back(DetectAuthoredZone{z.name, z.type, z.x, z.z});
            }
        }
        BaseDetectResult probe;
        DetectBases(probeIn, BaseDetectOptions(), probe);
        for (const BaseCandidate& c : probe.candidates)
        {
            if (stricmp((const char*)c.type, "AIRFIELD") != 0)
            {
                continue;
            }
            for (const ScaffoldZone& z : firstPass.zones)
            {
                if (stricmp((const char*)z.type, "CITY") == 0 && stricmp((const char*)z.name, (const char*)c.name) == 0)
                {
                    in.skipTownNames.push_back(z.name);
                }
            }
        }
        if (!in.skipTownNames.empty())
        {
            firstPass = ScaffoldResult();
            if (!BuildZones(in, opt, firstPass))
            {
                std::cerr << "Error: " << firstPass.error << "\n";
                return 2;
            }
        }
    }
    if (!keptZones.empty())
    {
        detectIn.authored = keptAuthored;
    }
    else
    {
        for (const ScaffoldZone& z : firstPass.zones)
        {
            DetectAuthoredZone a;
            a.name = z.name;
            a.type = z.type;
            a.x = z.x;
            a.z = z.z;
            detectIn.authored.push_back(a);
        }
    }
    BaseDetectOptions detectOpt;
    BaseDetectResult bases;
    DetectBases(detectIn, detectOpt, bases);

    // --outposts N is the OUTPOST total: detected outposts first, the
    // heuristic spread fills the rest and keeps clear of every detected base.
    const int detectedOutposts = DetectedOutpostsToEmit(bases, args.outposts);
    int offlineSeen = 0;
    for (const BaseCandidate& c : bases.candidates)
    {
        const bool hub = stricmp((const char*)c.type, "AIRFIELD") == 0 || stricmp((const char*)c.type, "SEAPORT") == 0;
        bool reserve = c.status == BaseStatus::Seed && hub;
        if (c.status == BaseStatus::Offline && stricmp((const char*)c.type, "OUTPOST") == 0)
        {
            reserve = offlineSeen < detectedOutposts;
            offlineSeen++;
        }
        if (reserve)
        {
            ScaffoldPoint p;
            p.x = c.x;
            p.z = c.z;
            in.reserved.push_back(p);
        }
    }
    opt.outposts = args.outposts - detectedOutposts;
    ScaffoldResult zones;
    if (!BuildZones(in, opt, zones))
    {
        std::cerr << "Error: " << zones.error << "\n";
        return 2;
    }
    MergeBaseCandidates(bases, args.outposts, zones);

    TemplateInfo info;
    info.worldClass = worldClass;
    info.worldDisplay = worldDisplay;
    info.date = RString(TodayIso().c_str());
    info.wrpPath = RString(world.WrpShown());
    info.randomSeed = SeedForWorld((const char*)worldClass);
    info.outposts = args.outposts;
    info.roadPoints = (int)in.roads.size();
    info.buildingPoints = (int)in.buildings.size();
    info.keptBaseZones = CountAirSea(keptAuthored);
    if (hasRegion)
    {
        info.hasRegion = true;
        info.regionMinX = args.playRegion[0];
        info.regionMinZ = args.playRegion[1];
        info.regionMaxX = args.playRegion[2];
        info.regionMaxZ = args.playRegion[3];
    }

    std::vector<std::string> notes;
    info.playerClass = PickPlayerClass(notes);
    AddUnique(info.addOns, WorldAddonName(worldEntry, (const char*)worldClass));
    AddUnique(info.addOns, ClassOwner("CfgVehicles", (const char*)info.playerClass));

    // The CIV descriptor is package-probed, not assumed: Civilian2..Civilian15
    // are Resistance-era classes a bare 1.99 core config does not carry, and a
    // mod's civilians have entirely different names.
    for (int i = 1; i <= 10; i++)
    {
        std::string name = i == 1 ? "Civilian" : ("Civilian" + std::to_string(i));
        if (ClassExists("CfgVehicles", name.c_str()))
        {
            info.civClasses.push_back(RString(name.c_str()));
        }
    }
    // Ambient-traffic hulls: the stock civilian car roster, minus whatever this
    // package lacks. A mod's own vans have to be added by hand - nothing in the
    // config marks a hull as "civilian traffic material".
    static const char* kCivVehicleCandidates[] = {"Skoda",   "SkodaBlue", "SkodaRed", "SkodaGreen", "Rapid",
                                                  "Trabant", "Mini",      "Bus",      "Tractor"};
    for (const char* candidate : kCivVehicleCandidates)
    {
        if (ClassExists("CfgVehicles", candidate))
        {
            info.civVehicles.push_back(RString(candidate));
        }
    }

    std::string descriptionExt = RenderDescriptionExt(info, zones, opt, keptZones);
    std::string missionSqm = RenderMissionSqm(info, zones);

    fs::create_directories(outDir, ec);
    if (ec)
    {
        std::cerr << "Error: cannot create " << outDir.string() << ": " << ec.message() << "\n";
        return 2;
    }
    // With a kept Zones block the generated zones are NOT what ends up in the
    // file, so the player start derived from the generated camp would drift away
    // from the kept one. Leave an existing mission.sqm alone in that case; write
    // it only when there is none.
    const bool keptBlock = !keptZones.empty();
    const bool sqmExists = fs::exists(outDir / "mission.sqm", ec);
    const bool writeSqm = !keptBlock || !sqmExists;
    if (!WriteTextFile(outDir / "description.ext", descriptionExt, error) ||
        (writeSqm && !WriteTextFile(outDir / "mission.sqm", missionSqm, error)) ||
        !WriteTextFile(outDir / "init.sqs", InitSqsText(), error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

    for (const std::string& note : notes)
    {
        std::cout << "NOTE " << note << "\n";
    }
    for (const std::string& warning : zones.warnings)
    {
        std::cout << "WARN " << warning << "\n";
    }
    for (const std::string& note : bases.notes)
    {
        std::cout << "BASENOTE " << note << "\n";
    }
    for (const BaseCandidate& c : bases.candidates)
    {
        printf("BASE %s %s %s %.0f %.0f conf=%.2f\n", BaseStatusName(c.status), (const char*)c.type,
               (const char*)c.name, c.x, c.z, c.confidence);
        if (c.status == BaseStatus::Covered)
        {
            printf("  covered-by: %s\n", (const char*)c.coveredBy);
        }
        for (const std::string& e : c.evidence)
        {
            printf("  evidence: %s\n", e.c_str());
        }
    }
    fflush(stdout);
    std::cout << "PLAYER " << (const char*)info.playerClass << " side=" << (const char*)info.playerSide << " at "
              << (int)zones.playerX << "," << (int)zones.playerZ << "\n";
    std::cout << "ADDONS";
    for (const RString& addon : info.addOns)
    {
        std::cout << " " << (const char*)addon;
    }
    if (info.addOns.empty())
    {
        std::cout << " (none: base-game world and player)";
    }
    std::cout << "\nCIV classes=" << info.civClasses.size() << " vehicles=" << info.civVehicles.size() << "\n";
    if (keptBlock)
    {
        std::cout << "NOTE --keep-zones: the zones listed below were computed but NOT written; the existing class "
                     "Zones block was kept\n";
    }
    std::cout.flush();
    for (const ScaffoldZone& z : zones.zones)
    {
        printf("ZONE %-9s %-10s %-24s %6.0f %6.0f %7.1f\n", (const char*)z.type, (const char*)z.owner,
               (const char*)z.name, z.x, z.z, z.y);
    }
    fflush(stdout);
    std::cout << "SUMMARY zones=" << zones.zones.size() << " cities=" << zones.cityCount
              << " outposts=" << zones.outpostCount << " (detected " << detectedOutposts << ")"
              << " bases=" << zones.baseCount << " warnings=" << zones.warnings.size() << "\n";
    std::cout << "WROTE " << outDir.string() << " (description.ext, "
              << (writeSqm ? "mission.sqm, " : "mission.sqm left as it was, ") << "init.sqs)\n";
    return 0;
}

} // namespace

// ---------------------------------------------------------------------------
// ScaffoldWorld
// ---------------------------------------------------------------------------

struct ScaffoldWorld::Impl
{
    const ParamEntry* entry = nullptr;
    RString worldClass;
    RString worldDisplay;
    std::string wrpShown;
    WrpReader reader;
    std::unique_ptr<WrpHeightField> height;
};

ScaffoldWorld::ScaffoldWorld() : _impl(std::make_unique<Impl>()) {}
ScaffoldWorld::~ScaffoldWorld() = default;

bool ScaffoldWorld::Load(const std::string& world, const std::string& dataDir, std::string& error)
{
    const ParamEntry* worlds = Pars.FindEntry("CfgWorlds");
    const ParamEntry* worldEntry = worlds ? worlds->FindEntry(world.c_str()) : nullptr;
    if (!worldEntry)
    {
        error = "CfgWorlds >> " + world + " not found in this package.";
        if (worlds)
        {
            error += "\nWorlds this package declares:";
            for (int i = 0; i < worlds->GetEntryCount(); i++)
            {
                const ParamEntry& e = worlds->GetEntry(i);
                if (e.IsClass() && e.FindEntry("worldName"))
                {
                    error += " ";
                    error += (const char*)e.GetName();
                }
            }
        }
        return false;
    }

    Impl& d = *_impl;
    d.entry = worldEntry;
    d.worldClass = worldEntry->GetName();
    d.worldDisplay = worldEntry->ReadValue("description", d.worldClass);
    RString worldName = worldEntry->ReadValue("worldName", RString());
    float landGrid = worldEntry->ReadValue("landGrid", 50.0f);
    if (worldName.GetLength() == 0)
    {
        error = "CfgWorlds >> " + world + " has no worldName, so its .wrp cannot be located";
        return false;
    }

    // Bank path first (a mod world lives inside a pbo), then the loose
    // <DataDir>/Worlds/<name>.wrp a stock install ships.
    RString wrpName = GetDefaultName(worldName, "worlds\\", ".wrp");
    {
        QIFStreamB f;
        f.AutoOpen(wrpName);
        if (!f.fail() && d.reader.Load(f))
        {
            d.wrpShown = (const char*)wrpName;
        }
    }
    if (d.wrpShown.empty())
    {
        fs::path loose = fs::path(dataDir) / "Worlds" / (std::string((const char*)d.worldClass) + ".wrp");
        if (d.reader.Load(loose.string().c_str()))
        {
            d.wrpShown = loose.string();
        }
    }
    if (d.wrpShown.empty())
    {
        error = std::string("cannot read the world's terrain (tried the mounted banks for ") + (const char*)wrpName +
                " and <data-dir>/Worlds/" + (const char*)d.worldClass + ".wrp)";
        return false;
    }
    d.height = std::make_unique<WrpHeightField>(d.reader, landGrid);
    return true;
}

const ParamEntry& ScaffoldWorld::Entry() const
{
    return *_impl->entry;
}

const char* ScaffoldWorld::WorldClass() const
{
    return _impl->worldClass;
}

const char* ScaffoldWorld::WorldDisplay() const
{
    return _impl->worldDisplay;
}

const char* ScaffoldWorld::WrpShown() const
{
    return _impl->wrpShown.c_str();
}

const HeightField& ScaffoldWorld::Height() const
{
    return *_impl->height;
}

float ScaffoldWorld::WorldSize() const
{
    return _impl->height->WorldSize();
}

std::string ScaffoldWorld::Summary() const
{
    std::ostringstream ss;
    ss << _impl->reader.GetFormatName() << ", " << _impl->reader.GetGridX() << "x" << _impl->reader.GetGridZ() << " at "
       << _impl->height->TerrainGrid() << " m, " << _impl->reader.GetObjectCount() << " objects";
    return ss.str();
}

int ScaffoldWorld::ObjectCount() const
{
    return _impl->reader.GetObjectCount();
}

const char* ScaffoldWorld::ObjectModel(int i) const
{
    return _impl->reader.GetObject(i).name;
}

float ScaffoldWorld::ObjectX(int i) const
{
    return _impl->reader.GetObject(i).position.X();
}

float ScaffoldWorld::ObjectZ(int i) const
{
    return _impl->reader.GetObject(i).position.Z();
}

void ScaffoldWorld::FillBaseInputs(BaseDetectInputs& in) const
{
    const Impl& d = *_impl;
    in.worldClass = d.worldClass;
    in.worldDisplay = d.worldDisplay;
    in.ils.clear();
    ReadIlsEntries(*d.entry, in.ils);
    in.height = d.height.get();
    in.worldSize = d.height->WorldSize();

    in.objects.clear();
    for (int i = 0; i < d.reader.GetObjectCount(); i++)
    {
        const WrpObjectInfo& obj = d.reader.GetObject(i);
        BaseModelClass cls = ClassifyBaseModel(obj.name);
        if (cls.harbour <= 0.0f && cls.military <= 0.0f && cls.aviation <= 0.0f && !ModelIsBuilding(obj.name))
        {
            continue;
        }
        DetectObject o;
        o.x = obj.position.X();
        o.z = obj.position.Z();
        o.model = RString(obj.name);
        o.radius = 0.0f; // unknown offline: the detector assumes 8 m
        in.objects.push_back(o);
    }

    in.towns.clear();
    const ParamEntry* names = d.entry->FindEntry("Names");
    if (names)
    {
        for (int i = 0; i < names->GetEntryCount(); i++)
        {
            RString name;
            Vector3 pos;
            if (!ZoneRegistry::NamesEntryIsTown(names->GetEntry(i), name, pos))
            {
                continue;
            }
            DetectPlace p;
            p.name = name;
            p.x = pos.X();
            p.z = pos.Z();
            in.towns.push_back(p);
        }
    }
}

void GuerrillaScaffoldCommand::Setup(CLI::App& parent)
{
    auto* scaffold = parent.add_subcommand(
        "scaffold",
        "Generate a Guerrilla Mode island template (description.ext + mission.sqm + init.sqs) for one CfgWorlds "
        "world: CITY zones from the world's own Names entries that sit on dry land with houses around them, the "
        "AIRFIELD / SEAPORT / OUTPOST bases the detector finds (ILS table, harbour and military object clusters; "
        "printed as BASE lines with their evidence), plus a resistance CAMP and occupier OUTPOSTs placed off-road "
        "on flat dry ground, every elevation sampled from the world's .wrp");

    static ScaffoldArgs args;

    scaffold->add_option("--world", args.world, "CfgWorlds class of the world to scaffold (e.g. Abel, Sinai)")
        ->required();
    scaffold->add_option("--data-dir", args.dataDir, "Game data package to mount (the directory holding bin/, addons/)")
        ->required();
    scaffold->add_option("--mod", args.mods,
                         "Mod folder to mount, repeatable and in -mod order (first listed = lowest priority)");
    scaffold->add_option("--out", args.outDir, "Output mission directory; the convention is <...>/Guerrilla.<world>")
        ->required();
    scaffold
        ->add_option("--outposts", args.outposts,
                     "OUTPOST zones in total (default 3): detected outposts (military object clusters) first, in "
                     "the detector's order, then heuristically placed ones fill the rest; detected outposts beyond "
                     "N are written as commented-out notes")
        ->check(CLI::Range(0, 32))
        ->default_val(3);
    scaffold->add_flag("--keep-zones", args.keepZones,
                       "Keep the class Zones block of an existing <out>/description.ext verbatim instead of "
                       "generating a new one, so hand edits survive a re-run; the detector then judges its "
                       "candidates against the kept zones");
    scaffold
        ->add_option("--play-region", args.playRegion,
                     "The theatre, minX minZ maxX maxZ in metres (engine x = easting, z = northing): written as "
                     "playRegion[]; towns, the camp and the outposts are placed inside it and detected bases "
                     "outside it are reported as outside-region")
        ->expected(4);

    scaffold->callback([]() { std::exit(RunScaffold(args)); });
}

} // namespace PoseidonTools
