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
capProbe = "HeliHEmpty" createVehicle [4400, 3300, 0]
capCam = "camera" camCreate [4400, 3300, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

capSkyOnly = { ["skyClouds=1", "cloudShadowSource=0", "r0=0", "r1=0", "r4=0", "r5=0", "r7=0", "debug=0"] call capKnobs; capModeName = "skyOnly" }
capShadowOnly = { ["skyClouds=0", "cloudShadowSource=1", "r0=0", "r1=0", "r4=0", "r5=0", "r7=0", "debug=0"] call capKnobs; capModeName = "shadowOnly" }

// ---- sunrise_oc interleaved timing
["sunrise_oc", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0.5, 0, 4000] call capBegin
triSimFrames 150
"remastered" call capMode
call { call capCur; capModeName = "remcur" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__remcur"
"sunrise_oc__remcur" call capRecord
call { call capProtoA; capModeName = "protoA" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__protoA"
"sunrise_oc__protoA" call capRecord
call { call capSkyOnly; capModeName = "skyOnly" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__skyOnly"
"sunrise_oc__skyOnly" call capRecord
call { call capShadowOnly; capModeName = "shadowOnly" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__shadowOnly"
"sunrise_oc__shadowOnly" call capRecord
call { call capCur; capModeName = "remcur2" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__remcur2"
"sunrise_oc__remcur2" call capRecord
call { call capProtoA; capModeName = "protoA2" }
triGpuTimes false
triGpuTimes true
triPerfReset 0
triSimFrames 120
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_oc__protoA2"
"sunrise_oc__protoA2" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
