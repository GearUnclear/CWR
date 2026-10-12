// Fixture: a soldier class owned by the addon `udunlisted`. The mission
// boot_fail_unlisted_addon.Demo places it without listing the addon in its
// addOns[], so CheckAccess raises the warning that latches the error level and
// World::InitVehicles rejects the --autotest boot. Base chain re-declared, not
// forward-declared (this config reader has no `class X;` form).

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class UDUnlistedFixture
    {
        units[] = {"UDUnlistedSoldier"};
        weapons[] = {};
        requiredVersion = 0.1;
    };
};

class CfgVehicles
{
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Soldier : Man {};
    class SoldierWB : Soldier {};

    class UDUnlistedSoldier : SoldierWB
    {
        scope = public;
        displayName = "UD Unlisted Soldier";
    };
};
