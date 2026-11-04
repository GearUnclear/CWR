// Quiet-GPU timing on Abel: gen_timing.py, 3 interleaved rounds.
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


// ---- sunrise_ridge round 1 (perf)
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge_r1", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r1__classic"
"sunrise_ridge_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r1__remastered"
"sunrise_ridge_r1__remastered" call capRecord
call { capDump = triShadowSceneDump "f10_sunrise_ridge"; format ["view=%1;event=f10;dump=%2", capView, capDump] call capLog }
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- forest_edge_golden round 1 (temporal, texsharp)
call { capRain = 0; capEyeAbs = false }
["forest_edge_golden_r1", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
    [["Jeep"], 25, 3, 90, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r1__classic"
"forest_edge_golden_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r1__remastered"
"forest_edge_golden_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night round 1 (light list)
call { capRain = 0; capEyeAbs = false }
["town_night_r1", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r1__classic"
"town_night_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r1__remastered"
"town_night_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coastline_sunset round 1 (sky table, water)
call { capRain = 0; capEyeAbs = false }
["coastline_sunset_r1", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r1__classic"
"coastline_sunset_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r1__remastered"
"coastline_sunset_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- woods_pine2_evening round 1 (shadows)
call { capRain = 0; capEyeAbs = false }
["woods_pine2_evening_r1", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r1__classic"
"woods_pine2_evening_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r1__remastered"
"woods_pine2_evening_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- lowsun_fp round 1 (shade)
call { capRain = 0; capEyeAbs = false }
["lowsun_fp_r1", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r1__classic"
"lowsun_fp_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r1__remastered"
"lowsun_fp_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- road_flat_drive_moonless round 1 (night)
call { capRain = 0; capEyeAbs = false }
["road_flat_drive_moonless_r1", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r1__classic"
"road_flat_drive_moonless_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r1__remastered"
"road_flat_drive_moonless_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- ffh round 1 (combat)
call { capRain = 0; capEyeAbs = false }
["ffh_r1", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
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
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r1__classic"
"ffh_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r1__remastered"
"ffh_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- bay_offshore round 1 (ssr)
call { capRain = 0; capEyeAbs = true }
["bay_offshore_r1", 9240, 6950, 3, 272, -3, [1985, 7, 5, 6, 30], 0, 0, 3000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r1__classic"
"bay_offshore_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r1__remastered"
"bay_offshore_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night_rain round 1 (materials)
call { capRain = 1; capEyeAbs = false }
["town_night_rain_r1", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r1__classic"
"town_night_rain_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r1__remastered"
"town_night_rain_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- pinewood_morn round 1 (air (beams))
call { capRain = 0; capEyeAbs = false }
["pinewood_morn_r1", 6830, 9905, 1.75, 72, 6, [1985, 7, 5, 6, 0], 0, 0.1, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r1__classic"
"pinewood_morn_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r1__remastered"
"pinewood_morn_r1__remastered" call capRecord
call { capDump = triShadowSceneDump "f10_pinewood_morn"; format ["view=%1;event=f10;dump=%2", capView, capDump] call capLog }
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- valley_0515 round 1 (air (haze, valley mist))
call { capRain = 0; capEyeAbs = false }
["valley_0515_r1", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r1__classic"
"valley_0515_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r1__remastered"
"valley_0515_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- hill_1730 round 1 (hfrt)
call { capRain = 0; capEyeAbs = false }
["hill_1730_r1", 5500, 6250, 15, 185, -14, [1985, 9, 20, 17, 30], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r1__classic"
"hill_1730_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r1__remastered"
"hill_1730_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coast_oc_up round 1 (clouds)
call { capRain = 0; capEyeAbs = false }
["coast_oc_up_r1", 1550, 5150, 3, 260, 14, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r1__classic"
"coast_oc_up_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r1__remastered"
"coast_oc_up_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sea_noon round 1 (water (noon); gate-off reference for low-sun features)
call { capRain = 0; capEyeAbs = false }
["sea_noon_r1", 1550, 5150, 12, 278, -12, [1985, 7, 5, 12, 30], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r1__classic"
"sea_noon_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r1__remastered"
"sea_noon_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- noon_fields round 1 (gate-off reference (noon, dry))
call { capRain = 0; capEyeAbs = false }
["noon_fields_r1", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r1__classic"
"noon_fields_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r1__remastered"
"noon_fields_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sunrise_ridge round 2 (perf)
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge_r2", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r2__classic"
"sunrise_ridge_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r2__remastered"
"sunrise_ridge_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- forest_edge_golden round 2 (temporal, texsharp)
call { capRain = 0; capEyeAbs = false }
["forest_edge_golden_r2", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
    [["Jeep"], 25, 3, 90, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r2__classic"
"forest_edge_golden_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r2__remastered"
"forest_edge_golden_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night round 2 (light list)
call { capRain = 0; capEyeAbs = false }
["town_night_r2", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r2__classic"
"town_night_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r2__remastered"
"town_night_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coastline_sunset round 2 (sky table, water)
call { capRain = 0; capEyeAbs = false }
["coastline_sunset_r2", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r2__classic"
"coastline_sunset_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r2__remastered"
"coastline_sunset_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- woods_pine2_evening round 2 (shadows)
call { capRain = 0; capEyeAbs = false }
["woods_pine2_evening_r2", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r2__classic"
"woods_pine2_evening_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r2__remastered"
"woods_pine2_evening_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- lowsun_fp round 2 (shade)
call { capRain = 0; capEyeAbs = false }
["lowsun_fp_r2", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r2__classic"
"lowsun_fp_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r2__remastered"
"lowsun_fp_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- road_flat_drive_moonless round 2 (night)
call { capRain = 0; capEyeAbs = false }
["road_flat_drive_moonless_r2", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r2__classic"
"road_flat_drive_moonless_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r2__remastered"
"road_flat_drive_moonless_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- ffh round 2 (combat)
call { capRain = 0; capEyeAbs = false }
["ffh_r2", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
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
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r2__classic"
"ffh_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r2__remastered"
"ffh_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- bay_offshore round 2 (ssr)
call { capRain = 0; capEyeAbs = true }
["bay_offshore_r2", 9240, 6950, 3, 272, -3, [1985, 7, 5, 6, 30], 0, 0, 3000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r2__classic"
"bay_offshore_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r2__remastered"
"bay_offshore_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night_rain round 2 (materials)
call { capRain = 1; capEyeAbs = false }
["town_night_rain_r2", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r2__classic"
"town_night_rain_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r2__remastered"
"town_night_rain_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- pinewood_morn round 2 (air (beams))
call { capRain = 0; capEyeAbs = false }
["pinewood_morn_r2", 6830, 9905, 1.75, 72, 6, [1985, 7, 5, 6, 0], 0, 0.1, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r2__classic"
"pinewood_morn_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r2__remastered"
"pinewood_morn_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- valley_0515 round 2 (air (haze, valley mist))
call { capRain = 0; capEyeAbs = false }
["valley_0515_r2", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r2__classic"
"valley_0515_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r2__remastered"
"valley_0515_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- hill_1730 round 2 (hfrt)
call { capRain = 0; capEyeAbs = false }
["hill_1730_r2", 5500, 6250, 15, 185, -14, [1985, 9, 20, 17, 30], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r2__classic"
"hill_1730_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r2__remastered"
"hill_1730_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coast_oc_up round 2 (clouds)
call { capRain = 0; capEyeAbs = false }
["coast_oc_up_r2", 1550, 5150, 3, 260, 14, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r2__classic"
"coast_oc_up_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r2__remastered"
"coast_oc_up_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sea_noon round 2 (water (noon); gate-off reference for low-sun features)
call { capRain = 0; capEyeAbs = false }
["sea_noon_r2", 1550, 5150, 12, 278, -12, [1985, 7, 5, 12, 30], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r2__classic"
"sea_noon_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r2__remastered"
"sea_noon_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- noon_fields round 2 (gate-off reference (noon, dry))
call { capRain = 0; capEyeAbs = false }
["noon_fields_r2", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r2__classic"
"noon_fields_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r2__remastered"
"noon_fields_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sunrise_ridge round 3 (perf)
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge_r3", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r3__classic"
"sunrise_ridge_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_r3__remastered"
"sunrise_ridge_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- forest_edge_golden round 3 (temporal, texsharp)
call { capRain = 0; capEyeAbs = false }
["forest_edge_golden_r3", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
    [["Jeep"], 25, 3, 90, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r3__classic"
"forest_edge_golden_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden_r3__remastered"
"forest_edge_golden_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night round 3 (light list)
call { capRain = 0; capEyeAbs = false }
["town_night_r3", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r3__classic"
"town_night_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_r3__remastered"
"town_night_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coastline_sunset round 3 (sky table, water)
call { capRain = 0; capEyeAbs = false }
["coastline_sunset_r3", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r3__classic"
"coastline_sunset_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset_r3__remastered"
"coastline_sunset_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- woods_pine2_evening round 3 (shadows)
call { capRain = 0; capEyeAbs = false }
["woods_pine2_evening_r3", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r3__classic"
"woods_pine2_evening_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening_r3__remastered"
"woods_pine2_evening_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- lowsun_fp round 3 (shade)
call { capRain = 0; capEyeAbs = false }
["lowsun_fp_r3", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r3__classic"
"lowsun_fp_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_r3__remastered"
"lowsun_fp_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- road_flat_drive_moonless round 3 (night)
call { capRain = 0; capEyeAbs = false }
["road_flat_drive_moonless_r3", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r3__classic"
"road_flat_drive_moonless_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "road_flat_drive_moonless_r3__remastered"
"road_flat_drive_moonless_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- ffh round 3 (combat)
call { capRain = 0; capEyeAbs = false }
["ffh_r3", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
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
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r3__classic"
"ffh_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "ffh_r3__remastered"
"ffh_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- bay_offshore round 3 (ssr)
call { capRain = 0; capEyeAbs = true }
["bay_offshore_r3", 9240, 6950, 3, 272, -3, [1985, 7, 5, 6, 30], 0, 0, 3000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r3__classic"
"bay_offshore_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore_r3__remastered"
"bay_offshore_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night_rain round 3 (materials)
call { capRain = 1; capEyeAbs = false }
["town_night_rain_r3", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r3__classic"
"town_night_rain_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain_r3__remastered"
"town_night_rain_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- pinewood_morn round 3 (air (beams))
call { capRain = 0; capEyeAbs = false }
["pinewood_morn_r3", 6830, 9905, 1.75, 72, 6, [1985, 7, 5, 6, 0], 0, 0.1, 1500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r3__classic"
"pinewood_morn_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pinewood_morn_r3__remastered"
"pinewood_morn_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- valley_0515 round 3 (air (haze, valley mist))
call { capRain = 0; capEyeAbs = false }
["valley_0515_r3", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r3__classic"
"valley_0515_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "valley_0515_r3__remastered"
"valley_0515_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- hill_1730 round 3 (hfrt)
call { capRain = 0; capEyeAbs = false }
["hill_1730_r3", 5500, 6250, 15, 185, -14, [1985, 9, 20, 17, 30], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r3__classic"
"hill_1730_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "hill_1730_r3__remastered"
"hill_1730_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coast_oc_up round 3 (clouds)
call { capRain = 0; capEyeAbs = false }
["coast_oc_up_r3", 1550, 5150, 3, 260, 14, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r3__classic"
"coast_oc_up_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up_r3__remastered"
"coast_oc_up_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sea_noon round 3 (water (noon); gate-off reference for low-sun features)
call { capRain = 0; capEyeAbs = false }
["sea_noon_r3", 1550, 5150, 12, 278, -12, [1985, 7, 5, 12, 30], 0, 0, 4000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r3__classic"
"sea_noon_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon_r3__remastered"
"sea_noon_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- noon_fields round 3 (gate-off reference (noon, dry))
call { capRain = 0; capEyeAbs = false }
["noon_fields_r3", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r3__classic"
"noon_fields_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields_r3__remastered"
"noon_fields_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
