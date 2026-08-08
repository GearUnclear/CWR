#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Asset/Addon/AddonSystem.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>

#include <initializer_list>

// Regression cover for the addon-registry merge.
//
// The stock CWA config.bin ships `CfgAddons` with access = PAReadOnly, and
// ParamClass::Update refuses a read-only destination on its first line, reporting
// through RptF - which is compiled to ((void)0). The result was silent: a mod that
// shipped `CfgAddons/PreloadAddons` merged nothing at all, World::ActivateAddons
// never saw its list, and every class the mod owned stayed unreachable at runtime
// ("Access denied ... owner addon is not activated"), which in turn produced an
// abstract vehicle type and a 0xC0000005 in Building::Building.
//
// AddonSystem::MergeConfigInto lifts the registry to PAReadAndCreate for the merge
// and restores the original mode afterwards. These tests pin both halves: the merge
// lands, and nothing that was protected before is writable after.

using namespace Poseidon;

namespace
{
ParamClass* EnsureClass(ParamClass& parent, const char* name)
{
    ParamEntry* existing = parent.FindEntry(name);
    if (existing && existing->GetClassInterface())
    {
        return existing->GetClassInterface();
    }
    return parent.AddClass(name);
}

// class CfgAddons { class PreloadAddons { class <roster> { list[] = {...}; }; }; };
void AddPreloadRoster(ParamFile& file, const char* roster, std::initializer_list<const char*> names)
{
    ParamClass* cfgAddons = EnsureClass(file, "CfgAddons");
    ParamClass* preload = EnsureClass(*cfgAddons, "PreloadAddons");
    ParamClass* cls = EnsureClass(*preload, roster);
    ParamEntry* list = cls->AddArray("list");
    for (const char* name : names)
    {
        list->AddValue(RStringB(name));
    }
}

const ParamEntry* FindRosterList(const ParamFile& file, const char* roster)
{
    const ParamEntry* cfgAddons = file.FindEntry("CfgAddons");
    if (!cfgAddons)
    {
        return nullptr;
    }
    const ParamEntry* preload = cfgAddons->FindEntry("PreloadAddons");
    if (!preload)
    {
        return nullptr;
    }
    const ParamEntry* cls = preload->FindEntry(roster);
    if (!cls)
    {
        return nullptr;
    }
    return cls->FindEntry("list");
}
} // namespace

TEST_CASE("AddonSystem - a plain Update cannot write a read-only addon registry", "[addon][config][access]")
{
    // The behaviour the fix works around; asserted so a future change to
    // ParamClass::Update that made this legal would not go unnoticed.
    ParamFile base;
    AddPreloadRoster(base, "BaseRoster", {"laserguided", "noe"});
    base.FindEntry("CfgAddons")->SetAccessModeForAll(PAReadOnly);

    ParamFile mod;
    AddPreloadRoster(mod, "ModRoster", {"lobowreck"});

    base.Update(mod);

    REQUIRE(FindRosterList(base, "ModRoster") == nullptr);
}

TEST_CASE("AddonSystem - MergeConfigInto extends a read-only addon registry", "[addon][config][access]")
{
    ParamFile base;
    AddPreloadRoster(base, "BaseRoster", {"laserguided", "noe"});
    base.FindEntry("CfgAddons")->SetAccessModeForAll(PAReadOnly);

    ParamFile mod;
    AddPreloadRoster(mod, "ModRoster", {"lobowreck", "map_editorupgrade"});

    AddonSystem::MergeConfigInto(base, mod);

    const ParamEntry* merged = FindRosterList(base, "ModRoster");
    REQUIRE(merged != nullptr);
    REQUIRE(merged->GetSize() == 2);
    CHECK(RString((*merged)[0]) == RString("lobowreck"));
    CHECK(RString((*merged)[1]) == RString("map_editorupgrade"));

    SECTION("the base game's own roster is untouched")
    {
        const ParamEntry* baseList = FindRosterList(base, "BaseRoster");
        REQUIRE(baseList != nullptr);
        CHECK(baseList->GetSize() == 2);
    }

    SECTION("and the registry is locked again afterwards")
    {
        CHECK(base.FindEntry("CfgAddons")->GetAccessMode() == PAReadOnly);

        ParamFile second;
        AddPreloadRoster(second, "LateRoster", {"nope"});
        base.Update(second); // plain Update, not the helper
        CHECK(FindRosterList(base, "LateRoster") == nullptr);
    }
}

TEST_CASE("AddonSystem - MergeConfigInto merges normally when the registry is unlocked", "[addon][config][access]")
{
    // No lift needed: the helper must be a drop-in for Update in the ordinary case.
    ParamFile base;
    AddPreloadRoster(base, "BaseRoster", {"noe"});

    ParamFile mod;
    AddPreloadRoster(mod, "ModRoster", {"lobowreck"});

    AddonSystem::MergeConfigInto(base, mod);

    REQUIRE(FindRosterList(base, "ModRoster") != nullptr);
    CHECK(base.FindEntry("CfgAddons")->GetAccessMode() == PADefault);
}

TEST_CASE("AddonSystem - MergeConfigInto is a no-op-safe merge with no registry at all", "[addon][config][access]")
{
    ParamFile base;
    ParamFile mod;
    mod.AddClass("CfgSomethingElse");

    AddonSystem::MergeConfigInto(base, mod);

    CHECK(base.FindEntry("CfgSomethingElse") != nullptr);
}
