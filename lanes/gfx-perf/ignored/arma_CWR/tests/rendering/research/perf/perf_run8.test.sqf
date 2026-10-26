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
call { ["Q0_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q0_classic" call pfRead }
call { ["Q0_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_classicall" call pfRead }
call { ["Q0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0" call pfRead }
call { ["Q0_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all" call pfRead }
call { ["Q0_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_shadowoff" call pfRead }
call { ["Q0_horizonoff", "remastered", 12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_horizonoff" call pfRead }
call { ["Q0_cloudoff", "remastered", 14, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_cloudoff" call pfRead }
call { ["Q0_fogoff", "remastered", 18, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_fogoff" call pfRead }
call { ["Q0_crownoff", "remastered", 74, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_crownoff" call pfRead }
call { ["Q0_featuresoff", "remastered", 137, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_featuresoff" call pfRead }
call { ["Q0_unlit", "remastered", 138, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_unlit" call pfRead }
call { ["Q0_onetap", "remastered", 266, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_onetap" call pfRead }
call { ["Q0_primarycascade", "remastered", 522, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_primarycascade" call pfRead }
call { ["Q0_postoff", "remastered", 10, 1.5, 4, 4000, 0, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_postoff" call pfRead }
call { ["Q1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_classic" call pfRead }
call { ["Q1_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_classicall" call pfRead }
call { ["Q1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0" call pfRead }
call { ["Q1_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all" call pfRead }
call { ["Q1_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_shadowoff" call pfRead }
call { ["Q1_horizonoff", "remastered", 12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_horizonoff" call pfRead }
call { ["Q1_cloudoff", "remastered", 14, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_cloudoff" call pfRead }
call { ["Q1_fogoff", "remastered", 18, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_fogoff" call pfRead }
call { ["Q1_crownoff", "remastered", 74, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_crownoff" call pfRead }
call { ["Q1_featuresoff", "remastered", 137, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_featuresoff" call pfRead }
call { ["Q1_unlit", "remastered", 138, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_unlit" call pfRead }
call { ["Q1_onetap", "remastered", 266, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_onetap" call pfRead }
call { ["Q1_primarycascade", "remastered", 522, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_primarycascade" call pfRead }
call { ["Q1_postoff", "remastered", 10, 1.5, 4, 4000, 0, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_postoff" call pfRead }
call { ["Q2_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_classic" call pfRead }
call { ["Q2_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_classicall" call pfRead }
call { ["Q2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0" call pfRead }
call { ["Q2_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all" call pfRead }
call { ["Q2_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_shadowoff" call pfRead }
call { ["Q2_horizonoff", "remastered", 12, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_horizonoff" call pfRead }
call { ["Q2_cloudoff", "remastered", 14, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_cloudoff" call pfRead }
call { ["Q2_fogoff", "remastered", 18, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_fogoff" call pfRead }
call { ["Q2_crownoff", "remastered", 74, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_crownoff" call pfRead }
call { ["Q2_featuresoff", "remastered", 137, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_featuresoff" call pfRead }
call { ["Q2_unlit", "remastered", 138, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_unlit" call pfRead }
call { ["Q2_onetap", "remastered", 266, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_onetap" call pfRead }
call { ["Q2_primarycascade", "remastered", 522, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_primarycascade" call pfRead }
call { ["Q2_postoff", "remastered", 10, 1.5, 4, 4000, 0, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_postoff" call pfRead }
call { ["SQ_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_classic" call pfRead }
triScreenshot "SQ_classic"
call { ["SQ_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0" call pfRead }
triScreenshot "SQ_m0"
call { ["SQ_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0all" call pfRead }
triScreenshot "SQ_m0all"

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 300
call { ["F0_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F0_classic" call pfRead }
call { ["F0_classicall", "classic", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F0_classicall" call pfRead }
call { ["F0_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F0_m0" call pfRead }
call { ["F0_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all" call pfRead }
call { ["F1_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_classic" call pfRead }
call { ["F1_classicall", "classic", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F1_classicall" call pfRead }
call { ["F1_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_m0" call pfRead }
call { ["F1_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all" call pfRead }
call { ["F2_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_classic" call pfRead }
call { ["F2_classicall", "classic", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F2_classicall" call pfRead }
call { ["F2_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_m0" call pfRead }
call { ["F2_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all" call pfRead }
call { ["SF_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_classic" call pfRead }
triScreenshot "SF_classic"
call { ["SF_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_m0" call pfRead }
triScreenshot "SF_m0"
call { ["SF_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SF_m0all" call pfRead }
triScreenshot "SF_m0all"
[] call capClear

["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 300
call { ["L0_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L0_classic" call pfRead }
call { ["L0_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L0_classicall" call pfRead }
call { ["L0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L0_m0" call pfRead }
call { ["L0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L0_m0all" call pfRead }
call { ["L1_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L1_classic" call pfRead }
call { ["L1_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L1_classicall" call pfRead }
call { ["L1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L1_m0" call pfRead }
call { ["L1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L1_m0all" call pfRead }
call { ["L2_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L2_classic" call pfRead }
call { ["L2_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L2_classicall" call pfRead }
call { ["L2_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L2_m0" call pfRead }
call { ["L2_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L2_m0all" call pfRead }
call { ["SL_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SL_classic" call pfRead }
triScreenshot "SL_classic"
call { ["SL_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SL_m0" call pfRead }
triScreenshot "SL_m0"
call { ["SL_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SL_m0all" call pfRead }
triScreenshot "SL_m0all"
[] call capClear

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 0, 180, "man"] call capSpawn;
}
triSimFrames 300
call { ["G0_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G0_classic" call pfRead }
call { ["G0_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_classicall" call pfRead }
call { ["G0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G0_m0" call pfRead }
call { ["G0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_m0all" call pfRead }
call { ["G1_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G1_classic" call pfRead }
call { ["G1_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_classicall" call pfRead }
call { ["G1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G1_m0" call pfRead }
call { ["G1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_m0all" call pfRead }
call { ["G2_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G2_classic" call pfRead }
call { ["G2_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_classicall" call pfRead }
call { ["G2_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "G2_m0" call pfRead }
call { ["G2_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_m0all" call pfRead }
call { ["SG_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SG_classic" call pfRead }
triScreenshot "SG_classic"
call { ["SG_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SG_m0" call pfRead }
triScreenshot "SG_m0"
call { ["SG_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_m0all" call pfRead }
triScreenshot "SG_m0all"
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
