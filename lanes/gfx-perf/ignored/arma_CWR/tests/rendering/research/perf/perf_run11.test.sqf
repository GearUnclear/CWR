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

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 0, 180, "man"] call capSpawn;
}
triSimFrames 300
call { ["H0_classicall", "classic", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_classicall" call pfRead }
call { ["H0_m0all", "remastered", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_m0all" call pfRead }
call { ["H0_unlit", "remastered", 138, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_unlit" call pfRead }
call { ["H0_cut", "remastered", 2058, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_cut" call pfRead }
call { ["H0_cutfol4", "remastered", 6154, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_cutfol4" call pfRead }
call { ["H0_cutfar4", "remastered", 10250, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_cutfar4" call pfRead }
call { ["H0_cutboth", "remastered", 14346, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_cutboth" call pfRead }
call { ["H0_onetap", "remastered", 266, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_onetap" call pfRead }
call { ["H0_shadowoff", "remastered", 11, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_shadowoff" call pfRead }
call { ["H0_crownoff", "remastered", 74, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_crownoff" call pfRead }
call { ["H0_horizonoff", "remastered", 12, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_horizonoff" call pfRead }
call { ["H0_cloudoff", "remastered", 14, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_cloudoff" call pfRead }
call { ["H0_fogoff", "remastered", 18, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_fogoff" call pfRead }
call { ["H0_featuresoff", "remastered", 137, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H0_featuresoff" call pfRead }
call { ["H1_classicall", "classic", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_classicall" call pfRead }
call { ["H1_m0all", "remastered", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_m0all" call pfRead }
call { ["H1_unlit", "remastered", 138, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_unlit" call pfRead }
call { ["H1_cut", "remastered", 2058, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_cut" call pfRead }
call { ["H1_cutfol4", "remastered", 6154, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_cutfol4" call pfRead }
call { ["H1_cutfar4", "remastered", 10250, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_cutfar4" call pfRead }
call { ["H1_cutboth", "remastered", 14346, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_cutboth" call pfRead }
call { ["H1_onetap", "remastered", 266, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_onetap" call pfRead }
call { ["H1_shadowoff", "remastered", 11, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_shadowoff" call pfRead }
call { ["H1_crownoff", "remastered", 74, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_crownoff" call pfRead }
call { ["H1_horizonoff", "remastered", 12, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_horizonoff" call pfRead }
call { ["H1_cloudoff", "remastered", 14, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_cloudoff" call pfRead }
call { ["H1_fogoff", "remastered", 18, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_fogoff" call pfRead }
call { ["H1_featuresoff", "remastered", 137, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H1_featuresoff" call pfRead }
call { ["H2_classicall", "classic", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_classicall" call pfRead }
call { ["H2_m0all", "remastered", 10, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_m0all" call pfRead }
call { ["H2_unlit", "remastered", 138, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_unlit" call pfRead }
call { ["H2_cut", "remastered", 2058, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_cut" call pfRead }
call { ["H2_cutfol4", "remastered", 6154, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_cutfol4" call pfRead }
call { ["H2_cutfar4", "remastered", 10250, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_cutfar4" call pfRead }
call { ["H2_cutboth", "remastered", 14346, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_cutboth" call pfRead }
call { ["H2_onetap", "remastered", 266, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_onetap" call pfRead }
call { ["H2_shadowoff", "remastered", 11, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_shadowoff" call pfRead }
call { ["H2_crownoff", "remastered", 74, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_crownoff" call pfRead }
call { ["H2_horizonoff", "remastered", 12, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_horizonoff" call pfRead }
call { ["H2_cloudoff", "remastered", 14, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_cloudoff" call pfRead }
call { ["H2_fogoff", "remastered", 18, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_fogoff" call pfRead }
call { ["H2_featuresoff", "remastered", 137, 2.0, 8, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "H2_featuresoff" call pfRead }
call { ["G0_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_classicall" call pfRead }
call { ["G0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_m0all" call pfRead }
call { ["G0_unlit", "remastered", 138, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_unlit" call pfRead }
call { ["G0_cut", "remastered", 2058, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_cut" call pfRead }
call { ["G0_cutboth", "remastered", 14346, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G0_cutboth" call pfRead }
call { ["G1_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_classicall" call pfRead }
call { ["G1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_m0all" call pfRead }
call { ["G1_unlit", "remastered", 138, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_unlit" call pfRead }
call { ["G1_cut", "remastered", 2058, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_cut" call pfRead }
call { ["G1_cutboth", "remastered", 14346, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G1_cutboth" call pfRead }
call { ["G2_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_classicall" call pfRead }
call { ["G2_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_m0all" call pfRead }
call { ["G2_unlit", "remastered", 138, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_unlit" call pfRead }
call { ["G2_cut", "remastered", 2058, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_cut" call pfRead }
call { ["G2_cutboth", "remastered", 14346, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "G2_cutboth" call pfRead }
call { ["SG_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_m0all" call pfRead }
triScreenshot "SG_m0all"
call { ["SG_cut", "remastered", 2058, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_cut" call pfRead }
triScreenshot "SG_cut"
call { ["SG_cutfol4", "remastered", 6154, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_cutfol4" call pfRead }
triScreenshot "SG_cutfol4"
call { ["SG_cutboth", "remastered", 14346, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_cutboth" call pfRead }
triScreenshot "SG_cutboth"
call { ["SG_m0all_b", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 90
call { "SG_m0all_b" call pfRead }
triScreenshot "SG_m0all_b"
call { ["SG_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SG_m0" call pfRead }
triScreenshot "SG_m0"
call { ["SG_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SG_m0all192" call pfRead }
triScreenshot "SG_m0all192"
call { ["K0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "K0_m0all" call pfRead }
call { ["K0_m0all64", "remastered", 10, 1.5, 4, 1500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "K0_m0all64" call pfRead }
call { ["K0_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "K0_m0all192" call pfRead }
call { ["K0_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "K0_m0tocc" call pfRead }
call { ["K0_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "K0_m0all192tocc" call pfRead }
call { ["K1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "K1_m0all" call pfRead }
call { ["K1_m0all64", "remastered", 10, 1.5, 4, 1500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "K1_m0all64" call pfRead }
call { ["K1_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "K1_m0all192" call pfRead }
call { ["K1_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "K1_m0tocc" call pfRead }
call { ["K1_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "K1_m0all192tocc" call pfRead }
[] call capClear

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 300
call { ["F0_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F0_classic" call pfRead }
call { ["F0_classicall192", "classic", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F0_classicall192" call pfRead }
call { ["F0_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F0_m0" call pfRead }
call { ["F0_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all" call pfRead }
call { ["F0_m0all64", "remastered", 10, 1.5, 4, 2500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all64" call pfRead }
call { ["F0_m0all192", "remastered", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all192" call pfRead }
call { ["F0_classictocc", "classic", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F0_classictocc" call pfRead }
call { ["F0_m0tocc", "remastered", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F0_m0tocc" call pfRead }
call { ["F0_m0all192tocc", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all192tocc" call pfRead }
call { ["F1_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_classic" call pfRead }
call { ["F1_classicall192", "classic", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F1_classicall192" call pfRead }
call { ["F1_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_m0" call pfRead }
call { ["F1_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all" call pfRead }
call { ["F1_m0all64", "remastered", 10, 1.5, 4, 2500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all64" call pfRead }
call { ["F1_m0all192", "remastered", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all192" call pfRead }
call { ["F1_classictocc", "classic", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F1_classictocc" call pfRead }
call { ["F1_m0tocc", "remastered", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F1_m0tocc" call pfRead }
call { ["F1_m0all192tocc", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all192tocc" call pfRead }
call { ["F2_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_classic" call pfRead }
call { ["F2_classicall192", "classic", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F2_classicall192" call pfRead }
call { ["F2_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_m0" call pfRead }
call { ["F2_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all" call pfRead }
call { ["F2_m0all64", "remastered", 10, 1.5, 4, 2500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all64" call pfRead }
call { ["F2_m0all192", "remastered", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all192" call pfRead }
call { ["F2_classictocc", "classic", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F2_classictocc" call pfRead }
call { ["F2_m0tocc", "remastered", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "F2_m0tocc" call pfRead }
call { ["F2_m0all192tocc", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all192tocc" call pfRead }
call { ["SF_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_m0" call pfRead }
triScreenshot "SF_m0"
call { ["SF_m0all192", "remastered", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SF_m0all192" call pfRead }
triScreenshot "SF_m0all192"
call { ["SF_m0_b", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_m0_b" call pfRead }
triScreenshot "SF_m0_b"
call { ["SF_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_classic" call pfRead }
triScreenshot "SF_classic"
call { ["SF_classicall192", "classic", 10, 1.5, 4, 2500, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SF_classicall192" call pfRead }
triScreenshot "SF_classicall192"
call { ["SF_classictocc", "classic", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SF_classictocc" call pfRead }
triScreenshot "SF_classictocc"
call { ["SF_m0tocc", "remastered", 10, 1.5, 4, 2500, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SF_m0tocc" call pfRead }
triScreenshot "SF_m0tocc"
call { ["SF_m0all192tocc", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 90
call { "SF_m0all192tocc" call pfRead }
triScreenshot "SF_m0all192tocc"
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
call { ["L0_classicall192", "classic", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L0_classicall192" call pfRead }
call { ["L0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L0_m0" call pfRead }
call { ["L0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L0_m0all" call pfRead }
call { ["L0_m0all64", "remastered", 10, 1.5, 4, 1500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "L0_m0all64" call pfRead }
call { ["L0_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L0_m0all192" call pfRead }
call { ["L0_classictocc", "classic", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L0_classictocc" call pfRead }
call { ["L0_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L0_m0tocc" call pfRead }
call { ["L0_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "L0_m0all192tocc" call pfRead }
call { ["L1_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L1_classic" call pfRead }
call { ["L1_classicall192", "classic", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L1_classicall192" call pfRead }
call { ["L1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L1_m0" call pfRead }
call { ["L1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L1_m0all" call pfRead }
call { ["L1_m0all64", "remastered", 10, 1.5, 4, 1500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "L1_m0all64" call pfRead }
call { ["L1_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L1_m0all192" call pfRead }
call { ["L1_classictocc", "classic", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L1_classictocc" call pfRead }
call { ["L1_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L1_m0tocc" call pfRead }
call { ["L1_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "L1_m0all192tocc" call pfRead }
call { ["L2_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L2_classic" call pfRead }
call { ["L2_classicall192", "classic", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L2_classicall192" call pfRead }
call { ["L2_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "L2_m0" call pfRead }
call { ["L2_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "L2_m0all" call pfRead }
call { ["L2_m0all64", "remastered", 10, 1.5, 4, 1500, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "L2_m0all64" call pfRead }
call { ["L2_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "L2_m0all192" call pfRead }
call { ["L2_classictocc", "classic", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L2_classictocc" call pfRead }
call { ["L2_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "L2_m0tocc" call pfRead }
call { ["L2_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "L2_m0all192tocc" call pfRead }
call { ["SL_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SL_m0" call pfRead }
triScreenshot "SL_m0"
call { ["SL_m0all192", "remastered", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SL_m0all192" call pfRead }
triScreenshot "SL_m0all192"
call { ["SL_m0_b", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SL_m0_b" call pfRead }
triScreenshot "SL_m0_b"
call { ["SL_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SL_classic" call pfRead }
triScreenshot "SL_classic"
call { ["SL_classicall192", "classic", 10, 1.5, 4, 1500, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SL_classicall192" call pfRead }
triScreenshot "SL_classicall192"
call { ["SL_classictocc", "classic", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SL_classictocc" call pfRead }
triScreenshot "SL_classictocc"
call { ["SL_m0tocc", "remastered", 10, 1.5, 4, 1500, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SL_m0tocc" call pfRead }
triScreenshot "SL_m0tocc"
call { ["SL_m0all192tocc", "remastered", 10, 1.5, 4, 1500, 1, 555, 1] call pfCfg }
triSimFrames 90
call { "SL_m0all192tocc" call pfRead }
triScreenshot "SL_m0all192tocc"
[] call capClear

["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 600
call { ["Q0_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q0_classic" call pfRead }
call { ["Q0_classicall192", "classic", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q0_classicall192" call pfRead }
call { ["Q0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0" call pfRead }
call { ["Q0_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all" call pfRead }
call { ["Q0_m0all64", "remastered", 10, 1.5, 4, 4000, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all64" call pfRead }
call { ["Q0_m0all192", "remastered", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all192" call pfRead }
call { ["Q0_classictocc", "classic", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q0_classictocc" call pfRead }
call { ["Q0_m0tocc", "remastered", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0tocc" call pfRead }
call { ["Q0_m0all192tocc", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all192tocc" call pfRead }
call { ["Q1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_classic" call pfRead }
call { ["Q1_classicall192", "classic", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q1_classicall192" call pfRead }
call { ["Q1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0" call pfRead }
call { ["Q1_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all" call pfRead }
call { ["Q1_m0all64", "remastered", 10, 1.5, 4, 4000, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all64" call pfRead }
call { ["Q1_m0all192", "remastered", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all192" call pfRead }
call { ["Q1_classictocc", "classic", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q1_classictocc" call pfRead }
call { ["Q1_m0tocc", "remastered", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0tocc" call pfRead }
call { ["Q1_m0all192tocc", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all192tocc" call pfRead }
call { ["Q2_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_classic" call pfRead }
call { ["Q2_classicall192", "classic", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q2_classicall192" call pfRead }
call { ["Q2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0" call pfRead }
call { ["Q2_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all" call pfRead }
call { ["Q2_m0all64", "remastered", 10, 1.5, 4, 4000, 1, 171, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all64" call pfRead }
call { ["Q2_m0all192", "remastered", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all192" call pfRead }
call { ["Q2_classictocc", "classic", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q2_classictocc" call pfRead }
call { ["Q2_m0tocc", "remastered", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0tocc" call pfRead }
call { ["Q2_m0all192tocc", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all192tocc" call pfRead }
call { ["SQ_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0" call pfRead }
triScreenshot "SQ_m0"
call { ["SQ_m0all192", "remastered", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0all192" call pfRead }
triScreenshot "SQ_m0all192"
call { ["SQ_m0_b", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0_b" call pfRead }
triScreenshot "SQ_m0_b"
call { ["SQ_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_classic" call pfRead }
triScreenshot "SQ_classic"
call { ["SQ_classicall192", "classic", 10, 1.5, 4, 4000, 1, 299, 1] call pfCfg }
triSimFrames 90
call { "SQ_classicall192" call pfRead }
triScreenshot "SQ_classicall192"
call { ["SQ_classictocc", "classic", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SQ_classictocc" call pfRead }
triScreenshot "SQ_classictocc"
call { ["SQ_m0tocc", "remastered", 10, 1.5, 4, 4000, 1, 356, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0tocc" call pfRead }
triScreenshot "SQ_m0tocc"
call { ["SQ_m0all192tocc", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0all192tocc" call pfRead }
triScreenshot "SQ_m0all192tocc"
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
