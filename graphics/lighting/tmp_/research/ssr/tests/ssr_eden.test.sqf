// SSR research captures, Eden: overcast_heavy with rain on (wet ground).
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
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
call { "view=setup;event=setup;world=Eden;track=ssr" call capLog }

// overcast_heavy_rain: the committed overcast_heavy frame with rain on.
capRain = 1
["overcast_heavy_rain", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.9, 0, 3500] call capBegin
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 240
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy_rain__classic"
"overcast_heavy_rain__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy_rain__remastered"
"overcast_heavy_rain__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0", "ssrWet=-1"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy_rain__ssr"
"overcast_heavy_rain__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=1"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy_rain__ssrwet"
"overcast_heavy_rain__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy_rain__ssrtags"
"overcast_heavy_rain__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssr=0" }
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
