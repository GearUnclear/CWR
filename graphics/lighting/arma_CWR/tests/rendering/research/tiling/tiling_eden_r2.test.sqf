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
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Eden" call capLog }

["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__classic"
"overcast_broken__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v0"
"overcast_broken__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v1"
"overcast_broken__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v2"
"overcast_broken__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v3"
"overcast_broken__v3" call capRecord
5 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v5"
"overcast_broken__v5" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__v6"
"overcast_broken__v6" call capRecord
0 call capVar
[] call capClear

["high_valley_eden", 9200, 3700, 45, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__classic"
"high_valley_eden__classic" call capRecord
"remastered" call capMode
0 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v0"
"high_valley_eden__v0" call capRecord
1 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v1"
"high_valley_eden__v1" call capRecord
2 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v2"
"high_valley_eden__v2" call capRecord
3 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v3"
"high_valley_eden__v3" call capRecord
5 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v5"
"high_valley_eden__v5" call capRecord
6 call capVar
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high_valley_eden__v6"
"high_valley_eden__v6" call capRecord
0 call capVar
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
