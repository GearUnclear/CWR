// Quiet-GPU timing on Eden: gen_timing.py, 3 interleaved rounds.
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
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Eden;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- overcast_broken round 1 (texsharp (meadow))
call { capRain = 0; capEyeAbs = false }
["overcast_broken_r1", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r1__classic"
"overcast_broken_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r1__remastered"
"overcast_broken_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- broken_high round 1 (clouds (shadows clip))
call { capRain = 0; capEyeAbs = false }
["broken_high_r1", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r1__classic"
"broken_high_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r1__remastered"
"broken_high_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- overcast_broken round 2 (texsharp (meadow))
call { capRain = 0; capEyeAbs = false }
["overcast_broken_r2", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r2__classic"
"overcast_broken_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r2__remastered"
"overcast_broken_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- broken_high round 2 (clouds (shadows clip))
call { capRain = 0; capEyeAbs = false }
["broken_high_r2", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r2__classic"
"broken_high_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r2__remastered"
"broken_high_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- overcast_broken round 3 (texsharp (meadow))
call { capRain = 0; capEyeAbs = false }
["overcast_broken_r3", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r3__classic"
"overcast_broken_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken_r3__remastered"
"overcast_broken_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- broken_high round 3 (clouds (shadows clip))
call { capRain = 0; capEyeAbs = false }
["broken_high_r3", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r3__classic"
"broken_high_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high_r3__remastered"
"broken_high_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
