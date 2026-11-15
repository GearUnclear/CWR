// Feature A/B hero stills on Abel: gen_features_ab.py; <view>__off then <view>__on.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call { format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [4400, 3300, 0]
capCam = "camera" camCreate [4400, 3300, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- perf_sunrise_ridge: perf (off = remastered,drawGrouping=0,hillCull=0)
call { capRain = 0; capEyeAbs = false }
["perf_sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,drawGrouping=0,hillCull=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "perf_sunrise_ridge__off"
"perf_sunrise_ridge__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "perf_sunrise_ridge__on"
"perf_sunrise_ridge__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- temporal_forest_edge_golden: temporal (off = remastered,taa=0)
call { capRain = 0; capEyeAbs = false }
["temporal_forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capSimT = 2600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
    [["Jeep"], 25, 3, 90, "veh"] call capSpawn;
}
triSimFrames 150
"remastered,taa=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "temporal_forest_edge_golden__off"
"temporal_forest_edge_golden__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "temporal_forest_edge_golden__on"
"temporal_forest_edge_golden__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- lighttiles_town_night: light list (off = remastered,lightTiles=0)
call { capRain = 0; capEyeAbs = false }
["lighttiles_town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered,lightTiles=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "lighttiles_town_night__off"
"lighttiles_town_night__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "lighttiles_town_night__on"
"lighttiles_town_night__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- atmosphere_coastline_sunset: sky table (off = remastered,atmosphere=0)
call { capRain = 0; capEyeAbs = false }
["atmosphere_coastline_sunset", 1550, 5150, 3, 250, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,atmosphere=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "atmosphere_coastline_sunset__off"
"atmosphere_coastline_sunset__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "atmosphere_coastline_sunset__on"
"atmosphere_coastline_sunset__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shadows_woods_pine2_evening: shadows (off = remastered,treeShadows=0,softShadows=0)
call { capRain = 0; capEyeAbs = false }
["shadows_woods_pine2_evening", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capSimT = 4400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 15, 2, 70, "veh"] call capSpawn;
}
triSimFrames 150
"remastered,treeShadows=0,softShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "shadows_woods_pine2_evening__off"
"shadows_woods_pine2_evening__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "shadows_woods_pine2_evening__on"
"shadows_woods_pine2_evening__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shade_lowsun_fp: shade (off = remastered,lowSunRig=0,crownCanopy=0)
call { capRain = 0; capEyeAbs = false }
["shade_lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capSimT = 5000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"remastered,lowSunRig=0,crownCanopy=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "shade_lowsun_fp__off"
"shade_lowsun_fp__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "shade_lowsun_fp__on"
"shade_lowsun_fp__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- night_road_flat_drive_moonless: night (off = remastered,nightLights=0,moonShadows=0,stars=0)
call { capRain = 0; capEyeAbs = false }
["night_road_flat_drive_moonless", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"remastered,nightLights=0,moonShadows=0,stars=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "night_road_flat_drive_moonless__off"
"night_road_flat_drive_moonless__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "night_road_flat_drive_moonless__on"
"night_road_flat_drive_moonless__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- combat_ffh: combat (off = remastered,combatLights=0,combatShadows=0)
call { capRain = 0; capEyeAbs = false }
["combat_ffh", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 6200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_ffh__off"
"combat_ffh__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_ffh__on"
"combat_ffh__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- water_coastline_sunset: water (off = remastered,seaWaves=0)
call { capRain = 0; capEyeAbs = true }
["water_coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 6800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,seaWaves=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "water_coastline_sunset__off"
"water_coastline_sunset__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "water_coastline_sunset__on"
"water_coastline_sunset__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- ssr_bay_offshore: ssr (off = remastered,ssr=0)
call { capRain = 0; capEyeAbs = true }
["ssr_bay_offshore", 9240, 6950, 3, 272, -3, [1985, 7, 5, 6, 30], 0, 0, 3000] call capBegin
call { capSimT = 7400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;
}
triSimFrames 150
"remastered,ssr=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "ssr_bay_offshore__off"
"ssr_bay_offshore__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "ssr_bay_offshore__on"
"ssr_bay_offshore__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- materials_town_night_rain: materials (off = remastered,wetWeather=0,lampStreaks=0)
call { capRain = 1; capEyeAbs = false }
["materials_town_night_rain", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capSimT = 8000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered,wetWeather=0,lampStreaks=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "materials_town_night_rain__off"
"materials_town_night_rain__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "materials_town_night_rain__on"
"materials_town_night_rain__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- airbeams_pinewood_morn: air (beams) (off = remastered,airBeams=0)
call { capRain = 0; capEyeAbs = false }
["airbeams_pinewood_morn", 6830, 9905, 1.75, 72, 6, [1985, 7, 5, 6, 0], 0, 0.1, 1500] call capBegin
call { capSimT = 8600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,airBeams=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "airbeams_pinewood_morn__off"
"airbeams_pinewood_morn__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "airbeams_pinewood_morn__on"
"airbeams_pinewood_morn__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- airhaze_valley_0515: air (haze, valley mist) (off = remastered,airHaze=0,valleyMist=0)
call { capRain = 0; capEyeAbs = false }
["airhaze_valley_0515", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capSimT = 9200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,airHaze=0,valleyMist=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "airhaze_valley_0515__off"
"airhaze_valley_0515__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "airhaze_valley_0515__on"
"airhaze_valley_0515__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- hfrt_hill_1730: hfrt (off = remastered,canopyShadows=0)
call { capRain = 0; capEyeAbs = false }
["hfrt_hill_1730", 5500, 6250, 15, 185, -14, [1985, 9, 20, 17, 30], 0, 0, 2000] call capBegin
call { capSimT = 9800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,canopyShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hfrt_hill_1730__off"
"hfrt_hill_1730__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hfrt_hill_1730__on"
"hfrt_hill_1730__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- clouds_coast_oc_up: clouds (off = remastered,litClouds=0,skyCloudShadows=0)
call { capRain = 0; capEyeAbs = false }
["clouds_coast_oc_up", 1550, 5150, 3, 260, 14, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
call { capSimT = 10400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,litClouds=0,skyCloudShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "clouds_coast_oc_up__off"
"clouds_coast_oc_up__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "clouds_coast_oc_up__on"
"clouds_coast_oc_up__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- texsharp_forest_edge: texsharp (off = remastered,terrainDetail=0,sharpen=0)
call { capRain = 0; capEyeAbs = false }
["texsharp_forest_edge", 4674, 10156, 1.75, 75, 0, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capSimT = 11000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,terrainDetail=0,sharpen=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "texsharp_forest_edge__off"
"texsharp_forest_edge__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "texsharp_forest_edge__on"
"texsharp_forest_edge__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
