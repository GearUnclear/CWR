// look-ref landed reference, 1080p look_final views: woods_pine2_evening,blue_hour_coast,dawn_valley,pine_rays,town_night
// look-director views on Abel: gen_look_director.py, modes landed.
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
capProbe = "HeliHEmpty" createVehicle [6250, 10800, 0]
capCam = "camera" camCreate [6250, 10800, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }

// ---- woods_pine2_evening
call { capRain = 0; capEyeAbs = false }
["woods_pine2_evening", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capSimT = 5000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__landed"
"woods_pine2_evening__landed" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- blue_hour_coast
call { capRain = 0; capEyeAbs = false }
["blue_hour_coast", 1550, 5150, 3, 295, 2, [1985, 7, 5, 19, 55], 0, 0, 4000] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "blue_hour_coast__landed"
"blue_hour_coast__landed" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- dawn_valley
call { capRain = 0; capEyeAbs = false }
["dawn_valley", 6100, 5000, 1.75, 40, -6, [1985, 7, 1, 5, 45], 0, 0, 4000] call capBegin
call { capSimT = 6200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "dawn_valley__landed"
"dawn_valley__landed" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- pine_rays
call { capRain = 0; capEyeAbs = false }
["pine_rays", 6830, 9905, 1.75, 72, 6, [1985, 7, 5, 6, 0], 0, 0.1, 1500] call capBegin
call { capSimT = 6800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pine_rays__landed"
"pine_rays__landed" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night
call { capRain = 0; capEyeAbs = false }
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call { capSimT = 7400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__landed"
"town_night__landed" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { "view=done;event=done" call capLog }
triEndTest
