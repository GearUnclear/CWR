// ============================================================================
//  Research-only capture run for the object-shadows bug fix (git-excluded).
//  Same rig and views as tests/rendering/captures/abel_lighting.test.sqf, plus
//  a boot-state shot (no triSetLighting call at all: what graphics.cfg set at
//  boot), the sun-visibility debug view, extra timing reads, a daytime twin of
//  town_night and the foliage track's into-the-sun view (backlit_hornbeam).
//  Run the same file on the branch-tip build (before) and the fixed build.
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

// Boot state: the lighting model, shadow maps and object casters exactly as the
// boot path left them (graphics.cfg of the fresh test user dir). Only the GPU
// timer is switched on.
capBootMode = {
    capModeName = "boot";
    capLit = "untouched";
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=boot;lit=untouched", capView] call capLog;
}

capDbgOn = {
    capModeName = "dbgsunvis";
    capDbg = triSetLighting "debug=1";
    format ["view=%1;event=mode;mode=dbgsunvis;lit=%2", capView, capDbg] call capLog;
}
capDbgOff = { capDbg = triSetLighting "debug=0" }

// ----------------------------------------------------------------------------
//  forest_edge_golden (abel_lighting view 4), first with the boot state.
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
triSimFrames 60
[] call capReadTimers
"forest_edge_golden__t2" call capRecord
triSimFrames 60
[] call capReadTimers
"forest_edge_golden__t3" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__dbgsunvis"
"forest_edge_golden__dbgsunvis" call capRecord
[] call capDbgOff
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__classic"
"forest_edge_golden__classic" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  noon_fields (abel_lighting view 3)
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
triSimFrames 60
[] call capReadTimers
"noon_fields__t2" call capRecord
triSimFrames 60
[] call capReadTimers
"noon_fields__t3" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__dbgsunvis"
"noon_fields__dbgsunvis" call capRecord
[] call capDbgOff
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__classic"
"noon_fields__classic" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  town_night (abel_lighting view 6)
// ----------------------------------------------------------------------------
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__remastered"
"town_night__remastered" call capRecord
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__classic"
"town_night__classic" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  town_day: town_night's camera at 17:00 on 5 July (sun 25.9 deg at az 279,
//  66 deg left of the view), so the houses cast across the street.
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
triSimFrames 60
[] call capReadTimers
"town_day__t2" call capRecord
triSimFrames 60
[] call capReadTimers
"town_day__t3" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "town_day__dbgsunvis"
"town_day__dbgsunvis" call capRecord
[] call capDbgOff
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_day__classic"
"town_day__classic" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  backlit_hornbeam (foliage track): camera into a 12 deg sun through a grove.
//  The foliage track saw sun visibility 1 everywhere here.
// ----------------------------------------------------------------------------
["backlit_hornbeam", 7495, 10210, 1.75, 283, 6, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "backlit_hornbeam__remastered"
"backlit_hornbeam__remastered" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "backlit_hornbeam__dbgsunvis"
"backlit_hornbeam__dbgsunvis" call capRecord
[] call capDbgOff
[] call capClear

// ----------------------------------------------------------------------------
//  pine_stand_noon (foliage track): the clearing its sun-visibility view showed
//  in flat shade at noon.
// ----------------------------------------------------------------------------
["pine_stand_noon", 4770, 10128, 1.75, 75, -3, [1985, 7, 5, 12, 30], 0, 0, 1500] call capBegin
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "pine_stand_noon__remastered"
"pine_stand_noon__remastered" call capRecord
[] call capDbgOn
triSimFrames 20
[] call capPin
triSimFrames 3
triScreenshot "pine_stand_noon__dbgsunvis"
"pine_stand_noon__dbgsunvis" call capRecord
[] call capDbgOff
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
