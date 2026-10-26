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
call { pfShapes = triPerfDumpShapes 0 }
triSimFrames 5
call { ["A0_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A0_classic" call pfRead }
call { ["A0_classicinst", "classic", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A0_classicinst" call pfRead }
call { ["A0_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A0_classicall" call pfRead }
call { ["A0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A0_m0" call pfRead }
call { ["A0_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A0_m0ubo" call pfRead }
call { ["A0_m0inst", "remastered", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A0_m0inst" call pfRead }
call { ["A0_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A0_m0all" call pfRead }
call { ["A0_m0band", "remastered", 10, 1.5, 4, 4000, 1, 108, 1] call pfCfg }
triSimFrames 120
call { "A0_m0band" call pfRead }
call { ["A0_m0all8", "remastered", 10, 1.5, 4, 4000, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "A0_m0all8" call pfRead }
call { ["A0_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A0_m0all16" call pfRead }
call { ["A0_classicall16", "classic", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A0_classicall16" call pfRead }
call { ["A1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A1_classic" call pfRead }
call { ["A1_classicinst", "classic", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A1_classicinst" call pfRead }
call { ["A1_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A1_classicall" call pfRead }
call { ["A1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A1_m0" call pfRead }
call { ["A1_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A1_m0ubo" call pfRead }
call { ["A1_m0inst", "remastered", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A1_m0inst" call pfRead }
call { ["A1_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A1_m0all" call pfRead }
call { ["A1_m0band", "remastered", 10, 1.5, 4, 4000, 1, 108, 1] call pfCfg }
triSimFrames 120
call { "A1_m0band" call pfRead }
call { ["A1_m0all8", "remastered", 10, 1.5, 4, 4000, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "A1_m0all8" call pfRead }
call { ["A1_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A1_m0all16" call pfRead }
call { ["A1_classicall16", "classic", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A1_classicall16" call pfRead }
call { ["A2_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A2_classic" call pfRead }
call { ["A2_classicinst", "classic", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A2_classicinst" call pfRead }
call { ["A2_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A2_classicall" call pfRead }
call { ["A2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A2_m0" call pfRead }
call { ["A2_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A2_m0ubo" call pfRead }
call { ["A2_m0inst", "remastered", 10, 1.5, 4, 4000, 1, 104, 1] call pfCfg }
triSimFrames 120
call { "A2_m0inst" call pfRead }
call { ["A2_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "A2_m0all" call pfRead }
call { ["A2_m0band", "remastered", 10, 1.5, 4, 4000, 1, 108, 1] call pfCfg }
triSimFrames 120
call { "A2_m0band" call pfRead }
call { ["A2_m0all8", "remastered", 10, 1.5, 4, 4000, 1, 115, 1] call pfCfg }
triSimFrames 120
call { "A2_m0all8" call pfRead }
call { ["A2_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A2_m0all16" call pfRead }
call { ["A2_classicall16", "classic", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "A2_classicall16" call pfRead }
call { ["S_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "S_classic" call pfRead }
triScreenshot "S_classic"
call { ["S_classicall", "classic", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "S_classicall" call pfRead }
triScreenshot "S_classicall"
call { ["S_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "S_m0" call pfRead }
triScreenshot "S_m0"
call { ["S_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "S_m0all" call pfRead }
triScreenshot "S_m0all"
call { ["S_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 60
call { "S_m0all16" call pfRead }
triScreenshot "S_m0all16"
call { ["S_classicall16", "classic", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 60
call { "S_classicall16" call pfRead }
triScreenshot "S_classicall16"
call { capDate = [1985, 7, 5, 12, 0]; [] call capPin }
triSimFrames 120
call { ["N0_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "N0_classic" call pfRead }
call { ["N0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "N0_m0" call pfRead }
call { ["N0_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "N0_m0all" call pfRead }
call { ["N0_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "N0_m0all16" call pfRead }
call { ["N1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "N1_classic" call pfRead }
call { ["N1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "N1_m0" call pfRead }
call { ["N1_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "N1_m0all" call pfRead }
call { ["N1_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "N1_m0all16" call pfRead }

["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 300
call { ["F0_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "F0_classic" call pfRead }
call { ["F0_classicall", "classic", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "F0_classicall" call pfRead }
call { ["F0_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "F0_m0" call pfRead }
call { ["F0_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "F0_m0all" call pfRead }
call { ["F0_m0all16", "remastered", 10, 1.5, 4, 2500, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "F0_m0all16" call pfRead }
call { ["F1_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "F1_classic" call pfRead }
call { ["F1_classicall", "classic", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "F1_classicall" call pfRead }
call { ["F1_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "F1_m0" call pfRead }
call { ["F1_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "F1_m0all" call pfRead }
call { ["F1_m0all16", "remastered", 10, 1.5, 4, 2500, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "F1_m0all16" call pfRead }
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
triSimFrames 120
call { "L0_classic" call pfRead }
call { ["L0_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "L0_classicall" call pfRead }
call { ["L0_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "L0_m0" call pfRead }
call { ["L0_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "L0_m0all" call pfRead }
call { ["L0_m0all16", "remastered", 10, 1.5, 4, 1500, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "L0_m0all16" call pfRead }
call { ["L1_classic", "classic", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "L1_classic" call pfRead }
call { ["L1_classicall", "classic", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "L1_classicall" call pfRead }
call { ["L1_m0", "remastered", 10, 1.5, 4, 1500, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "L1_m0" call pfRead }
call { ["L1_m0all", "remastered", 10, 1.5, 4, 1500, 1, 107, 1] call pfCfg }
triSimFrames 120
call { "L1_m0all" call pfRead }
call { ["L1_m0all16", "remastered", 10, 1.5, 4, 1500, 1, 123, 1] call pfCfg }
triSimFrames 120
call { "L1_m0all16" call pfRead }
call { ["SL_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 60
call { "SL_m0" call pfRead }
triScreenshot "SL_m0"
call { ["SL_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 60
call { "SL_m0all" call pfRead }
triScreenshot "SL_m0all"
call { ["SL_m0all16", "remastered", 10, 1.5, 4, 4000, 1, 123, 1] call pfCfg }
triSimFrames 60
call { "SL_m0all16" call pfRead }
triScreenshot "SL_m0all16"
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
