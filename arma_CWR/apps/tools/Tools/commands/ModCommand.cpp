#include "ModCommand.hpp"

#include <Poseidon/Asset/Addon/ModDoctor.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#include <CLI/App.hpp>
#include <CLI/Option.hpp>

namespace PoseidonTools
{

namespace
{

namespace fs = std::filesystem;
using namespace Poseidon::ModDoctor;

//! `addons` is spelled every possible way across mods; take whichever exists.
fs::path FindAddonsDir(const fs::path& modDir)
{
    std::error_code ec;
    for (const fs::directory_entry& entry : fs::directory_iterator(modDir, ec))
    {
        if (!entry.is_directory(ec))
        {
            continue;
        }
        std::string name = entry.path().filename().string();
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return static_cast<char>(::tolower(c)); });
        if (name == "addons")
        {
            return entry.path();
        }
    }
    return fs::path();
}

bool ReadWholeFile(const fs::path& path, std::vector<char>& out)
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in)
    {
        return false;
    }
    std::streamoff size = in.tellg();
    if (size < 0)
    {
        return false;
    }
    out.resize(static_cast<size_t>(size));
    in.seekg(0, std::ios::beg);
    if (size > 0 && !in.read(out.data(), size))
    {
        return false;
    }
    return true;
}

void PrintFinding(const Finding& finding, const char* indent)
{
    std::cout << indent << ToString(finding.defect) << "  " << finding.entry;
    if (finding.line > 0)
    {
        std::cout << ":" << finding.line;
    }
    if (!finding.detail.empty())
    {
        std::cout << "  " << finding.detail;
    }
    if (!finding.patchable)
    {
        std::cout << "  [not patchable]";
    }
    std::cout << "\n";
    // The model patch is four raw float bytes; its numbers are already in detail.
    if (finding.defect != DefectClass::BuriedModelOrigin)
    {
        for (const Patch& patch : finding.patches)
        {
            std::cout << indent << "    patch @" << patch.offset << ": '" << patch.original << "' -> '"
                      << patch.replacement << "'\n";
        }
    }
    for (const Finding& child : finding.children)
    {
        PrintFinding(child, "        ");
    }
}

//! Total patch sites a finding plans, children included.
int CountPatches(const Finding& finding)
{
    int n = static_cast<int>(finding.patches.size());
    for (const Finding& child : finding.children)
    {
        n += CountPatches(child);
    }
    return n;
}

