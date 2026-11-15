// look-light hero views on Abel: gen_look_light.py, modes classic, off, on.
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
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- lowsun_fp
call { capRain = 0; capEyeAbs = false }
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__off"
"lowsun_fp__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__on"
"lowsun_fp__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- noon_fields
call { capRain = 0; capEyeAbs = false }
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call { capSimT = 2600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triScreenshot "noon_fields__classic"
"noon_fields__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__off"
"noon_fields__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__on"
"noon_fields__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- forest_edge_golden
call { capRain = 0; capEyeAbs = false }
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triScreenshot "forest_edge_golden__classic"
"forest_edge_golden__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__off"
"forest_edge_golden__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__on"
"forest_edge_golden__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sunrise_ridge
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__classic"
"sunrise_ridge__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__off"
"sunrise_ridge__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__on"
"sunrise_ridge__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coastline_sunset
call { capRain = 0; capEyeAbs = false }
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 4400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__off"
"coastline_sunset__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__on"
"coastline_sunset__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night
call { capRain = 0; capEyeAbs = false }
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capSimT = 5000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triScreenshot "town_night__classic"
"town_night__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__off"
"town_night__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__on"
"town_night__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- woods_pine2_evening
call { capRain = 0; capEyeAbs = false }
["woods_pine2_evening", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triScreenshot "woods_pine2_evening__classic"
"woods_pine2_evening__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__off"
"woods_pine2_evening__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__on"
"woods_pine2_evening__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- blue_hour_coast
call { capRain = 0; capEyeAbs = false }
["blue_hour_coast", 1550, 5150, 3, 295, 2, [1985, 7, 5, 19, 55], 0, 0, 4000] call capBegin
call { capSimT = 6200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__classic"
"blue_hour_coast__classic" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__off"
"blue_hour_coast__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__on"
"blue_hour_coast__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
