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
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel" call capLog }

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
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
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v0"
"noon_fields__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v1"
"noon_fields__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v2"
"noon_fields__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v3"
"noon_fields__v3" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v4"
"noon_fields__v4" call capRecord
5 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v5"
"noon_fields__v5" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v6"
"noon_fields__v6" call capRecord
7 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v7"
"noon_fields__v7" call capRecord
9 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v9"
"noon_fields__v9" call capRecord
0 call capVar
[] call capClear

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__classic"
"forest_edge_golden__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v0"
"forest_edge_golden__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v1"
"forest_edge_golden__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v2"
"forest_edge_golden__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v3"
"forest_edge_golden__v3" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v4"
"forest_edge_golden__v4" call capRecord
5 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v5"
"forest_edge_golden__v5" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v6"
"forest_edge_golden__v6" call capRecord
7 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v7"
"forest_edge_golden__v7" call capRecord
9 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v9"
"forest_edge_golden__v9" call capRecord
0 call capVar
[] call capClear

["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v0"
"lowsun_fp__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v1"
"lowsun_fp__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v2"
"lowsun_fp__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v3"
"lowsun_fp__v3" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v4"
"lowsun_fp__v4" call capRecord
5 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v5"
"lowsun_fp__v5" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v6"
"lowsun_fp__v6" call capRecord
7 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v7"
"lowsun_fp__v7" call capRecord
9 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__v9"
"lowsun_fp__v9" call capRecord
0 call capVar
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
