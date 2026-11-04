// Shared body of the smoke_*.test.sqf files: toggles the lighting model
// classic -> remastered -> classic -> remastered on one sunlit view, a shot
// after each switch. triSetLighting must answer "OK" every time (an answer of
// "FAIL:remastered shaders unavailable" means the remastered programs did not
// build and the engine fell back to Classic). Trident's I-20 post-check
// (triGetGLErrorCount before triEndTest) fails the test on a KHR_debug
// high-severity GL error. The view globals (capView .. capVD) are set by the
// including file.
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
triSimFrames 30
call { capModeName = "classic"; capLit = "OK"; [] call capReadTimers }
triScreenshot "smoke_1__classic"
"smoke_1__classic" call capRecord
triAssertEq [(triSetLighting "remastered"), "OK"]
triSimFrames 30
call { capModeName = "remastered"; [] call capReadTimers }
triScreenshot "smoke_2__remastered"
"smoke_2__remastered" call capRecord
triAssertEq [(triSetLighting "classic"), "OK"]
triSimFrames 30
call { capModeName = "classic"; [] call capReadTimers }
triScreenshot "smoke_3__classic"
"smoke_3__classic" call capRecord
triAssertEq [(triSetLighting "remastered"), "OK"]
triSimFrames 30
call { capModeName = "remastered"; [] call capReadTimers }
triScreenshot "smoke_4__remastered"
"smoke_4__remastered" call capRecord
triEndTest
