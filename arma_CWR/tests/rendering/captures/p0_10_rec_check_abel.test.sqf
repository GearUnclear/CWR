// P0-10 recorder check (triRecord frames against triScreenshot; needs the P0-10 verbs). Abel.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
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
capProbe = "HeliHEmpty" createVehicle [6100, 5000, 0]
capCam = "camera" camCreate [6100, 5000, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }

call { capStepRes = triSetFixedStep 0.0166667; setAccTime 1; format ["view=setup;event=step;res=%1", capStepRes] call capLog }

// ---- valley_0515
call { capRain = 0; capEyeAbs = false }
["valley_0515", 6100, 5000, 1.75, 40, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered" call capMode
[] call capPin
triSimFrames 60

// ---- shots: triRecord mode 1
call { capRecRes = triRecord ["p0_10_rec/rec_shots.raw", 4, 1]; format ["view=%1;event=record;rec=shots;res=%2", capView, capRecRes] call capLog }
triSimFrames 3
triScreenshot "rec_shots_1"
"rec_shots_1" call capRecord
triSimFrames 3
triScreenshot "rec_shots_2"
"rec_shots_2" call capRecord
triSimFrames 3
triScreenshot "rec_shots_3"
"rec_shots_3" call capRecord
triSimFrames 3
triScreenshot "rec_shots_4"
"rec_shots_4" call capRecord
triRecordState
call { format ["view=%1;event=record_done;rec=shots;state=%2", capView, triRecordState] call capLog }

// ---- every: triRecord mode 0
call { capPathRes = triCamPath [[0, capOX, capEyeASL, capOZ, capH, capP], [6, capOX, capEyeASL, capOZ, capH + 12, capP + 1, 1]]; format ["view=%1;event=campath;rec=every;res=%2", capView, capPathRes] call capLog }
call { capRecRes = triRecord ["p0_10_rec/rec_every.raw", 600, 0]; format ["view=%1;event=record;rec=every;res=%2", capView, capRecRes] call capLog }
triSimFrames 5
triScreenshot "rec_every_1"
"rec_every_1" call capRecord
triSimFrames 5
triScreenshot "rec_every_2"
"rec_every_2" call capRecord
triSimFrames 5
triScreenshot "rec_every_3"
"rec_every_3" call capRecord
triSimFrames 4
call { capRecRes = triRecord ["p0_10_rec/rec_every.raw", 0, 0]; format ["view=%1;event=record_stop;rec=every;res=%2", capView, capRecRes] call capLog }
triRecordState
call { format ["view=%1;event=record_done;rec=every;state=%2", capView, triRecordState] call capLog }
call { capPathRes = triCamPath []; [] call capPin }

// ---- half: triRecord mode 2
call { capRecRes = triRecord ["p0_10_rec/rec_half.raw", 600, 2]; format ["view=%1;event=record;rec=half;res=%2", capView, capRecRes] call capLog }
triSimFrames 5
triScreenshot "rec_half_1"
"rec_half_1" call capRecord
triSimFrames 4
call { capRecRes = triRecord ["p0_10_rec/rec_half.raw", 0, 2]; format ["view=%1;event=record_stop;rec=half;res=%2", capView, capRecRes] call capLog }
triRecordState
call { format ["view=%1;event=record_done;rec=half;state=%2", capView, triRecordState] call capLog }
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }
triEndTest
