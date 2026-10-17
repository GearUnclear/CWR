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

// Generated occupier bases, step 2 (OCCUPIER-FORCES.md 5.7, ISLAND-PACKS.md
// 11): the layout every generated OUTPOST gets, read by the engine's
// BaseProps service at new game.  Rows are {class, dx, dz, dir} or
// {class, dx, dz, dir, jitter} in base-local metres: +dz towards the base's
// access road (the stored heading), +dx to the right when facing it, dir in
// degrees relative to the heading (0 faces the road), jitter 1 = a seeded
// turn of up to jitterDeg (tents and sandbags only; towers, wire and the
// fuel point stay on the grid).  A class may list alternates "A|B": the
// first the loaded package can spawn wins, none skips the row with a WARN.
// The yard (radius, m) stays prop-free for births and the hold posture;
// nothing stands within roadMargin of a road; a refused spot tries its
// left-right mirror, then the row is dropped.  Stock Remaster classes only:
// ViewTower is the stock watchtower (hlaska, scope 1), Fortress1 the
// sandbag bunker, Fence the sandbag wall (pytle_BSL), Wire the barbed wire,
// Camp / CampEast the tents, FlagCarrier the pole (it flies the base
// owner's flag), FuelStationNew2 the army fuel point (fuelstation_army).
class CfgGuerrillaBaseLayouts
{
    class Outpost
    {
        type = "OUTPOST";
        yard = 25;
        roadMargin = 6;
        jitterDeg = 5;
        minProps = 8;
        items[] =
        {
            // the gate on the road side: two sandbag bunkers
            {"Fortress1", -10, 40, 0},
            {"Fortress1", 10, 40, 0},
            // towers on two opposite corners, facing out
            {"ViewTower", -34, 34, 0},
            {"ViewTower", 34, -34, 180},
            // wire on the flanks
            {"Wire", -46, 12, 90},
            {"Wire", -46, -10, 90},
            {"Wire", 46, 12, 90},
            {"Wire", 46, -10, 90},
            // the rear: two tents and the flag
            {"CampEast|Camp", -14, -36, 0, 1},
            {"Camp", 6, -38, 0, 1},
            {"FlagCarrier", -2, -29, 0},
            // the one asymmetry: the fuel point on the road side
            {"FuelStationNew2", 32, 20, 270}
        };
    };
};
