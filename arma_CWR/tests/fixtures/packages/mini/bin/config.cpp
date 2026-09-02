// Minimal base config for the mod-intake CI lane (issue #54).
//
// A stand-in game data package: enough CfgVehicles hierarchy for
// `guerrilla lint` and `guerrilla probe` to have something real to resolve
// against, and nothing else. Written from scratch for this repo - no Bohemia
// config content, so it stays GPL-clean and can live in the tree next to the
// engine. The models it names are the repo's own tests/fixtures/p3d shapes,
// copied to mini/ so the shape-existence gate has a file to find.
//
// Deliberately NOT a playable package: no world, no campaign, no sounds. The
// two tools it feeds read config and probe for files, and that is all.

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class UDMiniPackage
    {
        units[] = {"SoldierWB", "SoldierEB", "SoldierGB", "Civilian"};
        weapons[] = {"UDRifle"};
        requiredVersion = 1.30;
        requiredAddons[] = {};
    };
};

class CfgWeapons
{
    class Default {};
    class UDRifle : Default
    {
        scope = public;
        displayName = "UD Rifle";
        magazines[] = {"UDRifleMag"};
    };

    // OFP-era configs declare magazines under BOTH banks, and the plan-15
    // resolution pass probes CfgWeapons for a descriptor's magazine keys - a
    // magazine that lives only in CfgMagazines reads as unresolvable there.
    class UDRifleMag : Default
    {
        scope = public;
        displayName = "UD Rifle Magazine";
    };
};

class CfgMagazines
{
    class Default {};
    class UDRifleMag : Default
    {
        scope = public;
        displayName = "UD Rifle Magazine";
        count = 30;
    };
};

class CfgVehicles
{
    class All {};
    class AllVehicles : All {};

    // Man and LandVehicle both hang off Land, the hierarchy the roster probe
    // tests membership against.
    class Land : AllVehicles {};
    class LandVehicle : Land {};

    class Man : Land
    {
        scope = private;
        model = "\mini\man.p3d";
        weapons[] = {"UDRifle"};
        magazines[] = {"UDRifleMag"};
    };

    class Soldier : Man
    {
        scope = private;
    };

    class SoldierWB : Soldier
    {
        scope = public;
        side = 1; // WEST
        displayName = "UD Rifleman (West)";
        model = "\mini\man.p3d";
    };

    class SoldierEB : Soldier
    {
        scope = public;
        side = 0; // EAST
        displayName = "UD Rifleman (East)";
        model = "\mini\man.p3d";
    };

    class SoldierGB : Soldier
    {
        scope = public;
        side = 2; // GUER
        displayName = "UD Rifleman (Resistance)";
        model = "\mini\man.p3d";
    };

    class Civilian : Man
    {
        scope = public;
        side = 3; // CIV
        displayName = "UD Civilian";
        model = "\mini\man.p3d";
        weapons[] = {};
        magazines[] = {};
    };

    class Car : LandVehicle
    {
        scope = private;
        model = "\mini\car.p3d";
    };

    class UDCar : Car
    {
        scope = public;
        side = 3;
        displayName = "UD Car";
        model = "\mini\car.p3d";
    };
};
