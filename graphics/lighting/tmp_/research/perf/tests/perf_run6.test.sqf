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
triSimFrames 600
call { ["SR_classic_a", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "SR_classic_a" call pfRead }
triScreenshot "SR_classic_a"
call { ["SR_classic_b", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "SR_classic_b" call pfRead }
triScreenshot "SR_classic_b"
call { ["SR_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "SR_classicall" call pfRead }
triScreenshot "SR_classicall"
call { ["SR_classicall8", "classic", 10, 1.5, 4, 4000, 1, 115, 1] call pfCfg }
triSimFrames 60
call { "SR_classicall8" call pfRead }
triScreenshot "SR_classicall8"
call { ["SR_m0_a", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "SR_m0_a" call pfRead }
triScreenshot "SR_m0_a"
call { ["SR_m0_b", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "SR_m0_b" call pfRead }
triScreenshot "SR_m0_b"
call { ["SR_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "SR_m0all" call pfRead }
triScreenshot "SR_m0all"
call { ["SR_m0all8", "remastered", 10, 1.5, 4, 4000, 1, 115, 1] call pfCfg }
triSimFrames 60
call { "SR_m0all8" call pfRead }
triScreenshot "SR_m0all8"

["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
triSimFrames 300
call { ["T0_classic", "classic", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "T0_classic" call pfRead }
call { ["T0_classicall", "classic", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "T0_classicall" call pfRead }
call { ["T0_m0", "remastered", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "T0_m0" call pfRead }
call { ["T0_m0all", "remastered", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "T0_m0all" call pfRead }
call { ["T0_m0all8", "remastered", 10, 1.5, 4, 800, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "T0_m0all8" call pfRead }
call { ["T1_classic", "classic", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "T1_classic" call pfRead }
call { ["T1_classicall", "classic", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "T1_classicall" call pfRead }
call { ["T1_m0", "remastered", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "T1_m0" call pfRead }
call { ["T1_m0all", "remastered", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "T1_m0all" call pfRead }
call { ["T1_m0all8", "remastered", 10, 1.5, 4, 800, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "T1_m0all8" call pfRead }
call { ["TN_classic_a", "classic", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "TN_classic_a" call pfRead }
triScreenshot "TN_classic_a"
call { ["TN_classic_b", "classic", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "TN_classic_b" call pfRead }
triScreenshot "TN_classic_b"
call { ["TN_classicall", "classic", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "TN_classicall" call pfRead }
triScreenshot "TN_classicall"
call { ["TN_classicall8", "classic", 10, 1.5, 4, 800, 1, 115, 1] call pfCfg }
triSimFrames 60
call { "TN_classicall8" call pfRead }
triScreenshot "TN_classicall8"
call { ["TN_m0_a", "remastered", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "TN_m0_a" call pfRead }
triScreenshot "TN_m0_a"
call { ["TN_m0_b", "remastered", 10, 1.5, 4, 800, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "TN_m0_b" call pfRead }
triScreenshot "TN_m0_b"
call { ["TN_m0all", "remastered", 10, 1.5, 4, 800, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "TN_m0all" call pfRead }
triScreenshot "TN_m0all"
call { ["TN_m0all8", "remastered", 10, 1.5, 4, 800, 1, 115, 1] call pfCfg }
triSimFrames 60
call { "TN_m0all8" call pfRead }
triScreenshot "TN_m0all8"

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 0, 180, "man"] call capSpawn;
}
triSimFrames 300
call { ["G0_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "G0_classic" call pfRead }
call { ["G0_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "G0_classicall" call pfRead }
call { ["G0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "G0_m0" call pfRead }
call { ["G0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "G0_m0all" call pfRead }
call { ["G0_m0all8", "remastered", 10, 1.5, 4, 1500, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "G0_m0all8" call pfRead }
call { ["G1_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "G1_classic" call pfRead }
call { ["G1_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "G1_classicall" call pfRead }
call { ["G1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "G1_m0" call pfRead }
call { ["G1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "G1_m0all" call pfRead }
call { ["G1_m0all8", "remastered", 10, 1.5, 4, 1500, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "G1_m0all8" call pfRead }
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
