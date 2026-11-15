// ============================================================================
//  Dedushka video, the night take: the far furnace, the wake and the walk in the dark
//  Written by gen_shoot.py (edit that, not this file). --nosound.
// ============================================================================
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
triSimUntil { !(isNil "KJ_ready") }
triSimUntil { KJ_ready }
if (true) then { KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }
kjCamObj = "camera" camCreate [5000, 3980, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
if (isNil "KJ_beamPhase") then { KJ_beamPhase = "" }
if (isNil "KJ_deathPhase") then { KJ_deathPhase = "" }
if (isNil "KJD_fightShots") then { KJD_fightShots = 0 }
if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1 }
if (isNil "KJ_woundStage") then { KJ_woundStage = 0 }
if (isNil "KJ_roarCount") then { KJ_roarCount = 0 }
if (isNil "KJ_brainBay") then { KJ_brainBay = false }
if (isNil "KJ_dead") then { KJ_dead = false }
kvGnd = { KJD_probe setPos [_this select 0, _this select 1, 0]; (getPosASL KJD_probe) select 2 }
kvMix = { private ["_a", "_b", "_k"]; _a = _this select 0; _b = _this select 1; _k = _this select 2; [(_a select 0) + _k * ((_b select 0) - (_a select 0)), (_a select 1) + _k * ((_b select 1) - (_a select 1)), (_a select 2) + _k * ((_b select 2) - (_a select 2))] }
kvLook = { private ["_e", "_a"]; _e = _this select 0; _a = _this select 1; [_e select 0, _e select 1, _e select 2, (_a select 0) - (_e select 0), (_a select 1) - (_e select 1), (_a select 2) - (_e select 2)] }
kvK = { private ["_p"]; _p = getPosASL kaiju; [(_p select 0) + (_this select 0), (_p select 2) + (_this select 2), (_p select 1) + (_this select 1)] }
kvF = { private ["_f", "_r"]; _r = [0, 0, 27] call kvK; if (KJ_native) then { _f = kjPoint [kaiju, KJ_furnacePoint]; if ((count _f) == 3) then { _r = [_f select 0, _f select 2, _f select 1] } }; _r }
kvP = { [_this select 0, ([_this select 0, _this select 1] call kvGnd) + (_this select 2), _this select 1] }
kvRel = { private ["_p", "_b", "_x", "_y"]; _p = getPosASL kaiju; _b = (getDir kaiju) + (_this select 0); _x = (_p select 0) + (_this select 1) * (sin _b); _y = (_p select 1) + (_this select 1) * (cos _b); [_x, ([_x, _y] call kvGnd) + (_this select 2), _y] }
kvBehind = { private ["_v", "_p", "_k", "_dx", "_dy", "_d", "_x", "_y"]; _v = _this select 0; _p = getPosASL _v; _k = getPosASL kaiju; _dx = (_p select 0) - (_k select 0); _dy = (_p select 1) - (_k select 1); _d = sqrt (_dx * _dx + _dy * _dy); if (_d < 1) then { _d = 1 }; _x = (_p select 0) + (_this select 1) * _dx / _d + (_this select 3) * _dy / _d; _y = (_p select 1) + (_this select 1) * _dy / _d - (_this select 3) * _dx / _d; [_x, ([_x, _y] call kvGnd) + (_this select 2), _y] }
kvPick = { private ["_best", "_bd", "_d"]; _best = objNull; _bd = 1e9; if (!(isNil "KJD_force")) then { { if (alive _x) then { _d = _x distance kaiju; if ((_d > (_this select 0)) && (_d < (_this select 1)) && (_d < _bd)) then { _bd = _d; _best = _x } } } forEach KJD_force }; _best }
kvFovFor = { private ["_e", "_k", "_d", "_f"]; _e = _this select 0; _k = getPosASL kaiju; _d = sqrt (((_e select 0) - (_k select 0)) ^ 2 + ((_e select 2) - (_k select 1)) ^ 2); _f = (_this select 1) / (2 * 1.778 * _d); if (_f < 0.02) then { _f = 0.02 }; if (_f > 0.6) then { _f = 0.6 }; _f }
kvEar = "HeliHEmpty" createVehicle [5000, 3984, 0]
KJ_listener = kvEar
kvLens = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov _this; kjCamObj camCommit 0 }
logInfo format ["KV start: creature %1, native %2, hour %3, start %4, params %5", typeOf kaiju, KJ_native, daytime, getPos kaiju, KJD_params]
0.5 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 8
triScreenshot "000_warmup_discard"
kvE1 = [4790, 4, -996]
kvA1 = call kvF
kvFv1 = 0.012
logInfo format ["KVC nfar-boat5 eye %1 aim %2 fov %3 alpha 1", kvE1, kvA1, kvFv1]
kvE2 = [4790, 4, -996]
kvA2 = call kvF
kvFv2 = 0.12
logInfo format ["KVC nfar-boat5w eye %1 aim %2 fov %3 alpha 1", kvE2, kvA2, kvFv2]
kvE3 = [4790, 4, 1704]
kvA3 = call kvF
kvFv3 = 0.014
logInfo format ["KVC nfar-boat2 eye %1 aim %2 fov %3 alpha 1", kvE3, kvA3, kvFv3]
kvFv1 call kvLens
triSimFrames 2
kvEar setPos [kvE1 select 0, kvE1 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nfar 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nfar 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "nfar-boat5"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "nfar-boat5w"
kvA3 = call kvF
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "nfar-boat2"
setAccTime 1
kvE4 = [4985, 3984, 1.7] call kvP
kvA4 = [0, 0, 22] call kvK
kvFv4 = 0.15
logInfo format ["KVC nwake-street eye %1 aim %2 fov %3 alpha 1", kvE4, kvA4, kvFv4]
kvFv4 call kvLens
triSimFrames 2
kvEar setPos [kvE4 select 0, kvE4 select 2, 0]
call { ["wake"] exec "kjd_beat.sqs"; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 180 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 180 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 181 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 181 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 182 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 182 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 183 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 183 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 184 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 184 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 185 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 185 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 186 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 186 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 187 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 187 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 188 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 188 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 189 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 189 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 190 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 190 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 191 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 191 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 192 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 192 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 193 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 193 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 194 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 194 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 195 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 195 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 196 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 196 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 197 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 197 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 198 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 198 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 199 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 199 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 200 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 200 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 201 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 201 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 202 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 202 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 203 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 203 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 204 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 204 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 205 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 205 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 206 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 206 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 207 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 207 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 208 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 208 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 209 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 209 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 210 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 210 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 211 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 211 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 212 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 212 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 213 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 213 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 214 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 214 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 215 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 215 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 216 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 216 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 217 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 217 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 218 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 218 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 219 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 219 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 220 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 220 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 221 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 221 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 222 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 222 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 223 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 223 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 224 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 224 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 225 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 225 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 226 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 226 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 227 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 227 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 228 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 228 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 229 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 229 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 230 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 230 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 231 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 231 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 232 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 232 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 233 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 233 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 234 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 234 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 235 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 235 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 236 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 236 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 237 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 237 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 238 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 238 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 239 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 239 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 240 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 240 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 241 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 241 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 242 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 242 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 243 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 243 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 244 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 244 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 245 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 245 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 246 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 246 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 247 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 247 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 248 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 248 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 249 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 249 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 250 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 250 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 251 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 251 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 252 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 252 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 253 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 253 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 254 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 254 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 255 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 255 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 256 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 256 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 257 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 257 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 258 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 258 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 259 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 259 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 260 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 260 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 261 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 261 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 262 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 262 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 263 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 263 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 264 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 264 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 265 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 265 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 266 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 266 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 267 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 267 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 268 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 268 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 269 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 269 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 270 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 270 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 271 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 271 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 272 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 272 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 273 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 273 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 274 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 274 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 275 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 275 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 276 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 276 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 277 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 277 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 278 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 278 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 279 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 279 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 280 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 280 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 281 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 281 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 282 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 282 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 283 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 283 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 284 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 284 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 285 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 285 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 286 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 286 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 287 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 287 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 288 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 288 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 289 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 289 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 290 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 290 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 291 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 291 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 292 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 292 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 293 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 293 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 294 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 294 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 295 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 295 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 296 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 296 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 297 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 297 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 298 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 298 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 299 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 299 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 300 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 300 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 301 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 301 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 302 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 302 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 303 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 303 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 304 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 304 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 305 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 305 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 306 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 306 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 307 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 307 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 308 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 308 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 309 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 309 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 310 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 310 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 311 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 311 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 312 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 312 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 313 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 313 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 314 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 314 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 315 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 315 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 316 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 316 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 317 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 317 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 318 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 318 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 319 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 319 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 320 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 320 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 321 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 321 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 322 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 322 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 323 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 323 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 324 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 324 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 325 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 325 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 326 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 326 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 327 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 327 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 328 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 328 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 329 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 329 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 330 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 330 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 331 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 331 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 332 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 332 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 333 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 333 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 334 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 334 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 335 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 335 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 336 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 336 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 337 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 337 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 338 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 338 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 339 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 339 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 340 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 340 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 341 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 341 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 342 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 342 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 343 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 343 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 344 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 344 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 345 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 345 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 346 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 346 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 347 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 347 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 348 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 348 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 349 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 349 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 350 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 350 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 351 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 351 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 352 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 352 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 353 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 353 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 354 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 354 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 355 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 355 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 356 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 356 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 357 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 357 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 358 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 358 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 359 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 359 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 360 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 360 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 361 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 361 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 362 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 362 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 363 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 363 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 364 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 364 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 365 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 365 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 366 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 366 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 367 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 367 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 368 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 368 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 369 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 369 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 370 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 370 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 371 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 371 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 372 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 372 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 373 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 373 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 374 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 374 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 375 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 375 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 376 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 376 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 377 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 377 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 378 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 378 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 379 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 379 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 380 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 380 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 381 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 381 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 382 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 382 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 383 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 383 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 384 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 384 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 385 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 385 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 386 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 386 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 387 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 387 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 388 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 388 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 389 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 389 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 390 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 390 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 391 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 391 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 392 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 392 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 393 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 393 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 394 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 394 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 395 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 395 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 396 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 396 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 397 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 397 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 398 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 398 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 399 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 399 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 400 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 400 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 401 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 401 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 402 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 402 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 403 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 403 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 404 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 404 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 405 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 405 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 406 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 406 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 407 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 407 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 408 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 408 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 409 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 409 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 410 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 410 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 411 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 411 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 412 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 412 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 413 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 413 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 414 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 414 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 415 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 415 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 416 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 416 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 417 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 417 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 418 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 418 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 419 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 419 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 420 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 420 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 421 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 421 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 422 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 422 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 423 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 423 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 424 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 424 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 425 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 425 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 426 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 426 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 427 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 427 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 428 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 428 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 429 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 429 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 430 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 430 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 431 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 431 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 432 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 432 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 433 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 433 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 434 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 434 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 435 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 435 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 436 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 436 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 437 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 437 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 438 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 438 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 439 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 439 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 440 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 440 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 441 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 441 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 442 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 442 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 443 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 443 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 444 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 444 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 445 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 445 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 446 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 446 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 447 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 447 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 448 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 448 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 449 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 449 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 450 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 450 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 451 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 451 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 452 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 452 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 453 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 453 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 454 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 454 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 455 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 455 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 456 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 456 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 457 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 457 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 458 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 458 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 459 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 459 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 460 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 460 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 461 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 461 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 462 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 462 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 463 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 463 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 464 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 464 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 465 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 465 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 466 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 466 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 467 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 467 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 468 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 468 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 469 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 469 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 470 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 470 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 471 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 471 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 472 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 472 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 473 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 473 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 474 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 474 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 475 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 475 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 476 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 476 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 477 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 477 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 478 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 478 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 479 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 479 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 480 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 480 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 481 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 481 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 482 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 482 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 483 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 483 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 484 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 484 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 485 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 485 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 486 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 486 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 487 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 487 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 488 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 488 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 489 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 489 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 490 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 490 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 491 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 491 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 492 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 492 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 493 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 493 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 494 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 494 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 495 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 495 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 496 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 496 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 497 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 497 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 498 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 498 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 499 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 499 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 500 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 500 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 501 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 501 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 502 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 502 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 503 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 503 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 504 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 504 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 505 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 505 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 506 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 506 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 507 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 507 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 508 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 508 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 509 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 509 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 510 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 510 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 511 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 511 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 512 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 512 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 513 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 513 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 514 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 514 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 515 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 515 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 516 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 516 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 517 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 517 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 518 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 518 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwake 519 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwake 519 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA4 = [0, 0, 22] call kvK
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "nwake-street"
setAccTime 1
kvTw = time
triSimUntil { ("wake" in KJD_done) || (time > kvTw + 60) }
kvE5 = [4790, 4, -996]
kvA5 = call kvF
kvFv5 = 0.012
logInfo format ["KVC nidle-boat5 eye %1 aim %2 fov %3 alpha 1", kvE5, kvA5, kvFv5]
kvE6 = [4790, 4, -996]
kvA6 = call kvF
kvFv6 = 0.12
logInfo format ["KVC nidle-boat5w eye %1 aim %2 fov %3 alpha 1", kvE6, kvA6, kvFv6]
kvE7 = [4790, 4, 1704]
kvA7 = call kvF
kvFv7 = 0.014
logInfo format ["KVC nidle-boat2 eye %1 aim %2 fov %3 alpha 1", kvE7, kvA7, kvFv7]
kvFv5 call kvLens
triSimFrames 2
kvEar setPos [kvE5 select 0, kvE5 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nidle 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nidle 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA5 = call kvF
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "nidle-boat5"
kvA6 = call kvF
kvFv6 call kvLens
triSetView ([kvE6, kvA6] call kvLook)
triScreenshot "nidle-boat5w"
kvA7 = call kvF
kvFv7 call kvLens
triSetView ([kvE7, kvA7] call kvLook)
triScreenshot "nidle-boat2"
setAccTime 1
["through"] exec "kjd_beat.sqs"
kvTw = time
triSimUntil { (((getPos kaiju) select 0) > 4826) || (time > kvTw + 120) }
kvE8 = [4985, 3984, 1.7] call kvP
kvA8 = [0, 0, 22] call kvK
kvFv8 = 0.2
logInfo format ["KVC nwalk-street eye %1 aim %2 fov %3 alpha 0.1", kvE8, kvA8, kvFv8]
kvE9 = [4790, 4, 1704]
kvA9 = call kvF
kvFv9 = 0.014
logInfo format ["KVC nwalk-boat2 eye %1 aim %2 fov %3 alpha 0.2", kvE9, kvA9, kvFv9]
kvFv8 call kvLens
triSimFrames 2
kvEar setPos [kvE8 select 0, kvE8 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 180 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 180 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 181 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 181 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 182 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 182 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 183 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 183 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 184 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 184 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 185 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 185 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 186 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 186 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 187 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 187 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 188 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 188 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 189 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 189 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 190 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 190 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 191 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 191 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 192 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 192 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 193 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 193 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 194 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 194 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 195 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 195 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 196 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 196 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 197 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 197 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 198 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 198 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 199 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 199 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 200 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 200 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 201 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 201 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 202 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 202 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 203 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 203 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 204 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 204 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 205 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 205 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 206 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 206 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 207 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 207 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 208 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 208 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 209 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 209 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 210 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 210 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 211 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 211 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 212 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 212 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 213 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 213 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 214 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 214 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 215 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 215 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 216 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 216 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 217 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 217 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 218 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 218 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 219 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 219 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 220 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 220 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 221 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 221 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 222 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 222 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 223 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 223 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 224 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 224 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 225 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 225 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 226 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 226 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 227 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 227 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 228 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 228 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 229 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 229 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 230 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 230 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 231 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 231 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 232 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 232 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 233 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 233 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 234 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 234 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 235 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 235 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 236 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 236 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 237 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 237 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 238 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 238 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 239 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 239 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 240 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 240 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 241 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 241 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 242 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 242 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 243 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 243 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 244 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 244 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 245 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 245 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 246 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 246 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 247 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 247 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 248 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 248 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 249 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 249 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 250 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 250 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 251 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 251 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 252 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 252 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 253 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 253 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 254 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 254 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 255 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 255 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 256 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 256 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 257 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 257 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 258 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 258 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 259 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 259 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 260 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 260 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 261 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 261 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 262 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 262 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 263 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 263 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 264 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 264 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 265 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 265 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 266 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 266 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 267 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 267 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 268 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 268 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 269 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 269 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 270 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 270 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 271 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 271 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 272 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 272 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 273 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 273 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 274 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 274 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 275 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 275 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 276 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 276 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 277 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 277 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 278 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 278 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 279 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 279 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 280 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 280 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 281 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 281 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 282 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 282 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 283 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 283 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 284 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 284 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 285 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 285 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 286 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 286 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 287 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 287 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 288 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 288 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 289 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 289 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 290 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 290 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 291 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 291 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 292 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 292 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 293 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 293 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 294 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 294 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 295 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 295 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 296 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 296 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 297 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 297 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 298 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 298 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 299 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 299 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 300 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 300 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 301 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 301 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 302 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 302 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 303 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 303 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 304 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 304 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 305 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 305 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 306 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 306 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 307 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 307 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 308 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 308 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 309 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 309 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 310 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 310 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 311 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 311 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 312 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 312 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 313 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 313 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 314 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 314 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 315 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 315 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 316 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 316 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 317 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 317 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 318 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 318 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 319 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 319 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 320 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 320 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 321 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 321 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 322 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 322 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 323 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 323 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 324 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 324 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 325 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 325 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 326 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 326 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 327 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 327 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 328 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 328 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF nwalk 329 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG nwalk 329 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA8 = [kvA8, [0, 0, 22] call kvK, 0.1] call kvMix
kvFv8 call kvLens
triSetView ([kvE8, kvA8] call kvLook)
triScreenshot "nwalk-street"
kvA9 = [kvA9, call kvF, 0.2] call kvMix
kvFv9 call kvLens
triSetView ([kvE9, kvA9] call kvLook)
triScreenshot "nwalk-boat2"
setAccTime 1
setAccTime 1
logInfo format ["KV end: states %1", KJT_states]
logInfo format ["KV end: moves %1", KJT_moves]
logInfo format ["KV end: jumps (time, metres, seconds, from, to, men, vehicles) %1, top speed %2", KJT_jumps, KJT_maxSpeed]
logInfo "KV END"
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false; KJD_firing = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
triEndTest
