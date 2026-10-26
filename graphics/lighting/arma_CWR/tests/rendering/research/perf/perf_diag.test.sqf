// PERF RESEARCH (not committed): where the remastered GPU time goes.
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

// [label, mode, sat, scale, msaa, vd, post] call pfCfg
pfCfg = {
    pfLabel = _this select 0;
    pfLit = triSetLighting (_this select 1);
    pfSat = triSetLighting (format ["saturation=%1", _this select 2]);
    pfPost = triSetLighting (format ["post=%1", _this select 6]);
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
call { ["sr_classic", "classic", 10, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic" call pfRead }
triScreenshot "sr_classic"
call { ["sr_stock", "remastered", 1.12, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_stock" call pfRead }
triScreenshot "sr_stock"
call { ["sr_m0", "remastered", 10, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0" call pfRead }
call { ["sr_m1_shadowoff", "remastered", 11, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m1_shadowoff" call pfRead }
call { ["sr_m2_terrainoff", "remastered", 12, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m2_terrainoff" call pfRead }
triScreenshot "sr_m2_terrainoff"
call { ["sr_m4_cloudoff", "remastered", 14, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m4_cloudoff" call pfRead }
call { ["sr_m8_fogoff", "remastered", 18, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m8_fogoff" call pfRead }
call { ["sr_m16_glossyoff", "remastered", 26, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m16_glossyoff" call pfRead }
call { ["sr_m32_localoff", "remastered", 42, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m32_localoff" call pfRead }
call { ["sr_m64_foliageoff", "remastered", 74, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m64_foliageoff" call pfRead }
call { ["sr_m128_unlit", "remastered", 138, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m128_unlit" call pfRead }
triScreenshot "sr_m128_unlit"
call { ["sr_m256_shadow1tap", "remastered", 266, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m256_shadow1tap" call pfRead }
call { ["sr_m512_shadownoblend", "remastered", 522, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m512_shadownoblend" call pfRead }
call { ["sr_m1024_shadow4tap", "remastered", 1034, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m1024_shadow4tap" call pfRead }
call { ["sr_m127_allfeaturesoff", "remastered", 137, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m127_allfeaturesoff" call pfRead }
call { ["sr_m3_shadow+terrainoff", "remastered", 13, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m3_shadow+terrainoff" call pfRead }
call { ["sr_postoff", "remastered", 10, 1.5, 4, 4000, 0] call pfCfg }
triSimFrames 90
call { "sr_postoff" call pfRead }
call { ["sr_classic_again", "classic", 10, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_again" call pfRead }
call { ["sr_m0_again", "remastered", 10, 1.5, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_again" call pfRead }
call { ["sr_classic_s1.0_msaa4", "classic", 10, 1.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_s1.0_msaa4" call pfRead }
call { ["sr_m0_s1.0_msaa4", "remastered", 10, 1.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_s1.0_msaa4" call pfRead }
call { ["sr_unlit_s1.0_msaa4", "remastered", 138, 1.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_unlit_s1.0_msaa4" call pfRead }
call { ["sr_classic_s1.0_msaa0", "classic", 10, 1.0, 0, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_s1.0_msaa0" call pfRead }
call { ["sr_m0_s1.0_msaa0", "remastered", 10, 1.0, 0, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_s1.0_msaa0" call pfRead }
call { ["sr_unlit_s1.0_msaa0", "remastered", 138, 1.0, 0, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_unlit_s1.0_msaa0" call pfRead }
call { ["sr_classic_s2.0_msaa4", "classic", 10, 2.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_s2.0_msaa4" call pfRead }
call { ["sr_m0_s2.0_msaa4", "remastered", 10, 2.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_s2.0_msaa4" call pfRead }
call { ["sr_unlit_s2.0_msaa4", "remastered", 138, 2.0, 4, 4000, 1] call pfCfg }
triSimFrames 90
call { "sr_unlit_s2.0_msaa4" call pfRead }
call { ["sr_classic_vd2000", "classic", 10, 1.5, 4, 2000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_vd2000" call pfRead }
call { ["sr_m0_vd2000", "remastered", 10, 1.5, 4, 2000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_vd2000" call pfRead }
call { ["sr_unlit_vd2000", "remastered", 138, 1.5, 4, 2000, 1] call pfCfg }
triSimFrames 90
call { "sr_unlit_vd2000" call pfRead }
call { ["sr_classic_vd1000", "classic", 10, 1.5, 4, 1000, 1] call pfCfg }
triSimFrames 90
call { "sr_classic_vd1000" call pfRead }
call { ["sr_m0_vd1000", "remastered", 10, 1.5, 4, 1000, 1] call pfCfg }
triSimFrames 90
call { "sr_m0_vd1000" call pfRead }
call { ["sr_unlit_vd1000", "remastered", 138, 1.5, 4, 1000, 1] call pfCfg }
triSimFrames 90
call { "sr_unlit_vd1000" call pfRead }

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
call { ["nf_classic", "classic", 10, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_classic" call pfRead }
call { ["nf_m0", "remastered", 10, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_m0" call pfRead }
call { ["nf_unlit", "remastered", 138, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_unlit" call pfRead }
call { ["nf_shadowoff", "remastered", 11, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_shadowoff" call pfRead }
call { ["nf_terrainoff", "remastered", 12, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_terrainoff" call pfRead }
call { ["nf_allfeaturesoff", "remastered", 137, 1.5, 4, 2500, 1] call pfCfg }
triSimFrames 90
call { "nf_allfeaturesoff" call pfRead }
[] call capClear

["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
call { ["lf_classic", "classic", 10, 1.5, 4, 1500, 1] call pfCfg }
triSimFrames 90
call { "lf_classic" call pfRead }
call { ["lf_m0", "remastered", 10, 1.5, 4, 1500, 1] call pfCfg }
triSimFrames 90
call { "lf_m0" call pfRead }
call { ["lf_unlit", "remastered", 138, 1.5, 4, 1500, 1] call pfCfg }
triSimFrames 90
call { "lf_unlit" call pfRead }
call { ["lf_shadowoff", "remastered", 11, 1.5, 4, 1500, 1] call pfCfg }
triSimFrames 90
call { "lf_shadowoff" call pfRead }
call { ["lf_allfeaturesoff", "remastered", 137, 1.5, 4, 1500, 1] call pfCfg }
triSimFrames 90
call { "lf_allfeaturesoff" call pfRead }
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
