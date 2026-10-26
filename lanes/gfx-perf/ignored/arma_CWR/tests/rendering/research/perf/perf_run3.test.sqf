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
call { ["A0_classic", "classic", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A0_classic" call pfRead }
call { ["A0_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A0_classicubo" call pfRead }
call { ["A0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A0_m0" call pfRead }
call { ["A0_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A0_m0ubo" call pfRead }
call { ["A0_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A0_unlit" call pfRead }
call { ["A0_allfeatoff", "remastered", 137, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A0_allfeatoff" call pfRead }
call { ["A0_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A0_shadowoff" call pfRead }
call { ["A1_classic", "classic", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A1_classic" call pfRead }
call { ["A1_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A1_classicubo" call pfRead }
call { ["A1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A1_m0" call pfRead }
call { ["A1_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A1_m0ubo" call pfRead }
call { ["A1_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A1_unlit" call pfRead }
call { ["A1_allfeatoff", "remastered", 137, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A1_allfeatoff" call pfRead }
call { ["A1_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A1_shadowoff" call pfRead }
call { ["A2_classic", "classic", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A2_classic" call pfRead }
call { ["A2_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A2_classicubo" call pfRead }
call { ["A2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A2_m0" call pfRead }
call { ["A2_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A2_m0ubo" call pfRead }
call { ["A2_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A2_unlit" call pfRead }
call { ["A2_allfeatoff", "remastered", 137, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A2_allfeatoff" call pfRead }
call { ["A2_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A2_shadowoff" call pfRead }
call { ["A3_classic", "classic", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A3_classic" call pfRead }
call { ["A3_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A3_classicubo" call pfRead }
call { ["A3_m0", "remastered", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A3_m0" call pfRead }
call { ["A3_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "A3_m0ubo" call pfRead }
call { ["A3_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A3_unlit" call pfRead }
call { ["A3_allfeatoff", "remastered", 137, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A3_allfeatoff" call pfRead }
call { ["A3_shadowoff", "remastered", 11, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "A3_shadowoff" call pfRead }
call { ["B0_classic", "classic", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B0_classic" call pfRead }
call { ["B0_m0", "remastered", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B0_m0" call pfRead }
call { ["B0_m0ubo", "remastered", 10, 2.0, 8, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "B0_m0ubo" call pfRead }
call { ["B0_unlit", "remastered", 138, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B0_unlit" call pfRead }
call { ["B1_classic", "classic", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B1_classic" call pfRead }
call { ["B1_m0", "remastered", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B1_m0" call pfRead }
call { ["B1_m0ubo", "remastered", 10, 2.0, 8, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "B1_m0ubo" call pfRead }
call { ["B1_unlit", "remastered", 138, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B1_unlit" call pfRead }
call { ["B2_classic", "classic", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B2_classic" call pfRead }
call { ["B2_m0", "remastered", 10, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B2_m0" call pfRead }
call { ["B2_m0ubo", "remastered", 10, 2.0, 8, 4000, 1, 103] call pfCfg }
triSimFrames 120
call { "B2_m0ubo" call pfRead }
call { ["B2_unlit", "remastered", 138, 2.0, 8, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "B2_unlit" call pfRead }
call { ["C0_classic", "classic", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C0_classic" call pfRead }
call { ["C0_m0", "remastered", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C0_m0" call pfRead }
call { ["C0_unlit", "remastered", 138, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C0_unlit" call pfRead }
call { ["C1_classic", "classic", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C1_classic" call pfRead }
call { ["C1_m0", "remastered", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C1_m0" call pfRead }
call { ["C1_unlit", "remastered", 138, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C1_unlit" call pfRead }
call { ["C2_classic", "classic", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C2_classic" call pfRead }
call { ["C2_m0", "remastered", 10, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C2_m0" call pfRead }
call { ["C2_unlit", "remastered", 138, 1.0, 0, 4000, 1, 0] call pfCfg }
triSimFrames 120
call { "C2_unlit" call pfRead }
call { ["S_classic", "classic", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 60
call { "S_classic" call pfRead }
triScreenshot "S_classic"
call { ["S_m0", "remastered", 10, 1.5, 4, 4000, 1, 0] call pfCfg }
triSimFrames 60
call { "S_m0" call pfRead }
triScreenshot "S_m0"
call { ["S_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103] call pfCfg }
triSimFrames 60
call { "S_m0ubo" call pfRead }
triScreenshot "S_m0ubo"

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 300
call { ["N0_classic", "classic", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N0_classic" call pfRead }
call { ["N0_m0", "remastered", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N0_m0" call pfRead }
call { ["N0_m0ubo", "remastered", 10, 1.5, 4, 2500, 1, 103] call pfCfg }
triSimFrames 120
call { "N0_m0ubo" call pfRead }
call { ["N0_unlit", "remastered", 138, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N0_unlit" call pfRead }
call { ["N1_classic", "classic", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N1_classic" call pfRead }
call { ["N1_m0", "remastered", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N1_m0" call pfRead }
call { ["N1_m0ubo", "remastered", 10, 1.5, 4, 2500, 1, 103] call pfCfg }
triSimFrames 120
call { "N1_m0ubo" call pfRead }
call { ["N1_unlit", "remastered", 138, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N1_unlit" call pfRead }
call { ["N2_classic", "classic", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N2_classic" call pfRead }
call { ["N2_m0", "remastered", 10, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N2_m0" call pfRead }
call { ["N2_m0ubo", "remastered", 10, 1.5, 4, 2500, 1, 103] call pfCfg }
triSimFrames 120
call { "N2_m0ubo" call pfRead }
call { ["N2_unlit", "remastered", 138, 1.5, 4, 2500, 1, 0] call pfCfg }
triSimFrames 120
call { "N2_unlit" call pfRead }
[] call capClear

["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 300
call { ["L0_classic", "classic", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L0_classic" call pfRead }
call { ["L0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L0_m0" call pfRead }
call { ["L0_m0ubo", "remastered", 10, 1.5, 4, 1500, 1, 103] call pfCfg }
triSimFrames 120
call { "L0_m0ubo" call pfRead }
call { ["L0_unlit", "remastered", 138, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L0_unlit" call pfRead }
call { ["L1_classic", "classic", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L1_classic" call pfRead }
call { ["L1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L1_m0" call pfRead }
call { ["L1_m0ubo", "remastered", 10, 1.5, 4, 1500, 1, 103] call pfCfg }
triSimFrames 120
call { "L1_m0ubo" call pfRead }
call { ["L1_unlit", "remastered", 138, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L1_unlit" call pfRead }
call { ["L2_classic", "classic", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L2_classic" call pfRead }
call { ["L2_m0", "remastered", 10, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L2_m0" call pfRead }
call { ["L2_m0ubo", "remastered", 10, 1.5, 4, 1500, 1, 103] call pfCfg }
triSimFrames 120
call { "L2_m0ubo" call pfRead }
call { ["L2_unlit", "remastered", 138, 1.5, 4, 1500, 1, 0] call pfCfg }
triSimFrames 120
call { "L2_unlit" call pfRead }
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
