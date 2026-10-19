#include "GuerrillaLintCommand.hpp"

#include "../PackageMount.hpp"
#include "GuerrillaScaffoldCommand.hpp"

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>
#include <Poseidon/Game/Guerrilla/FactionSources.hpp>
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>

#include <CLI/App.hpp>
#include <CLI/Option.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace PoseidonTools
{

namespace
{
namespace fs = std::filesystem;
using namespace Poseidon;
using namespace Poseidon::Guerrilla;

const char* const kWarSides[] = {"WEST", "EAST", "GUER"};

RString Dash(RString s)
{
    return s.GetLength() > 0 ? s : RString("-");
}

const char* OriginName(FactionSources::Origin origin)
{
    return origin == FactionSources::Origin::Island ? "Island" : "Global";
}

// Find a faction record by class name; the two loads share the descriptor order,
// but matching by name survives a pass that appends or reorders records.
const FactionRecord* FindByName(const ZoneRegistry& reg, RString name)
{
    for (int i = 0; i < reg.NFactions(); i++)
    {
        const FactionRecord* f = reg.GetFaction(i);
        if (f && stricmp(f->className, name) == 0)
        {
            return f;
        }
    }
    return nullptr;
}

struct LintTotals
{
    int factions = 0;
    int substituted = 0;
    int dropped = 0;
    int unresolvable = 0;
};

// Audit one merged faction table. `label` names the source (an island template or
// the global config) so a multi-island run stays greppable.
void LintTable(const std::string& label, const FactionSources& sources, LintTotals& totals)
{
    const ParamEntry* factionsCfg = sources.Factions();
    std::cout << "== TABLE " << label << " ==\n";
    if (!factionsCfg)
    {
        std::cout << "TABLE " << label << " empty (no CfgGuerrillaFactions in either source)\n";
        return;
    }

    // The same descriptor loaded twice: once as authored, once through the
    // plan-15 resolution pass. The diff between them IS the audit.
    ZoneRegistry rawReg;
    rawReg.LoadFromParams(nullptr, factionsCfg, nullptr, nullptr, nullptr, nullptr, nullptr);

    ParsClassProbe realProbe;
    RecordingClassProbe probe(realProbe);
    ZoneRegistry resolvedReg;
    resolvedReg.LoadFromParams(nullptr, factionsCfg, nullptr, nullptr, nullptr, &probe, nullptr);

    for (int i = 0; i < resolvedReg.NFactions(); i++)
    {
        const FactionRecord* resolved = resolvedReg.GetFaction(i);
        if (!resolved)
        {
            continue;
        }
        const FactionRecord* raw = FindByName(rawReg, resolved->className);
        if (!raw)
        {
            continue;
        }
        totals.factions++;

        const FactionSources::Record* rec = sources.FindRecord(resolved->className);
        std::cout << "FACTION " << (const char*)resolved->className << " side=" << (const char*)Dash(resolved->side)
                  << " origin=" << (rec ? OriginName(rec->origin) : "?")
                  << " owner=" << (rec ? (const char*)Dash(rec->owner) : "-")
                  << " overrodeGlobal=" << (rec && rec->overrodeGlobal ? 1 : 0) << "\n";

        for (const LintFinding& f : DiffFactionRecord(*raw, *resolved))
        {
            const char* outcome =
                (f.outcome == LintOutcome::Substituted) ? (const char*)f.substitute : ToString(f.outcome);
            std::cout << "LINT " << (const char*)resolved->className << " " << (const char*)f.key << " "
                      << (const char*)f.value << " -> " << outcome << "\n";
            if (f.outcome == LintOutcome::Substituted)
            {
                totals.substituted++;
            }
            else if (f.outcome == LintOutcome::Dropped)
            {
                totals.dropped++;
            }
        }

        // The sideTwin chain: which class this roster becomes when the campaign
        // has to move it onto (or off) a side.
        for (const char* side : kWarSides)
        {
            RString on = TwinOnSide(factionsCfg, resolved->className, side);
            RString off = TwinOffSide(factionsCfg, resolved->className, side);
            std::cout << "TWIN " << (const char*)resolved->className << " " << side << " on=" << (const char*)Dash(on)
                      << " off=" << (const char*)Dash(off) << "\n";
        }

        // The menu-time verdict, the gate a player would actually hit.
        RString issue = GuerrillaFactionIssue(factionsCfg, resolved->className, realProbe);
        bool isCiv = stricmp(resolved->side, "CIV") == 0;
        if (issue.GetLength() > 0 && !isCiv)
        {
            totals.unresolvable++;
            std::cout << "ISSUE " << (const char*)resolved->className << " UNRESOLVABLE " << (const char*)issue << "\n";
        }
        else if (issue.GetLength() > 0)
        {
            std::cout << "ISSUE " << (const char*)resolved->className << " civ-exempt " << (const char*)issue << "\n";
        }
        else
        {
            std::cout << "ISSUE " << (const char*)resolved->className << " ok\n";
        }
    }

    for (const RString& miss : probe.Misses())
    {
        std::cout << "PROBEMISS " << label << " " << (const char*)miss << "\n";
    }
}

// The world a template plays on: --world when given, else the folder's
// extension (Guerrilla.Abel -> Abel, the install convention).
std::string TemplateWorld(const std::string& island, const std::string& worldOverride)
{
    if (!worldOverride.empty())
    {
        return worldOverride;
    }
    std::string folder = fs::path(island).filename().string();
    size_t dot = folder.rfind('.');
    return dot == std::string::npos ? std::string() : folder.substr(dot + 1);
}

struct BaseTotals
{
    int covered = 0;
    int missing = 0;
    int seeded = 0;
    int skipped = 0;
};

// issue #76: the detector's candidates for the template's world, judged
// against the zones the template authors. Returns false on a world that
// cannot be read (reported, counted as an error by the caller).
bool LintTemplateBasesFor(const std::string& label, const std::string& world, const std::string& dataDir,
                          const ParamEntry* zonesCfg, BaseTotals& totals)
{
    std::cout << "== BASES " << label << " world=" << world << " ==\n";
    std::string error;
    ScaffoldWorld scan;
    if (world.empty() || !scan.Load(world, dataDir, error))
    {
        std::cout << "BASES " << label << " ERROR " << (world.empty() ? "no world (pass --world)" : error) << "\n";
        return false;
    }

    std::vector<DetectAuthoredZone> authored;
    ReadTemplateZones(zonesCfg, authored);
    SeedBasesSetting seed = ResolveSeedBases(zonesCfg, authored);

    BaseDetectInputs in;
    scan.FillBaseInputs(in);
    in.authored = authored;
    // the runtime's theatre: the authored playRegion[], else the authored
    // zones' bounding box padded by 5 km
    bool regionIsDefault = false;
    in.hasRegion = ResolvePlayRegion(zonesCfg, authored, in.regionMinX, in.regionMinZ, in.regionMaxX, in.regionMaxZ,
                                     &regionIsDefault);
    BaseDetectOptions opt;
    BaseDetectResult result;
    DetectBases(in, opt, result);

    std::cout << FormatSeedBasesRow(label.c_str(), seed) << " authored=" << authored.size()
              << " candidates=" << result.candidates.size();
    if (in.hasRegion)
    {
        fflush(stdout);
        std::cout.flush();
        printf(" playRegion={%.0f, %.0f, %.0f, %.0f} (%s)", in.regionMinX, in.regionMinZ, in.regionMaxX, in.regionMaxZ,
               regionIsDefault ? "default: authored zones + 5 km" : "authored");
        fflush(stdout);
    }
    std::cout << "\n";
    for (const std::string& note : result.notes)
    {
        std::cout << "BASENOTE " << label << " " << note << "\n";
    }
    std::vector<BaseLintRow> rows = LintTemplateBases(result, authored, seed);
    for (size_t i = 0; i < rows.size(); i++)
    {
        const BaseLintRow& r = rows[i];
        std::cout << "BASE " << label << " " << r.line << "\n";
        for (const std::string& e : result.candidates[i].evidence)
        {
            std::cout << "  evidence: " << e << "\n";
        }
        if (r.verdict == "COVERED")
        {
            totals.covered++;
        }
        else if (r.verdict == "SKIPPED")
        {
            totals.skipped++;
        }
        else
        {
            totals.missing++;
            totals.seeded += r.seeded ? 1 : 0;
        }
    }
    return true;
}

int RunLint(const std::string& dataDir, const std::vector<std::string>& modDirs,
            const std::vector<std::string>& islandDirs, bool bases, const std::string& worldOverride)
{
    // Absolute before the mount: PackageMount chdirs into the data dir, and an
    // island template path is read after that.
    std::error_code ec;
    std::vector<std::string> islands;
    for (const std::string& island : islandDirs)
    {
        fs::path abs = fs::absolute(fs::path(island), ec);
        if (ec || !fs::is_directory(abs, ec))
        {
            std::cerr << "Error: not an island template directory: " << island << "\n";
            return 2;
        }
        islands.push_back(abs.string());
    }

    PackageMount mount;
    std::string error;
    if (!mount.Mount(dataDir, modDirs, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

    if (bases && islands.empty())
    {
        std::cerr << "Error: --bases needs at least one --island template\n";
        return 2;
    }

    LintTotals totals;
    BaseTotals baseTotals;
    int baseErrors = 0;
    const ParamEntry* globalCfg = Pars.FindEntry("CfgGuerrillaFactions");

    if (islands.empty())
    {
        FactionSources sources;
        sources.Build(globalCfg, nullptr);
        LintTable("(global)", sources, totals);
    }
    else
    {
        for (const std::string& island : islands)
        {
            ParamFile islandCfg;
            std::string desc = (fs::path(island) / "description.ext").string();
            if (!fs::is_regular_file(fs::path(desc), ec))
            {
                std::cerr << "Error: no description.ext in " << island << "\n";
                return 2;
            }
            islandCfg.Parse(desc.c_str());
            FactionSources sources;
            sources.Build(globalCfg, islandCfg.FindEntry("CfgGuerrillaFactions"));
            LintTable(fs::path(island).filename().string(), sources, totals);
            if (bases &&
                !LintTemplateBasesFor(fs::path(island).filename().string(), TemplateWorld(island, worldOverride),
                                      mount.DataDir(), islandCfg.FindEntry("CfgGuerrillaZones"), baseTotals))
            {
                baseErrors++;
            }
        }
    }

    std::cout << "SUMMARY factions=" << totals.factions << " substituted=" << totals.substituted
              << " dropped=" << totals.dropped << " unresolvable=" << totals.unresolvable << "\n";
    if (bases)
    {
        // advisory: a missing base is the author's call, never a failure
        std::cout << "SUMMARY bases covered=" << baseTotals.covered << " missing=" << baseTotals.missing
                  << " (seeded at runtime " << baseTotals.seeded << ") skipped=" << baseTotals.skipped
                  << " errors=" << baseErrors << "\n";
    }
    if (baseErrors > 0)
    {
        return 2;
    }
    return totals.unresolvable > 0 ? 1 : 0;
}

} // namespace

void GuerrillaLintCommand::Setup(CLI::App& parent)
{
    auto* lint = parent.add_subcommand("lint", "Audit CfgGuerrillaFactions against what the mounted package can field: "
                                               "report every descriptor key the plan-15 resolution pass substitutes or "
                                               "drops, and fail when a faction's tiers[0] cannot resolve");

    static std::string dataDir;
    static std::vector<std::string> modDirs;
    static std::vector<std::string> islandDirs;
    static bool bases = false;
    static std::string world;

    lint->add_option("--data-dir", dataDir, "Game data package to mount")->required();
    lint->add_option("--mod", modDirs, "Mod folder to mount, repeatable, in -mod order");
    lint->add_option("--island", islandDirs,
                     "Guerrilla.<island> template directory whose description.ext block joins the table, "
                     "repeatable; with none, only the global table is linted");
    lint->add_flag("--bases", bases,
                   "Also audit each --island template's occupation network (issue #76): run the base detector "
                   "on the template's world (.wrp objects and the CfgWorlds ILS table) against the template's "
                   "authored zones, and print one BASE row per candidate: COVERED (the authored zone that "
                   "already models it, and how far), MISSING (not authored; for an AIRFIELD / SEAPORT whether "
                   "the runtime seed adds it under the template's seedBases, an OUTPOST is never seeded) or "
                   "SKIPPED (outside playRegion[] or low confidence). Advisory: missing bases never fail the run; "
                   "a world that cannot be read exits 2");
    lint->add_option("--world", world,
                     "With --bases: the CfgWorlds class every --island plays on; default is the template folder's "
                     "extension (Guerrilla.Abel -> Abel)");

    lint->callback([]() { std::exit(RunLint(dataDir, modDirs, islandDirs, bases, world)); });
}

} // namespace PoseidonTools
