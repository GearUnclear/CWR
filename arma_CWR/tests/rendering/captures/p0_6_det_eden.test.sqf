// P0-6 determinism stills (deterministic recipe, two shots per view; needs the P0-6 verbs). Eden.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
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
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Eden;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// P0-6 determinism helpers (a build with the P0-6 verbs).
capDetBlock = { capDetRes = triDeterministic true; capRstRes = triResetTemporal [0, 0]; capSeedRes = triSeedRandom 1; format ["view=%1;event=det;det=%2;reset=%3;seed=%4", capView, capDetRes, capRstRes, capSeedRes] call capLog }
capDetState = { format ["view=%1;event=detstate;label=%2;state=%3", capView, _this, triTemporalState] call capLog }
call { capStepRes = triSetFixedStep 0.0166667; format ["view=setup;event=step;res=%1", capStepRes] call capLog }


// ---- broken_high
call { capRain = 0; capEyeAbs = false }
["broken_high", 9200, 3700, 250, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 5000] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
[] call capPin
triSimFrames 60
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "broken_high__det1"
"broken_high__det1" call capRecord
"broken_high__det1" call capDetState
[] call capPin
[] call capDetBlock
triSimFrames 32
triScreenshot "broken_high__det2"
"broken_high__det2" call capRecord
"broken_high__det2" call capDetState
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { capDetRes = triDeterministic false; capRstRes = triResetTemporal []; capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }
triEndTest
