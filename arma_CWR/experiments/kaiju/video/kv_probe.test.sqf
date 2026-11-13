// ============================================================================
//  Dedushka video probe: stepped clock, portrait lenses, the map, first motion
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
kvLens = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov _this; kjCamObj camCommit 0 }
logInfo format ["KV start: creature %1, native %2, hour %3, start %4, params %5", typeOf kaiju, KJ_native, daytime, getPos kaiju, KJD_params]
0.5 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 8
triScreenshot "000_warmup_discard"
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3500] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3500%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3550] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3550%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3600] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3600%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3650] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3650%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3700] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3700%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3750] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3750%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3800] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3800%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3850] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3850%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3900] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3900%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 3950] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 3950%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4000] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4000%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4050] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4050%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4100] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4100%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4150] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4150%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4200] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4200%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4250] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4250%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4300] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4300%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4350] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4350%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4400] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4400%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4450] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4450%1", kvRow]
kvRow = ""; { kvRow = kvRow + format [" %1", [_x, 4500] call kvGnd] } forEach [4300, 4350, 4400, 4450, 4500, 4550, 4600, 4650, 4700, 4750, 4800, 4850, 4900, 4950, 5000, 5050, 5100, 5150, 5200, 5250, 5300, 5350, 5400, 5450, 5500]; logInfo format ["KVH 4500%1", kvRow]
0.5 call kvLens
triSetView [5000, 600, 3950, 0, -1, 0.02]
triSimFrames 12
triScreenshot "top_5000_3950_600"
triSetView [4900, 1500, 3900, 0, -1, 0.02]
triSimFrames 12
triScreenshot "top_4900_3900_1500"
0.35 call kvLens
triSetView ([[4690, 3990, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_west_35"
0.03 call kvLens
triSetView ([[4880, 2160, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_bay_03"
0.06 call kvLens
triSetView ([[4880, 2160, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_bay_06"
0.12 call kvLens
triSetView ([[4790, 4404, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_n400_12"
0.12 call kvLens
triSetView ([[4790, 3604, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_s400_12"
0.08 call kvLens
triSetView ([[4190, 4004, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_w600_08"
0.1 call kvLens
triSetView ([[5150, 4350, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_ne_10"
0.45 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_street_45"
0.15 call kvLens
triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_street_15"
0.3 call kvLens
triSetView ([[5050, 3975, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_square_30"
0.12 call kvLens
triSetView ([[5128, 3958, 2.6] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_armour_12"
0.08 call kvLens
triSetView ([[5300, 4004, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_east_08"
triCaptureAtPresent false
0.35 call kvLens
triSetView ([[4690, 3990, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
triSimFrames 6
triScreenshot "dorm_west_35_midframe"
triCaptureAtPresent true
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 0 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 1 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 2 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 3 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 4 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 5 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 6 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 7 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 8 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 9 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 10 %1", time]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 60 11 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 0 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 1 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 2 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 3 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 4 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 5 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 6 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 7 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 8 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 9 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 10 %1", time]
call { triWaitFrames 30; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 30 11 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 0 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 1 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 2 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 3 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 4 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 5 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 6 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 7 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 8 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 9 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 10 %1", time]
call { triWaitFrames 15; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 15 11 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 0 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 1 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 2 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 3 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 4 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 5 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 6 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 7 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 8 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 9 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 10 %1", time]
call { triWaitFrames 8; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVCLOCK 8 11 %1", time]
triSetView ([[4690, 3990, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)
0.12 call kvLens
triScreenshot "frozen_lens_12_no_tick"
triSimFrames 1
triScreenshot "frozen_lens_12_after_tick"
logInfo format ["KVCLOCK tick %1", time]
0.35 call kvLens
triSimFrames 1
triScreenshot "frozen_lens_35_after_tick"
setAccTime 1
0.35 call kvLens
triSimFrames 2
kvE1 = [4985, 3984, 1.7] call kvP
kvA1 = [0, 0, 22] call kvK
logInfo format ["KVC wake-street eye %1 aim %2 fov 0.35 alpha 1", kvE1, kvA1]
kvE2 = [4690, 3990, 1.7] call kvP
kvA2 = [0, 0, 22] call kvK
logInfo format ["KVC wake-west eye %1 aim %2 fov 0.35 alpha 1", kvE2, kvA2]
call { ["wake"] exec "kjd_beat.sqs"; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF wake 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG wake 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA1 = [0, 0, 22] call kvK
triSetView ([kvE1, kvA1] call kvLook)
triScreenshot "wake-street"
kvA2 = [0, 0, 22] call kvK
triSetView ([kvE2, kvA2] call kvLook)
triScreenshot "wake-west"
setAccTime 1
triSimUntil { ("wake" in KJD_done) || (time > 200) }
logInfo format ["KV wake done at %1, state %2", time, KJ_state]
["through"] exec "kjd_beat.sqs"
triSimUntil { (((getPos kaiju) select 0) > 4860) || (time > 400) }
0.35 call kvLens
triSimFrames 2
kvE3 = [4985, 3984, 1.7] call kvP
kvA3 = [0, 0, 22] call kvK
logInfo format ["KVC walk-street eye %1 aim %2 fov 0.35 alpha 0.1", kvE3, kvA3]
kvE4 = [4930, 4130, 1.7] call kvP
kvA4 = [0, 0, 22] call kvK
logInfo format ["KVC walk-north eye %1 aim %2 fov 0.35 alpha 0.1", kvE4, kvA4]
kvE5 = [4930, 4130, 1.7] call kvP
kvA5 = [4900, 4004, 22] call kvP
logInfo format ["KVC walk-fixed eye %1 aim %2 fov 0.35 alpha 1", kvE5, kvA5]
call { kvNop = 0; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 0 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 0 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 1 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 1 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 2 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 2 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 3 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 3 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 4 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 4 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 5 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 5 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 6 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 6 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 7 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 7 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 8 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 8 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 9 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 9 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 10 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 10 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 11 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 11 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 12 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 12 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 13 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 13 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 14 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 14 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 15 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 15 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 16 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 16 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 17 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 17 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 18 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 18 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 19 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 19 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 20 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 20 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 21 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 21 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 22 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 22 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 23 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 23 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 24 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 24 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 25 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 25 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 26 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 26 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 27 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 27 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 28 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 28 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 29 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 29 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 30 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 30 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 31 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 31 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 32 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 32 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 33 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 33 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 34 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 34 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 35 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 35 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 36 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 36 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 37 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 37 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 38 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 38 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 39 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 39 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 40 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 40 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 41 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 41 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 42 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 42 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 43 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 43 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 44 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 44 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 45 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 45 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 46 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 46 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 47 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 47 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 48 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 48 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 49 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 49 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 50 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 50 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 51 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 51 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 52 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 52 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 53 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 53 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 54 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 54 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 55 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 55 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 56 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 56 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 57 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 57 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 58 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 58 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 59 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 59 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 60 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 60 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 61 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 61 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 62 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 62 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 63 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 63 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 64 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 64 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 65 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 65 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 66 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 66 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 67 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 67 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 68 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 68 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 69 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 69 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 70 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 70 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 71 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 71 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 72 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 72 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 73 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 73 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 74 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 74 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 75 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 75 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 76 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 76 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 77 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 77 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 78 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 78 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 79 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 79 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 80 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 80 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 81 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 81 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 82 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 82 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 83 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 83 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 84 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 84 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 85 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 85 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 86 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 86 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 87 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 87 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 88 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 88 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 89 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 89 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 90 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 90 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 91 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 91 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 92 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 92 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 93 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 93 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 94 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 94 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 95 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 95 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 96 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 96 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 97 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 97 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 98 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 98 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 99 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 99 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 100 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 100 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 101 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 101 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 102 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 102 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 103 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 103 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 104 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 104 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 105 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 105 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 106 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 106 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 107 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 107 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 108 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 108 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 109 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 109 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 110 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 110 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 111 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 111 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 112 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 112 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 113 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 113 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 114 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 114 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 115 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 115 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 116 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 116 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 117 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 117 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 118 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 118 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 119 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 119 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 120 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 120 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 121 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 121 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 122 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 122 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 123 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 123 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 124 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 124 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 125 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 125 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 126 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 126 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 127 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 127 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 128 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 128 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 129 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 129 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 130 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 130 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 131 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 131 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 132 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 132 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 133 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 133 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 134 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 134 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 135 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 135 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 136 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 136 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 137 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 137 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 138 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 138 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 139 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 139 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 140 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 140 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 141 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 141 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 142 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 142 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 143 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 143 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 144 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 144 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 145 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 145 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 146 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 146 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 147 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 147 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 148 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 148 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 149 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 149 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 150 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 150 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 151 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 151 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 152 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 152 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 153 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 153 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 154 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 154 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 155 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 155 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 156 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 156 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 157 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 157 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 158 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 158 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 159 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 159 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 160 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 160 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 161 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 161 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 162 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 162 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 163 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 163 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 164 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 164 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 165 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 165 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 166 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 166 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 167 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 167 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 168 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 168 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 169 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 169 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 170 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 170 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 171 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 171 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 172 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 172 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 173 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 173 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 174 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 174 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 175 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 175 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 176 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 176 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 177 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 177 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 178 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 178 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["KVF walk 179 %1 %2 %3 %4 %5 %6 %7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]
logInfo format ["KVG walk 179 %1 %2 %3 %4 %5 %6 %7 %8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]
kvA3 = [kvA3, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE3, kvA3] call kvLook)
triScreenshot "walk-street"
kvA4 = [kvA4, [0, 0, 22] call kvK, 0.1] call kvMix
triSetView ([kvE4, kvA4] call kvLook)
triScreenshot "walk-north"
kvA5 = [4900, 4004, 22] call kvP
triSetView ([kvE5, kvA5] call kvLook)
triScreenshot "walk-fixed"
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
