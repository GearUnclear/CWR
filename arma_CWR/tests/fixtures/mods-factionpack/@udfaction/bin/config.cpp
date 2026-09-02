// A well-formed faction pack for the mod-intake CI lane (issue #54).
//
// Mounted over tests/fixtures/packages/mini, this is what a third-party
// Guerrilla faction pack looks like when it is intact: every class its
// descriptor names exists, so `guerrilla lint` reports zero substitutions and
// `guerrilla probe` passes the whole roster. Its twin @udbroken names a class
// nothing ships, so the same two commands must fail on it.

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class UDFaction
    {
        units[] = {"UDSoldier", "UDOfficer", "UDCivFighter"};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {"UDMiniPackage"};
    };
};

class CfgVehicles
{
    // The base chain is re-declared, not forward-declared: this config reader
    // has no `class X;` form (ParamClass::Parse wants ':' or '{' after the
    // name, and a ';' aborts the whole file), and a mod config is parsed
    // standalone before the deferred merge layers it onto Pars, so the base
    // package's classes are not visible yet. Re-declaring them empty merges
    // back onto the originals without changing them.
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Soldier : Man {};
    class SoldierWB : Soldier {};

    class UDSoldier : SoldierWB
    {
        scope = public;
        side = 1;
        displayName = "UD Fighter";
        weapons[] = {"M16"};
        magazines[] = {"M16"};
    };

    class UDOfficer : SoldierWB
    {
        scope = public;
        side = 1;
        displayName = "UD Fighter Leader";
        weapons[] = {"M16"};
        magazines[] = {"M16"};
    };

    class UDCivFighter : SoldierWB
    {
        scope = public;
        side = 1;
        displayName = "UD Fighter (civilian clothes)";
        weapons[] = {"M16"};
        magazines[] = {"M16"};
    };
};

class CfgGuerrillaFactions
{
    class UDFaction
    {
        side = "WEST";
        displayName = "UD Faction";
        tiers[] = {"UDSoldier", "UDOfficer"};
        tierThresholds[] = {4};
        civTier[] = {"UDCivFighter"};
        playerClassWarrior = "UDSoldier";
        playerClassCiv = "UDCivFighter";
        officer = "UDOfficer";
        vehicles[] = {"Jeep"};
        vehicleThresholds[] = {3};
        civVehicles[] = {"Jeep"};
        baseWeapon = "M16";
        baseMagazine = "M16";
    };
};
