"""The view tables the generated capture tests share.

    TIMING   the twenty A0 timing views per world (gen_timing.py; plan section 5 A0)
    HERO     one hero view per feature, the start view of the feature's first
             row in plan section 7.3 (gen_features_ab.py; plan section 8 item 5)

A view is a dict:

    name           label stem of its shots
    x, z           view origin [east, north] in map metres
    eye            metres above the ground probe, or above sea level with eyeAbs
    h, p           heading (0 = north, clockwise) and pitch in degrees
    date           [year, month, day, hour, minute]
    overcast, fog, rain, vd   weather and view distance (defaults 0, 0, 0)
    props          SQF statements run inside one call {} after capBegin
    refresh        SQF run right before each shot (relight a fire)
    cleanup        SQF run after capClear
    feature        what the view is for (documentation)
    note           how it deviates from plan section 7.3 (documentation only)
    keys           HERO only: the feature's switch keys (the "off" shot sets them 0)

The notes never reach the generated tests.
"""

FIRE_REFRESH = "call { if (not (isNull capFire)) then { capFire inflame true } }"
FFH_REFRESH = ("call { if (not (isNull capFire)) then { capFire inflame true }; "
               "if (not (isNull capBurnV)) then { capBurnV inflame true } }")
TOWN_FIRE = ['capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;',
             'if (not (isNull capFire)) then { capFire inflame true };']
# A SAFE AI driver in a Jeep, headlights on at night, held in place (MOVE off).
NIGHT_JEEP = ['capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;',
              'capDrv = objNull;',
              'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
              '"capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver '
              'capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
              'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;']
NIGHT_JEEP_CLEANUP = 'call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }'
FFH_PROPS = ['{ [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], '
             '[12.5, -2.5]];',
             '{ [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], '
             '[18, 6], [15, 8.5]];',
             'capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;',
             'if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame '
             'true };',
             'capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;',
             'if (not (isNull capFire)) then { capFire inflame true };']
LOWSUN_FP_PROPS = ['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;',
                   '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
                   '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
                   '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;']

