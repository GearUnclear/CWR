// Research (shadow-sampler bug, git-excluded). Drives every path that can
// leave texture unit 2 (the shadowMap sampler of every lit program) without a
// complete depth array: start-up before any shadow map, classic with shadow
// maps, a shadow-map resolution and cascade-count change in both lighting
// models (the cascade array is deleted and recreated inside the world pass),
// the shadow-map self-test from script (deleted and recreated outside the
// world pass) and the classic/remastered toggle. The evidence is
// game_stdout.log: no GL id 131204 or 131222 line.
#include "capture_lib.sqf"
capWorld = "Abel"
capView = "sampler_abel"
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
triAssertEq [(triSetLighting "classic"), "OK"]
triEnableShadowMaps
triSimFrames 30
triScreenshot "paths_1__classic_maps"
"paths_1__classic_maps" call capRecord
triAssertEq [(triShadowSetRes 1024), "OK:1024"]
triSimFrames 10
triAssertEq [(triShadowSetCascades 2), "OK:2"]
triSimFrames 30
triScreenshot "paths_2__classic_res1024_c2"
"paths_2__classic_res1024_c2" call capRecord
triAssertEq [(triSetLighting "remastered"), "OK"]
triSimFrames 30
triAssertEq [(triShadowSetRes 2048), "OK:2048"]
triSimFrames 10
triAssertEq [(triShadowSetCascades 4), "OK:4"]
triSimFrames 30
triScreenshot "paths_3__remastered_res2048_c4"
"paths_3__remastered_res2048_c4" call capRecord
capSelfTest = triGetShadowMapCacheTest
format ["view=%1;event=selftest;result=%2", capView, capSelfTest] call capLog
triSimFrames 10
triAssertEq [(triSetLighting "classic"), "OK"]
triSimFrames 10
triAssertEq [(triSetLighting "remastered"), "OK"]
triSimFrames 30
triScreenshot "paths_4__remastered_after"
"paths_4__remastered_after" call capRecord
triEndTest
