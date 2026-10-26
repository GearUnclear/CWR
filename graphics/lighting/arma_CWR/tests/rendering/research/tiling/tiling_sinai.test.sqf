#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Sinai"
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

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [8350, 2950, 0]
capCam = "camera" camCreate [8350, 2950, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Sinai" call capLog }

["sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__classic"
"sinai_desert_noon__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__v0"
"sinai_desert_noon__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__v1"
"sinai_desert_noon__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__v2"
"sinai_desert_noon__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__v3"
"sinai_desert_noon__v3" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__v4"
"sinai_desert_noon__v4" call capRecord
0 call capVar
[] call capClear

["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__classic"
"sinai_lowsun_fp__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__v0"
"sinai_lowsun_fp__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__v1"
"sinai_lowsun_fp__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__v2"
"sinai_lowsun_fp__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__v3"
"sinai_lowsun_fp__v3" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__v4"
"sinai_lowsun_fp__v4" call capRecord
0 call capVar
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
