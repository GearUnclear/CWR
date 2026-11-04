// look-ref vanilla reference, 1080p look_final views: town_fog_night,town_night_rain,jeep_close_noon,town_noon,forest_morning
// look-director views on Abel: gen_look_director.py, modes vanilla.
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
// (vanilla: no triCaptureAtPresent; gamma 1.2 is applied offline by the collector)
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

// ---- town_fog_night
call { capRain = 0; capEyeAbs = false }
["town_fog_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0.35, 800] call capBegin
call { capSimT = 8000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
    capJeep = [["Jeep"], 22, 1.5, 180, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_fog_night__vanilla"
"town_fog_night__vanilla" call capRecord
[] call capClear
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- town_night_rain
call { capRain = 1; capEyeAbs = false }
["town_night_rain", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0.9, 0, 800] call capBegin
call { capSimT = 8600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night_rain__vanilla"
"town_night_rain__vanilla" call capRecord
[] call capClear

// ---- jeep_close_noon
call { capRain = 0; capEyeAbs = false }
["jeep_close_noon", 6250, 10800, 1.75, 150, -12, [1985, 7, 5, 12, 30], 0, 0, 1500] call capBegin
call { capSimT = 9200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 6.5, 0.5, 70, "veh"] call capSpawn;
    [["SoldierWB"], 5, -2.2, 200, "man"] call capSpawn;
    [["M113"], 14, 4, 30, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "jeep_close_noon__vanilla"
"jeep_close_noon__vanilla" call capRecord
[] call capClear

// ---- town_noon
call { capRain = 0; capEyeAbs = false }
["town_noon", 7152, 8915, 1.75, 345, 3, [1985, 7, 5, 11, 0], 0, 0, 800] call capBegin
call { capSimT = 9800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 14, -3, 30, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_noon__vanilla"
"town_noon__vanilla" call capRecord
[] call capClear

// ---- forest_morning
call { capRain = 0; capEyeAbs = false }
["forest_morning", 4674, 10156, 1.75, 75, 0, [1985, 7, 5, 9, 30], 0, 0, 1500] call capBegin
call { capSimT = 10400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["SoldierWB"], 10, 1, 200, "man"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_morning__vanilla"
"forest_morning__vanilla" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
