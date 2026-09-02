// The broken twin of @udfaction, for the mod-intake CI lane (issue #54).
//
// Identical except that tiers[0] names a class nothing ships. That is the
// failure the intake lane exists to catch: the descriptor still loads, the
// resolution pass still substitutes a side fallback, and the faction still
// "works" - while every spawn wears the wrong body. `guerrilla lint` must exit
// non-zero on this and name UDBrokenFaction.

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class UDBroken
    {
        units[] = {"UDBrokenOfficer"};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {"UDMiniPackage"};
    };
};

class CfgVehicles
{
    // See @udfaction: the base chain is re-declared because this reader has no
    // forward-declaration form.
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Soldier : Man {};
    class SoldierWB : Soldier {};

    class UDBrokenOfficer : SoldierWB
    {
        scope = public;
        side = 1;
        displayName = "UD Broken Leader";
        model = "\mini\man.p3d";
        weapons[] = {"UDRifle"};
        magazines[] = {"UDRifleMag"};
    };
};

class CfgGuerrillaFactions
{
    class UDBrokenFaction
    {
        side = "WEST";
        displayName = "UD Broken Faction";
        tiers[] = {"NoSuchClass"};
        playerClassWarrior = "NoSuchClass";
        officer = "UDBrokenOfficer";
    };
};
