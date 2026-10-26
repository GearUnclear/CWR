// Research-only (clouds track, git-excluded): lit sky clouds and sky-consistent
// cloud shadows. Shots per view: Classic, current Remastered (skyClouds=0,
// cloudShadowSource=0), prototype A (defaults), B (stronger relief and silver
// lining), C (A + a lower, denser shadow deck).
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
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// ---------------------------------------------------------------- sunrise_oc
["sunrise_oc", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0.5, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__classic"
"sunrise_oc__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__remcur"
"sunrise_oc__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__protoA"
"sunrise_oc__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__protoB"
"sunrise_oc__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__protoC"
"sunrise_oc__protoC" call capRecord

// ---------------------------------------------------------------- coast_oc
["coast_oc", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc__classic"
"coast_oc__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc__remcur"
"coast_oc__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc__protoA"
"coast_oc__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc__protoB"
"coast_oc__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc__protoC"
"coast_oc__protoC" call capRecord

// ---------------------------------------------------------------- coast_oc_up
["coast_oc_up", 1550, 5150, 3, 278, 14, [1985, 7, 5, 18, 35], 0.5, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up__classic"
"coast_oc_up__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up__remcur"
"coast_oc_up__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up__protoA"
"coast_oc_up__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up__protoB"
"coast_oc_up__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_oc_up__protoC"
"coast_oc_up__protoC" call capRecord

// ---------------------------------------------------------------- coastline_sunset
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remcur"
"coastline_sunset__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__protoA"
"coastline_sunset__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__protoB"
"coastline_sunset__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__protoC"
"coastline_sunset__protoC" call capRecord

// ---------------------------------------------------------------- sunrise_ridge
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__classic"
"sunrise_ridge__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__remcur"
"sunrise_ridge__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__protoA"
"sunrise_ridge__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__protoB"
"sunrise_ridge__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__protoC"
"sunrise_ridge__protoC" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
