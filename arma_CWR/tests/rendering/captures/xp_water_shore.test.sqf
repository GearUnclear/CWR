// Experiment xp-water (POSEIDON_XP_WATER): shore stills on Abel's west coast,
// the vgate golden setup (deterministic, fixed 1/60 step, 300 settle frames,
// remastered only). Views that show the waterline, which the standard views
// barely do: along the coast both ways at noon and sunset, and down on the
// waterline in the early afternoon. Run through run-captures.ps1
// (the squad bench's job command); the bench sets POSEIDON_XP_WATER per side.
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
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call { format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [1550, 5150, 0]
capCam = "camera" camCreate [1550, 5150, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }

capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }

call { capDetOn = triDeterministic true; format ["view=setup;event=det;deterministic=%1", capDetOn] call capLog }
call { capStepRes = triSetFixedStep (1/60); format ["view=setup;event=step;res=%1", capStepRes] call capLog }
call { capFixedSettle = 300; capSettleWaitMs = 1000; format ["view=setup;event=fixedframes;frames=%1;tail=%2;wait_ms=%3", capFixedSettle, capSettleTail, capSettleWaitMs] call capLog }

// ---- shore_along_north_sunset: 4 m over the beach, north along the coast at sunset
call { capRain = 0; capEyeAbs = false }
["shore_along_north_sunset", 1550, 5150, 4, 355, -6, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
call { capSimT = 900; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "shore_along_north_sunset__gold"
"shore_along_north_sunset__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shore_down_beach: 8 m up, down on the waterline in the early afternoon
call { capRain = 0; capEyeAbs = false }
["shore_down_beach", 1550, 5150, 8, 300, -15, [1985, 7, 5, 14, 0], 0, 0, 4000] call capBegin
call { capSimT = 1100; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "shore_down_beach__gold"
"shore_down_beach__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shore_along_evening: 4 m over the beach, south along the coast
call { capRain = 0; capEyeAbs = false }
["shore_along_evening", 1550, 5150, 4, 200, -6, [1985, 7, 5, 17, 45], 0, 0, 4000] call capBegin
call { capSimT = 1300; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "shore_along_evening__gold"
"shore_along_evening__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shore_along_north_noon: 4 m over the beach, north along the coast
call { capRain = 0; capEyeAbs = false }
["shore_along_north_noon", 1550, 5150, 4, 355, -6, [1985, 7, 5, 13, 0], 0, 0, 4000] call capBegin
call { capSimT = 1500; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "shore_along_north_noon__gold"
"shore_along_north_noon__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
