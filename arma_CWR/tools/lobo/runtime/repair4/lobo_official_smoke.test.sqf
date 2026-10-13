// ============================================================================
//  Official Remaster exe smoke test of the rebuilt LoBo family package
//  (repair 2026-10-11.4 runtime plan R5, step S2). Runs the official 3.05
//  PoseidonGame.exe in a throwaway shadow of the Remaster folder with
//  @LoBo_Extras;@LoBo;@LoBo_Deps copied next to it (never @LoBo_UD).
//  Content, per SPEC 10 R5:
//    1. R1 case G: IL-28, player in the gunner seat, no pilot, on the ground.
//       Before .4 tailgun.sqs selected index -1 here ("Zero divisor").
//    2. The R2 spawns and screenshots (lobo_missing_refs_body.sqf).
//    3. A LoBo_Ter_Toy_R killed with setDammage 1 must create exactly one
//       M1Smoker (the classic-profile ground wreck script in the package).
//  Official-exe safe: every command here and in lobo_missing_refs_body.sqf
//  is registered by the official 3.05 build (checked against the upstream
//  3.05 source, commit ffc6183 in this repository, 2026-10-12). It lacks
//  nearestObjects (UD only), so the turret and the smokers are found with
//  NearestObjectDistance [position, "exactClass", radius], which both engines
//  have and which matches the class name exactly, like the typeOf filter the
//  UD probes use. The toml passes no --dev: the release exe refuses it
//  (exit 2) and both engines register the tri* commands for a harness or
//  test-mission run without it.
//  Asserts compare strings or numbers only. The 3.05 triAssert family reads
//  a bool as "" (UD fixed that later): triAssert [bool] always fails there
//  and triAssertEq [bool, bool] always passes. Bools go through
//  format ["%1", ...] and are compared with "true" / "false".
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1500
showCinemaBorder false
player allowDammage false
player setCaptive true
rfView = {_o = _this select 0; _b = _this select 1; _d = _this select 2; _h = _this select 3; _t = _this select 4; _p = getPosASL _o; [(_p select 0) + _d * (sin _b), (_p select 2) + _h, (_p select 1) + _d * (cos _b), 0 - (sin _b), (_t - _h) / _d, 0 - (cos _b)]}
rfHold = {_u = _this select 0; _u setCaptive true; _u disableAI "MOVE"; _u disableAI "TARGET"; _u disableAI "AUTOTARGET"; _u stop true; _u setDir 0}
rfWound = {_u = _this select 0; _u setDammage 0.5; _u setSelectionDamage ["telo", 0.8]; _u setSelectionDamage ["ruce", 0.8]; _u setSelectionDamage ["nohy", 0.8]; _u setSelectionDamage ["hlava", 0.85]}
rfHits = {_u = _this select 0; [_u getSelectionDamage "hlava", _u getSelectionDamage "telo", _u getSelectionDamage "ruce", _u getSelectionDamage "nohy"]}
tgTurretOf = {NearestObjectDistance [getPos _this, "LoBo_rearturret", 60]}
// [position, "exactClass"] call wsCount: objects of that class within 30 m.
// NearestObjectDistance returns one object, so each one found is parked 5 km
// away and the next query runs; all go back to where they were before the
// call returns, in the same frame (setPos moves an object at once). The loop
// stops at an object it already holds (one setPos could not move) or at 10.
wsCount = {_p = _this select 0; _c = _this select 1; _objs = []; _poss = []; _o = NearestObjectDistance [_p, _c, 30]; while {!(isNull _o) && !(_o in _objs) && (count _objs) < 10} do {_objs = _objs + [_o]; _poss = _poss + [getPos _o]; _o setPos [(getPos _o select 0) + 5000, getPos _o select 1, 0]; _o = NearestObjectDistance [_p, _c, 30]}; _i = 0; {_x setPos (_poss select _i); _i = _i + 1} forEach _objs; count _objs}
logInfo "LOBO_R4_SMOKE_START"

// --- 1. IL-28 case G: player gunner, no pilot, ground (same as R1) ---
call {tgPlane = "LoBo_IL28_egy" createVehicle [8000, 9620, 0]; player moveInGunner tgPlane; tgT = time}
triAssertEq [typeOf tgPlane, "LoBo_IL28_egy"]
triSimUntil { time > tgT + 5 }
tgTurret = tgPlane call tgTurretOf
logInfo format ["LOBO_R4_TG case=G inPlane=%1 isGunner=%2 driverNull=%3 turret=%4 gunnerNull=%5 alt=%6", vehicle player == tgPlane, gunner tgPlane == player, isNull driver tgPlane, typeOf tgTurret, isNull gunner tgTurret, getPos tgPlane select 2]
triAssertEq [format ["%1", vehicle player == tgPlane], "true"]
triAssertEq [typeOf tgTurret, "LoBo_rearturret"]
triAssertEq [format ["%1", isNull gunner tgTurret], "true"]
triAssertLt [getPos tgPlane select 2, 20]
tgP = getPosASL tgPlane
triSetView [(tgP select 0) - 9, (tgP select 2) + 3.5, (tgP select 1) - 16, 9, -2, 10]
triSimFrames 5
triScreenshot "g_player_gunner_no_pilot"
triClearView
call {player action ["GETOUT", vehicle player]}
triSimUntil { vehicle player == player }
call {deleteVehicle tgTurret; deleteVehicle tgPlane; player setPos [7985, 9450, 0]}
triSimFrames 10

// --- 2. R2 spawns and screenshots ---
#include "lobo_missing_refs_body.sqf"

// --- 3. Toyota wreck: exactly one M1Smoker ---
wsWreck = "LoBo_Ter_Toy_R" createVehicle [8000, 9900, 0]
triAssertEq [typeOf wsWreck, "LoBo_Ter_Toy_R"]
triSimFrames 10
triAssertEq [[getPos wsWreck, "M1Smoker"] call wsCount, 0]
wsWreck setDammage 1
triSimUntil { ([getPos wsWreck, "M1Smoker"] call wsCount) >= 1 }
triSimFrames 120
logInfo format ["LOBO_R4_SMOKE_SMOKER wreck=%1 alive=%2 smokers=%3", typeOf wsWreck, alive wsWreck, [getPos wsWreck, "M1Smoker"] call wsCount]
triAssertEq [format ["%1", alive wsWreck], "false"]
triAssertEq [[getPos wsWreck, "M1Smoker"] call wsCount, 1]
wsP = getPosASL wsWreck
triSetView [(wsP select 0) - 9, (wsP select 2) + 3, (wsP select 1) - 9, 9, -2.5, 9]
triSimFrames 5
triScreenshot "toyota_wreck_smoker"
triClearView
logInfo "LOBO_R4_SMOKE_DONE"
triSimFrames 30
triEndTest
