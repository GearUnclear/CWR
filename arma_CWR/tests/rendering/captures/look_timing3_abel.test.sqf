// look-light hero views on Abel: gen_look_light.py, modes off1, on1, off2, on2, off3, on3.
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
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__off1"
"lowsun_fp__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__on1"
"lowsun_fp__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__off2"
"lowsun_fp__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__on2"
"lowsun_fp__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__off3"
"lowsun_fp__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__on3"
"lowsun_fp__on3" call capRecord
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
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__off1"
"noon_fields__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__on1"
"noon_fields__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__off2"
"noon_fields__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__on2"
"noon_fields__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__off3"
"noon_fields__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__on3"
"noon_fields__on3" call capRecord
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
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__off1"
"forest_edge_golden__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__on1"
"forest_edge_golden__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__off2"
"forest_edge_golden__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__on2"
"forest_edge_golden__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__off3"
"forest_edge_golden__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__on3"
"forest_edge_golden__on3" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sunrise_ridge
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__off1"
"sunrise_ridge__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__on1"
"sunrise_ridge__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__off2"
"sunrise_ridge__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__on2"
"sunrise_ridge__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__off3"
"sunrise_ridge__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__on3"
"sunrise_ridge__on3" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- coastline_sunset
call { capRain = 0; capEyeAbs = false }
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 4400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__off1"
"coastline_sunset__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__on1"
"coastline_sunset__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__off2"
"coastline_sunset__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__on2"
"coastline_sunset__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__off3"
"coastline_sunset__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__on3"
"coastline_sunset__on3" call capRecord
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
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__off1"
"town_night__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__on1"
"town_night__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__off2"
"town_night__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__on2"
"town_night__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__off3"
"town_night__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__on3"
"town_night__on3" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- blue_hour_coast
call { capRain = 0; capEyeAbs = false }
["blue_hour_coast", 1550, 5150, 3, 295, 2, [1985, 7, 5, 19, 55], 0, 0, 4000] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__off1"
"blue_hour_coast__off1" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__on1"
"blue_hour_coast__on1" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__off2"
"blue_hour_coast__off2" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__on2"
"blue_hour_coast__on2" call capRecord
"remastered,photoLight=0" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__off3"
"blue_hour_coast__off3" call capRecord
"remastered" call capMode
triSimFrames 150
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__on3"
"blue_hour_coast__on3" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
