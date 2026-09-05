#include "GuerrillaLintCommand.hpp"

#include "../PackageMount.hpp"

#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>
#include <Poseidon/Game/Guerrilla/FactionSources.hpp>
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>

#include <CLI/App.hpp>
#include <CLI/Option.hpp>

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

int RunLint(const std::string& dataDir, const std::vector<std::string>& modDirs,
            const std::vector<std::string>& islandDirs)
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

    LintTotals totals;
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
        }
    }

    std::cout << "SUMMARY factions=" << totals.factions << " substituted=" << totals.substituted
              << " dropped=" << totals.dropped << " unresolvable=" << totals.unresolvable << "\n";
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

    lint->add_option("--data-dir", dataDir, "Game data package to mount")->required();
    lint->add_option("--mod", modDirs, "Mod folder to mount, repeatable, in -mod order");
    lint->add_option("--island", islandDirs,
                     "Guerrilla.<island> template directory whose description.ext block joins the table, "
                     "repeatable; with none, only the global table is linted");

    lint->callback([]() { std::exit(RunLint(dataDir, modDirs, islandDirs)); });
}

} // namespace PoseidonTools
