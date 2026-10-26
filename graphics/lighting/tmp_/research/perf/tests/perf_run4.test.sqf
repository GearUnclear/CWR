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
call { ["A0_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A0_classic" call pfRead }
call { ["A0_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A0_classicubo" call pfRead }
call { ["A0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A0_m0" call pfRead }
call { ["A0_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A0_m0ubo" call pfRead }
call { ["A0_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A0_unlit" call pfRead }
call { ["A0_amp4", "remastered", 10, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A0_amp4" call pfRead }
call { ["A0_amp4shadow", "remastered", 4106, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A0_amp4shadow" call pfRead }
call { ["A1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A1_classic" call pfRead }
call { ["A1_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A1_classicubo" call pfRead }
call { ["A1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A1_m0" call pfRead }
call { ["A1_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A1_m0ubo" call pfRead }
call { ["A1_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A1_unlit" call pfRead }
call { ["A1_amp4", "remastered", 10, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A1_amp4" call pfRead }
call { ["A1_amp4shadow", "remastered", 4106, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A1_amp4shadow" call pfRead }
call { ["A2_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A2_classic" call pfRead }
call { ["A2_classicubo", "classic", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A2_classicubo" call pfRead }
call { ["A2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A2_m0" call pfRead }
call { ["A2_m0ubo", "remastered", 10, 1.5, 4, 4000, 1, 103, 1] call pfCfg }
triSimFrames 120
call { "A2_m0ubo" call pfRead }
call { ["A2_unlit", "remastered", 138, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "A2_unlit" call pfRead }
call { ["A2_amp4", "remastered", 10, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A2_amp4" call pfRead }
call { ["A2_amp4shadow", "remastered", 4106, 1.5, 4, 4000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "A2_amp4shadow" call pfRead }
call { ["C0_classic", "classic", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C0_classic" call pfRead }
call { ["C0_m0", "remastered", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C0_m0" call pfRead }
call { ["C0_unlit", "remastered", 138, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C0_unlit" call pfRead }
call { ["C0_amp2", "remastered", 10, 2.0, 8, 1000, 1, 0, 102] call pfCfg }
triSimFrames 120
call { "C0_amp2" call pfRead }
call { ["C0_amp4", "remastered", 10, 2.0, 8, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "C0_amp4" call pfRead }
call { ["C0_amp8", "remastered", 10, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C0_amp8" call pfRead }
call { ["C0_amp8shadow", "remastered", 4106, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C0_amp8shadow" call pfRead }
call { ["C0_amp8terrain", "remastered", 8202, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C0_amp8terrain" call pfRead }
call { ["C1_classic", "classic", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C1_classic" call pfRead }
call { ["C1_m0", "remastered", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C1_m0" call pfRead }
call { ["C1_unlit", "remastered", 138, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C1_unlit" call pfRead }
call { ["C1_amp2", "remastered", 10, 2.0, 8, 1000, 1, 0, 102] call pfCfg }
triSimFrames 120
call { "C1_amp2" call pfRead }
call { ["C1_amp4", "remastered", 10, 2.0, 8, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "C1_amp4" call pfRead }
call { ["C1_amp8", "remastered", 10, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C1_amp8" call pfRead }
call { ["C1_amp8shadow", "remastered", 4106, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C1_amp8shadow" call pfRead }
call { ["C1_amp8terrain", "remastered", 8202, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C1_amp8terrain" call pfRead }
call { ["C2_classic", "classic", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C2_classic" call pfRead }
call { ["C2_m0", "remastered", 10, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C2_m0" call pfRead }
call { ["C2_unlit", "remastered", 138, 2.0, 8, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "C2_unlit" call pfRead }
call { ["C2_amp2", "remastered", 10, 2.0, 8, 1000, 1, 0, 102] call pfCfg }
triSimFrames 120
call { "C2_amp2" call pfRead }
call { ["C2_amp4", "remastered", 10, 2.0, 8, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "C2_amp4" call pfRead }
call { ["C2_amp8", "remastered", 10, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C2_amp8" call pfRead }
call { ["C2_amp8shadow", "remastered", 4106, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C2_amp8shadow" call pfRead }
call { ["C2_amp8terrain", "remastered", 8202, 2.0, 8, 1000, 1, 0, 108] call pfCfg }
triSimFrames 120
call { "C2_amp8terrain" call pfRead }
call { ["D0_classic", "classic", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D0_classic" call pfRead }
call { ["D0_m0", "remastered", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D0_m0" call pfRead }
call { ["D0_unlit", "remastered", 138, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D0_unlit" call pfRead }
call { ["D0_amp4", "remastered", 10, 1.5, 4, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "D0_amp4" call pfRead }
call { ["D1_classic", "classic", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D1_classic" call pfRead }
call { ["D1_m0", "remastered", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D1_m0" call pfRead }
call { ["D1_unlit", "remastered", 138, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D1_unlit" call pfRead }
call { ["D1_amp4", "remastered", 10, 1.5, 4, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "D1_amp4" call pfRead }
call { ["D2_classic", "classic", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D2_classic" call pfRead }
call { ["D2_m0", "remastered", 10, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D2_m0" call pfRead }
call { ["D2_unlit", "remastered", 138, 1.5, 4, 1000, 1, 0, 1] call pfCfg }
triSimFrames 120
call { "D2_unlit" call pfRead }
call { ["D2_amp4", "remastered", 10, 1.5, 4, 1000, 1, 0, 104] call pfCfg }
triSimFrames 120
call { "D2_amp4" call pfRead }
call { ["S_m0_amp8", "remastered", 10, 1.5, 4, 4000, 1, 0, 108] call pfCfg }
triSimFrames 60
call { "S_m0_amp8" call pfRead }
triScreenshot "S_m0_amp8"

call { logInfo "UDPERF;event=done" }
triEndTest
