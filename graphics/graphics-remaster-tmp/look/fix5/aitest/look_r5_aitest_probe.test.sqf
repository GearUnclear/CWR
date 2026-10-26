// fix5 aitest probe: an east soldier's real AI visibility (EntityAI::CalcVisibility,
// triAiSight) of west soldiers in the mist at the look_director_optics staging
// (Eden 9200,3700, heading 288, 09:30 5 July 1985, overcast 0.7, fog 0.3),
// next to the drawn fog's air on the same lines, in each render path state.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Eden"
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
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240

call { capRain = 0; capEyeAbs = false }
["aiprobe", 9200, 3700, 1.75, 288, -2, [1985, 7, 5, 9, 30], 0.7, 0.3, 1500] call capBegin
call {
    capObs = [["SoldierEB"], -3, 0, 0, "man", capGrpE] call capSpawn;
    capT1 = [["SoldierWB"], 120, -8, 180, "man"] call capSpawn;
    capT2 = [["SoldierWB"], 250, 14, 180, "man"] call capSpawn;
    capT3 = [["SoldierWB"], 450, -30, 180, "man"] call capSpawn;
    capT4 = [["SoldierWB"], 750, 40, 180, "man"] call capSpawn;
    capT5 = [["SoldierWB"], 1100, -60, 180, "man"] call capSpawn;
}
capTargets = [capT1, capT2, capT3, capT4, capT5]
capProbeLog = {
    private ["_st", "_i"];
    _st = _this;
    _i = 1;
    {
        format ["view=%4;event=sight;state=%1;t=%2;%3", _st, _i, triAiSight [capObs, _x], capView] call capLog;
        _i = _i + 1;
    } forEach capTargets;
}
"remastered" call capMode
[] call capPin
triSimFrames 90
[] call capPin
triSimFrames 3
"overhaul" call capProbeLog
triScreenshot "aiprobe__overhaul"
"aiprobe__overhaul" call capRecord
triSimFrames 3

capLit = triSetLighting "remastered,post=0"
triSimFrames 30
"post_off" call capProbeLog

capLit = triSetLighting "remastered,fogMist=0.5"
triSimFrames 30
"mist_half" call capProbeLog

capLit = triSetLighting "classic"
triSimFrames 30
"classic" call capProbeLog

capLit = triSetLighting "remastered"
triSimFrames 30
"back" call capProbeLog

// ---- the valley in that shot: the lowest ground ahead on the heading, then
// the lowest point 250 m from it; an east soldier on the first sees a west
// soldier on the second along the valley floor, where the mist lies.
capLowH = 9999
capLowD = 0
{
    capTmpP = [_x, 0] call capRel;
    capTmpH = capTmpP call capGround;
    if (capTmpH < capLowH) then { capLowH = capTmpH; capLowD = _x };
} forEach [300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1100, 1200, 1300, 1400]
capLowP = [capLowD, 0] call capRel
capBestH = 9999
capBestDir = 0
{
    capTmpP = [(capLowP select 0) + 250 * (sin _x), (capLowP select 1) + 250 * (cos _x)];
    capTmpH = capTmpP call capGround;
    format ["view=aivalley;event=dir;dir=%1;h=%2", _x, capTmpH] call capLog;
    if (capTmpH < capBestH) then { capBestH = capTmpH; capBestDir = _x };
} forEach [0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330]
format ["view=aivalley;event=valley;d=%1;at=%2;h=%3;dir=%4;hEnd=%5", capLowD, capLowP, capLowH, capBestDir, capBestH] call capLog
capView = "aivalley"
capOX = capLowP select 0
capOZ = capLowP select 1
capH = capBestDir
capP = -1
call {
    capObsV = [["SoldierEB"], -3, 0, 0, "man", capGrpE] call capSpawn;
    capTV1 = [["SoldierWB"], 120, 4, 180, "man"] call capSpawn;
    capTV2 = [["SoldierWB"], 250, -6, 180, "man"] call capSpawn;
}
[] call capPin
triSimFrames 60
[] call capPin
triSimFrames 3
capObs = capObsV
capTargets = [capTV1, capTV2]
"valley" call capProbeLog
triScreenshot "aivalley__overhaul"
"aivalley__overhaul" call capRecord
triSimFrames 3
capLit = triSetLighting "remastered,post=0"
triSimFrames 30
"valley_post_off" call capProbeLog
capLit = triSetLighting "classic"
triSimFrames 30
"valley_classic" call capProbeLog
capLit = triSetLighting "remastered"
triSimFrames 30
"valley_back" call capProbeLog
call { "view=done;event=done" call capLog }
triEndTest
