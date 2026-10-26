// B6 landing check on the merged build: the smoke_abel view with the ground
// forced wet (ssrWet=1), so the reflections' gate opens while the air pass's
// haze runs: ssr off, ssr on, ssr on with the air off, and the reflection view.
#include "capture_lib.sqf"
capWorld = "Abel"
capView = "landchk_abel"
capOX = 7300
capOZ = 10900
capH = 152.5
capDate = [1985, 3, 1, 8, 0]
triSimUntil { alive player }
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
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
capProbe = "HeliHEmpty" createVehicle [capOX, capOZ, 0]
capCam = "camera" camCreate [capOX, capOZ, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
[capView, capOX, capOZ, 1.75, capH, -3, capDate, 0, 0, 1500] call capBegin
call { [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn; [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn }
triSimFrames 120
triAssertEq [(triSetLighting "remastered,ssr=0,ssrWet=1"), "OK"]
triSimFrames 40
call { capModeName = "ssr0"; capLit = "OK"; [] call capReadTimers }
triScreenshot "land_1__ssr0"
"land_1__ssr0" call capRecord
triAssertEq [(triSetLighting "remastered,ssrWet=1"), "OK"]
triSimFrames 40
call { capModeName = "ssr1"; [] call capReadTimers }
triScreenshot "land_2__ssr1"
"land_2__ssr1" call capRecord
triAssertEq [(triSetLighting "remastered,ssrWet=1,airHaze=0,airBeams=0,valleyMist=0"), "OK"]
triSimFrames 40
call { capModeName = "ssr1air0"; [] call capReadTimers }
triScreenshot "land_3__ssr1air0"
"land_3__ssr1air0" call capRecord
triAssertEq [(triSetLighting "remastered,ssrWet=1,debug=10"), "OK"]
triSimFrames 40
call { capModeName = "ssrview"; [] call capReadTimers }
triScreenshot "land_4__ssrview"
"land_4__ssrview" call capRecord
triEndTest
