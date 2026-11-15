// ============================================================================
//  Dedushka video probe 2: the framing of every planned camera (stills)
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
kaiju setPos [4840, 4004, 0]
triSimFrames 30
0.5 call kvLens
triSetView ([[4876, 3989, 1.2] call kvP, [4840, 4004, 27] call kvP] call kvLook)
triSimFrames 6
triScreenshot "a_low"
0.2 call kvLens
triSetView ([[4930, 4130, 1.7] call kvP, [4840, 4004, 20] call kvP] call kvLook)
triSimFrames 6
triScreenshot "a_north"
0.12 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [4850, 4004, 15] call kvP] call kvLook)
triSimFrames 6
triScreenshot "a_street"
kaiju setPos [4925, 4004, 0]
triSimFrames 30
0.35 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [4925, 4004, 25] call kvP] call kvLook)
triSimFrames 6
triScreenshot "b_street"
0.12 call kvLens
triSetView ([[5078, 3977, 1.7] call kvP, [4925, 4004, 22] call kvP] call kvLook)
triSimFrames 6
triScreenshot "b_square"
kaiju setPos [5003, 4004, 0]
triSimFrames 30
0.45 call kvLens
triSetView ([[5050, 3975, 1.7] call kvP, [5003, 4004, 26] call kvP] call kvLook)
triSimFrames 6
triScreenshot "r_square"
0.2 call kvLens
triSetView ([[5128, 3958, 2.6] call kvP, [5003, 4004, 24] call kvP] call kvLook)
triSimFrames 6
triScreenshot "r_armour"
0.3 call kvLens
triSetView ([[5082, 4042, 1.7] call kvP, [5003, 4004, 26] call kvP] call kvLook)
triSimFrames 6
triScreenshot "r_civ"
0.16 call kvLens
triSetView ([[5142, 3952, 2.6] call kvP, [5003, 4004, 22] call kvP] call kvLook)
triSimFrames 6
triScreenshot "f_armour"
0.2 call kvLens
triSetView ([[5075, 4125, 2] call kvP, [5003, 4004, 22] call kvP] call kvLook)
triSimFrames 6
triScreenshot "f_north"
0.3 call kvLens
triSetView ([[5075, 4125, 2] call kvP, [5023, 4004, 22] call kvP] call kvLook)
triSimFrames 6
triScreenshot "m_north"
0.06 call kvLens
triSetView ([[5128, 3958, 2.6] call kvP, [5003, 4004, 28] call kvP] call kvLook)
triSimFrames 6
triScreenshot "m_chest"
0.35 call kvLens
triSetView ([[5142, 3950, 2.6] call kvP, [5003, 4004, 20] call kvP] call kvLook)
triSimFrames 6
triScreenshot "m_armour"
0.5 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [5003, 4004, 26] call kvP] call kvLook)
triSimFrames 6
triScreenshot "s_street"
0.08 call kvLens
triSetView ([[5300, 4004, 1.7] call kvP, [5003, 4004, 22] call kvP] call kvLook)
triSimFrames 6
triScreenshot "r_east"
0.4 call kvLens
triSetView ([[5010, 3895, 1.7] call kvP, [5003, 4004, 24] call kvP] call kvLook)
triSimFrames 6
triScreenshot "r_south"
kaiju setPos [5046, 3931, 0]
kaiju setDir 117
triSimFrames 30
0.32 call kvLens
triSetView ([[5097, 3888, 1.7] call kvP, [5057, 3925, 20] call kvP] call kvLook)
triSimFrames 6
triScreenshot "c_se"
0.36 call kvLens
triSetView ([[5052, 3998, 1.7] call kvP, [5057, 3925, 17] call kvP] call kvLook)
triSimFrames 6
triScreenshot "c_north"
0.05 call kvLens
triSetView ([[5300, 3950, 1.7] call kvP, [5057, 3925, 20] call kvP] call kvLook)
triSimFrames 6
triScreenshot "c_east"
0.3 call kvLens
triSetView ([[5062, 3862, 1.7] call kvP, [5057, 3925, 20] call kvP] call kvLook)
triSimFrames 6
triScreenshot "c_south"
0.3 call kvLens
triSetView ([[4990, 3900, 1.7] call kvP, [5057, 3925, 18] call kvP] call kvLook)
triSimFrames 6
triScreenshot "c_west"
kaiju setPos [5000, 3700, 0]
kaiju setDir 0
triSimFrames 60
logInfo format ["KV bay: he is at %1, asl %2, depth %3", getPos kaiju, getPosASL kaiju, call KJB_fnDepth]
0.1 call kvLens
triSetView ([[5012, 3856, 1.7] call kvP, call kvF] call kvLook)
triSimFrames 6
triScreenshot "h_shelf"
0.3 call kvLens
triSetView ([[5012, 3856, 1.7] call kvP, call kvF] call kvLook)
triSimFrames 6
triScreenshot "h_wide"
0.12 call kvLens
triSetView ([[5050, 3975, 1.7] call kvP, call kvF] call kvLook)
triSimFrames 6
triScreenshot "h_town"
0.2 call kvLens
triSetView ([[5095, 3835, 1.7] call kvP, call kvF] call kvLook)
triSimFrames 6
triScreenshot "h_shore"
skipTime -6
triSimFrames 30
0.5 call kvLens
triSetView [5000, 600, 3950, 0, -1, 0.02]
triSimFrames 12
triScreenshot "top_day_5000_3950_600"
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
