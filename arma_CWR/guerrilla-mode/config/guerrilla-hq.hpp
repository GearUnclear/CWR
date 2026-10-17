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
// left-right mirror, then the row is dropped; a prop keeps lane metres from
// any solid map object (a wall, a fence, a gate, a building).  Stock
// Remaster classes as the base of every row: ViewTower is the stock
// watchtower (hlaska, scope 1), Fortress1 / Fortress2 the sandbag bunkers,
// Fence the sandbag wall (pytle_BSL), Wire the barbed wire, Camp /
// CampEast the tents, FlagCarrier the pole (it flies the base owner's flag),
// FuelStationNew2 the army fuel point (fuelstation_army), Housebouda1 /
// Houseplechbud the huts (scope 1), Danger the warning sign, HeliH the pad.
// An alternate "$key" is the occupier faction's descriptor key (the faction
// style, generated bases step 3): lobo-factions.hpp gives the IDF and the
// Arab armies LoBo's brown Baracken pieces (basePropTower = Brown_Turm,
// basePropHut = Brown_Wachhaus); a descriptor without the key, or a package
// without the class, falls through to the stock class after the bar.
// One layout per generated kind (the zone type): Outpost, Checkpoint and
// Coastal are placed by the generator; the Helipad layout waits for the
// helipad as an air origin (never generated yet).
class CfgGuerrillaBaseLayouts
{
    class Outpost
    {
        type = "OUTPOST";
        yard = 25;
        roadMargin = 6;
        jitterDeg = 5;
        lane = 4;
        minProps = 8;
        items[] =
        {
            // the gate on the road side: two sandbag bunkers
            {"Fortress1", -10, 40, 0},
            {"Fortress1", 10, 40, 0},
            // towers on two opposite corners, facing out
            {"$basePropTower|ViewTower", -34, 34, 0},
            {"$basePropTower|ViewTower", 34, -34, 180},
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
    // a road tripwire (a minor post, garrison 5): its centre 60 to 100 m off
    // the road, so the road-side rows stand on the shoulder; a hut facing
    // the road, the MG nest up-road firing along it, the tower, tent and
    // flag behind
    class Checkpoint
    {
        type = "CHECKPOINT";
        yard = 15;
        roadMargin = 6;
        jitterDeg = 5;
        lane = 4;
        minProps = 6;
        items[] =
        {
            {"$basePropHut|Housebouda1|Houseplechbud", 8, 36, 0},
            {"Fortress1", -22, 38, 90},
            {"Fence", -6, 42, 0, 1},
            {"Danger", 18, 44, 0},
            {"$basePropTower|ViewTower", 20, -18, 180},
            {"CampEast|Camp", -12, -16, 0, 1},
            {"FlagCarrier", 4, -20, 0},
            {"Wire", -30, 8, 90},
            {"Wire", 30, 8, 90}
        };
    };
    // a coastal tripwire (a minor post, garrison 5): a tower and two bunkers
    // covering the back, a sandbag line, a hut, a tent and the flag
    class Coastal
    {
        type = "COASTAL";
        yard = 20;
        roadMargin = 6;
        jitterDeg = 5;
        lane = 4;
        minProps = 6;
        items[] =
        {
            {"$basePropTower|ViewTower", 0, -34, 180},
            {"Fortress1", -30, -20, 225},
            {"Fortress1", 30, -20, 135},
            {"Fence", -14, -40, 180, 1},
            {"Fence", 14, -40, 180, 1},
            {"$basePropHut|Houseplechbud|Housebouda1", 22, 26, 0},
            {"CampEast|Camp", -20, 24, 0, 1},
            {"FlagCarrier", 0, 30, 0},
            {"Wire", -40, 6, 90},
            {"Wire", 40, 6, 90}
        };
    };
    // an air annex (a minor post, garrison 6): two pads 44 m apart at the
    // back, the fuel point, a crew tent, sandbag rings on the front corners,
    // one tower.  Not generated until the helipad is an air origin.
    class Helipad
    {
        type = "HELIPAD";
        yard = 25;
        roadMargin = 6;
        jitterDeg = 5;
        lane = 4;
        minProps = 6;
        items[] =
        {
            {"HeliH", -22, -32, 0},
            {"HeliH", 22, -32, 0},
            {"FuelStationNew2", 40, 4, 270},
            {"CampEast|Camp", -32, 20, 0, 1},
            {"Fortress2", -36, 36, 315},
            {"Fortress2", 36, 36, 45},
            {"$basePropTower|ViewTower", 0, -46, 180},
            {"FlagCarrier", 8, 30, 0}
        };
    };
};
