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
call { ["Q0_classic427", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q0_classic427" call pfRead }
call { ["Q0_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0" call pfRead }
call { ["Q0_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all" call pfRead }
call { ["Q0_m0all427", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all427" call pfRead }
call { ["Q0_m0all555", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q0_m0all555" call pfRead }
call { ["Q1_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_classic" call pfRead }
call { ["Q1_classic427", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q1_classic427" call pfRead }
call { ["Q1_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0" call pfRead }
call { ["Q1_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all" call pfRead }
call { ["Q1_m0all427", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all427" call pfRead }
call { ["Q1_m0all555", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q1_m0all555" call pfRead }
call { ["Q2_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_classic" call pfRead }
call { ["Q2_classic427", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q2_classic427" call pfRead }
call { ["Q2_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0" call pfRead }
call { ["Q2_m0all", "remastered", 10, 1.5, 4, 4000, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all" call pfRead }
call { ["Q2_m0all427", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all427" call pfRead }
call { ["Q2_m0all555", "remastered", 10, 1.5, 4, 4000, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "Q2_m0all555" call pfRead }
call { ["SQ_m0", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0" call pfRead }
triScreenshot "SQ_m0"
call { ["SQ_m0all427", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0all427" call pfRead }
triScreenshot "SQ_m0all427"
call { ["SQ_m0_b", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_m0_b" call pfRead }
triScreenshot "SQ_m0_b"
call { ["SQ_classic", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SQ_classic" call pfRead }
triScreenshot "SQ_classic"
call { ["SQ_classic427", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 90
call { "SQ_classic427" call pfRead }
triScreenshot "SQ_classic427"
call { ["MQ0_m0_setup", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 10
call { "MQ0_m0_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ0_m0"; "MQ0_m0" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ0_m0all427_setup", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 10
call { "MQ0_m0all427_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ0_m0all427"; "MQ0_m0all427" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ0_classic_setup", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 10
call { "MQ0_classic_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ0_classic"; "MQ0_classic" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ0_classic427_setup", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 10
call { "MQ0_classic427_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ0_classic427"; "MQ0_classic427" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ1_m0_setup", "remastered", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 10
call { "MQ1_m0_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ1_m0"; "MQ1_m0" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ1_m0all427_setup", "remastered", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 10
call { "MQ1_m0all427_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ1_m0all427"; "MQ1_m0all427" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ1_classic_setup", "classic", 10, 1.5, 4, 4000, 1, 0, 1] call pfCfg }
triSimFrames 10
call { "MQ1_classic_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ1_classic"; "MQ1_classic" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
call { ["MQ1_classic427_setup", "classic", 10, 1.5, 4, 4000, 1, 427, 1] call pfCfg }
triSimFrames 10
call { "MQ1_classic427_setup" call pfRead }
call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }
call { capOX = capOX0 + 0.5 * (sin capH); capOZ = capOZ0 + 0.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.0 * (sin capH); capOZ = capOZ0 + 1.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 1.5 * (sin capH); capOZ = capOZ0 + 1.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.0 * (sin capH); capOZ = capOZ0 + 2.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 2.5 * (sin capH); capOZ = capOZ0 + 2.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.0 * (sin capH); capOZ = capOZ0 + 3.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 3.5 * (sin capH); capOZ = capOZ0 + 3.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.0 * (sin capH); capOZ = capOZ0 + 4.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 4.5 * (sin capH); capOZ = capOZ0 + 4.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.0 * (sin capH); capOZ = capOZ0 + 5.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 5.5 * (sin capH); capOZ = capOZ0 + 5.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.0 * (sin capH); capOZ = capOZ0 + 6.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 6.5 * (sin capH); capOZ = capOZ0 + 6.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.0 * (sin capH); capOZ = capOZ0 + 7.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 7.5 * (sin capH); capOZ = capOZ0 + 7.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.0 * (sin capH); capOZ = capOZ0 + 8.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 8.5 * (sin capH); capOZ = capOZ0 + 8.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.0 * (sin capH); capOZ = capOZ0 + 9.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 9.5 * (sin capH); capOZ = capOZ0 + 9.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.0 * (sin capH); capOZ = capOZ0 + 10.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 10.5 * (sin capH); capOZ = capOZ0 + 10.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.0 * (sin capH); capOZ = capOZ0 + 11.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 11.5 * (sin capH); capOZ = capOZ0 + 11.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.0 * (sin capH); capOZ = capOZ0 + 12.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 12.5 * (sin capH); capOZ = capOZ0 + 12.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.0 * (sin capH); capOZ = capOZ0 + 13.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 13.5 * (sin capH); capOZ = capOZ0 + 13.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.0 * (sin capH); capOZ = capOZ0 + 14.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 14.5 * (sin capH); capOZ = capOZ0 + 14.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.0 * (sin capH); capOZ = capOZ0 + 15.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 15.5 * (sin capH); capOZ = capOZ0 + 15.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.0 * (sin capH); capOZ = capOZ0 + 16.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 16.5 * (sin capH); capOZ = capOZ0 + 16.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.0 * (sin capH); capOZ = capOZ0 + 17.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 17.5 * (sin capH); capOZ = capOZ0 + 17.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.0 * (sin capH); capOZ = capOZ0 + 18.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 18.5 * (sin capH); capOZ = capOZ0 + 18.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.0 * (sin capH); capOZ = capOZ0 + 19.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 19.5 * (sin capH); capOZ = capOZ0 + 19.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.0 * (sin capH); capOZ = capOZ0 + 20.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 20.5 * (sin capH); capOZ = capOZ0 + 20.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.0 * (sin capH); capOZ = capOZ0 + 21.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 21.5 * (sin capH); capOZ = capOZ0 + 21.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.0 * (sin capH); capOZ = capOZ0 + 22.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 22.5 * (sin capH); capOZ = capOZ0 + 22.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.0 * (sin capH); capOZ = capOZ0 + 23.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 23.5 * (sin capH); capOZ = capOZ0 + 23.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.0 * (sin capH); capOZ = capOZ0 + 24.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 24.5 * (sin capH); capOZ = capOZ0 + 24.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.0 * (sin capH); capOZ = capOZ0 + 25.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 25.5 * (sin capH); capOZ = capOZ0 + 25.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.0 * (sin capH); capOZ = capOZ0 + 26.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 26.5 * (sin capH); capOZ = capOZ0 + 26.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.0 * (sin capH); capOZ = capOZ0 + 27.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 27.5 * (sin capH); capOZ = capOZ0 + 27.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.0 * (sin capH); capOZ = capOZ0 + 28.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 28.5 * (sin capH); capOZ = capOZ0 + 28.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.0 * (sin capH); capOZ = capOZ0 + 29.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 29.5 * (sin capH); capOZ = capOZ0 + 29.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.0 * (sin capH); capOZ = capOZ0 + 30.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 30.5 * (sin capH); capOZ = capOZ0 + 30.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.0 * (sin capH); capOZ = capOZ0 + 31.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 31.5 * (sin capH); capOZ = capOZ0 + 31.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.0 * (sin capH); capOZ = capOZ0 + 32.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 32.5 * (sin capH); capOZ = capOZ0 + 32.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.0 * (sin capH); capOZ = capOZ0 + 33.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 33.5 * (sin capH); capOZ = capOZ0 + 33.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.0 * (sin capH); capOZ = capOZ0 + 34.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 34.5 * (sin capH); capOZ = capOZ0 + 34.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.0 * (sin capH); capOZ = capOZ0 + 35.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 35.5 * (sin capH); capOZ = capOZ0 + 35.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.0 * (sin capH); capOZ = capOZ0 + 36.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 36.5 * (sin capH); capOZ = capOZ0 + 36.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.0 * (sin capH); capOZ = capOZ0 + 37.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 37.5 * (sin capH); capOZ = capOZ0 + 37.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.0 * (sin capH); capOZ = capOZ0 + 38.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 38.5 * (sin capH); capOZ = capOZ0 + 38.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.0 * (sin capH); capOZ = capOZ0 + 39.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 39.5 * (sin capH); capOZ = capOZ0 + 39.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.0 * (sin capH); capOZ = capOZ0 + 40.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 40.5 * (sin capH); capOZ = capOZ0 + 40.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.0 * (sin capH); capOZ = capOZ0 + 41.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 41.5 * (sin capH); capOZ = capOZ0 + 41.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.0 * (sin capH); capOZ = capOZ0 + 42.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 42.5 * (sin capH); capOZ = capOZ0 + 42.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.0 * (sin capH); capOZ = capOZ0 + 43.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 43.5 * (sin capH); capOZ = capOZ0 + 43.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.0 * (sin capH); capOZ = capOZ0 + 44.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 44.5 * (sin capH); capOZ = capOZ0 + 44.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.0 * (sin capH); capOZ = capOZ0 + 45.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 45.5 * (sin capH); capOZ = capOZ0 + 45.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.0 * (sin capH); capOZ = capOZ0 + 46.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 46.5 * (sin capH); capOZ = capOZ0 + 46.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.0 * (sin capH); capOZ = capOZ0 + 47.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 47.5 * (sin capH); capOZ = capOZ0 + 47.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.0 * (sin capH); capOZ = capOZ0 + 48.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 48.5 * (sin capH); capOZ = capOZ0 + 48.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.0 * (sin capH); capOZ = capOZ0 + 49.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 49.5 * (sin capH); capOZ = capOZ0 + 49.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.0 * (sin capH); capOZ = capOZ0 + 50.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 50.5 * (sin capH); capOZ = capOZ0 + 50.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.0 * (sin capH); capOZ = capOZ0 + 51.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 51.5 * (sin capH); capOZ = capOZ0 + 51.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.0 * (sin capH); capOZ = capOZ0 + 52.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 52.5 * (sin capH); capOZ = capOZ0 + 52.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.0 * (sin capH); capOZ = capOZ0 + 53.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 53.5 * (sin capH); capOZ = capOZ0 + 53.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.0 * (sin capH); capOZ = capOZ0 + 54.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 54.5 * (sin capH); capOZ = capOZ0 + 54.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.0 * (sin capH); capOZ = capOZ0 + 55.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 55.5 * (sin capH); capOZ = capOZ0 + 55.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.0 * (sin capH); capOZ = capOZ0 + 56.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 56.5 * (sin capH); capOZ = capOZ0 + 56.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.0 * (sin capH); capOZ = capOZ0 + 57.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 57.5 * (sin capH); capOZ = capOZ0 + 57.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.0 * (sin capH); capOZ = capOZ0 + 58.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 58.5 * (sin capH); capOZ = capOZ0 + 58.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.0 * (sin capH); capOZ = capOZ0 + 59.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 59.5 * (sin capH); capOZ = capOZ0 + 59.5 * (cos capH); [] call capPinView }
triSimFrames 1
call { capOX = capOX0 + 60.0 * (sin capH); capOZ = capOZ0 + 60.0 * (cos capH); [] call capPinView }
triSimFrames 1
call { pfLabel = "MQ1_classic427"; "MQ1_classic427" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }
triSimFrames 5
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
call { ["F0_classic427", "classic", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F0_classic427" call pfRead }
call { ["F0_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F0_m0" call pfRead }
call { ["F0_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all" call pfRead }
call { ["F0_m0all427", "remastered", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all427" call pfRead }
call { ["F0_m0all555", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F0_m0all555" call pfRead }
call { ["F1_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_classic" call pfRead }
call { ["F1_classic427", "classic", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F1_classic427" call pfRead }
call { ["F1_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F1_m0" call pfRead }
call { ["F1_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all" call pfRead }
call { ["F1_m0all427", "remastered", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all427" call pfRead }
call { ["F1_m0all555", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F1_m0all555" call pfRead }
call { ["F2_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_classic" call pfRead }
call { ["F2_classic427", "classic", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F2_classic427" call pfRead }
call { ["F2_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 150
call { "F2_m0" call pfRead }
call { ["F2_m0all", "remastered", 10, 1.5, 4, 2500, 1, 107, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all" call pfRead }
call { ["F2_m0all427", "remastered", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all427" call pfRead }
call { ["F2_m0all555", "remastered", 10, 1.5, 4, 2500, 1, 555, 1] call pfCfg }
triSimFrames 150
call { "F2_m0all555" call pfRead }
call { ["SF_m0", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_m0" call pfRead }
triScreenshot "SF_m0"
call { ["SF_m0all427", "remastered", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 90
call { "SF_m0all427" call pfRead }
triScreenshot "SF_m0all427"
call { ["SF_m0_b", "remastered", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_m0_b" call pfRead }
triScreenshot "SF_m0_b"
call { ["SF_classic", "classic", 10, 1.5, 4, 2500, 1, 0, 1] call pfCfg }
triSimFrames 90
call { "SF_classic" call pfRead }
triScreenshot "SF_classic"
call { ["SF_classic427", "classic", 10, 1.5, 4, 2500, 1, 427, 1] call pfCfg }
triSimFrames 90
call { "SF_classic427" call pfRead }
triScreenshot "SF_classic427"
[] call capClear

call { logInfo "UDPERF;event=done" }
triEndTest
