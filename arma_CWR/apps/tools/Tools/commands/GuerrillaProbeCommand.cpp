#include "GuerrillaProbeCommand.hpp"

#include "../PackageMount.hpp"

#include <Poseidon/Game/Guerrilla/RosterProbe.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <CLI/App.hpp>
#include <CLI/Option.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace PoseidonTools
{

namespace
{
using namespace Poseidon;
using namespace Poseidon::Guerrilla;

// Every CfgPatches class the mounted package declares. An addon's owner string is
// the first CfgPatches class of its config (AddonSystem::CheckAddonName), so this
// is the vocabulary owners are drawn from.
std::vector<RString> CollectPatchNames()
{
    std::vector<RString> out;
    const ParamEntry* patches = Pars.FindEntry("CfgPatches");
    if (!patches)
    {
        return out;
    }
    for (int i = 0; i < patches->GetEntryCount(); i++)
    {
        const ParamEntry& e = patches->GetEntry(i);
        if (e.IsClass())
        {
            out.push_back(e.GetName());
        }
    }
    return out;
}

// Every CfgVehicles class name the mounted package declares.
std::vector<RString> CollectVehicleNames()
{
    std::vector<RString> out;
    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    if (!vehicles)
    {
        return out;
    }
    for (int i = 0; i < vehicles->GetEntryCount(); i++)
    {
        const ParamEntry& e = vehicles->GetEntry(i);
        if (e.IsClass())
        {
            out.push_back(e.GetName());
        }
    }
    return out;
}

bool Contains(const std::vector<RString>& haystack, RString needle)
{
    for (const RString& s : haystack)
    {
        if (stricmp(s, needle) == 0)
        {
            return true;
        }
    }
    return false;
}

int RunProbe(const std::string& dataDir, const std::vector<std::string>& modDirs, const std::string& filter)
{
    if (modDirs.empty())
    {
        std::cerr << "Error: --mod is required; probe reports on the last --mod folder's roster\n";
        return 2;
    }

    // What does the LAST mod actually bring? Mount the stack without it, note
    // what is there, then mount the full stack: the difference is exactly that
    // mod's contribution, with no pbo-name guessing and no dependence on a pbo
    // being named after its CfgPatches class.
    //
    // Two diffs, because a mod contributes classes two ways. A pbo addon's
    // classes carry an owner (its first CfgPatches class), so the owner diff
    // takes the whole addon, including classes that merely override a base one.
    // A mod folder's bin/config.cpp carries NO owner - the deferred merge copies
    // an empty one - so those are caught by the class-name diff instead.
    std::vector<RString> baseOwners;
    std::vector<RString> baseVehicles;
    {
        std::vector<std::string> withoutLast(modDirs.begin(), modDirs.end() - 1);
        PackageMount base;
        std::string error;
        if (!base.Mount(dataDir, withoutLast, error))
        {
            std::cerr << "Error: " << error << "\n";
            return 2;
        }
        baseOwners = CollectPatchNames();
        baseVehicles = CollectVehicleNames();
    }

    PackageMount mount;
    std::string error;
    if (!mount.Mount(dataDir, modDirs, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

    RosterProbeOptions options;
    for (const RString& name : CollectPatchNames())
    {
        if (!Contains(baseOwners, name))
        {
            options.owners.push_back(name);
        }
    }
    for (const RString& name : CollectVehicleNames())
    {
        if (!Contains(baseVehicles, name))
        {
            options.classNames.push_back(name);
        }
    }
    if (!filter.empty())
    {
        options.filter = filter.c_str();
    }
    if (options.owners.empty() && options.classNames.empty())
    {
        std::cout << "PROBE-OWNERS none (the last --mod folder adds no addon and no CfgVehicles class)\n";
        std::cout << "SUMMARY probed=0 ok=0 fail=0\n";
        return 0;
    }
    std::cout << "PROBE-OWNERS";
    for (const RString& owner : options.owners)
    {
        std::cout << " " << (const char*)owner;
    }
    std::cout << "\nPROBE-NEWCLASSES " << options.classNames.size() << "\n";

    // The same existence test the BODY browser uses, against the mounted banks.
    auto fileExists = [](RString path) { return QIFStreamB::FileExist(path); };

    int ok = 0;
    int fail = 0;
    for (const RosterProbeResult& r : ProbeRoster(Pars.FindEntry("CfgVehicles"), options, fileExists))
    {
        if (r.ok)
        {
            ++ok;
            std::cout << "PROBE " << (const char*)r.className << " ok";
            if (!r.scriptRefs.empty())
            {
                std::cout << " scripts=" << r.scriptRefs.size();
            }
            std::cout << "\n";
        }
        else
        {
            ++fail;
            std::cout << "PROBE " << (const char*)r.className << " FAIL " << (const char*)r.reason << "\n";
        }
    }

    std::cout << "SUMMARY probed=" << (ok + fail) << " ok=" << ok << " fail=" << fail << "\n";
    return fail > 0 ? 1 : 0;
}

} // namespace

void GuerrillaProbeCommand::Setup(CLI::App& parent)
{
    auto* probe =
        parent.add_subcommand("probe", "Static spawn gate over the last --mod folder's CfgVehicles roster: every "
                                       "createable Man/Land class must author a model whose shape file the package "
                                       "ships, and every script its EventHandlers exec must exist");

    static std::string dataDir;
    static std::vector<std::string> modDirs;
    static std::string filter;

    probe->add_option("--data-dir", dataDir, "Game data package to mount")->required();
    probe->add_option("--mod", modDirs, "Mod folder to mount, repeatable; the LAST one is the one reported on")
        ->required();
    probe->add_option("--filter", filter, "Wildcard limiting which class names are probed");

    probe->callback([]() { std::exit(RunProbe(dataDir, modDirs, filter)); });
}

} // namespace PoseidonTools
