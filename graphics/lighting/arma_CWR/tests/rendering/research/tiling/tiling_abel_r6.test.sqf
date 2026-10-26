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
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__v4"
"noon_fields__v4" call capRecord
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
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__v4"
"forest_edge_golden__v4" call capRecord
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
0 call capVar
[] call capClear

["high_fields_noon", 6250, 10800, 60, 165, -14, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_fields_noon__classic"
"high_fields_noon__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_fields_noon__v0"
"high_fields_noon__v0" call capRecord
4 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_fields_noon__v4"
"high_fields_noon__v4" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_fields_noon__v6"
"high_fields_noon__v6" call capRecord
7 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_fields_noon__v7"
"high_fields_noon__v7" call capRecord
0 call capVar
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
