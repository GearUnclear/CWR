// ============================================================================
//  Chupacabra 9:16 film shots: s13_lookback, s16_burst, s17_heels
//  Written by gen_chupa_shots.py (edit that, not this file).
//  Quiet: no weapons, nobody fires, --nosound.
// ============================================================================
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4800, 9000, 0]
setDate [1985, 5, 10, 13, 0]
0 setOvercast 0.35
0 setFog 0
setViewDistance 1500
showCinemaBorder false
cvC = createCenter civilian
cvHud = "camera" camCreate [4800, 9000, 40]
cvHud cameraEffect ["internal", "back"]
cvHud camSetTarget [4800, 12000, 40]
cvHud camSetFov 0.7
cvHud camCommit 0
cvProbe = "HeliHEmpty" createVehicle [4800, 9000, 0]
cvGnd = { cvProbe setPos [_this select 0, _this select 1, 0]; (getPosASL cvProbe) select 2 }
cvLook = { private ["_e", "_a"]; _e = _this select 0; _a = _this select 1; [_e select 0, _e select 2, _e select 1, (_a select 0) - (_e select 0), (_a select 2) - (_e select 2), (_a select 1) - (_e select 1)] }
cvSA = []
cvSB = []
cvSmA = { private ["_p", "_a"]; _p = _this select 0; _a = _this select 1; if ((count cvSA) == 0) then { cvSA = _p }; cvSA = [(cvSA select 0) + ((_p select 0) - (cvSA select 0)) * _a, (cvSA select 1) + ((_p select 1) - (cvSA select 1)) * _a, (cvSA select 2) + ((_p select 2) - (cvSA select 2)) * _a]; cvSA }
cvSmB = { private ["_p", "_a"]; _p = _this select 0; _a = _this select 1; if ((count cvSB) == 0) then { cvSB = _p }; cvSB = [(cvSB select 0) + ((_p select 0) - (cvSB select 0)) * _a, (cvSB select 1) + ((_p select 1) - (cvSB select 1)) * _a, (cvSB select 2) + ((_p select 2) - (cvSB select 2)) * _a]; cvSB }
cvShk = { private ["_k", "_a"]; _k = _this select 0; _a = _this select 1; [_a * ((sin (_k * 21)) + 0.5 * (sin (_k * 53 + 40))), _a * ((sin (_k * 27 + 200)) + 0.4 * (sin (_k * 47))), _a * 0.7 * ((sin (_k * 17 + 90)) + 0.5 * (sin (_k * 61 + 10)))] }
cvEase = { _this * _this * (3 - 2 * _this) }
cvSide = 2.0
cvPursue = { private ["_v", "_c", "_w", "_dx", "_dz", "_g", "_k", "_lx", "_lz"]; _v = getPosASL victim; _c = getPosASL chupa; _w = velocity victim; _dx = (_v select 0) - (_c select 0); _dz = (_v select 1) - (_c select 1); _g = sqrt (_dx * _dx + _dz * _dz); _k = _g / 10; if (_k > 1) then { _k = 1 }; _lx = (_v select 0) + 0.4 * (_w select 0); _lz = (_v select 1) + 0.4 * (_w select 1) + cvSide * _k; chupa setDir (((_lx - (_c select 0)) atan2 (_lz - (_c select 1)))); cvGap = _g }
cvGap = 0
cvO = "Hangar_Hall" createVehicle [4900, 11152, 0]
cvO setDir 90
cvO setPos [4900, 11152, 0]
cvO = "Barracks" createVehicle [4906, 11078, 0]
cvO setDir 90
cvO setPos [4906, 11078, 0]
cvO = "Su25" createVehicle [4848, 11100, 0]
cvO setDir 180
cvO setPos [4848, 11100, 0]
cvO = "UralRefuel" createVehicle [4866, 11084, 0]
cvO setDir 0
cvO setPos [4866, 11084, 0]
cvO = "Mi17" createVehicle [4870, 11192, 0]
cvO setDir 225
cvO setPos [4870, 11192, 0]
cvO = "Ural" createVehicle [4996, 11109, 0]
cvO setDir 0
cvO setPos [4996, 11109, 0]
cvO = "Barrels" createVehicle [4985, 11123.6, 0]
cvO setDir 0
cvO setPos [4985, 11123.6, 0]
cvO = "Barrel4" createVehicle [4984.8, 11126.2, 0]
cvO setDir 0
cvO setPos [4984.8, 11126.2, 0]
cvO = "Barrel4" createVehicle [4983.9, 11125.4, 0]
cvO setDir 0
cvO setPos [4983.9, 11125.4, 0]
cvO = "Barrel1" createVehicle [4985.6, 11127.4, 0]
cvO setDir 0
cvO setPos [4985.6, 11127.4, 0]
cvGV = createGroup civilian
cvGC = createGroup civilian
victim = objNull
chupa = objNull
"Civilian" createUnit [[4988, 11125, 0], cvGV, "victim = this", 0.5, "PRIVATE"]
"UDChupacabraAnim" createUnit [[5092, 11125, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull victim)]
triAssert [!(isNull chupa)]
{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false } forEach [victim, chupa]
cvGV setBehaviour "CARELESS"
cvGV setSpeedMode "FULL"
cvGC setBehaviour "SAFE"
cvGC setSpeedMode "FULL"
chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]
victim addEventHandler ["AnimChanged", {logInfo format ["CVA victim %1 %2", time, _this select 1]}]
triSimFrames 60
// ---- s13_lookback (77 frames)
victim stop false
victim setPos [4964.000, 11122.000, 0]
victim setDir 270
victim enableAI "MOVE"
victim enableAI "ANIM"
victim setUnitPos "UP"
victim doMove (getPos victim)
victim switchMove "Civil"
deleteVehicle chupa
triSimFrames 2
"UDChupacabraAnim" createUnit [[4976.000, 11124.000, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull chupa)]
removeAllWeapons chupa
chupa setCaptive true
chupa allowDammage false
chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]
chupa setPos [4976.000, 11124.000, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
cvGC setBehaviour "SAFE"
cvGC setSpeedMode "FULL"
cvSide = 2
victim doMove [4870.000, 11116.000, 0]
setAccTime 1
if (true) then { cvT0 = time }
triSimUntil { time > cvT0 + 0.5 }
chupa disableAI "MOVE"
chupa disableAI "ANIM"
chupa switchMove "ChupaGallop"
cvSA = []
cvSB = []
cvCamF = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 76; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.35] call cvSmA; _pc = [getPosASL chupa, 0.35] call cvSmB; _h = _pv; _h = [(_h select 0) + (-5.000 + (-4.000 - (-5.000)) * _s), (_h select 1) + (1.000 + (0.900 - (1.000)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (1.350 + (1.350 - (1.350)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.900 + (0.900 - (0.900)) * _s)]; _h = [_k, 0.035] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.025] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.36
cvHud camCommit 0
call { setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVS %1 begin %2 frames %3 victim %4 %5 chupa %6 %7", "s13_lookback", time, 77, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa]
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([0] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 0, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([1] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 1, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([2] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 2, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([3] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 3, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([4] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 4, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([5] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 5, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([6] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 6, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([7] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 7, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([8] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 8, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([9] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 9, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([10] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 10, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([11] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 11, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([12] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 12, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([13] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 13, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([14] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 14, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([15] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 15, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([16] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 16, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([17] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 17, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([18] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 18, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([19] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 19, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([20] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 20, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([21] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 21, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([22] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 22, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([23] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 23, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([24] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 24, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([25] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 25, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([26] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 26, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([27] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 27, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([28] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 28, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([29] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 29, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([30] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 30, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([31] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 31, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([32] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 32, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([33] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 33, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([34] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 34, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([35] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 35, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([36] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 36, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([37] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 37, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([38] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 38, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([39] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 39, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([40] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 40, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([41] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 41, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([42] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 42, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([43] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 43, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([44] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 44, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([45] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 45, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([46] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 46, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([47] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 47, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([48] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 48, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([49] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 49, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([50] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 50, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([51] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 51, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([52] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 52, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([53] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 53, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([54] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 54, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([55] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 55, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([56] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 56, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([57] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 57, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([58] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 58, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([59] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 59, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([60] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 60, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([61] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 61, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([62] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 62, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([63] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 63, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([64] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 64, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([65] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 65, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([66] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 66, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([67] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 67, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([68] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 68, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([69] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 69, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([70] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 70, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([71] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 71, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([72] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 72, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([73] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 73, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([74] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 74, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([75] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 75, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([76] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s13_lookback", 76, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s13_lookback"
setAccTime 1
logInfo format ["CVS %1 end %2 gap %3", "s13_lookback", time, cvGap]
triSimFrames 2
// ---- s16_burst (76 frames)
victim stop false
victim setPos [4860.000, 11114.000, 0]
victim setDir 270
victim enableAI "MOVE"
victim enableAI "ANIM"
victim setUnitPos "UP"
victim doMove (getPos victim)
victim switchMove "Civil"
deleteVehicle chupa
triSimFrames 2
"UDChupacabraAnim" createUnit [[4871.000, 11116.000, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull chupa)]
removeAllWeapons chupa
chupa setCaptive true
chupa allowDammage false
chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]
chupa setPos [4871.000, 11116.000, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
cvGC setBehaviour "SAFE"
cvGC setSpeedMode "FULL"
cvSide = 2
victim doMove [4770.000, 11114.000, 0]
setAccTime 1
if (true) then { cvT0 = time }
triSimUntil { time > cvT0 + 0.5 }
chupa disableAI "MOVE"
chupa disableAI "ANIM"
chupa switchMove "ChupaGallop"
cvSA = []
cvSB = []
cvCamF = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 75; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.45] call cvSmA; _pc = [getPosASL chupa, 0.45] call cvSmB; _h = _pc; _h = [(_h select 0) + (-3.600 + (-3.600 - (-3.600)) * _s), (_h select 1) + (-2.400 + (-2.400 - (-2.400)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.900 + (0.900 - (0.900)) * _s)]; _h = _pc; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.550 + (0.550 - (0.550)) * _s)]; _h = [_k, 0.035] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.02] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
call { setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVS %1 begin %2 frames %3 victim %4 %5 chupa %6 %7", "s16_burst", time, 76, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa]
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([0] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 0, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([1] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 1, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([2] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 2, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([3] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 3, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([4] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 4, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([5] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 5, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([6] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 6, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([7] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 7, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([8] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 8, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([9] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 9, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([10] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 10, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([11] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 11, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([12] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 12, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([13] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 13, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([14] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 14, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([15] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 15, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([16] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 16, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([17] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 17, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([18] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 18, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([19] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 19, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([20] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 20, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([21] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 21, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([22] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 22, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([23] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 23, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([24] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 24, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([25] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 25, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([26] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 26, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([27] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 27, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([28] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 28, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([29] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 29, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([30] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 30, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([31] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 31, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([32] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 32, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([33] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 33, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([34] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 34, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([35] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 35, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([36] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 36, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([37] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 37, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([38] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 38, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([39] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 39, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([40] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 40, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([41] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 41, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([42] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 42, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([43] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 43, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([44] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 44, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([45] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 45, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([46] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 46, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([47] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 47, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([48] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 48, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([49] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 49, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([50] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 50, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([51] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 51, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([52] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 52, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([53] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 53, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([54] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 54, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([55] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 55, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([56] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 56, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([57] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 57, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([58] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 58, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([59] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 59, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([60] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 60, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([61] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 61, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([62] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 62, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([63] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 63, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([64] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 64, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([65] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 65, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([66] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 66, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([67] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 67, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([68] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 68, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([69] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 69, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([70] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 70, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([71] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 71, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([72] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 72, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([73] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 73, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([74] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 74, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([75] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s16_burst", 75, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s16_burst"
setAccTime 1
logInfo format ["CVS %1 end %2 gap %3", "s16_burst", time, cvGap]
triSimFrames 2
// ---- s17_heels (77 frames)
victim stop false
victim setPos [4836.000, 11114.000, 0]
victim setDir 270
victim enableAI "MOVE"
victim enableAI "ANIM"
victim setUnitPos "UP"
victim doMove (getPos victim)
victim switchMove "Civil"
deleteVehicle chupa
triSimFrames 2
"UDChupacabraAnim" createUnit [[4847.500, 11116.000, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull chupa)]
removeAllWeapons chupa
chupa setCaptive true
chupa allowDammage false
chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]
chupa setPos [4847.500, 11116.000, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
cvGC setBehaviour "SAFE"
cvGC setSpeedMode "FULL"
cvSide = 2
victim doMove [4740.000, 11114.000, 0]
setAccTime 1
if (true) then { cvT0 = time }
triSimUntil { time > cvT0 + 0.5 }
chupa disableAI "MOVE"
chupa disableAI "ANIM"
chupa switchMove "ChupaGallop"
cvSA = []
cvSB = []
cvCamF = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 76; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.5] call cvSmA; _pc = [getPosASL chupa, 0.5] call cvSmB; _h = _pc; _h = [(_h select 0) + (3.000 + (3.000 - (3.000)) * _s), (_h select 1) + (-1.600 + (-1.600 - (-1.600)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.500 + (0.500 - (0.500)) * _s)]; _h = [(_pv select 0) * 0.3 + (_pc select 0) * 0.7, (_pv select 1) * 0.3 + (_pc select 1) * 0.7, (_pv select 2) * 0.3 + (_pc select 2) * 0.7]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.700 + (0.700 - (0.700)) * _s)]; _h = [_k, 0.035] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.02] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
call { setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVS %1 begin %2 frames %3 victim %4 %5 chupa %6 %7", "s17_heels", time, 77, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa]
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([0] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 0, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([1] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 1, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([2] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 2, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([3] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 3, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([4] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 4, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([5] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 5, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([6] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 6, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([7] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 7, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([8] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 8, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([9] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 9, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([10] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 10, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([11] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 11, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([12] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 12, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([13] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 13, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([14] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 14, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([15] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 15, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([16] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 16, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([17] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 17, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([18] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 18, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([19] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 19, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([20] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 20, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([21] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 21, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([22] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 22, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([23] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 23, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([24] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 24, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([25] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 25, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([26] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 26, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([27] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 27, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([28] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 28, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([29] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 29, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([30] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 30, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([31] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 31, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([32] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 32, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([33] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 33, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([34] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 34, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([35] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 35, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([36] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 36, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([37] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 37, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([38] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 38, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([39] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 39, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([40] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 40, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([41] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 41, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([42] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 42, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([43] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 43, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([44] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 44, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([45] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 45, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([46] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 46, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([47] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 47, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([48] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 48, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([49] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 49, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([50] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 50, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([51] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 51, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([52] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 52, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([53] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 53, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([54] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 54, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([55] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 55, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([56] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 56, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([57] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 57, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([58] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 58, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([59] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 59, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([60] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 60, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([61] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 61, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([62] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 62, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([63] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 63, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([64] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 64, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([65] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 65, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([66] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 66, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([67] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 67, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([68] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 68, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([69] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 69, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([70] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 70, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([71] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 71, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([72] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 72, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([73] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 73, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([74] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 74, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([75] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 75, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
[] call cvPursue
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView ([76] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8", "s17_heels", 76, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]
triScreenshot "s17_heels"
setAccTime 1
logInfo format ["CVS %1 end %2 gap %3", "s17_heels", time, cvGap]
triSimFrames 2
triEndTest
