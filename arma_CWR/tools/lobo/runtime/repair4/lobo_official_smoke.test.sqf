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
//  The official exe may lack UD-only commands (triSetView, getPosASL,
//  setSelectionDamage, logInfo); if Trident cannot drive it, use the plain
//  mission fallback in the R5 runbook instead of editing this file.
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
tgTurretOf = {_t = objNull; {if (isNull _t) then {_t = _x}} forEach (nearestObjects [getPos _this, ["LoBo_rearturret"], 60]); _t}
wsCount = {wsN = 0; {if ((typeOf _x) == (_this select 1)) then {wsN = wsN + 1}} forEach (nearestObjects [_this select 0, [], 30]); wsN}
logInfo "LOBO_R4_SMOKE_START"

// --- 1. IL-28 case G: player gunner, no pilot, ground (same as R1) ---
call {tgPlane = "LoBo_IL28_egy" createVehicle [8000, 9620, 0]; player moveInGunner tgPlane; tgT = time}
triAssertEq [typeOf tgPlane, "LoBo_IL28_egy"]
triSimUntil { time > tgT + 5 }
tgTurret = tgPlane call tgTurretOf
logInfo format ["LOBO_R4_TG case=G inPlane=%1 isGunner=%2 driverNull=%3 turret=%4 gunnerNull=%5 alt=%6", vehicle player == tgPlane, gunner tgPlane == player, isNull driver tgPlane, typeOf tgTurret, isNull gunner tgTurret, getPos tgPlane select 2]
triAssertEq [vehicle player == tgPlane, true]
triAssert [not isNull tgTurret]
triAssertEq [isNull gunner tgTurret, true]
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
triAssertEq [alive wsWreck, false]
triAssertEq [[getPos wsWreck, "M1Smoker"] call wsCount, 1]
wsP = getPosASL wsWreck
triSetView [(wsP select 0) - 9, (wsP select 2) + 3, (wsP select 1) - 9, 9, -2.5, 9]
triSimFrames 5
triScreenshot "toyota_wreck_smoker"
triClearView
logInfo "LOBO_R4_SMOKE_DONE"
triSimFrames 30
triEndTest
