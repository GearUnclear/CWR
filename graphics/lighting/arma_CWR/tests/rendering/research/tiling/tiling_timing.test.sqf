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
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 120
[] call capReadTimers
"noon_fields__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r0__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r0__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r0__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r1__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r1__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r1__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r2__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r2__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r2__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r3__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r3__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"noon_fields__r3__v4" call capRecord
0 call capVar

["high_fields_noon", 6250, 10800, 60, 165, -14, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 120
[] call capReadTimers
"high_fields_noon__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r0__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r0__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r0__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r1__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r1__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r1__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r2__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r2__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r2__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r3__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r3__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"high_fields_noon__r3__v4" call capRecord
0 call capVar

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r0__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r0__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r0__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r1__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r1__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r1__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r2__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r2__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r2__v4" call capRecord
0 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r3__v0" call capRecord
1 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r3__v1" call capRecord
4 call capVar
triSimFrames 120
[] call capReadTimers
"forest_edge_golden__r3__v4" call capRecord
0 call capVar

call { "view=done;event=done" call capLog }
triEndTest
