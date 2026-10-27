// look-ref vanilla reference, 1080p look_final views: overcast_broken,overcast_heavy,rain_day,noon_vista
// look-director views on Eden: gen_look_director.py, modes vanilla.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Eden"
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
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Eden;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- overcast_broken
call { capRain = 0; capEyeAbs = false }
["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call { capSimT = 11000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__vanilla"
"overcast_broken__vanilla" call capRecord
[] call capClear

// ---- overcast_heavy
call { capRain = 0; capEyeAbs = false }
["overcast_heavy", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.9, 0, 3500] call capBegin
call { capSimT = 11600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__vanilla"
"overcast_heavy__vanilla" call capRecord
[] call capClear

// ---- rain_day
call { capRain = 0.8; capEyeAbs = false }
["rain_day", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 1, 0, 3500] call capBegin
call { capSimT = 12200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "rain_day__vanilla"
"rain_day__vanilla" call capRecord
[] call capClear

// ---- noon_vista
call { capRain = 0; capEyeAbs = false }
["noon_vista", 9200, 3700, 150, 288, -5, [1985, 7, 5, 12, 30], 0.05, 0, 5000] call capBegin
call { capSimT = 12800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_vista__vanilla"
"noon_vista__vanilla" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
