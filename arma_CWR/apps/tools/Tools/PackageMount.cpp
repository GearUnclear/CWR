#include "PackageMount.hpp"

#include <Poseidon/Asset/Addon/AddonSystem.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/Config/Configuration.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Core/ModSystem.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <system_error>

namespace PoseidonTools
{

namespace
{
namespace fs = std::filesystem;
} // namespace

PackageMount::~PackageMount()
{
    Unmount();
}

bool PackageMount::Mount(const std::string& dataDir, const std::vector<std::string>& modDirs, std::string& error)
{
    error.clear();
    if (_mounted)
    {
        Unmount();
    }

    std::error_code ec;
    fs::path root = fs::absolute(fs::path(dataDir), ec);
    if (ec || !fs::is_directory(root, ec))
    {
        error = "not a directory: " + dataDir;
        return false;
    }

    // Mod paths are read after the chdir (ModSystem walks them relative to the
    // process CWD), so resolve them while the old CWD still applies.
    RStringB modPath;
    for (const std::string& mod : modDirs)
    {
        fs::path abs = fs::absolute(fs::path(mod), ec);
        if (ec || !fs::is_directory(abs, ec))
        {
            error = "not a mod directory: " + mod;
            return false;
        }
        std::string one = abs.string();
        modPath = (modPath.GetLength() > 0) ? RStringB((std::string(modPath.Data()) + ";" + one).c_str())
                                            : RStringB(one.c_str());
    }

    fs::path previous = fs::current_path(ec);
    if (ec)
    {
        error = "cannot read the current directory";
        return false;
    }
    fs::current_path(root, ec);
    if (ec)
    {
        error = "cannot enter " + root.string() + ": " + ec.message();
        return false;
    }
    _previousDir = previous.string();
    _dataDir = root.string();
    _mounted = true;

    Poseidon::ModSystem::SetModPath(modPath);

    // Base bin/config + the deferred mod configs + bin/config-extra.cpp, all
    // merged into Pars. Also loads the stringtables, which the display names in
    // a config report resolve through.
    Poseidon::GApp->GetConfig().InitializeGameConfiguration(nullptr);

    // Banks second: the addon configs merge on top of the base config, which is
    // the order Globals::Init uses and the order a class-existence probe needs.
    // Campaigns are skipped - no config-reading tool looks inside one.
    GUseFileBanks = true;
    Poseidon::AddonSystem::ClearRegistry();
    GFileBanks.Clear();
    Poseidon::LoadFileBanksFrom("dta", true);
    Poseidon::LoadFileBanksFrom("addons", true, true);
    Poseidon::AddonSystem::ParseAllAddonConfigs();
    Poseidon::AddonSystem::ClearAddonConfigs();
    Poseidon::AddonSystem::MarkAllAddonsLockable();

    return true;
}

void PackageMount::Unmount()
{
    if (!_mounted)
    {
        return;
    }
    // The Globals::Clear subset that matches what Mount started. GFileServer,
    // shapes, Res and the message formats were never created here.
    QIFStreamB::ClearBanks();
    Poseidon::AddonSystem::ClearRegistry();
    GFileBanks.Clear();
    GUseFileBanks = false;
    Pars.Clear();

    std::error_code ec;
    fs::current_path(fs::path(_previousDir), ec);
    _previousDir.clear();
    _dataDir.clear();
    _mounted = false;
}

} // namespace PoseidonTools
