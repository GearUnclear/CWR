// ============================================================================
//  Dedushka video, the church: he walks through the church of Morton
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
["wake"] exec "kjd_beat.sqs"
kvTw = time
triSimUntil { ("wake" in KJD_done) || (time > kvTw + 90) }
KJ_brainOrder = ["drawn", [5104, 3901, 0]]
kvTw = time
triSimUntil { ((sqrt ((((getPos kaiju) select 0) - 5057) ^ 2 + (((getPos kaiju) select 1) - 3925) ^ 2)) < 58) || (time > kvTw + 200) }
logInfo format ["KV church: he is at %1, state %2", getPos kaiju, KJ_state]
kvE1 = [5097, 3888, 1.7] call kvP
kvA1 = [5057, 3925, 20] call kvP
kvFv1 = 0.32
logInfo format ["KVC church-se eye %1 aim %2 fov %3 alpha 1", kvE1, kvA1, kvFv1]
kvE2 = [5062, 3862, 1.7] call kvP
kvA2 = [5057, 3925, 20] call kvP
kvFv2 = 0.3
logInfo format ["KVC church-south eye %1 aim %2 fov %3 alpha 1", kvE2, kvA2, kvFv2]
kvE3 = [4990, 3900, 1.7] call kvP
kvA3 = [5057, 3925, 18] call kvP
kvFv3 = 0.3
logInfo format ["KVC church-west eye %1 aim %2 fov %3 alpha 1", kvE3, kvA3, kvFv3]
kvFv1 call kvLens
triSimFrames 2
kvEar setPos [kvE1 select 0, kvE1 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 0 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 1 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 2 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 3 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 4 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 5 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 6 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 7 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 8 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 9 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 10 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 11 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 12 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 13 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 14 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 15 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 16 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 17 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 18 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 19 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 20 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 21 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 22 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 23 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 24 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 25 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 26 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 27 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 28 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 29 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 30 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 31 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 32 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 33 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 34 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 35 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 36 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 37 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 38 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 39 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 40 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 41 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 42 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 43 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 44 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 45 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 46 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 47 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 48 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 49 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 50 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 51 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 52 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 53 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 54 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 55 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 56 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 57 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 58 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 59 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 60 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 61 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 62 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 63 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 64 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 65 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 66 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 67 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 68 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 69 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 70 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 71 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 72 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 73 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 74 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 75 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 76 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 77 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 78 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 79 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 80 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 81 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 82 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 83 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 84 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 85 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 86 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 87 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 88 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 89 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 90 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 91 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 92 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 93 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 94 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 95 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 96 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 97 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 98 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 99 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 100 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 101 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 102 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 103 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 104 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 105 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 106 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 107 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 108 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 109 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 110 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 111 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 112 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 113 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 114 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 115 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 116 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 117 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 118 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 119 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 120 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 121 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 122 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 123 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 124 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 125 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 126 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 127 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 128 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 129 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 130 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 131 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 132 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 133 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 134 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 135 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 136 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 137 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 138 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 139 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 140 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 141 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 142 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 143 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 144 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 145 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 146 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 147 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 148 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 149 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 150 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 151 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 152 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 153 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 154 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 155 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 156 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 157 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 158 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 159 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 160 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 161 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 162 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 163 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 164 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 165 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 166 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 167 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 168 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 169 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 170 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 171 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 172 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 173 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 174 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 175 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 176 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 177 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 178 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 179 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 180 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 180 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 180 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 181 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 181 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 181 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 182 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 182 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 182 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 183 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 183 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 183 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 184 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 184 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 184 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 185 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 185 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 185 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 186 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 186 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 186 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 187 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 187 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 187 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 188 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 188 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 188 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 189 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 189 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 189 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 190 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 190 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 190 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 191 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 191 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 191 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 192 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 192 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 192 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 193 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 193 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 193 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 194 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 194 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 194 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 195 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 195 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 195 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 196 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 196 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 196 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 197 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 197 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 197 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 198 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 198 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 198 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 199 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 199 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 199 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 200 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 200 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 200 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 201 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 201 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 201 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 202 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 202 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 202 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 203 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 203 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 203 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 204 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 204 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 204 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 205 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 205 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 205 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 206 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 206 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 206 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 207 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 207 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 207 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 208 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 208 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 208 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 209 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 209 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 209 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 210 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 210 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 210 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 211 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 211 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 211 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 212 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 212 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 212 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 213 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 213 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 213 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 214 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 214 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 214 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 215 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 215 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 215 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 216 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 216 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 216 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 217 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 217 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 217 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 218 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 218 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 218 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 219 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 219 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 219 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 220 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 220 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 220 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 221 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 221 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 221 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 222 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 222 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 222 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 223 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 223 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 223 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 224 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 224 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 224 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 225 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 225 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 225 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 226 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 226 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 226 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 227 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 227 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 227 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 228 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 228 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 228 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 229 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 229 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 229 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 230 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 230 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 230 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 231 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 231 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 231 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 232 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 232 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 232 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 233 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 233 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 233 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 234 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 234 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 234 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 235 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 235 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 235 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 236 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 236 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 236 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 237 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 237 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 237 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 238 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 238 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 238 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 239 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 239 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 239 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 240 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 240 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 240 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 241 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 241 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 241 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 242 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 242 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 242 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 243 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 243 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 243 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 244 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 244 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 244 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 245 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 245 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 245 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 246 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 246 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 246 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 247 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 247 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 247 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 248 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 248 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 248 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 249 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 249 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 249 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 250 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 250 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 250 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 251 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 251 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 251 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 252 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 252 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 252 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 253 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 253 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 253 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 254 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 254 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 254 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 255 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 255 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 255 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 256 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 256 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 256 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 257 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 257 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 257 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 258 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 258 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 258 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 259 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 259 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 259 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 260 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 260 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 260 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 261 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 261 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 261 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 262 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 262 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 262 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 263 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 263 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 263 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 264 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 264 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 264 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 265 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 265 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 265 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 266 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 266 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 266 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 267 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 267 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 267 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 268 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 268 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 268 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 269 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 269 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 269 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 270 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 270 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 270 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 271 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 271 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 271 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 272 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 272 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 272 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 273 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 273 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 273 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 274 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 274 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 274 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 275 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 275 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 275 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 276 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 276 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 276 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 277 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 277 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 277 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 278 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 278 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 278 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 279 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 279 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 279 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 280 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 280 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 280 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 281 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 281 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 281 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 282 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 282 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 282 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 283 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 283 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 283 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 284 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 284 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 284 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 285 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 285 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 285 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 286 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 286 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 286 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 287 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 287 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 287 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 288 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 288 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 288 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 289 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 289 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 289 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 290 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 290 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 290 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 291 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 291 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 291 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 292 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 292 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 292 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 293 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 293 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 293 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 294 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 294 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 294 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 295 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 295 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 295 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 296 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 296 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 296 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 297 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 297 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 297 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 298 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 298 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 298 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 299 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 299 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 299 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 300 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 300 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 300 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 301 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 301 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 301 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 302 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 302 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 302 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 303 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 303 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 303 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 304 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 304 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 304 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 305 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 305 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 305 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 306 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 306 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 306 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 307 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 307 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 307 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 308 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 308 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 308 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 309 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 309 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 309 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 310 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 310 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 310 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 311 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 311 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 311 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 312 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 312 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 312 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 313 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 313 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 313 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 314 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 314 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 314 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 315 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 315 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 315 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 316 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 316 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 316 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 317 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 317 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 317 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 318 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 318 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 318 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 319 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 319 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 319 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 320 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 320 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 320 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 321 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 321 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 321 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 322 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 322 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 322 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 323 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 323 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 323 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 324 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 324 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 324 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 325 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 325 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 325 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 326 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 326 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 326 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 327 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 327 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 327 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 328 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 328 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 328 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 329 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 329 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 329 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 330 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 330 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 330 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 331 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 331 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 331 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 332 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 332 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 332 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 333 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 333 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 333 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 334 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 334 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 334 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 335 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 335 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 335 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 336 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 336 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 336 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 337 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 337 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 337 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 338 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 338 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 338 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 339 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 339 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 339 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 340 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 340 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 340 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 341 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 341 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 341 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 342 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 342 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 342 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 343 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 343 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 343 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 344 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 344 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 344 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 345 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 345 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 345 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 346 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 346 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 346 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 347 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 347 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 347 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 348 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 348 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 348 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 349 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 349 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 349 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 350 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 350 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 350 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 351 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 351 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 351 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 352 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 352 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 352 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 353 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 353 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 353 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 354 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 354 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 354 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 355 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 355 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 355 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 356 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 356 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 356 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 357 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 357 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 357 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 358 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 358 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 358 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 359 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 359 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 359 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 360 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 360 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 360 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 361 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 361 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 361 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 362 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 362 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 362 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 363 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 363 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 363 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 364 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 364 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 364 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 365 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 365 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 365 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 366 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 366 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 366 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 367 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 367 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 367 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 368 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 368 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 368 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 369 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 369 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 369 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 370 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 370 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 370 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 371 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 371 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 371 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 372 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 372 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 372 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 373 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 373 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 373 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 374 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 374 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 374 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 375 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 375 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 375 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 376 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 376 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 376 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 377 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 377 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 377 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 378 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 378 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 378 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 379 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 379 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 379 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 380 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 380 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 380 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 381 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 381 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 381 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 382 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 382 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 382 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 383 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 383 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 383 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 384 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 384 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 384 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 385 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 385 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 385 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 386 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 386 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 386 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 387 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 387 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 387 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 388 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 388 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 388 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 389 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 389 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 389 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 390 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 390 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 390 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 391 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 391 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 391 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 392 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 392 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 392 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 393 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 393 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 393 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 394 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 394 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 394 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 395 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 395 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 395 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 396 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 396 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 396 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 397 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 397 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 397 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 398 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 398 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 398 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 399 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 399 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 399 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 400 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 400 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 400 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 401 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 401 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 401 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 402 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 402 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 402 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 403 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 403 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 403 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 404 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 404 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 404 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 405 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 405 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 405 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 406 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 406 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 406 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 407 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 407 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 407 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 408 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 408 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 408 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 409 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 409 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 409 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 410 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 410 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 410 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 411 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 411 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 411 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 412 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 412 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 412 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 413 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 413 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 413 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 414 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 414 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 414 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 415 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 415 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 415 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 416 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 416 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 416 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 417 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 417 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 417 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 418 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 418 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 418 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 419 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 419 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 419 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 420 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 420 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 420 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 421 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 421 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 421 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 422 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 422 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 422 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 423 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 423 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 423 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 424 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 424 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 424 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 425 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 425 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 425 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 426 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 426 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 426 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 427 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 427 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 427 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 428 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 428 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 428 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 429 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 429 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 429 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 430 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 430 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 430 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 431 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 431 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 431 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 432 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 432 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 432 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 433 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 433 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 433 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 434 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 434 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 434 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 435 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 435 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 435 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 436 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 436 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 436 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 437 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 437 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 437 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 438 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 438 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 438 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 439 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 439 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 439 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 440 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 440 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 440 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 441 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 441 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 441 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 442 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 442 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 442 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 443 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 443 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 443 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 444 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 444 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 444 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 445 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 445 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 445 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 446 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 446 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 446 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 447 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 447 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 447 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 448 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 448 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 448 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 449 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 449 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 449 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 450 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 450 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 450 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 451 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 451 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 451 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 452 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 452 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 452 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 453 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 453 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 453 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 454 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 454 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 454 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 455 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 455 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 455 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 456 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 456 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 456 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 457 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 457 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 457 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 458 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 458 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 458 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 459 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 459 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 459 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 460 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 460 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 460 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 461 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 461 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 461 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 462 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 462 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 462 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 463 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 463 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 463 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 464 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 464 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 464 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 465 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 465 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 465 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 466 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 466 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 466 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 467 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 467 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 467 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 468 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 468 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 468 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 469 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 469 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 469 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 470 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 470 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 470 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 471 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 471 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 471 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 472 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 472 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 472 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 473 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 473 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 473 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 474 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 474 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 474 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 475 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 475 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 475 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 476 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 476 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 476 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 477 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 477 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 477 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 478 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 478 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 478 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 479 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 479 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 479 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 480 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 480 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 480 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 481 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 481 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 481 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 482 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 482 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 482 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 483 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 483 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 483 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 484 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 484 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 484 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 485 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 485 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 485 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 486 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 486 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 486 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 487 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 487 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 487 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 488 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 488 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 488 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 489 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 489 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 489 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 490 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 490 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 490 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 491 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 491 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 491 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 492 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 492 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 492 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 493 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 493 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 493 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 494 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 494 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 494 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 495 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 495 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 495 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 496 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 496 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 496 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 497 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 497 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 497 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 498 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 498 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 498 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 499 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 499 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 499 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 500 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 500 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 500 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 501 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 501 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 501 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 502 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 502 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 502 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 503 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 503 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 503 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 504 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 504 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 504 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 505 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 505 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 505 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 506 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 506 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 506 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 507 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 507 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 507 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 508 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 508 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 508 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 509 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 509 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 509 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 510 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 510 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 510 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 511 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 511 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 511 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 512 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 512 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 512 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 513 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 513 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 513 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 514 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 514 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 514 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 515 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 515 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 515 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 516 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 516 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 516 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 517 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 517 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 517 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 518 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 518 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 518 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 519 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 519 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 519 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 520 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 520 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 520 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 521 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 521 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 521 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 522 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 522 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 522 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 523 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 523 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 523 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 524 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 524 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 524 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 525 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 525 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 525 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 526 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 526 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 526 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 527 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 527 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 527 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 528 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 528 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 528 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 529 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 529 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 529 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 530 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 530 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 530 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 531 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 531 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 531 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 532 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 532 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 532 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 533 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 533 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 533 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 534 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 534 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 534 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 535 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 535 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 535 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 536 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 536 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 536 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 537 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 537 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 537 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 538 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 538 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 538 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 539 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 539 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 539 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 540 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 540 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 540 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 541 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 541 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 541 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 542 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 542 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 542 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 543 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 543 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 543 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 544 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 544 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 544 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 545 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 545 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 545 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 546 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 546 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 546 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 547 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 547 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 547 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 548 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 548 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 548 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 549 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 549 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 549 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 550 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 550 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 550 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 551 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 551 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 551 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 552 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 552 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 552 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 553 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 553 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 553 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 554 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 554 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 554 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 555 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 555 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 555 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 556 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 556 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 556 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 557 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 557 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 557 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 558 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 558 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 558 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 559 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 559 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 559 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 560 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 560 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 560 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 561 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 561 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 561 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 562 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 562 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 562 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 563 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 563 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 563 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 564 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 564 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 564 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 565 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 565 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 565 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 566 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 566 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 566 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 567 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 567 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 567 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 568 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 568 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 568 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 569 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 569 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 569 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 570 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 570 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 570 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 571 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 571 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 571 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 572 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 572 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 572 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 573 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 573 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 573 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 574 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 574 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 574 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 575 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 575 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 575 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 576 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 576 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 576 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 577 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 577 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 577 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 578 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 578 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 578 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 579 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 579 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 579 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 580 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 580 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 580 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 581 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 581 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 581 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 582 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 582 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 582 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 583 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 583 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 583 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 584 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 584 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 584 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 585 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 585 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 585 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 586 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 586 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 586 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 587 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 587 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 587 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 588 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 588 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 588 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 589 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 589 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 589 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 590 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 590 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 590 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 591 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 591 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 591 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 592 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 592 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 592 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 593 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 593 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 593 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 594 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 594 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 594 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 595 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 595 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 595 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 596 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 596 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 596 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 597 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 597 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 597 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 598 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 598 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 598 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 599 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 599 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 599 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 600 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 600 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 600 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 601 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 601 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 601 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 602 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 602 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 602 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 603 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 603 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 603 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 604 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 604 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 604 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 605 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 605 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 605 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 606 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 606 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 606 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 607 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 607 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 607 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 608 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 608 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 608 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 609 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 609 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 609 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 610 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 610 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 610 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 611 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 611 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 611 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 612 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 612 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 612 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 613 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 613 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 613 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 614 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 614 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 614 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 615 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 615 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 615 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 616 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 616 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 616 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 617 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 617 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 617 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 618 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 618 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 618 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 619 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 619 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 619 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 620 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 620 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 620 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 621 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 621 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 621 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 622 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 622 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 622 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 623 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 623 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 623 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 624 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 624 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 624 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 625 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 625 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 625 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 626 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 626 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 626 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 627 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 627 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 627 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 628 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 628 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 628 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 629 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 629 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 629 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 630 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 630 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 630 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 631 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 631 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 631 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 632 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 632 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 632 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 633 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 633 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 633 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 634 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 634 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 634 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 635 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 635 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 635 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 636 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 636 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 636 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 637 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 637 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 637 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 638 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 638 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 638 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 639 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 639 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 639 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 640 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 640 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 640 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 641 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 641 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 641 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 642 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 642 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 642 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 643 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 643 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 643 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 644 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 644 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 644 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 645 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 645 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 645 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 646 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 646 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 646 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 647 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 647 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 647 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 648 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 648 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 648 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 649 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 649 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 649 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 650 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 650 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 650 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 651 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 651 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 651 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 652 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 652 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 652 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 653 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 653 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 653 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 654 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 654 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 654 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 655 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 655 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 655 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 656 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 656 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 656 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 657 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 657 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 657 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 658 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 658 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 658 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 659 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 659 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 659 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 660 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 660 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 660 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 661 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 661 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 661 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 662 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 662 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 662 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 663 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 663 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 663 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 664 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 664 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 664 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 665 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 665 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 665 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 666 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 666 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 666 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 667 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 667 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 667 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 668 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 668 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 668 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 669 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 669 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 669 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 670 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 670 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 670 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 671 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 671 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 671 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 672 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 672 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 672 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 673 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 673 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 673 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 674 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 674 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 674 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 675 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 675 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 675 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 676 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 676 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 676 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 677 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 677 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 677 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 678 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 678 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 678 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 679 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 679 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 679 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 680 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 680 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 680 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 681 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 681 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 681 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 682 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 682 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 682 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 683 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 683 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 683 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 684 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 684 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 684 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 685 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 685 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 685 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 686 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 686 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 686 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 687 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 687 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 687 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 688 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 688 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 688 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 689 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 689 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 689 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 690 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 690 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 690 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 691 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 691 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 691 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 692 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 692 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 692 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 693 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 693 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 693 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 694 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 694 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 694 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 695 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 695 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 695 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 696 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 696 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 696 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 697 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 697 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 697 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 698 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 698 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 698 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 699 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 699 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 699 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 700 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 700 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 700 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 701 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 701 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 701 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 702 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 702 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 702 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 703 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 703 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 703 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 704 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 704 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 704 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 705 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 705 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 705 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 706 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 706 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 706 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 707 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 707 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 707 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 708 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 708 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 708 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 709 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 709 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 709 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 710 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 710 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 710 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 711 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 711 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 711 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 712 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 712 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 712 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 713 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 713 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 713 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 714 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 714 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 714 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 715 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 715 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 715 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 716 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 716 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 716 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 717 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 717 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 717 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 718 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 718 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 718 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 719 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 719 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 719 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 720 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 720 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 720 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 721 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 721 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 721 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 722 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 722 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 722 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 723 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 723 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 723 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 724 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 724 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 724 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 725 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 725 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 725 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 726 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 726 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 726 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 727 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 727 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 727 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 728 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 728 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 728 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 729 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 729 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 729 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 730 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 730 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 730 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 731 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 731 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 731 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 732 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 732 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 732 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 733 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 733 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 733 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 734 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 734 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 734 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 735 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 735 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 735 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 736 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 736 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 736 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 737 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 737 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 737 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 738 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 738 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 738 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 739 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 739 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 739 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 740 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 740 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 740 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 741 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 741 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 741 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 742 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 742 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 742 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 743 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 743 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 743 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 744 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 744 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 744 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 745 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 745 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 745 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 746 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 746 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 746 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 747 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 747 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 747 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 748 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 748 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 748 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 749 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 749 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 749 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 750 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 750 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 750 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 751 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 751 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 751 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 752 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 752 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 752 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 753 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 753 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 753 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 754 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 754 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 754 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 755 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 755 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 755 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 756 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 756 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 756 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 757 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 757 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 757 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 758 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 758 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 758 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 759 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 759 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 759 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 760 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 760 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 760 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 761 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 761 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 761 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 762 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 762 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 762 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 763 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 763 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 763 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 764 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 764 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 764 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 765 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 765 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 765 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 766 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 766 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 766 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 767 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 767 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 767 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 768 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 768 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 768 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 769 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 769 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 769 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 770 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 770 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 770 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 771 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 771 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 771 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 772 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 772 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 772 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 773 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 773 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 773 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 774 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 774 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 774 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 775 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 775 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 775 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 776 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 776 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 776 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 777 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 777 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 777 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 778 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 778 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 778 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF church 779 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG church 779 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX church 779 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = [5057, 3925, 20] call kvP
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "church-se"
kvA2 = [5057, 3925, 20] call kvP
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "church-south"
kvA3 = [5057, 3925, 18] call kvP
kvFv3 call kvLens
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "church-west"
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
