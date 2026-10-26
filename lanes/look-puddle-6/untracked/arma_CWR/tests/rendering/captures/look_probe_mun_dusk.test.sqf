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
triSimFrames 1
triScreenshot "mun_dusk_he_f041"
triSimFrames 1
triScreenshot "mun_dusk_he_f042"
triSimFrames 1
triScreenshot "mun_dusk_he_f043"
triSimFrames 1
triScreenshot "mun_dusk_he_f044"
triSimFrames 1
triScreenshot "mun_dusk_he_f045"
triSimFrames 1
triScreenshot "mun_dusk_he_f046"
triSimFrames 1
triScreenshot "mun_dusk_he_f047"
triSimFrames 1
triScreenshot "mun_dusk_he_f048"
triSimFrames 1
triScreenshot "mun_dusk_he_f049"
triSimFrames 1
triScreenshot "mun_dusk_he_f050"
triSimFrames 1
triScreenshot "mun_dusk_he_f051"
triSimFrames 1
triScreenshot "mun_dusk_he_f052"
triSimFrames 1
triScreenshot "mun_dusk_he_f053"
triSimFrames 1
triScreenshot "mun_dusk_he_f054"
triSimFrames 1
triScreenshot "mun_dusk_he_f055"
triSimFrames 1
triScreenshot "mun_dusk_he_f056"
triSimFrames 1
triScreenshot "mun_dusk_he_f057"
triSimFrames 1
triScreenshot "mun_dusk_he_f058"
triSimFrames 1
triScreenshot "mun_dusk_he_f059"
triSimFrames 1
triScreenshot "mun_dusk_he_f060"
triSimFrames 1
triScreenshot "mun_dusk_he_f061"
triSimFrames 1
triScreenshot "mun_dusk_he_f062"
triSimFrames 1
triScreenshot "mun_dusk_he_f063"
triSimFrames 1
triScreenshot "mun_dusk_he_f064"
triSimFrames 1
triScreenshot "mun_dusk_he_f065"
triSimFrames 1
triScreenshot "mun_dusk_he_f066"
triSimFrames 1
triScreenshot "mun_dusk_he_f067"
triSimFrames 1
triScreenshot "mun_dusk_he_f068"
triSimFrames 1
triScreenshot "mun_dusk_he_f069"
triSimFrames 1
triScreenshot "mun_dusk_he_f070"
triSimFrames 1
triScreenshot "mun_dusk_he_f071"
triSimFrames 1
triScreenshot "mun_dusk_he_f072"
triSimFrames 1
triScreenshot "mun_dusk_he_f073"
triSimFrames 1
triScreenshot "mun_dusk_he_f074"
triSimFrames 1
triScreenshot "mun_dusk_he_f075"
triSimFrames 1
triScreenshot "mun_dusk_he_f076"
triSimFrames 1
triScreenshot "mun_dusk_he_f077"
triSimFrames 1
triScreenshot "mun_dusk_he_f078"
triSimFrames 1
triScreenshot "mun_dusk_he_f079"
triSimFrames 1
triScreenshot "mun_dusk_he_f080"
triSimFrames 1
triScreenshot "mun_dusk_he_f081"
triSimFrames 1
triScreenshot "mun_dusk_he_f082"
triSimFrames 1
triScreenshot "mun_dusk_he_f083"
triSimFrames 1
triScreenshot "mun_dusk_he_f084"
triSimFrames 1
triScreenshot "mun_dusk_he_f085"
triSimFrames 1
triScreenshot "mun_dusk_he_f086"
triSimFrames 1
triScreenshot "mun_dusk_he_f087"
triSimFrames 1
triScreenshot "mun_dusk_he_f088"
triSimFrames 1
triScreenshot "mun_dusk_he_f089"
triSimFrames 1
triScreenshot "mun_dusk_he_f090"
triSimFrames 1
triScreenshot "mun_dusk_he_f091"
triSimFrames 1
triScreenshot "mun_dusk_he_f092"
triSimFrames 1
triScreenshot "mun_dusk_he_f093"
triSimFrames 1
triScreenshot "mun_dusk_he_f094"
triSimFrames 1
triScreenshot "mun_dusk_he_f095"
triSimFrames 1
triScreenshot "mun_dusk_he_f096"
triSimFrames 1
triScreenshot "mun_dusk_he_f097"
triSimFrames 1
triScreenshot "mun_dusk_he_f098"
triSimFrames 90
triSimFrames 1
triScreenshot "mun_dusk_grenade_f001"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f002"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f003"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f004"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f005"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f006"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f007"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f008"
call { [["GrenadeHand", "Grenade"], [22, -2], 1, [0, 0, -2]] call capMunition }
triSimFrames 1
triScreenshot "mun_dusk_grenade_f009"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f010"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f011"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f012"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f013"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f014"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f015"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f016"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f017"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f018"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f019"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f020"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f021"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f022"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f023"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f024"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f025"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f026"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f027"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f028"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f029"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f030"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f031"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f032"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f033"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f034"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f035"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f036"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f037"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f038"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f039"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f040"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f041"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f042"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f043"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f044"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f045"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f046"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f047"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f048"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f049"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f050"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f051"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f052"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f053"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f054"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f055"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f056"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f057"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f058"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f059"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f060"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f061"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f062"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f063"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f064"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f065"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f066"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f067"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f068"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f069"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f070"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f071"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f072"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f073"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f074"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f075"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f076"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f077"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f078"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f079"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f080"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f081"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f082"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f083"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f084"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f085"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f086"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f087"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f088"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f089"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f090"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f091"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f092"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f093"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f094"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f095"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f096"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f097"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f098"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f099"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f100"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f101"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f102"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f103"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f104"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f105"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f106"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f107"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f108"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f109"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f110"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f111"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f112"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f113"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f114"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f115"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f116"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f117"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f118"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f119"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f120"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f121"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f122"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f123"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f124"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f125"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f126"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f127"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f128"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f129"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f130"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f131"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f132"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f133"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f134"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f135"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f136"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f137"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f138"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f139"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f140"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f141"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f142"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f143"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f144"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f145"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f146"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f147"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f148"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f149"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f150"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f151"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f152"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f153"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f154"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f155"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f156"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f157"
triSimFrames 1
triScreenshot "mun_dusk_grenade_f158"
triSimFrames 90
triSimFrames 1
triScreenshot "mun_dusk_heat_f001"
triSimFrames 1
triScreenshot "mun_dusk_heat_f002"
triSimFrames 1
triScreenshot "mun_dusk_heat_f003"
triSimFrames 1
triScreenshot "mun_dusk_heat_f004"
triSimFrames 1
triScreenshot "mun_dusk_heat_f005"
triSimFrames 1
triScreenshot "mun_dusk_heat_f006"
triSimFrames 1
triScreenshot "mun_dusk_heat_f007"
triSimFrames 1
triScreenshot "mun_dusk_heat_f008"
call { [["Heat120", "Heat125", "Heat105"], [40, 0], 40, [0, 0, -250]] call capMunition }
triSimFrames 1
triScreenshot "mun_dusk_heat_f009"
triSimFrames 1
triScreenshot "mun_dusk_heat_f010"
triSimFrames 1
triScreenshot "mun_dusk_heat_f011"
triSimFrames 1
triScreenshot "mun_dusk_heat_f012"
triSimFrames 1
triScreenshot "mun_dusk_heat_f013"
triSimFrames 1
triScreenshot "mun_dusk_heat_f014"
triSimFrames 1
triScreenshot "mun_dusk_heat_f015"
triSimFrames 1
triScreenshot "mun_dusk_heat_f016"
triSimFrames 1
triScreenshot "mun_dusk_heat_f017"
triSimFrames 1
triScreenshot "mun_dusk_heat_f018"
triSimFrames 1
triScreenshot "mun_dusk_heat_f019"
triSimFrames 1
triScreenshot "mun_dusk_heat_f020"
triSimFrames 1
triScreenshot "mun_dusk_heat_f021"
triSimFrames 1
triScreenshot "mun_dusk_heat_f022"
triSimFrames 1
triScreenshot "mun_dusk_heat_f023"
triSimFrames 1
triScreenshot "mun_dusk_heat_f024"
triSimFrames 1
triScreenshot "mun_dusk_heat_f025"
triSimFrames 1
triScreenshot "mun_dusk_heat_f026"
triSimFrames 1
triScreenshot "mun_dusk_heat_f027"
triSimFrames 1
triScreenshot "mun_dusk_heat_f028"
triSimFrames 1
triScreenshot "mun_dusk_heat_f029"
triSimFrames 1
triScreenshot "mun_dusk_heat_f030"
triSimFrames 1
triScreenshot "mun_dusk_heat_f031"
triSimFrames 1
triScreenshot "mun_dusk_heat_f032"
triSimFrames 1
triScreenshot "mun_dusk_heat_f033"
triSimFrames 1
triScreenshot "mun_dusk_heat_f034"
triSimFrames 1
triScreenshot "mun_dusk_heat_f035"
triSimFrames 1
triScreenshot "mun_dusk_heat_f036"
triSimFrames 1
triScreenshot "mun_dusk_heat_f037"
triSimFrames 1
triScreenshot "mun_dusk_heat_f038"
triSimFrames 1
triScreenshot "mun_dusk_heat_f039"
triSimFrames 1
triScreenshot "mun_dusk_heat_f040"
triSimFrames 1
triScreenshot "mun_dusk_heat_f041"
triSimFrames 1
triScreenshot "mun_dusk_heat_f042"
triSimFrames 1
triScreenshot "mun_dusk_heat_f043"
triSimFrames 1
triScreenshot "mun_dusk_heat_f044"
triSimFrames 1
triScreenshot "mun_dusk_heat_f045"
triSimFrames 1
triScreenshot "mun_dusk_heat_f046"
triSimFrames 1
triScreenshot "mun_dusk_heat_f047"
triSimFrames 1
triScreenshot "mun_dusk_heat_f048"
triSimFrames 1
triScreenshot "mun_dusk_heat_f049"
triSimFrames 1
triScreenshot "mun_dusk_heat_f050"
triSimFrames 1
triScreenshot "mun_dusk_heat_f051"
triSimFrames 1
triScreenshot "mun_dusk_heat_f052"
triSimFrames 1
triScreenshot "mun_dusk_heat_f053"
triSimFrames 1
triScreenshot "mun_dusk_heat_f054"
triSimFrames 1
triScreenshot "mun_dusk_heat_f055"
triSimFrames 1
triScreenshot "mun_dusk_heat_f056"
triSimFrames 1
triScreenshot "mun_dusk_heat_f057"
triSimFrames 1
triScreenshot "mun_dusk_heat_f058"
triSimFrames 1
triScreenshot "mun_dusk_heat_f059"
triSimFrames 1
triScreenshot "mun_dusk_heat_f060"
triSimFrames 1
triScreenshot "mun_dusk_heat_f061"
triSimFrames 1
triScreenshot "mun_dusk_heat_f062"
triSimFrames 1
triScreenshot "mun_dusk_heat_f063"
triSimFrames 1
triScreenshot "mun_dusk_heat_f064"
triSimFrames 1
triScreenshot "mun_dusk_heat_f065"
triSimFrames 1
triScreenshot "mun_dusk_heat_f066"
triSimFrames 1
triScreenshot "mun_dusk_heat_f067"
triSimFrames 1
triScreenshot "mun_dusk_heat_f068"
triSimFrames 1
triScreenshot "mun_dusk_heat_f069"
triSimFrames 1
triScreenshot "mun_dusk_heat_f070"
triSimFrames 1
triScreenshot "mun_dusk_heat_f071"
triSimFrames 1
triScreenshot "mun_dusk_heat_f072"
triSimFrames 1
triScreenshot "mun_dusk_heat_f073"
triSimFrames 1
triScreenshot "mun_dusk_heat_f074"
triSimFrames 1
triScreenshot "mun_dusk_heat_f075"
triSimFrames 1
triScreenshot "mun_dusk_heat_f076"
triSimFrames 1
triScreenshot "mun_dusk_heat_f077"
triSimFrames 1
triScreenshot "mun_dusk_heat_f078"
triSimFrames 1
triScreenshot "mun_dusk_heat_f079"
triSimFrames 1
triScreenshot "mun_dusk_heat_f080"
triSimFrames 1
triScreenshot "mun_dusk_heat_f081"
triSimFrames 1
triScreenshot "mun_dusk_heat_f082"
triSimFrames 1
triScreenshot "mun_dusk_heat_f083"
triSimFrames 1
triScreenshot "mun_dusk_heat_f084"
triSimFrames 1
triScreenshot "mun_dusk_heat_f085"
triSimFrames 1
triScreenshot "mun_dusk_heat_f086"
triSimFrames 1
triScreenshot "mun_dusk_heat_f087"
triSimFrames 1
triScreenshot "mun_dusk_heat_f088"
triSimFrames 1
triScreenshot "mun_dusk_heat_f089"
triSimFrames 1
triScreenshot "mun_dusk_heat_f090"
triSimFrames 1
triScreenshot "mun_dusk_heat_f091"
triSimFrames 1
triScreenshot "mun_dusk_heat_f092"
triSimFrames 1
triScreenshot "mun_dusk_heat_f093"
triSimFrames 1
triScreenshot "mun_dusk_heat_f094"
triSimFrames 1
triScreenshot "mun_dusk_heat_f095"
triSimFrames 1
triScreenshot "mun_dusk_heat_f096"
triSimFrames 1
triScreenshot "mun_dusk_heat_f097"
triSimFrames 1
triScreenshot "mun_dusk_heat_f098"
triSimFrames 90
triSimFrames 1
triScreenshot "mun_dusk_fuel_f001"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f002"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f003"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f004"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f005"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f006"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f007"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f008"
call { capFuelV = [["Jeep", "UAZ"], 45, 12, 60, "veh"] call capSpawn; if (not (isNull capFuelV)) then { capFuelV allowDamage true; capFuelV setDammage 1 }; format ["view=munition;event=fuel;veh=%1", capFuelV] call capLog }
triSimFrames 1
triScreenshot "mun_dusk_fuel_f009"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f010"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f011"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f012"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f013"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f014"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f015"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f016"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f017"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f018"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f019"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f020"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f021"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f022"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f023"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f024"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f025"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f026"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f027"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f028"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f029"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f030"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f031"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f032"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f033"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f034"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f035"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f036"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f037"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f038"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f039"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f040"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f041"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f042"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f043"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f044"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f045"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f046"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f047"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f048"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f049"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f050"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f051"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f052"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f053"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f054"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f055"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f056"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f057"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f058"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f059"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f060"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f061"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f062"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f063"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f064"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f065"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f066"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f067"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f068"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f069"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f070"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f071"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f072"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f073"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f074"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f075"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f076"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f077"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f078"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f079"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f080"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f081"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f082"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f083"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f084"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f085"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f086"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f087"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f088"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f089"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f090"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f091"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f092"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f093"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f094"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f095"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f096"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f097"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f098"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f099"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f100"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f101"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f102"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f103"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f104"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f105"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f106"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f107"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f108"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f109"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f110"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f111"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f112"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f113"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f114"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f115"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f116"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f117"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f118"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f119"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f120"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f121"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f122"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f123"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f124"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f125"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f126"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f127"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f128"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f129"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f130"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f131"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f132"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f133"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f134"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f135"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f136"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f137"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f138"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f139"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f140"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f141"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f142"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f143"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f144"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f145"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f146"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f147"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f148"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f149"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f150"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f151"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f152"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f153"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f154"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f155"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f156"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f157"
triSimFrames 1
triScreenshot "mun_dusk_fuel_f158"
triSimFrames 90
0 call capFixStep
[] call capFFClear
call { "view=done;event=done" call capLog }
triEndTest
