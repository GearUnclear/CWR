// SSR research captures, Abel. Three-way: classic | remastered | remastered + SSR,
// plus debug shots of the surface tags. Research copy (gfx-ssr), not committed.
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
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;track=ssr" call capLog }

// lowsun_fp: the acceptance stand-in (glossy materials; wet variant).
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssr"
"lowsun_fp__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=0.8"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrwet"
"lowsun_fp__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=0.8"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrtags"
"lowsun_fp__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

call { "view=done;event=done" call capLog }
triEndTest
