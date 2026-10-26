// ============================================================================
//  Remastered lighting: Classic | Remastered capture pairs on Abel (Malden).
//  Not a regression test: a repeatable photo run, driven by run-captures.ps1
//  (see README.md beside this file for the view table and how to run it).
//
//  Every matched view runs the same block twice, Classic then Remastered:
//    capMode                            pin the object detail, switch the lighting
//                                       model (every knob at its default), wait
//                                       until the view has loaded, restart the
//                                       GPU / CPU timers
//    triSimFrames 90                    enough frames for the smoothed timers
//    capReadTimers                      keep the timings of this mode
//    capPin                             same date, weather, sim clock, props, view
//    triSimFrames 3
//    triScreenshot "<view>__<mode>"     the presented frame (triCaptureAtPresent)
//    capRecord                          one UDCAP;event=shot log line
//  The remastered exposure meter is a pure function of the sun height
//  (LightRig.cpp), with no temporal adaptation, so nothing has to "settle" in
//  the tonemap between the two shots.
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

// Capture settings shared by every view: 4x MSAA, 150% render scale (both
// applied at the next frame boundary), and the presented, post-gamma frame.
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

// The horizon map (TerrainHorizon) is built off-thread when the island loads;
// give it time before the first frame that matters.
triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }


//  7. firefight - NOT a matched pair. Open flat ground east of view 1
//  [7600, 10850] (29 m), camera looking north (0) at 2 m. 19:35 on 5 July,
//  sun -1.7 deg: dusk, so muzzle flashes, explosions and fire read as local
//  light. Four SoldierWB (west, left) and three SoldierEB (east, right) face
//  each other 22-28 m apart across the frame; everyone is captive, and each
//  man is ordered to doFire a man of the other line (the tools/screenshots
//  rig: reveal + per-man doFire aims level at a human). A Ural is destroyed
//  and kept burning behind the east line, and a 120 mm shell is dropped
//  30+ m behind the lines just before shots 1 and 2 (run 1 dropped them into
//  the gap and the blasts killed both squads before the Remastered half,
//  allowDamage false notwithstanding). The whole set is staged afresh for
//  each mode (capFFStage / capFFClear), so both modes start from the same
//  layout; AI fire and the sim clock are not re-pinned, so the flashes differ.
//  The shells are timed: from the first drop to the third shot the world steps
//  a fixed 1/30 s a frame (capFixStep, triSetFixedStep), so every run and every
//  round catches the second shell's burst at the same age: firefight_2 6
//  frames (0.2 s) after its drop, a young fireball (the shell falls 40 m in
//  about 5 frames), firefight_3 26 frames (0.87 s) after it, a smoke column
//  with its dust foot. Real time again after the third shot.
// ----------------------------------------------------------------------------
["firefight", 7600, 10850, 2, 0, 0, [1985, 7, 5, 19, 35], 0, 0, 1000] call capBegin
capPinTime = false
capFW = []
capFE = []
capBurnV = objNull
capFFStage = {
    capGrpFW = createGroup west;
    capGrpFE = createGroup east;
    east setFriend [west, 0];
    west setFriend [east, 0];
    capFW = [];
    capFE = [];
    { capNew = [["SoldierWB"], _x select 0, _x select 1, 90, "fighter", capGrpFW] call capSpawn; if (not (isNull capNew)) then { capFW = capFW + [capNew] } } forEach [[24, -15], [26, -12], [23, -9], [27, -6]];
    { capNew = [["SoldierEB"], _x select 0, _x select 1, -90, "fighter", capGrpFE] call capSpawn; if (not (isNull capNew)) then { capFE = capFE + [capNew] } } forEach [[25, 9], [28, 12], [24, 15]];
    capBurnV = [["Ural", "Truck5t"], 60, 24, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    format ["view=firefight;event=squads;west=%1;east=%2", count capFW, count capFE] call capLog;
}
capFFClear = {
    capAliveW = 0;
    capAliveE = 0;
    { if (alive _x) then { capAliveW = capAliveW + 1 } } forEach capFW;
    { if (alive _x) then { capAliveE = capAliveE + 1 } } forEach capFE;
    format ["view=firefight;event=survivors;west=%1;east=%2", capAliveW, capAliveE] call capLog;
    [] call capClear;
    capFW = [];
    capFE = [];
}
capEngage = {
    private ["_i"];
    if (((count capFW) > 0) and ((count capFE) > 0)) then {
        { (capFW select 0) reveal _x } forEach capFE;
        { (capFE select 0) reveal _x } forEach capFW;
        _i = 0;
        { _x doFire (capFE select (_i mod (count capFE))); _i = _i + 1 } forEach capFW;
        _i = 0;
        { _x doFire (capFW select (_i mod (count capFW))); _i = _i + 1 } forEach capFE;
    };
}
// [forward, right] call capBoom: a 120 mm shell dropped from 40 m into that spot.
capBoom = {
    private ["_p"];
    _p = _this call capRel;
    if ((count capFE) > 0) then { capShell = createShell ["Shell120", [_p select 0, _p select 1, 40], [0, 0, -250], capFE select 0, objNull] };
}
capBurn = { if (not (isNull capBurnV)) then { capBurnV inflame true } }
// <seconds> call capFixStep: a fixed sim step (0 = real time again).
capFixStep = { capFixRes = triSetFixedStep _this; format ["view=firefight;event=fixstep;step=%1;res=%2", _this, capFixRes] call capLog }

// [[classes], [forward, right], height, velocity] call capMunition: the first class that spawns.
capMunition = {
    private ["_cls", "_p", "_h", "_v", "_s"];
    _cls = _this select 0; _p = (_this select 1) call capRel; _h = _this select 2; _v = _this select 3;
    _s = objNull;
    { if (isNull _s) then { _s = createShell [_x, [_p select 0, _p select 1, _h], _v, player, objNull]; if (not (isNull _s)) then { capMunClass = _x } } } forEach _cls;
    format ["view=munition;event=spawn;class=%1;obj=%2", capMunClass, _s] call capLog;
}
capMunClass = "none"

[] call capFFStage
"remastered" call capMode
triSimFrames 200
call { [] call capPinEnv; [] call capPinView; [] call capBurn }
0.0333333 call capFixStep
triSimFrames 1
triScreenshot "mun_dusk_he_f001"
triSimFrames 1
triScreenshot "mun_dusk_he_f002"
triSimFrames 1
triScreenshot "mun_dusk_he_f003"
triSimFrames 1
triScreenshot "mun_dusk_he_f004"
triSimFrames 1
triScreenshot "mun_dusk_he_f005"
triSimFrames 1
triScreenshot "mun_dusk_he_f006"
triSimFrames 1
triScreenshot "mun_dusk_he_f007"
triSimFrames 1
triScreenshot "mun_dusk_he_f008"
call { [["Shell120", "Shell125", "Shell105"], [50, -10], 40, [0, 0, -250]] call capMunition }
triSimFrames 1
triScreenshot "mun_dusk_he_f009"
triSimFrames 1
triScreenshot "mun_dusk_he_f010"
triSimFrames 1
triScreenshot "mun_dusk_he_f011"
triSimFrames 1
triScreenshot "mun_dusk_he_f012"
triSimFrames 1
triScreenshot "mun_dusk_he_f013"
triSimFrames 1
triScreenshot "mun_dusk_he_f014"
triSimFrames 1
triScreenshot "mun_dusk_he_f015"
triSimFrames 1
triScreenshot "mun_dusk_he_f016"
triSimFrames 1
triScreenshot "mun_dusk_he_f017"
triSimFrames 1
triScreenshot "mun_dusk_he_f018"
triSimFrames 1
triScreenshot "mun_dusk_he_f019"
triSimFrames 1
triScreenshot "mun_dusk_he_f020"
triSimFrames 1
triScreenshot "mun_dusk_he_f021"
triSimFrames 1
triScreenshot "mun_dusk_he_f022"
triSimFrames 1
triScreenshot "mun_dusk_he_f023"
triSimFrames 1
triScreenshot "mun_dusk_he_f024"
triSimFrames 1
triScreenshot "mun_dusk_he_f025"
triSimFrames 1
triScreenshot "mun_dusk_he_f026"
triSimFrames 1
triScreenshot "mun_dusk_he_f027"
triSimFrames 1
triScreenshot "mun_dusk_he_f028"
triSimFrames 1
triScreenshot "mun_dusk_he_f029"
triSimFrames 1
triScreenshot "mun_dusk_he_f030"
triSimFrames 1
triScreenshot "mun_dusk_he_f031"
triSimFrames 1
triScreenshot "mun_dusk_he_f032"
triSimFrames 1
triScreenshot "mun_dusk_he_f033"
triSimFrames 1
triScreenshot "mun_dusk_he_f034"
triSimFrames 1
triScreenshot "mun_dusk_he_f035"
triSimFrames 1
triScreenshot "mun_dusk_he_f036"
triSimFrames 1
triScreenshot "mun_dusk_he_f037"
triSimFrames 1
triScreenshot "mun_dusk_he_f038"
triSimFrames 1
triScreenshot "mun_dusk_he_f039"
triSimFrames 1
triScreenshot "mun_dusk_he_f040"
0 call capFixStep
[] call capFFClear
call { "view=done;event=done" call capLog }
triEndTest
