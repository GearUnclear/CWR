#include <catch2/catch_test_macros.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/UI/Locale/LanguageRegistry.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include "test_fixtures.hpp"

#include <filesystem>

namespace Poseidon
{
bool ParseConfig(RStringB dir, void* context);
bool IsConfigOverriddenByMod();
void MergeBaseConfigExtra();
} // namespace Poseidon

using Poseidon::ParamEntry;

namespace
{
// Replay the enumeration for one mod: mod before base, stop at the first true. Returns true when
// the mod's bin/config won outright (upstream's replace model). The overhaul defers the mod config
// and layers it over the base instead, so this returns false and the base pass does the merging.
bool LoadConfigModThenBase(const char* modDir)
{
    if (Poseidon::ParseConfig(modDir, nullptr))
        return true;
    Poseidon::ParseConfig("", nullptr);
    return false;
}

struct CwdGuard
{
    std::filesystem::path prev;
    explicit CwdGuard(const std::filesystem::path& to) : prev(std::filesystem::current_path())
    {
        std::filesystem::current_path(to);
    }
    ~CwdGuard() { std::filesystem::current_path(prev); }
};

// Resolve the fixture root via a known file inside it (GetTestFixturePath validates a regular
// file), then step up out of bin/.
std::filesystem::path FixtureRoot()
{
    return std::filesystem::path(TestFixtures::GetTestFixturePath("config-replace/bin/config.cpp"))
        .parent_path()
        .parent_path();
}

const ParamEntry* Child(const ParamEntry* parent, const char* name)
{
    return parent ? parent->FindEntry(name) : nullptr;
}
} // namespace

// Upstream 3.05 made a mod's bin/config REPLACE the base master config outright. The overhaul
// keeps the deferred-merge model: the base loads first and every config-bearing mod is merged on
// top of it in mount order, so a stack like "@LoBo;@lobofixup;@udshowcase" contributes all of its
// classes AND the vanilla roster stays available for Guerrilla Mode's island/faction swap. This
// test is the upstream fixture re-pointed at that behaviour.
TEST_CASE("a mod's bin/config is layered over the base, and config-extra still applies",
          "[config][mods][replace]")
{
    // ParseConfig("") resolves bin/config.cpp relative to the cwd; the fixture keeps the base
    // config + config-extra in bin/ and the mod config in mod/bin/, so run from the fixture root.
    CwdGuard cwd(FixtureRoot());

    const bool modWon = LoadConfigModThenBase("mod");
    // The mod pass defers rather than winning, so the enumeration continues to the base.
    CHECK_FALSE(modWon);

    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    REQUIRE(vehicles != nullptr);
    CHECK(vehicles->FindEntry("ModTank") != nullptr);
    // Merge, not replace: the vanilla class the mod omits survives.
    CHECK(vehicles->FindEntry("VanillaJeep") != nullptr);

    const ParamEntry* infantry = Child(Child(Pars.FindEntry("CfgGroups"), "West"), "Infantry");
    REQUIRE(infantry != nullptr);
    CHECK(infantry->FindEntry("GrpModOnly") != nullptr);
    CHECK(infantry->FindEntry("GrpShared") != nullptr);
    CHECK(infantry->FindEntry("GrpVanilla") != nullptr);

    // Nothing shadowed the base config, so the override flag stays clear and config-extra was
    // already applied by ParseConfig itself.
    CHECK_FALSE(Poseidon::IsConfigOverriddenByMod());
    REQUIRE(Pars.FindEntry("CfgLanguages") != nullptr);

    // The restore path Configuration.cpp runs when the flag is set stays idempotent.
    Poseidon::MergeBaseConfigExtra();
    REQUIRE(Pars.FindEntry("CfgLanguages") != nullptr);

    auto& registry = CfgLib::LanguageRegistry::Instance();
    registry.ResetToDefaults();
    if (const ParamEntry* cfgLangs = Pars.FindEntry("CfgLanguages"))
        registry.LoadFromConfig(*cfgLangs);
    // The marker language proves config-extra reached the registry (not just the 8 defaults).
    CHECK(registry.Find("Tundra") != nullptr);
    registry.ResetToDefaults();
}

TEST_CASE("with no mod config the base loads and applies its own config-extra", "[config][mods][replace]")
{
    CwdGuard cwd(FixtureRoot());

    REQUIRE(Poseidon::ParseConfig("", nullptr));
    CHECK_FALSE(Poseidon::IsConfigOverriddenByMod());

    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    REQUIRE(vehicles != nullptr);
    CHECK(vehicles->FindEntry("VanillaJeep") != nullptr);

    const ParamEntry* infantry = Child(Child(Pars.FindEntry("CfgGroups"), "West"), "Infantry");
    REQUIRE(infantry != nullptr);
    CHECK(infantry->FindEntry("GrpVanilla") != nullptr);

    // config-extra.cpp in the same bin/ is applied during ParseConfig, so the base config carries
    // CfgLanguages even though config.cpp itself does not.
    REQUIRE(Pars.FindEntry("CfgLanguages") != nullptr);
    auto& registry = CfgLib::LanguageRegistry::Instance();
    registry.ResetToDefaults();
    if (const ParamEntry* cfgLangs = Pars.FindEntry("CfgLanguages"))
        registry.LoadFromConfig(*cfgLangs);
    CHECK(registry.Find("Tundra") != nullptr);
    registry.ResetToDefaults();
}
