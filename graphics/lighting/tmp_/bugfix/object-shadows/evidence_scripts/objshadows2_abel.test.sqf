// ============================================================================
//  Research-only capture run for the object-shadows fix, round 2 (not part of
//  the patch). Same rig as abel_lighting.test.sqf. Adds a boot-state shot (no
//  triSetLighting at all: what graphics.cfg set at boot), the sun-visibility
//  debug view (triSetLighting "debug=1": white is lit) and town_day, the
//  town_night camera at 17:00 so the houses cast in sunlight. Run the same file
//  on the branch-tip build (before) and the fixed build (after).
// ============================================================================

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
capDbg = "none"

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
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// Boot state: the lighting model, shadow maps and object casters as the boot
// path left them. Detail pinned and the view settled as capMode does, but no
// triSetLighting.
capBootMode = {
    capModeName = "boot";
    capLit = "untouched";
    capPinRes = triPinObjectDetail capDetail;
    capSettleRes = triSettleScene capSettleArgs;
    capModeStats = triSceneStats;
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=boot;lit=untouched;detail=%2;settle=%3;%4", capView, capPinRes, capSettleRes, capModeStats] call capLog;
}

capDbgOn = {
    capModeName = "dbgsunvis";
    capDbg = triSetLighting "debug=1";
    format ["view=%1;event=mode;mode=dbgsunvis;lit=%2", capView, capDbg] call capLog;
}
capDbgOff = { capDbg = triSetLighting "debug=0" }

// ----------------------------------------------------------------------------
//  forest_edge_golden (abel_lighting view 4): boot state, remastered, debug.
// ----------------------------------------------------------------------------
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
[] call capBootMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__boot"
"forest_edge_golden__boot" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__dbgsunvis"
"forest_edge_golden__dbgsunvis" call capRecord
[] call capDbgOff
[] call capClear

// ----------------------------------------------------------------------------
//  noon_fields (abel_lighting view 3): remastered, debug.
// ----------------------------------------------------------------------------
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__remastered"
"noon_fields__remastered" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__dbgsunvis"
"noon_fields__dbgsunvis" call capRecord
[] call capDbgOff
[] call capClear

// ----------------------------------------------------------------------------
//  town_day: town_night's camera at 17:00 on 5 July, so the houses cast
//  across the street. Remastered, debug.
// ----------------------------------------------------------------------------
["town_day", 7152, 8915, 1.75, 345, 3, [1985, 7, 5, 17, 0], 0, 0, 800] call capBegin
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_day__remastered"
"town_day__remastered" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "town_day__dbgsunvis"
"town_day__dbgsunvis" call capRecord
[] call capDbgOff
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
