// P0-6 determinism stills (deterministic recipe, two shots per view; needs the P0-6 verbs). Abel.
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
capProbe = "HeliHEmpty" createVehicle [6100, 5000, 0]
capCam = "camera" camCreate [6100, 5000, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// P0-6 determinism helpers (a build with the P0-6 verbs).
capDetBlock = { capDetRes = triDeterministic true; capRstRes = triResetTemporal [0, 0]; capSeedRes = triSeedRandom 1; format ["view=%1;event=det;det=%2;reset=%3;seed=%4", capView, capDetRes, capRstRes, capSeedRes] call capLog }
capDetState = { format ["view=%1;event=detstate;label=%2;state=%3", capView, _this, triTemporalState] call capLog }
call { capStepRes = triSetFixedStep 0.0166667; format ["view=setup;event=step;res=%1", capStepRes] call capLog }


// ---- valley_0515
call { capRain = 0; capEyeAbs = false }
["valley_0515", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
[] call capPin
triSimFrames 60
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "valley_0515__det1"
"valley_0515__det1" call capRecord
"valley_0515__det1" call capDetState
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "valley_0515__det2"
"valley_0515__det2" call capRecord
"valley_0515__det2" call capDetState
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
"remastered" call capMode
[] call capPin
triSimFrames 60
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "noon_fields__det1"
"noon_fields__det1" call capRecord
"noon_fields__det1" call capDetState
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "noon_fields__det2"
"noon_fields__det2" call capRecord
"noon_fields__det2" call capDetState
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sunrise_ridge
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
[] call capPin
triSimFrames 60
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "sunrise_ridge__det1"
"sunrise_ridge__det1" call capRecord
"sunrise_ridge__det1" call capDetState
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "sunrise_ridge__det2"
"sunrise_ridge__det2" call capRecord
"sunrise_ridge__det2" call capDetState
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- town_night_rain
call { capRain = 1; capEyeAbs = false }
["town_night_rain", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 60
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
[] call capDetBlock
triSimFrames 32
triScreenshot "town_night_rain__det1"
"town_night_rain__det1" call capRecord
"town_night_rain__det1" call capDetState
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
[] call capDetBlock
triSimFrames 32
triScreenshot "town_night_rain__det2"
"town_night_rain__det2" call capRecord
"town_night_rain__det2" call capDetState
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { capDetRes = triDeterministic false; capRstRes = triResetTemporal []; capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }
triEndTest
