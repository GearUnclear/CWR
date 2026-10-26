// Research-only (clouds track, git-excluded): lit sky clouds and sky-consistent
// cloud shadows. Shots per view: Classic, current Remastered (skyClouds=0,
// cloudShadowSource=0), prototype A (defaults), B (stronger relief and silver
// lining), C (A + a lower, denser shadow deck).
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
capKnobs = { { capKnobRes = triSetLighting _x; format ["view=%1;event=knob;set=%2;res=%3", capView, _x, capKnobRes] call capLog } forEach _this }
capCur = { ["skyClouds=0", "cloudShadowSource=0", "r0=0", "r1=0", "r4=0", "r5=0", "r7=0", "debug=0"] call capKnobs; capModeName = "remcur" }
capProtoA = { ["skyClouds=1", "cloudShadowSource=1", "r0=0", "r1=0", "r4=0", "r5=0", "r7=0", "debug=0"] call capKnobs; capModeName = "protoA" }
capProtoB = { ["skyClouds=1", "cloudShadowSource=1", "r0=0", "r1=0", "r4=2.8", "r5=1.8", "r7=0", "debug=0"] call capKnobs; capModeName = "protoB" }
capProtoC = { ["skyClouds=1", "cloudShadowSource=1", "r0=1200", "r1=0.42", "r4=0", "r5=0", "r7=0", "debug=0"] call capKnobs; capModeName = "protoC" }

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

// ---------------------------------------------------------------- broken_high sequence
["broken_high", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
triSimFrames 150
"remastered" call capMode
call capProtoA
triSimFrames 60
call { ["r6=0"] call capKnobs; capModeName = "drift00" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift00"
"broken_high__drift00" call capRecord
call { ["r6=3"] call capKnobs; capModeName = "drift03" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift03"
"broken_high__drift03" call capRecord
call { ["r6=6"] call capKnobs; capModeName = "drift06" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift06"
"broken_high__drift06" call capRecord
call { ["r6=9"] call capKnobs; capModeName = "drift09" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift09"
"broken_high__drift09" call capRecord
call { ["r6=12"] call capKnobs; capModeName = "drift12" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift12"
"broken_high__drift12" call capRecord
call { ["r6=15"] call capKnobs; capModeName = "drift15" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_high__drift15"
"broken_high__drift15" call capRecord
call { ["r6=0"] call capKnobs }

// ---------------------------------------------------------------- broken_up sequence
["broken_up", 9200, 3700, 1.75, 250, 22, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
triSimFrames 150
"remastered" call capMode
call capProtoA
triSimFrames 60
call { ["r6=0"] call capKnobs; capModeName = "drift00" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift00"
"broken_up__drift00" call capRecord
call { ["r6=3"] call capKnobs; capModeName = "drift03" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift03"
"broken_up__drift03" call capRecord
call { ["r6=6"] call capKnobs; capModeName = "drift06" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift06"
"broken_up__drift06" call capRecord
call { ["r6=9"] call capKnobs; capModeName = "drift09" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift09"
"broken_up__drift09" call capRecord
call { ["r6=12"] call capKnobs; capModeName = "drift12" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift12"
"broken_up__drift12" call capRecord
call { ["r6=15"] call capKnobs; capModeName = "drift15" }
triSimFrames 4
[] call capPin
triSimFrames 3
triScreenshot "broken_up__drift15"
"broken_up__drift15" call capRecord
call { ["r6=0"] call capKnobs }

call { "view=done;event=done" call capLog }
triEndTest