int RunDoctor(const std::string& modPath, bool fix, const std::string& pboFilter)
{
    std::error_code ec;
    fs::path modDir(modPath);
    if (!fs::is_directory(modDir, ec))
    {
        std::cerr << "Error: not a directory: " << modPath << "\n";
        return 2;
    }
    fs::path addons = FindAddonsDir(modDir);
    if (addons.empty())
    {
        std::cerr << "Error: no addons directory under " << modPath << "\n";
        return 2;
    }

    std::vector<fs::path> pbos;
    for (const fs::directory_entry& entry : fs::directory_iterator(addons, ec))
    {
        if (!entry.is_regular_file(ec))
        {
            continue;
        }
        std::string name = entry.path().filename().string();
        if (!WildcardMatch("*.pbo", name.c_str()))
        {
            continue;
        }
        if (!pboFilter.empty() && !WildcardMatch(pboFilter.c_str(), name.c_str()))
        {
            continue;
        }
        pbos.push_back(entry.path());
    }
    std::sort(pbos.begin(), pbos.end());

    const fs::path backupDir = modDir / "_ud-orig";
    int defectFiles = 0;
    int defectSites = 0;
    int unpatchable = 0;
    int patchedFiles = 0;
    int patchedSites = 0;
    int ioErrors = 0;

    for (const fs::path& pbo : pbos)
    {
        std::vector<char> bytes;
        if (!ReadWholeFile(pbo, bytes) || bytes.empty())
        {
            std::cerr << "Error: cannot read " << pbo.string() << "\n";
            ++ioErrors;
            continue;
        }

        std::string error;
        std::vector<Finding> findings = ScanPbo(bytes.data(), bytes.size(), error);
        if (!error.empty())
        {
            std::cerr << "Warning: " << pbo.filename().string() << ": " << error << ", skipped\n";
            continue;
        }
        if (findings.empty())
        {
            continue;
        }

        std::cout << pbo.filename().string() << ":\n";
        int sitesHere = 0;
        for (const Finding& finding : findings)
        {
            PrintFinding(finding, "    ");
            if (!finding.patchable)
            {
                ++unpatchable;
                continue;
            }
            sitesHere += CountPatches(finding);
        }
        // An entry we simply cannot read is a note, not a defect, so it must not
        // move the exit code on its own.
        if (sitesHere > 0)
        {
            defectSites += sitesHere;
            ++defectFiles;
        }

        bool anyPatch = false;
        for (const Finding& finding : findings)
        {
            anyPatch = anyPatch || HasPatches(finding);
        }
        if (!fix || !anyPatch)
        {
            continue;
        }

        // The scripts' convention: keep the untouched original once, the first
        // time this mod is repaired.
        fs::create_directories(backupDir, ec);
        fs::path backup = backupDir / pbo.filename();
        if (!fs::exists(backup, ec))
        {
            fs::copy_file(pbo, backup, ec);
            if (ec)
            {
                std::cerr << "Error: cannot back up " << pbo.filename().string() << ": " << ec.message() << "\n";
                ++ioErrors;
                continue;
            }
            std::cout << "    backed up original -> " << backup.string() << "\n";
        }

        int applied = ApplyPatches(bytes.data(), bytes.size(), findings);
        if (applied == 0)
        {
            continue;
        }

        // Some mods ship read-only pbos (they came off a 2006 CD image).
        fs::permissions(pbo, fs::perms::owner_write, fs::perm_options::add, ec);
        std::ofstream out(pbo, std::ios::binary | std::ios::trunc);
        if (!out || !out.write(bytes.data(), static_cast<std::streamsize>(bytes.size())))
        {
            std::cerr << "Error: cannot write " << pbo.string() << "\n";
            ++ioErrors;
            continue;
        }
        out.close();
        ++patchedFiles;
        patchedSites += applied;
        std::cout << "    patched " << applied << " site(s) in place\n";
    }

    if (ioErrors > 0)
    {
        return 2;
    }
    if (defectFiles == 0)
    {
        std::cout << "Nothing to do: no known defect found in " << pbos.size() << " pbo(s)"
                  << (unpatchable ? " (" + std::to_string(unpatchable) + " entry/entries not readable)" : "") << ".\n";
        return 0;
    }
    if (!fix)
    {
        std::cout << "Found " << defectSites << " patch site(s) in " << defectFiles << " pbo(s)"
                  << (unpatchable ? ", plus " + std::to_string(unpatchable) + " not patchable" : "")
                  << ". Rerun with --fix to apply.\n";
        return 1;
    }
    std::cout << "Patched " << patchedSites << " site(s) in " << patchedFiles << " pbo(s). Originals kept in "
              << backupDir.string() << ".\n";
    return unpatchable > 0 ? 1 : 0;
}

} // namespace

void ModCommand::Setup(CLI::App& app)
{
    auto* mod = app.add_subcommand("mod", "Third-party mod maintenance");
    mod->require_subcommand(1);

    auto* doctor =
        mod->add_subcommand("doctor", "Scan a mod folder's addons for known content defects (undefined scope "
                                      "keyword, malformed float literal, buried model origin) and optionally "
                                      "repair them with same-length in-place byte patches");

    static std::string modPath;
    static std::string pboFilter;
    static bool fix = false;

    doctor->add_option("modfolder", modPath, "Mod folder containing an addons/ directory")->required();
    doctor->add_flag("--fix", fix, "Apply the patches in place (originals are copied to <modfolder>/_ud-orig first)");
    doctor->add_option("--pbo", pboFilter, "Wildcard limiting which pbos are scanned (default: all)");

    doctor->callback([]() { std::exit(RunDoctor(modPath, fix, pboFilter)); });
}

} // namespace PoseidonTools
