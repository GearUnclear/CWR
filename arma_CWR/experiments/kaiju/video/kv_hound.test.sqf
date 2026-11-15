// ============================================================================
//  Dedushka video, the last shot: a second light on the water (the other creature, by night)
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
kvPick = { private ["_best", "_bd", "_d"]; _best = objNull; _bd = 1000000; { if (alive _x) then { _d = _x distance kaiju; if ((_d > (_this select 0)) && (_d < (_this select 1)) && (_d < _bd)) then { _bd = _d; _best = _x } } } forEach (_this select 2); _best }
kvFovFor = { private ["_e", "_k", "_d", "_f"]; _e = _this select 0; _k = getPosASL kaiju; _d = sqrt (((_e select 0) - (_k select 0)) ^ 2 + ((_e select 2) - (_k select 1)) ^ 2); _f = (_this select 1) / (2 * 1.778 * _d); if (_f < 0.02) then { _f = 0.02 }; if (_f > 0.6) then { _f = 0.6 }; _f }
kvEar = "HeliHEmpty" createVehicle [5000, 3984, 0]
KJ_listener = kvEar
kvLens = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov _this; kjCamObj camCommit 0 }
logInfo format ["KV start: creature %1, native %2, hour %3, start %4, params %5", typeOf kaiju, KJ_native, daytime, getPos kaiju, KJD_params]
0.5 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 8
triScreenshot "000_warmup_discard"
logInfo format ["KV hound: at %1 asl %2, depth %3, state %4", getPos kaiju, getPosASL kaiju, call KJB_fnDepth, KJ_state]
kvE1 = [5012, 3856, 1.7] call kvP
kvA1 = call kvF
kvFv1 = 0.1
logInfo format ["KVC hdorm-shelf eye %1 aim %2 fov %3 alpha 1", kvE1, kvA1, kvFv1]
kvE2 = [5012, 3856, 1.7] call kvP
kvA2 = call kvF
kvFv2 = 0.3
logInfo format ["KVC hdorm-wide eye %1 aim %2 fov %3 alpha 1", kvE2, kvA2, kvFv2]
kvFv1 call kvLens
triSimFrames 2
kvEar setPos [kvE1 select 0, kvE1 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 0 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 1 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 2 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 3 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 4 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 5 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 6 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 7 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 8 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 9 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 10 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 11 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 12 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 13 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 14 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 15 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 16 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 17 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 18 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 19 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 20 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 21 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 22 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 23 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 24 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 25 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 26 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 27 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 28 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 29 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 30 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 31 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 32 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 33 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 34 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 35 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 36 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 37 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 38 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 39 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 40 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 41 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 42 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 43 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 44 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 45 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 46 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 47 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 48 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 49 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 50 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 51 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 52 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 53 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 54 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 55 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 56 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 57 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 58 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 59 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 60 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 61 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 62 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 63 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 64 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 65 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 66 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 67 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 68 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 69 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 70 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 71 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 72 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 73 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 74 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 75 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 76 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 77 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 78 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 79 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 80 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 81 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 82 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 83 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 84 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 85 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 86 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 87 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 88 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hdorm 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hdorm 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hdorm 89 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA1 = call kvF
kvFv1 call kvLens
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "hdorm-shelf"
kvA2 = call kvF
kvFv2 call kvLens
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "hdorm-wide"
setAccTime 1
kvE3 = [5012, 3856, 1.7] call kvP
kvA3 = call kvF
kvFv3 = 0.1
logInfo format ["KVC hwake-shelf eye %1 aim %2 fov %3 alpha 0.2", kvE3, kvA3, kvFv3]
kvFv3 call kvLens
triSimFrames 2
kvEar setPos [kvE3 select 0, kvE3 select 2, 0]
call { ["wake"] exec "kjd_beat.sqs"; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 0 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 1 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 2 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 3 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 4 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 5 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 6 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 7 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 8 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 9 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 10 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 11 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 12 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 13 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 14 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 15 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 16 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 17 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 18 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 19 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 20 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 21 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 22 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 23 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 24 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 25 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 26 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 27 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 28 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 29 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 30 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 31 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 32 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 33 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 34 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 35 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 36 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 37 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 38 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 39 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 40 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 41 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 42 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 43 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 44 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 45 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 46 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 47 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 48 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 49 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 50 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 51 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 52 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 53 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 54 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 55 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 56 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 57 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 58 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 59 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 60 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 61 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 62 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 63 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 64 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 65 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 66 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 67 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 68 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 69 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 70 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 71 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 72 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 73 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 74 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 75 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 76 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 77 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 78 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 79 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 80 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 81 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 82 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 83 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 84 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 85 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 86 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 87 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 88 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 89 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 90 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 91 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 92 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 93 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 94 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 95 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 96 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 97 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 98 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 99 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 100 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 101 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 102 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 103 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 104 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 105 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 106 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 107 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 108 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 109 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 110 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 111 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 112 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 113 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 114 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 115 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 116 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 117 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 118 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 119 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 120 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 121 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 122 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 123 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 124 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 125 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 126 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 127 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 128 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 129 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 130 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 131 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 132 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 133 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 134 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 135 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 136 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 137 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 138 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 139 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 140 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 141 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 142 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 143 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 144 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 145 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 146 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 147 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 148 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 149 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 150 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 151 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 152 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 153 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 154 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 155 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 156 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 157 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 158 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 159 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 160 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 161 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 162 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 163 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 164 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 165 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 166 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 167 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 168 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 169 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 170 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 171 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 172 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 173 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 174 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 175 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 176 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 177 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 178 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 179 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 180 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 180 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 180 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 181 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 181 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 181 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 182 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 182 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 182 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 183 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 183 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 183 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 184 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 184 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 184 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 185 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 185 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 185 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 186 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 186 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 186 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 187 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 187 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 187 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 188 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 188 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 188 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 189 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 189 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 189 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 190 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 190 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 190 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 191 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 191 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 191 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 192 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 192 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 192 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 193 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 193 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 193 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 194 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 194 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 194 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 195 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 195 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 195 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 196 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 196 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 196 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 197 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 197 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 197 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 198 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 198 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 198 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 199 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 199 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 199 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 200 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 200 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 200 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 201 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 201 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 201 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 202 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 202 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 202 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 203 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 203 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 203 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 204 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 204 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 204 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 205 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 205 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 205 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 206 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 206 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 206 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 207 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 207 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 207 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 208 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 208 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 208 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 209 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 209 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 209 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 210 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 210 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 210 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 211 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 211 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 211 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 212 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 212 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 212 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 213 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 213 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 213 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 214 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 214 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 214 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 215 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 215 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 215 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 216 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 216 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 216 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 217 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 217 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 217 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 218 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 218 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 218 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 219 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 219 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 219 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 220 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 220 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 220 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 221 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 221 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 221 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 222 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 222 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 222 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 223 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 223 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 223 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 224 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 224 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 224 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 225 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 225 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 225 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 226 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 226 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 226 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 227 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 227 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 227 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 228 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 228 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 228 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 229 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 229 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 229 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 230 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 230 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 230 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 231 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 231 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 231 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 232 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 232 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 232 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 233 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 233 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 233 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 234 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 234 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 234 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 235 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 235 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 235 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 236 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 236 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 236 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 237 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 237 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 237 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 238 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 238 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 238 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 239 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 239 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 239 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 240 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 240 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 240 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 241 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 241 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 241 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 242 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 242 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 242 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 243 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 243 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 243 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 244 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 244 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 244 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 245 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 245 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 245 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 246 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 246 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 246 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 247 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 247 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 247 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 248 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 248 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 248 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 249 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 249 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 249 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 250 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 250 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 250 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 251 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 251 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 251 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 252 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 252 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 252 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 253 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 253 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 253 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 254 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 254 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 254 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 255 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 255 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 255 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 256 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 256 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 256 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 257 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 257 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 257 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 258 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 258 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 258 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 259 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 259 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 259 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 260 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 260 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 260 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 261 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 261 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 261 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 262 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 262 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 262 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 263 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 263 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 263 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 264 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 264 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 264 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 265 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 265 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 265 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 266 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 266 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 266 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 267 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 267 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 267 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 268 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 268 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 268 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 269 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 269 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 269 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 270 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 270 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 270 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 271 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 271 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 271 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 272 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 272 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 272 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 273 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 273 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 273 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 274 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 274 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 274 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 275 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 275 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 275 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 276 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 276 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 276 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 277 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 277 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 277 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 278 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 278 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 278 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 279 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 279 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 279 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 280 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 280 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 280 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 281 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 281 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 281 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 282 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 282 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 282 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 283 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 283 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 283 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 284 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 284 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 284 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 285 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 285 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 285 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 286 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 286 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 286 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 287 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 287 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 287 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 288 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 288 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 288 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 289 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 289 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 289 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 290 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 290 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 290 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 291 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 291 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 291 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 292 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 292 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 292 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 293 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 293 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 293 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 294 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 294 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 294 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 295 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 295 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 295 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 296 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 296 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 296 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 297 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 297 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 297 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 298 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 298 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 298 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 299 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 299 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 299 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 300 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 300 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 300 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 301 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 301 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 301 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 302 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 302 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 302 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 303 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 303 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 303 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 304 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 304 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 304 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 305 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 305 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 305 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 306 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 306 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 306 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 307 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 307 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 307 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 308 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 308 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 308 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 309 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 309 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 309 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 310 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 310 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 310 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 311 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 311 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 311 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 312 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 312 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 312 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 313 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 313 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 313 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 314 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 314 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 314 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 315 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 315 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 315 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 316 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 316 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 316 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 317 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 317 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 317 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 318 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 318 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 318 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 319 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 319 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 319 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 320 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 320 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 320 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 321 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 321 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 321 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 322 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 322 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 322 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 323 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 323 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 323 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 324 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 324 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 324 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 325 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 325 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 325 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 326 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 326 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 326 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 327 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 327 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 327 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 328 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 328 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 328 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 329 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 329 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 329 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 330 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 330 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 330 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 331 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 331 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 331 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 332 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 332 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 332 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 333 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 333 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 333 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 334 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 334 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 334 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 335 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 335 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 335 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 336 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 336 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 336 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 337 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 337 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 337 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 338 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 338 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 338 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 339 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 339 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 339 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 340 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 340 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 340 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 341 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 341 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 341 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 342 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 342 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 342 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 343 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 343 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 343 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 344 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 344 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 344 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 345 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 345 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 345 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 346 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 346 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 346 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 347 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 347 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 347 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 348 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 348 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 348 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 349 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 349 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 349 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 350 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 350 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 350 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 351 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 351 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 351 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 352 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 352 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 352 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 353 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 353 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 353 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 354 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 354 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 354 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 355 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 355 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 355 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 356 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 356 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 356 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 357 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 357 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 357 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 358 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 358 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 358 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 359 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 359 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 359 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 360 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 360 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 360 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 361 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 361 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 361 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 362 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 362 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 362 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 363 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 363 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 363 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 364 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 364 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 364 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 365 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 365 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 365 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 366 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 366 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 366 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 367 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 367 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 367 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 368 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 368 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 368 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 369 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 369 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 369 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 370 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 370 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 370 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 371 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 371 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 371 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 372 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 372 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 372 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 373 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 373 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 373 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 374 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 374 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 374 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 375 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 375 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 375 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 376 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 376 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 376 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 377 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 377 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 377 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 378 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 378 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 378 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 379 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 379 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 379 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 380 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 380 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 380 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 381 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 381 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 381 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 382 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 382 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 382 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 383 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 383 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 383 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 384 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 384 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 384 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 385 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 385 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 385 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 386 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 386 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 386 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 387 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 387 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 387 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 388 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 388 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 388 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 389 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 389 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 389 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 390 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 390 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 390 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 391 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 391 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 391 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 392 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 392 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 392 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 393 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 393 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 393 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 394 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 394 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 394 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 395 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 395 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 395 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 396 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 396 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 396 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 397 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 397 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 397 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 398 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 398 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 398 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 399 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 399 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 399 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 400 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 400 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 400 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 401 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 401 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 401 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 402 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 402 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 402 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 403 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 403 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 403 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 404 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 404 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 404 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 405 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 405 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 405 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 406 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 406 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 406 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 407 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 407 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 407 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 408 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 408 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 408 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 409 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 409 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 409 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 410 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 410 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 410 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 411 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 411 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 411 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 412 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 412 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 412 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 413 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 413 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 413 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 414 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 414 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 414 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 415 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 415 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 415 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 416 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 416 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 416 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 417 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 417 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 417 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 418 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 418 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 418 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 419 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 419 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 419 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 420 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 420 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 420 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 421 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 421 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 421 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 422 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 422 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 422 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 423 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 423 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 423 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 424 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 424 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 424 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 425 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 425 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 425 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 426 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 426 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 426 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 427 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 427 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 427 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 428 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 428 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 428 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 429 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 429 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 429 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 430 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 430 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 430 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 431 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 431 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 431 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 432 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 432 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 432 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 433 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 433 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 433 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 434 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 434 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 434 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 435 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 435 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 435 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 436 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 436 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 436 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 437 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 437 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 437 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 438 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 438 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 438 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 439 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 439 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 439 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 440 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 440 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 440 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 441 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 441 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 441 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 442 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 442 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 442 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 443 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 443 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 443 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 444 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 444 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 444 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 445 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 445 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 445 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 446 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 446 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 446 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 447 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 447 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 447 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 448 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 448 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 448 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 449 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 449 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 449 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 450 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 450 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 450 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 451 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 451 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 451 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 452 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 452 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 452 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 453 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 453 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 453 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 454 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 454 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 454 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 455 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 455 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 455 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 456 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 456 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 456 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 457 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 457 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 457 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 458 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 458 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 458 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 459 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 459 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 459 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 460 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 460 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 460 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 461 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 461 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 461 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 462 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 462 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 462 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 463 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 463 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 463 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 464 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 464 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 464 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 465 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 465 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 465 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 466 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 466 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 466 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 467 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 467 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 467 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 468 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 468 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 468 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 469 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 469 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 469 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 470 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 470 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 470 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 471 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 471 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 471 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 472 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 472 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 472 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 473 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 473 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 473 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 474 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 474 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 474 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 475 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 475 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 475 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 476 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 476 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 476 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 477 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 477 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 477 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 478 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 478 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 478 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 479 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 479 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 479 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 480 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 480 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 480 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 481 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 481 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 481 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 482 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 482 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 482 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 483 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 483 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 483 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 484 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 484 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 484 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 485 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 485 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 485 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 486 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 486 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 486 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 487 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 487 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 487 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 488 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 488 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 488 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 489 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 489 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 489 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 490 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 490 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 490 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 491 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 491 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 491 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 492 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 492 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 492 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 493 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 493 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 493 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 494 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 494 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 494 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 495 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 495 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 495 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 496 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 496 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 496 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 497 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 497 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 497 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 498 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 498 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 498 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 499 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 499 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 499 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 500 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 500 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 500 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 501 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 501 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 501 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 502 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 502 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 502 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 503 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 503 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 503 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 504 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 504 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 504 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 505 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 505 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 505 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 506 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 506 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 506 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 507 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 507 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 507 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 508 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 508 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 508 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 509 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 509 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 509 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 510 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 510 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 510 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 511 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 511 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 511 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 512 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 512 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 512 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 513 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 513 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 513 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 514 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 514 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 514 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 515 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 515 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 515 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 516 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 516 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 516 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 517 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 517 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 517 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 518 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 518 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 518 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwake 519 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwake 519 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwake 519 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA3 = [kvA3, call kvF, 0.2] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "hwake-shelf"
setAccTime 1
kvTw = time
triSimUntil { ("wake" in KJD_done) || (time > kvTw + 60) }
KJ_brainOrder = ["drawn", [5010, 3790, 0]]
triSimFrames 60
kvE4 = [5012, 3856, 1.7] call kvP
kvA4 = call kvF
kvFv4 = 0.1
logInfo format ["KVC hwalk-shelf eye %1 aim %2 fov %3 alpha 0.15", kvE4, kvA4, kvFv4]
kvE5 = [5012, 3856, 1.7] call kvP
kvA5 = call kvF
kvFv5 = 0.3
logInfo format ["KVC hwalk-wide eye %1 aim %2 fov %3 alpha 0.15", kvE5, kvA5, kvFv5]
kvFv4 call kvLens
triSimFrames 2
kvEar setPos [kvE4 select 0, kvE4 select 2, 0]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 0 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 1 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 2 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 3 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 4 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 5 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 6 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 7 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 8 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 9 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 10 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 11 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 12 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 13 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 14 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 15 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 16 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 17 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 18 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 19 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 20 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 21 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 22 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 23 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 24 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 25 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 26 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 27 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 28 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 29 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 30 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 31 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 32 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 33 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 34 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 35 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 36 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 37 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 38 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 39 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 40 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 41 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 42 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 43 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 44 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 45 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 46 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 47 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 48 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 49 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 50 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 51 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 52 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 53 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 54 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 55 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 56 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 57 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 58 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 59 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 60 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 61 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 62 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 63 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 64 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 65 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 66 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 67 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 68 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 69 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 70 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 71 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 72 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 73 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 74 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 75 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 76 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 77 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 78 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 79 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 80 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 81 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 82 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 83 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 84 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 85 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 86 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 87 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 88 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 89 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 90 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 91 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 92 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 93 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 94 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 95 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 96 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 97 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 98 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 99 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 100 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 101 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 102 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 103 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 104 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 105 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 106 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 107 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 108 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 109 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 110 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 111 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 112 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 113 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 114 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 115 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 116 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 117 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 118 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 119 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 120 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 121 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 122 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 123 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 124 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 125 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 126 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 127 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 128 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 129 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 130 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 131 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 132 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 133 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 134 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 135 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 136 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 137 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 138 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 139 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 140 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 141 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 142 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 143 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 144 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 145 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 146 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 147 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 148 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 149 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 150 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 151 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 152 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 153 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 154 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 155 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 156 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 157 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 158 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 159 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 160 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 161 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 162 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 163 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 164 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 165 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 166 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 167 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 168 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 169 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 170 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 171 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 172 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 173 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 174 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 175 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 176 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 177 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 178 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 179 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 180 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 180 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 180 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 181 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 181 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 181 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 182 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 182 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 182 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 183 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 183 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 183 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 184 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 184 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 184 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 185 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 185 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 185 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 186 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 186 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 186 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 187 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 187 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 187 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 188 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 188 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 188 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 189 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 189 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 189 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 190 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 190 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 190 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 191 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 191 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 191 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 192 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 192 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 192 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 193 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 193 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 193 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 194 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 194 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 194 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 195 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 195 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 195 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 196 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 196 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 196 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 197 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 197 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 197 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 198 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 198 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 198 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 199 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 199 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 199 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 200 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 200 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 200 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 201 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 201 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 201 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 202 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 202 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 202 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 203 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 203 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 203 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 204 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 204 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 204 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 205 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 205 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 205 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 206 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 206 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 206 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 207 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 207 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 207 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 208 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 208 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 208 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 209 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 209 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 209 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 210 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 210 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 210 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 211 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 211 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 211 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 212 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 212 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 212 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 213 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 213 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 213 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 214 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 214 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 214 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 215 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 215 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 215 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 216 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 216 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 216 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 217 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 217 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 217 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 218 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 218 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 218 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 219 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 219 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 219 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 220 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 220 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 220 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 221 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 221 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 221 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 222 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 222 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 222 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 223 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 223 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 223 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 224 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 224 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 224 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 225 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 225 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 225 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 226 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 226 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 226 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 227 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 227 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 227 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 228 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 228 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 228 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 229 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 229 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 229 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 230 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 230 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 230 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 231 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 231 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 231 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 232 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 232 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 232 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 233 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 233 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 233 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 234 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 234 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 234 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 235 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 235 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 235 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 236 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 236 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 236 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 237 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 237 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 237 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 238 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 238 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 238 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 239 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 239 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 239 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 240 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 240 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 240 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 241 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 241 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 241 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 242 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 242 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 242 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 243 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 243 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 243 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 244 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 244 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 244 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 245 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 245 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 245 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 246 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 246 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 246 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 247 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 247 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 247 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 248 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 248 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 248 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 249 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 249 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 249 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 250 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 250 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 250 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 251 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 251 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 251 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 252 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 252 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 252 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 253 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 253 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 253 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 254 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 254 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 254 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 255 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 255 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 255 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 256 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 256 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 256 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 257 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 257 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 257 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 258 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 258 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 258 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 259 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 259 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 259 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 260 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 260 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 260 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 261 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 261 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 261 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 262 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 262 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 262 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 263 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 263 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 263 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 264 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 264 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 264 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 265 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 265 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 265 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 266 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 266 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 266 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 267 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 267 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 267 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 268 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 268 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 268 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 269 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 269 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 269 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 270 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 270 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 270 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 271 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 271 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 271 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 272 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 272 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 272 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 273 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 273 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 273 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 274 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 274 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 274 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 275 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 275 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 275 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 276 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 276 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 276 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 277 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 277 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 277 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 278 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 278 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 278 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 279 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 279 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 279 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 280 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 280 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 280 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 281 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 281 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 281 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 282 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 282 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 282 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 283 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 283 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 283 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 284 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 284 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 284 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 285 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 285 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 285 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 286 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 286 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 286 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 287 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 287 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 287 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 288 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 288 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 288 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 289 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 289 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 289 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 290 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 290 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 290 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 291 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 291 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 291 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 292 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 292 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 292 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 293 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 293 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 293 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 294 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 294 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 294 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 295 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 295 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 295 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 296 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 296 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 296 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 297 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 297 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 297 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 298 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 298 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 298 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 299 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 299 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 299 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 300 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 300 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 300 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 301 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 301 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 301 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 302 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 302 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 302 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 303 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 303 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 303 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 304 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 304 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 304 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 305 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 305 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 305 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 306 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 306 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 306 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 307 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 307 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 307 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 308 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 308 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 308 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 309 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 309 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 309 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 310 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 310 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 310 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 311 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 311 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 311 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 312 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 312 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 312 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 313 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 313 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 313 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 314 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 314 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 314 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 315 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 315 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 315 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 316 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 316 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 316 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 317 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 317 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 317 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 318 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 318 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 318 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 319 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 319 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 319 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 320 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 320 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 320 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 321 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 321 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 321 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 322 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 322 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 322 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 323 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 323 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 323 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 324 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 324 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 324 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 325 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 325 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 325 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 326 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 326 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 326 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 327 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 327 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 327 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 328 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 328 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 328 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 329 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 329 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 329 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 330 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 330 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 330 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 331 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 331 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 331 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 332 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 332 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 332 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 333 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 333 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 333 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 334 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 334 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 334 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 335 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 335 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 335 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 336 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 336 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 336 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 337 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 337 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 337 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 338 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 338 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 338 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 339 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 339 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 339 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 340 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 340 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 340 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 341 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 341 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 341 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 342 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 342 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 342 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 343 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 343 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 343 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 344 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 344 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 344 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 345 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 345 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 345 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 346 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 346 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 346 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 347 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 347 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 347 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 348 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 348 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 348 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 349 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 349 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 349 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 350 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 350 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 350 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 351 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 351 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 351 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 352 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 352 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 352 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 353 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 353 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 353 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 354 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 354 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 354 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 355 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 355 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 355 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 356 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 356 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 356 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 357 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 357 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 357 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 358 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 358 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 358 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 359 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 359 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 359 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 360 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 360 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 360 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 361 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 361 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 361 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 362 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 362 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 362 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 363 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 363 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 363 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 364 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 364 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 364 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 365 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 365 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 365 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 366 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 366 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 366 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 367 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 367 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 367 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 368 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 368 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 368 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 369 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 369 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 369 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 370 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 370 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 370 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 371 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 371 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 371 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 372 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 372 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 372 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 373 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 373 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 373 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 374 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 374 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 374 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 375 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 375 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 375 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 376 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 376 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 376 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 377 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 377 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 377 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 378 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 378 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 378 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 379 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 379 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 379 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 380 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 380 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 380 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 381 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 381 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 381 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 382 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 382 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 382 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 383 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 383 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 383 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 384 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 384 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 384 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 385 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 385 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 385 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 386 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 386 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 386 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 387 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 387 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 387 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 388 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 388 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 388 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 389 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 389 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 389 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 390 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 390 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 390 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 391 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 391 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 391 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 392 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 392 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 392 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 393 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 393 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 393 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 394 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 394 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 394 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 395 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 395 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 395 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 396 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 396 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 396 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 397 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 397 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 397 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 398 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 398 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 398 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 399 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 399 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 399 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 400 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 400 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 400 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 401 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 401 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 401 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 402 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 402 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 402 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 403 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 403 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 403 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 404 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 404 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 404 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 405 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 405 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 405 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 406 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 406 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 406 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 407 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 407 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 407 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 408 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 408 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 408 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 409 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 409 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 409 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 410 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 410 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 410 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 411 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 411 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 411 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 412 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 412 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 412 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 413 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 413 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 413 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 414 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 414 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 414 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 415 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 415 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 415 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 416 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 416 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 416 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 417 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 417 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 417 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 418 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 418 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 418 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 419 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 419 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 419 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 420 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 420 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 420 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 421 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 421 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 421 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 422 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 422 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 422 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 423 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 423 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 423 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 424 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 424 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 424 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 425 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 425 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 425 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 426 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 426 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 426 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 427 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 427 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 427 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 428 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 428 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 428 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 429 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 429 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 429 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 430 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 430 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 430 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 431 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 431 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 431 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 432 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 432 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 432 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 433 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 433 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 433 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 434 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 434 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 434 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 435 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 435 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 435 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 436 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 436 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 436 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 437 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 437 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 437 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 438 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 438 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 438 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 439 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 439 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 439 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 440 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 440 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 440 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 441 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 441 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 441 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 442 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 442 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 442 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 443 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 443 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 443 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 444 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 444 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 444 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 445 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 445 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 445 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 446 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 446 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 446 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 447 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 447 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 447 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 448 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 448 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 448 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
call { triWaitFrames 45; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF hwalk 449 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG hwalk 449 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
logInfo format ["KVX hwalk 449 %1 %2 %3 %4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]
kvA4 = [kvA4, call kvF, 0.15] call kvMix
kvFv4 call kvLens
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "hwalk-shelf"
kvA5 = [kvA5, call kvF, 0.15] call kvMix
kvFv5 call kvLens
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "hwalk-wide"
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
