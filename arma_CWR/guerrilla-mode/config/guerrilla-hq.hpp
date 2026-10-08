// Persistent HQ scenery and campaign-owned supplies. Never inherit the
// prefilled stock ammo-box variants: a new cache must start completely empty.
class CfgVehicles
{
    // Supplements are parsed independently. Empty base declarations retain
    // their stock fields when merged; this parser has no forward declarations.
    class All {};
    class Static: All {};
    class Building: Static {};
    class Strategic: Building {};
    class ReammoBox: Strategic {};
    class WeaponHolder: ReammoBox {};
    class Camp: Strategic {};
    class CampEmpty: Camp {};
    class GMHqCacheIndoor: WeaponHolder
    {
        scope=2;
        model="\data3d\bedna_ammo.p3d";
        displayName="Headquarters cache";
        showWeaponCargo=0;
        class TransportWeapons {};
        class TransportMagazines {};
    };
    class GMHqCacheOutdoor: GMHqCacheIndoor
    {
        model="\data3d\hromada_beden.p3d";
    };
    // Static scenery keeps the desk at its selected interior floor height.
    class GMHqDesk: Camp
    {
        scope=2;
        model="\data3d\stulsuplikmapa.p3d";
        displayName="Headquarters map desk";
        mapSize=1;
    };
    class GMHqTent: CampEmpty
    {
        scope=2;
        displayName="Headquarters tent";
    };
};
