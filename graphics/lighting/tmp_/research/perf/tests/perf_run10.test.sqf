// PERF RESEARCH (not committed): where the remastered frame time goes.
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
pfLabel = "none"
pfLit = "none"
pfSat = "none"
pfPost = "none"
pfDbg = "none"

// [label, mode, sat, scale, msaa, vd, post, debug] call pfCfg
pfCfg = {
    pfLabel = _this select 0;
    pfLit = triSetLighting (_this select 1);
    pfSat = triSetLighting (format ["saturation=%1", _this select 2]);
    pfPost = triSetLighting (format ["post=%1", _this select 6]);
    pfDbg = triSetLighting (format ["debug=%1", _this select 7]);
    pfLL = triSetLighting (format ["localLights=%1", _this select 8]);
    pfScale = triSetRenderScale (_this select 3);
    pfMsaa = triSetMsaa (_this select 4);
    capVD = _this select 5;
    pfArgs = _this;
    [] call capPin;
    pfG0 = triGpuTimes false;
    pfG1 = triGpuTimes true;
    pfPR = triPerfReset 0;
}

pfRead = {
    pfGpu = triGpuTimes true;
    pfPerf = triPerfStats 0;
    logInfo (format ["UDPERF;view=%1;cfg=%2;args=%3;lit=%4;sat=%5;post=%6;perf=%7;gpu=%8", capView, pfLabel, pfArgs, pfLit, pfSat, pfPost, pfPerf, pfGpu]);
}

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240

["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
call { ["B0_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "B0_classic" call pfRead }
triScreenshot "B0_classic"
call { ["B0_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "B0_m0" call pfRead }
triScreenshot "B0_m0"
triSimFrames 600
call { ["P1_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "P1_classic" call pfRead }
call { ["P1_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "P1_m0" call pfRead }
call { ["R1_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "R1_m0" call pfRead }
call { ["R1_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "R1_classic" call pfRead }
call { ["X1_classicall", "classic", 1.12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "X1_classicall" call pfRead }
call { ["X1_m0all", "remastered", 1.12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "X1_m0all" call pfRead }
call { ["P2_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "P2_classic" call pfRead }
call { ["P2_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "P2_m0" call pfRead }
call { ["R2_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "R2_m0" call pfRead }
call { ["R2_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "R2_classic" call pfRead }
call { ["X2_classicall", "classic", 1.12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "X2_classicall" call pfRead }
call { ["X2_m0all", "remastered", 1.12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "X2_m0all" call pfRead }
call { ["S1_classic", "classic", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "S1_classic" call pfRead }
triScreenshot "S1_classic"
call { ["S1_m0", "remastered", 1.12, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "S1_m0" call pfRead }
triScreenshot "S1_m0"
call { ["S1_m0all", "remastered", 1.12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "S1_m0all" call pfRead }
triScreenshot "S1_m0all"

call { logInfo "UDPERF;event=done" }
triEndTest
