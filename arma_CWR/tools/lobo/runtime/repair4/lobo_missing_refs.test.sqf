// ============================================================================
//  LoBo #14 runtime probe (repair 2026-10-11.4, installed @LoBo only). R2.
//
//  Repair .4 repoints references that named files no pbo ships:
//    - LoBoMig  soundLandCrash \LoBoMig\soundss\jet_crash.wss -> sounds\
//    - LoBoTer  Hilux and Hilux MG, LOD 3.0 and 5.65: black_matte.pac -> "black matte.pac"
//    - LoBo_Pilot lobo_epilot.p3d LOD 1.0 and 5.0: EAF\tex\back.pac, legsbk.pac -> .paa
//    - LoBo_Pilot wounds[]: EAF, RJAF back/legsbk and IAF\tex2 legsbk -> .paa
//    - LoBo_airammo lobo_aim9.p3d Geometry LOD: rktf5\swtex.paa -> LoBo_airammo\tex\sw.pac
//  The F-16 backfuse.pac face is known missing and stays (one log line per
//  F-16 variant is expected).
//
//  The objects are spawned from the script after the boot (body in
//  lobo_missing_refs_body.sqf) and photographed. Pass criteria are in the log:
//  run repair4/check_logs.py r2 on the output folder. The assertions here only
//  prove each class spawned. Wound shots raise per-part hits to 0.8, because
//  setDammage below 0.7 shows no wound texture.
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
logInfo format ["LOBO_R4_REFS_START lodBias=%1", triGetObjectLodBias]
#include "lobo_missing_refs_body.sqf"
triSimFrames 30
triEndTest
