#include <catch2/catch_test_macros.hpp>

#include "../../../../../apps/tools/Tools/PackageMount.hpp"

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace Poseidon;
using PoseidonTools::PackageMount;

namespace
{
namespace fs = std::filesystem;

// The CI fixture case: a package with a text bin/config.cpp and no pbos at all.
fs::path MakeMinimalPackage(const char* name, const char* configBody)
{
    fs::path root = fs::temp_directory_path() / "poseidon-package-mount" / name;
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "bin", ec);
    std::ofstream out(root / "bin" / "config.cpp", std::ios::binary);
    out << configBody;
    out.close();
    return root;
}

} // namespace

TEST_CASE("PackageMount: a bin/config.cpp-only package reaches Pars", "[guerrilla][packagemount]")
{
    const char* config = "class CfgPatches\n"
                         "{\n"
                         "  class UDMountFixture { units[]={}; weapons[]={}; requiredVersion=1.30; };\n"
                         "};\n"
                         "class CfgVehicles\n"
                         "{\n"
                         "  class All {};\n"
                         "  class Land : All {};\n"
                         "  class Man : Land {};\n"
                         "  class UDMountSoldier : Man { scope=2; displayName=\"UD Mount Soldier\"; };\n"
                         "};\n";
    fs::path root = MakeMinimalPackage("base", config);
    fs::path before = fs::current_path();

    PackageMount mount;
    std::string error;
    REQUIRE(mount.Mount(root.string(), {}, error));
    REQUIRE(error.empty());
    REQUIRE(mount.IsMounted());

    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    REQUIRE(vehicles != nullptr);
    const ParamEntry* soldier = vehicles->FindEntry("UDMountSoldier");
    REQUIRE(soldier != nullptr);
    REQUIRE(soldier->ReadValue("scope", 0) == 2);
    REQUIRE(Pars.FindEntry("CfgPatches")->FindEntry("UDMountFixture") != nullptr);

    mount.Unmount();
    REQUIRE_FALSE(mount.IsMounted());
    // Unmount restores the working directory and empties Pars, so a tool can
    // mount a second package in the same process.
    REQUIRE(fs::current_path() == before);
    REQUIRE(Pars.FindEntry("CfgVehicles") == nullptr);

    std::error_code ec;
    fs::remove_all(root, ec);
}

TEST_CASE("PackageMount: a mod folder's bin/config.cpp merges on top", "[guerrilla][packagemount]")
{
    const char* baseConfig = "class CfgPatches { class UDMountBase { units[]={}; requiredVersion=1.30; }; };\n"
                             "class CfgVehicles\n"
                             "{\n"
                             "  class All {};\n"
                             "  class Land : All {};\n"
                             "  class Man : Land {};\n"
                             "  class BaseSoldier : Man { scope=2; };\n"
                             "};\n";
    // No inheritance from a base-config class: the deferred merge layers whole
    // classes, and a forward declaration of an external base is a separate
    // question from whether the mod's own classes arrive.
    const char* modConfig = "class CfgPatches { class UDMountMod { units[]={}; requiredVersion=1.30; }; };\n"
                            "class CfgVehicles\n"
                            "{\n"
                            "  class ModSoldier { scope=2; displayName=\"Mod Soldier\"; };\n"
                            "};\n";
    fs::path root = MakeMinimalPackage("withmod-base", baseConfig);
    fs::path mod = MakeMinimalPackage("withmod-mod", modConfig);

    PackageMount mount;
    std::string error;
    REQUIRE(mount.Mount(root.string(), {mod.string()}, error));

    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    REQUIRE(vehicles != nullptr);
    REQUIRE(vehicles->FindEntry("BaseSoldier") != nullptr);
    REQUIRE(vehicles->FindEntry("ModSoldier") != nullptr);

    mount.Unmount();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::remove_all(mod, ec);
}

TEST_CASE("PackageMount: a missing directory fails without mounting", "[guerrilla][packagemount]")
{
    fs::path before = fs::current_path();
    PackageMount mount;
    std::string error;
    REQUIRE_FALSE(mount.Mount((fs::temp_directory_path() / "poseidon-no-such-package").string(), {}, error));
    REQUIRE_FALSE(error.empty());
    REQUIRE_FALSE(mount.IsMounted());
    REQUIRE(fs::current_path() == before);
}
