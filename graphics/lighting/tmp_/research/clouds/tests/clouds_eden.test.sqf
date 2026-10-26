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

// ---------------------------------------------------------------- overcast_broken
["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call { [["Jeep"], 12, -5, 70, "veh"] call capSpawn; }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__classic"
"overcast_broken__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__remcur"
"overcast_broken__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__protoA"
"overcast_broken__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__protoB"
"overcast_broken__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__protoC"
"overcast_broken__protoC" call capRecord
call { call capProtoA; ["r7=1"] call capKnobs; capModeName = "dbgdens" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__dbgdens"
"overcast_broken__dbgdens" call capRecord
call { ["r7=2"] call capKnobs; capModeName = "dbgrelief" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__dbgrelief"
"overcast_broken__dbgrelief" call capRecord
call { ["r7=0", "debug=1"] call capKnobs; capModeName = "dbgsunA" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__dbgsunA"
"overcast_broken__dbgsunA" call capRecord
call { call capCur; ["debug=1"] call capKnobs; capModeName = "dbgsunCur" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__dbgsunCur"
"overcast_broken__dbgsunCur" call capRecord
call { ["debug=0"] call capKnobs }
[] call capClear

// ---------------------------------------------------------------- broken_up
["broken_up", 9200, 3700, 1.75, 250, 22, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_up__classic"
"broken_up__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_up__remcur"
"broken_up__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_up__protoA"
"broken_up__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_up__protoB"
"broken_up__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_up__protoC"
"broken_up__protoC" call capRecord
call { call capProtoA; ["r7=1"] call capKnobs; capModeName = "dbgdens" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_up__dbgdens"
"broken_up__dbgdens" call capRecord
call { ["r7=2"] call capKnobs; capModeName = "dbgrelief" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_up__dbgrelief"
"broken_up__dbgrelief" call capRecord
call { ["r7=0", "debug=1"] call capKnobs; capModeName = "dbgsunA" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_up__dbgsunA"
"broken_up__dbgsunA" call capRecord
call { call capCur; ["debug=1"] call capKnobs; capModeName = "dbgsunCur" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_up__dbgsunCur"
"broken_up__dbgsunCur" call capRecord
call { ["debug=0"] call capKnobs }

// ---------------------------------------------------------------- broken_high
["broken_high", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high__classic"
"broken_high__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high__remcur"
"broken_high__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high__protoA"
"broken_high__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high__protoB"
"broken_high__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "broken_high__protoC"
"broken_high__protoC" call capRecord
call { call capProtoA; ["r7=1"] call capKnobs; capModeName = "dbgdens" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_high__dbgdens"
"broken_high__dbgdens" call capRecord
call { ["r7=2"] call capKnobs; capModeName = "dbgrelief" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_high__dbgrelief"
"broken_high__dbgrelief" call capRecord
call { ["r7=0", "debug=1"] call capKnobs; capModeName = "dbgsunA" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_high__dbgsunA"
"broken_high__dbgsunA" call capRecord
call { call capCur; ["debug=1"] call capKnobs; capModeName = "dbgsunCur" }
triSimFrames 5
[] call capPin
triSimFrames 3
triScreenshot "broken_high__dbgsunCur"
"broken_high__dbgsunCur" call capRecord
call { ["debug=0"] call capKnobs }

// ---------------------------------------------------------------- overcast_heavy
["overcast_heavy", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.9, 0, 3500] call capBegin
call { [["Jeep"], 12, -5, 70, "veh"] call capSpawn; }
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__classic"
"overcast_heavy__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__remcur"
"overcast_heavy__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__protoA"
"overcast_heavy__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__protoB"
"overcast_heavy__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__protoC"
"overcast_heavy__protoC" call capRecord
[] call capClear

// ---------------------------------------------------------------- heavy_up
["heavy_up", 9200, 3700, 1.75, 250, 22, [1985, 7, 5, 14, 30], 0.9, 0, 3500] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "heavy_up__classic"
"heavy_up__classic" call capRecord
"remastered" call capMode
call capCur
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "heavy_up__remcur"
"heavy_up__remcur" call capRecord
call capProtoA
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "heavy_up__protoA"
"heavy_up__protoA" call capRecord
call capProtoB
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "heavy_up__protoB"
"heavy_up__protoB" call capRecord
call capProtoC
triGpuTimes false
triGpuTimes true
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "heavy_up__protoC"
"heavy_up__protoC" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