TIMING_ABEL = [
    dict(name="sunrise_ridge", x=4400, z=3300, eye=1.75, h=60, p=2, date=[1985, 7, 5, 5, 8], vd=4000,
         feature="perf", f10=True),
    dict(name="forest_edge_golden", x=4674, z=10156, eye=1.75, h=75, p=3, date=[1985, 7, 5, 18, 15], vd=1500,
         props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;',
                '[["Jeep"], 25, 3, 90, "veh"] call capSpawn;'],
         feature="temporal, texsharp",
         note="Jeep parked at 25 m (the video teleports it across); also stands in for texsharp's first row "
              "(7.3: p 0, no props)"),
    dict(name="town_night", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 2, 27, 22, 0], vd=800,
         props=TOWN_FIRE, refresh=FIRE_REFRESH, feature="light list",
         note="capture-set version: auto street lamps and the Fire at 9 m, no driven Jeeps (7.3 has two)"),
    dict(name="coastline_sunset", x=1550, z=5150, eye=3, h=278, p=-4, date=[1985, 7, 5, 18, 35], vd=4000,
         feature="sky table, water",
         note="capture-set heading 278 (7.3 water row; the sky row says 250); eye 3 m above the beach probe "
              "(the sky row's convention; the water row says 3 m ASL)"),
    dict(name="woods_pine2_evening", x=6940, z=9964, eye=1.75, h=240, p=-2, date=[1985, 7, 5, 17, 30], vd=1500,
         props=['[["Jeep"], 14, 2, 70, "veh"] call capSpawn;',
                '[["SoldierWB"], 9, -2, 150, "man"] call capSpawn;'],
         feature="shadows",
         note="props from shadows/gen_r8.py (Jeep at 14 m, SoldierWB at 9 m; 7.3 says a Jeep at 15 m); "
              "pitch -2 per 7.3 (the research shot -5)"),
    dict(name="lowsun_fp", x=7300, z=10900, eye=1.75, h=152.5, p=-3, date=[1985, 3, 1, 8, 0], vd=1500,
         props=LOWSUN_FP_PROPS, feature="shade", note="capture-set camera instead of the orbit start"),
    dict(name="road_flat_drive_moonless", x=6895.2, z=8836.6, eye=1.5, h=69.6, p=-2, date=[1985, 3, 17, 23, 0],
         vd=800, props=NIGHT_JEEP, cleanup=NIGHT_JEEP_CLEANUP, feature="night",
         note="night track's driver's-eye setup: the Jeep stands 3.4 m behind the eye with a SAFE AI driver, "
              "not driven (7.3 teleports it 0.4 m per frame)"),
    dict(name="ffh", x=7600, z=10850, eye=5, h=0, p=-12, date=[1985, 7, 12, 23, 0], vd=1000,
         props=FFH_PROPS, refresh=FFH_REFRESH,
         feature="combat", note="static stand-in of capFFStageN: posed men, burning Ural and Fire, no volleys or shells"),
    dict(name="bay_offshore", x=9240, z=6950, eye=3, eyeAbs=True, h=272, p=-3, date=[1985, 7, 5, 6, 30], vd=3000,
         props=['[["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;'],
         feature="ssr", note="eye 3 m ASL (capEyeAbs); boat heading 70"),
    dict(name="town_night_rain", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 2, 27, 22, 0], vd=800,
         overcast=0.9, rain=1, props=TOWN_FIRE, refresh=FIRE_REFRESH, feature="materials"),
    dict(name="pinewood_morn", x=6830, z=9905, eye=1.75, h=72, p=6, date=[1985, 7, 5, 6, 0], vd=1500, fog=0.1,
         feature="air (beams)", f10=True),
    dict(name="valley_0515", x=6100, z=5000, eye=1.75, h=40, p=-6, date=[1985, 7, 5, 5, 15], vd=4000,
         feature="air (haze, valley mist)"),
    dict(name="hill_1730", x=5500, z=6250, eye=15, h=185, p=-14, date=[1985, 9, 20, 17, 30], vd=2000,
         feature="hfrt"),
    dict(name="coast_oc_up", x=1550, z=5150, eye=3, h=260, p=14, date=[1985, 7, 5, 18, 35], vd=4000, overcast=0.5,
         feature="clouds"),
    dict(name="sea_noon", x=1550, z=5150, eye=12, h=278, p=-12, date=[1985, 7, 5, 12, 30], vd=4000,
         feature="water (noon); gate-off reference for low-sun features"),
    dict(name="noon_fields", x=6250, z=10800, eye=1.75, h=150, p=-2, date=[1985, 7, 5, 12, 30], vd=2500,
         props=['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;',
                '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;'],
         feature="gate-off reference (noon, dry)"),
]
TIMING_EDEN = [
    dict(name="overcast_broken", x=9200, z=3700, eye=1.75, h=288, p=-4, date=[1985, 7, 5, 14, 30], vd=3500,
         overcast=0.5, props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;'], feature="texsharp (meadow)"),
    dict(name="broken_high", x=9200, z=3700, eye=250, h=288, p=-12, date=[1985, 7, 5, 14, 30], vd=5000,
         overcast=0.5, feature="clouds (shadows clip)",
         note="sky drift not stepped (still; 7.3 steps it 0 to 15 deg over the clip)"),
]
TIMING_SINAI = [
    dict(name="sinai_lowsun_fp", x=8350, z=2950, eye=1.75, h=338.8, p=-3, date=[2005, 7, 21, 16, 10], vd=2000,
         props=['[["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;',
                '[["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;',
                '[["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;',
                '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;'],
         feature="temporal, shade (Sinai clips)"),
    dict(name="sinai_desert_noon", x=7300, z=10300, eye=1.75, h=210, p=1, date=[2005, 7, 21, 12, 15], vd=4000,
         props=['[["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;',
                '[["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;'],
         feature="gate-off reference (Sinai noon)"),
]

WORLDS = {
    "Abel": dict(mission="tests/rendering/captures/missions/udcap_abel.Abel",
                 tags='["lighting_capture", "headful", "exclusive"]', mods=[]),
    "Eden": dict(mission="tests/rendering/captures/missions/udcap_eden.Eden",
                 tags='["lighting_capture", "headful", "exclusive"]', mods=[]),
    "Sinai": dict(mission="tests/rendering/captures/missions/udcap_sinai.Sinai",
                  tags='["lighting_capture", "lobo", "headful", "exclusive"]',
                  mods=['"--mods-dir"', '"."', '"--mod"', '"@LoBo"']),
}


def _timing(name):
    for v in TIMING_ABEL:
        if v["name"] == name:
            return dict(v)
    raise KeyError(name)


def _hero(base, name, feature, keys, **changes):
    v = _timing(base) if base else {}
    v.pop("f10", None)
    v.pop("note", None)
    v.update(name=name, feature=feature, keys=keys)
    v.update(changes)
    return v


# One hero view per feature (plan 7.3: the start view of the feature's first
# row), in section 4 order. All on Abel.
HERO = [
    _hero("sunrise_ridge", "perf_sunrise_ridge", "perf", ["drawGrouping", "hillCull"]),
    _hero("forest_edge_golden", "temporal_forest_edge_golden", "temporal", ["taa"],
          note="Jeep parked at the start of its path, 25 m out (the clip teleports it across)"),
    _hero("town_night", "lighttiles_town_night", "light list", ["lightTiles"],
          note="auto street lamps and the Fire at 9 m; the two Jeeps with headlights are not staged"),
    _hero("coastline_sunset", "atmosphere_coastline_sunset", "sky table", ["atmosphere"], h=250),
    _hero("woods_pine2_evening", "shadows_woods_pine2_evening", "shadows", ["treeShadows", "softShadows"],
          props=['[["Jeep"], 15, 2, 70, "veh"] call capSpawn;']),
    _hero("lowsun_fp", "shade_lowsun_fp", "shade", ["lowSunRig", "crownCanopy"],
          note="capture-set camera instead of the orbit start"),
    _hero("road_flat_drive_moonless", "night_road_flat_drive_moonless", "night",
          ["nightLights", "moonShadows", "stars"],
          note="the Jeep stands 3.4 m behind the eye with a SAFE AI driver (the clip drives it)"),
    _hero("ffh", "combat_ffh", "combat", ["combatLights", "combatShadows"],
          note="static stand-in of capFFStageN: posed men, burning Ural and Fire, no volleys or shells"),
    _hero("coastline_sunset", "water_coastline_sunset", "water", ["seaWaves"], eye=3, eyeAbs=True),
    _hero("bay_offshore", "ssr_bay_offshore", "ssr", ["ssr"]),
    _hero("town_night_rain", "materials_town_night_rain", "materials", ["wetWeather", "lampStreaks"]),
    _hero("pinewood_morn", "airbeams_pinewood_morn", "air (beams)", ["airBeams"]),
    _hero("valley_0515", "airhaze_valley_0515", "air (haze, valley mist)", ["airHaze", "valleyMist"]),
    _hero("hill_1730", "hfrt_hill_1730", "hfrt", ["canopyShadows"]),
    _hero("coast_oc_up", "clouds_coast_oc_up", "clouds", ["litClouds", "skyCloudShadows"]),
    _hero("forest_edge_golden", "texsharp_forest_edge", "texsharp", ["terrainDetail", "sharpen"], p=0, props=[]),
]
