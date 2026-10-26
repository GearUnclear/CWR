// Research-only: where are the remastered cloud shadows at overcast 0.5?
// overcast_broken as in eden_lighting, plus debug=1 (sun visibility), plus a
// high vantage over the same valley.
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
capKnob = { capKnobRes = triSetLighting _this; format ["view=%1;event=knob;set=%2;res=%3", capView, _this, capKnobRes] call capLog }

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
call { "view=setup;event=setup;world=Eden;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

["ob", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "ob__remastered"
"ob__remastered" call capRecord
call { "debug=1" call capKnob }
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "ob_dbgsun__remastered"
"ob_dbgsun__remastered" call capRecord
call { "debug=0" call capKnob }

["high", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high__classic"
"high__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "high__remastered"
"high__remastered" call capRecord
call { "debug=1" call capKnob }
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "high_dbgsun__remastered"
"high_dbgsun__remastered" call capRecord
call { "debug=0" call capKnob }

call { "view=done;event=done" call capLog }
triEndTest
