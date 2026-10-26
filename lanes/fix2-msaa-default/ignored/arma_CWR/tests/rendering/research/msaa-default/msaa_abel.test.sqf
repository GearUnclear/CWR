// Bug fix msaa-default: forest_edge_golden at the SHIPPED anti-aliasing
// default. Research copy of tests/rendering/captures/abel_lighting.test.sqf
// view 4 with triSetMsaa and triSetRenderScale removed, so the frame target
// carries whatever graphics.cfg gives a fresh profile (tri boots every test
// with an empty user dir). Not a regression test.

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
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [4674, 10156, 0]
capCam = "camera" camCreate [4674, 10156, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=default;scale=default;window=960x640;capture=present" call capLog }

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
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
