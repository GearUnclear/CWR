// ============================================================================
//  RESEARCH ONLY (bush-instancing bug fix evidence, not committed).
//  Abel noon_fields as in tests/rendering/captures/abel_lighting.test.sqf,
//  plus a zoomed view of the bush-covered slope to the right of it, so that a
//  bush drawn with another bush's terrain fit is large enough to see.
// ============================================================================

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
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;research=bush-instancing" call capLog }

// 1. noon_fields, unchanged from the capture set.
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
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__remastered"
"noon_fields__remastered" call capRecord
[] call capClear

// 2. noon_bushes_zoom: same eye point and time, turned 14 degrees right and
// zoomed (fov 0.25) onto the slope of scattered bushes at 150 to 400 m.
call { capFov = 0.25 }
["noon_bushes_zoom", 6250, 10800, 1.75, 164, -3, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_bushes_zoom__classic"
"noon_bushes_zoom__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_bushes_zoom__remastered"
"noon_bushes_zoom__remastered" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
