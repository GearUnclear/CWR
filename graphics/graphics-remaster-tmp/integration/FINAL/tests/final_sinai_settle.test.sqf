// FINAL check probe: which switch makes the draw count of sinai_lowsun_fp
// unsteady (Remastered UNSETTLED:300 since the stacked branch, OK:30 up to A3).
// Same view and props as sinai_lobo_lighting.test.sqf view 1; per setting it
// logs the triSettleScene answer and the draw count of 8 frames.
#include "capture_lib.sqf"

triSimUntil { alive player }
capWorld = "Sinai"
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
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false
createCenter west
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [8350, 2950, 0]
capCam = "camera" camCreate [8350, 2950, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240

["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;
    [["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;
}
triSimFrames 150

capProbeSet = {
    _this call capMode;
    capDraws = [];
    triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; triSimFrames 1; capDraws set [count capDraws, triSceneStats]; 
    format ["view=settleprobe;event=probe;setting=%1;settle=%2;stats=%3", _this, capSettleRes, capDraws] call capLog;
}
"classic" call capProbeSet
"remastered" call capProbeSet
"baseline" call capProbeSet
"remastered,hillCull=0" call capProbeSet
"remastered,drawGrouping=0" call capProbeSet
"remastered,hillCull=0,drawGrouping=0" call capProbeSet
"remastered,taa=0" call capProbeSet
"remastered,treeShadows=0" call capProbeSet
"remastered,canopyShadows=0" call capProbeSet
"remastered,airBeams=0,airHaze=0,valleyMist=0" call capProbeSet
"remastered" call capProbeSet

call { "view=done;event=done" call capLog }
triEndTest
