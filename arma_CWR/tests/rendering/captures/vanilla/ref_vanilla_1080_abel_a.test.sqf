// look-ref vanilla reference, 1080p look_final views: noon_fields,lowsun_fp,forest_edge_golden,sunrise_ridge,coastline_sunset
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


// ---- noon_fields
call { capRain = 0; capEyeAbs = false }
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__vanilla"
"noon_fields__vanilla" call capRecord
[] call capClear

// ---- lowsun_fp
call { capRain = 0; capEyeAbs = false }
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capSimT = 2600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__vanilla"
"lowsun_fp__vanilla" call capRecord
[] call capClear

// ---- forest_edge_golden
call { capRain = 0; capEyeAbs = false }
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
    [["Jeep"], 25, 3, 90, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__vanilla"
"forest_edge_golden__vanilla" call capRecord
[] call capClear

// ---- sunrise_ridge
call { capRain = 0; capEyeAbs = false }
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__vanilla"
"sunrise_ridge__vanilla" call capRecord
[] call capClear

// ---- coastline_sunset
call { capRain = 0; capEyeAbs = false }
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 4400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__vanilla"
"coastline_sunset__vanilla" call capRecord
[] call capClear
call { "view=done;event=done" call capLog }
triEndTest
