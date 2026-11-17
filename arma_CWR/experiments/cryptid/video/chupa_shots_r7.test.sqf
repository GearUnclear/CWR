// ============================================================================
//  Chupacabra 9:16 film shots: chase_a
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
// ---- chase_a (450 frames)
CHS_DX = -1
CHS_POUNCE_AT = 3.5
CHS_DROP_AFTER = 0.35
CHS_LUNGE_AT = 3.5
CHS_FEED_HOLD = 1.6
CHS_RUSH_DELAY = 1.4
CHS_CHASE_MAX = 20
CHS_REORDER = 30
CHS_SIDE = -2.2
CHS_RUSH_GAP = 6
CHS_STAGE = "pre"
victim stop false
victim setPos [4826.000, 11114.000, 0]
victim setDir 270
victim enableAI "MOVE"
victim enableAI "ANIM"
victim setUnitPos "UP"
victim doMove (getPos victim)
victim switchMove "Civil"
deleteVehicle chupa
triSimFrames 2
"UDChupacabraAnim" createUnit [[4835.000, 11116.200, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull chupa)]
removeAllWeapons chupa
chupa setCaptive true
chupa allowDammage false
chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]
chupa setPos [4835.000, 11116.200, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
cvGC setBehaviour "SAFE"
cvGC setSpeedMode "FULL"
victim doMove [4780, 11114, 0]
chupa doMove [4780, 11116.2, 0]
setAccTime 1
if (true) then { cvT0 = time }
triSimUntil { time > cvT0 + 0.6 }
cvCamS_chase = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.35] call cvSmA; _pc = [getPosASL chupa, 0.35] call cvSmB; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (-11.000 + (-11.000 - (-11.000)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.700 + (0.700 - (0.700)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.800 + (0.800 - (0.800)) * _s)]; _h = [_k, 0.03] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.02] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvCamS_pounce = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.5] call cvSmA; _pc = [getPosASL chupa, 0.5] call cvSmB; _h = _pv; _h = [(_h select 0) + (-1.000 + (-1.000 - (-1.000)) * _s), (_h select 1) + (-4.200 + (-4.200 - (-4.200)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.550 + (0.550 - (0.550)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.600 + (0.600 - (0.600)) * _s)]; _h = [_k, 0.02] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.015] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvCamS_feed = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.3] call cvSmA; _pc = [getPosASL chupa, 0.3] call cvSmB; _h = _pv; _h = [(_h select 0) + (-1.000 + (-1.000 - (-1.000)) * _s), (_h select 1) + (-2.700 + (-2.700 - (-2.700)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.600 + (0.600 - (0.600)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.400 + (0.400 - (0.400)) * _s)]; _h = [_k, 0.012] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.01] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvCamS_recoil = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.3] call cvSmA; _pc = [getPosASL chupa, 0.3] call cvSmB; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (-1.000 + (-1.000 - (-1.000)) * _s), (_h select 1) + (3.600 + (3.600 - (3.600)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.500 + (0.500 - (0.500)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (0.500 + (0.500 - (0.500)) * _s)]; _h = [_k, 0.02] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.015] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvCamS_rush = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.45] call cvSmA; _pc = [getPosASL chupa, 0.45] call cvSmB; _h = _pc; _h = [(_h select 0) + (2.600 + (2.600 - (2.600)) * _s), (_h select 1) + (-0.900 + (-0.900 - (-0.900)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (0.900 + (0.900 - (0.900)) * _s)]; _h = _pv; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (1.000 + (1.000 - (1.000)) * _s)]; _h = [_k, 0.035] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.02] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvCamS_lunge = { private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]; _k = _this select 0; _s = _k / 449; if (_s > 1) then { _s = 1 }; _pv = [getPosASL victim, 0.6] call cvSmA; _pc = [getPosASL chupa, 0.6] call cvSmB; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.600 + (0.600 - (0.600)) * _s), (_h select 1) + (-3.800 + (-3.800 - (-3.800)) * _s), _h select 2]; _g = [_h select 0, _h select 1] call cvGnd; _e = [_h select 0, _h select 1, _g + (1.100 + (1.100 - (1.100)) * _s)]; _h = [(_pv select 0) * 0.5 + (_pc select 0) * 0.5, (_pv select 1) * 0.5 + (_pc select 1) * 0.5, (_pv select 2) * 0.5 + (_pc select 2) * 0.5]; _h = [(_h select 0) + (0.000 + (0.000 - (0.000)) * _s), (_h select 1) + (0.000 + (0.000 - (0.000)) * _s), _h select 2]; _a = [_h select 0, _h select 1, (_h select 2) + (1.000 + (1.000 - (1.000)) * _s)]; _h = [_k, 0.04] call cvShk; _e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]; _h = [_k + 37, 0.03] call cvShk; _a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]; [_e, _a] call cvLook }
cvStageLast = ""
cvCamF = { private ["_c"]; _c = cvCamS_chase; if (CHS_STAGE == "chase") then { _c = cvCamS_chase }; if (CHS_STAGE == "pounce") then { _c = cvCamS_pounce }; if (CHS_STAGE == "feed") then { _c = cvCamS_feed }; if (CHS_STAGE == "recoil") then { _c = cvCamS_recoil }; if (CHS_STAGE == "rush") then { _c = cvCamS_rush }; if (CHS_STAGE == "lunge") then { _c = cvCamS_lunge }; if (CHS_STAGE == "end") then { _c = cvCamS_lunge }; _this call _c }
cvLensF = { if ((CHS_STAGE == "chase") && (cvStageLast != "chase")) then { cvHud camSetFov 0.5; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; if ((CHS_STAGE == "pounce") && (cvStageLast != "pounce")) then { cvHud camSetFov 0.4; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; if ((CHS_STAGE == "feed") && (cvStageLast != "feed")) then { cvHud camSetFov 0.4; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; if ((CHS_STAGE == "recoil") && (cvStageLast != "recoil")) then { cvHud camSetFov 0.4; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; if ((CHS_STAGE == "rush") && (cvStageLast != "rush")) then { cvHud camSetFov 0.4; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; if ((CHS_STAGE == "lunge") && (cvStageLast != "lunge")) then { cvHud camSetFov 0.45; cvHud camCommit 0; cvSA = []; cvSB = []; logInfo format ["CVS stage %1 %2", CHS_STAGE, time] }; cvStageLast = CHS_STAGE }
CHS_DONE = false
[chupa, victim] exec "cv_chase.sqs"
CHS_STAGE = "chase"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
logInfo format ["CVS %1 begin %2 frames %3", "chase_a", time, 450]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([0] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 0, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([1] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 1, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([2] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 2, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([3] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 3, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([4] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 4, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([5] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 5, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([6] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 6, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([7] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 7, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([8] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 8, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([9] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 9, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([10] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 10, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([11] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 11, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([12] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 12, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([13] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 13, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([14] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 14, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([15] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 15, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([16] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 16, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([17] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 17, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([18] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 18, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([19] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 19, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([20] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 20, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([21] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 21, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([22] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 22, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([23] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 23, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([24] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 24, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([25] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 25, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([26] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 26, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([27] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 27, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([28] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 28, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([29] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 29, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([30] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 30, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([31] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 31, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([32] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 32, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([33] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 33, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([34] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 34, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([35] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 35, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([36] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 36, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([37] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 37, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([38] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 38, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([39] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 39, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([40] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 40, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([41] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 41, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([42] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 42, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([43] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 43, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([44] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 44, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([45] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 45, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([46] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 46, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([47] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 47, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([48] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 48, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([49] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 49, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([50] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 50, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([51] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 51, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([52] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 52, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([53] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 53, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([54] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 54, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([55] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 55, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([56] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 56, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([57] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 57, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([58] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 58, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([59] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 59, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([60] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 60, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([61] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 61, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([62] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 62, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([63] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 63, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([64] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 64, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([65] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 65, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([66] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 66, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([67] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 67, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([68] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 68, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([69] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 69, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([70] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 70, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([71] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 71, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([72] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 72, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([73] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 73, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([74] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 74, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([75] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 75, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([76] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 76, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([77] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 77, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([78] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 78, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([79] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 79, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([80] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 80, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([81] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 81, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([82] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 82, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([83] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 83, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([84] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 84, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([85] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 85, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([86] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 86, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([87] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 87, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([88] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 88, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([89] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 89, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([90] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 90, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([91] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 91, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([92] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 92, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([93] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 93, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([94] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 94, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([95] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 95, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([96] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 96, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([97] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 97, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([98] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 98, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([99] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 99, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([100] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 100, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([101] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 101, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([102] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 102, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([103] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 103, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([104] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 104, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([105] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 105, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([106] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 106, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([107] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 107, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([108] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 108, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([109] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 109, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([110] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 110, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([111] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 111, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([112] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 112, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([113] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 113, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([114] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 114, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([115] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 115, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([116] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 116, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([117] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 117, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([118] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 118, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([119] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 119, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([120] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 120, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([121] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 121, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([122] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 122, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([123] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 123, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([124] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 124, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([125] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 125, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([126] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 126, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([127] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 127, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([128] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 128, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([129] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 129, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([130] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 130, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([131] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 131, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([132] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 132, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([133] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 133, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([134] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 134, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([135] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 135, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([136] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 136, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([137] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 137, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([138] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 138, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([139] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 139, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([140] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 140, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([141] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 141, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([142] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 142, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([143] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 143, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([144] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 144, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([145] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 145, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([146] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 146, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([147] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 147, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([148] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 148, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([149] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 149, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([150] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 150, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([151] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 151, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([152] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 152, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([153] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 153, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([154] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 154, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([155] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 155, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([156] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 156, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([157] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 157, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([158] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 158, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([159] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 159, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([160] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 160, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([161] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 161, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([162] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 162, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([163] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 163, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([164] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 164, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([165] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 165, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([166] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 166, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([167] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 167, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([168] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 168, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([169] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 169, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([170] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 170, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([171] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 171, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([172] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 172, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([173] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 173, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([174] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 174, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([175] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 175, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([176] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 176, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([177] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 177, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([178] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 178, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([179] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 179, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([180] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 180, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([181] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 181, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([182] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 182, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([183] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 183, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([184] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 184, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([185] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 185, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([186] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 186, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([187] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 187, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([188] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 188, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([189] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 189, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([190] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 190, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([191] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 191, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([192] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 192, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([193] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 193, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([194] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 194, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([195] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 195, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([196] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 196, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([197] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 197, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([198] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 198, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([199] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 199, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([200] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 200, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([201] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 201, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([202] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 202, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([203] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 203, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([204] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 204, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([205] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 205, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([206] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 206, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([207] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 207, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([208] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 208, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([209] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 209, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([210] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 210, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([211] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 211, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([212] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 212, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([213] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 213, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([214] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 214, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([215] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 215, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([216] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 216, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([217] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 217, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([218] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 218, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([219] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 219, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([220] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 220, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([221] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 221, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([222] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 222, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([223] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 223, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([224] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 224, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([225] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 225, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([226] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 226, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([227] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 227, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([228] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 228, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([229] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 229, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([230] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 230, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([231] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 231, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([232] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 232, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([233] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 233, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([234] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 234, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([235] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 235, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([236] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 236, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([237] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 237, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([238] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 238, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([239] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 239, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([240] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 240, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([241] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 241, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([242] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 242, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([243] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 243, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([244] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 244, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([245] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 245, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([246] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 246, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([247] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 247, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([248] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 248, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([249] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 249, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([250] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 250, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([251] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 251, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([252] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 252, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([253] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 253, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([254] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 254, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([255] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 255, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([256] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 256, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([257] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 257, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([258] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 258, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([259] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 259, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([260] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 260, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([261] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 261, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([262] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 262, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([263] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 263, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([264] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 264, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([265] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 265, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([266] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 266, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([267] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 267, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([268] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 268, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([269] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 269, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([270] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 270, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([271] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 271, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([272] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 272, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([273] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 273, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([274] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 274, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([275] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 275, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([276] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 276, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([277] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 277, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([278] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 278, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([279] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 279, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([280] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 280, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([281] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 281, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([282] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 282, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([283] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 283, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([284] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 284, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([285] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 285, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([286] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 286, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([287] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 287, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([288] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 288, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([289] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 289, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([290] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 290, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([291] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 291, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([292] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 292, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([293] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 293, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([294] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 294, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([295] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 295, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([296] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 296, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([297] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 297, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([298] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 298, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([299] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 299, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([300] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 300, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([301] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 301, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([302] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 302, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([303] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 303, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([304] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 304, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([305] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 305, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([306] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 306, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([307] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 307, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([308] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 308, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([309] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 309, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([310] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 310, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([311] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 311, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([312] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 312, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([313] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 313, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([314] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 314, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([315] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 315, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([316] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 316, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([317] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 317, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([318] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 318, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([319] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 319, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([320] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 320, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([321] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 321, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([322] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 322, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([323] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 323, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([324] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 324, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([325] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 325, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([326] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 326, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([327] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 327, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([328] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 328, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([329] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 329, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([330] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 330, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([331] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 331, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([332] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 332, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([333] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 333, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([334] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 334, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([335] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 335, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([336] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 336, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([337] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 337, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([338] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 338, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([339] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 339, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([340] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 340, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([341] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 341, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([342] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 342, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([343] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 343, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([344] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 344, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([345] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 345, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([346] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 346, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([347] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 347, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([348] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 348, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([349] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 349, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([350] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 350, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([351] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 351, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([352] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 352, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([353] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 353, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([354] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 354, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([355] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 355, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([356] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 356, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([357] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 357, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([358] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 358, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([359] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 359, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([360] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 360, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([361] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 361, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([362] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 362, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([363] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 363, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([364] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 364, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([365] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 365, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([366] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 366, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([367] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 367, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([368] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 368, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([369] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 369, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([370] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 370, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([371] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 371, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([372] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 372, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([373] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 373, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([374] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 374, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([375] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 375, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([376] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 376, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([377] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 377, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([378] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 378, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([379] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 379, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([380] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 380, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([381] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 381, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([382] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 382, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([383] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 383, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([384] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 384, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([385] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 385, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([386] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 386, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([387] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 387, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([388] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 388, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([389] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 389, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([390] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 390, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([391] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 391, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([392] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 392, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([393] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 393, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([394] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 394, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([395] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 395, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([396] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 396, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([397] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 397, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([398] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 398, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([399] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 399, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([400] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 400, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([401] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 401, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([402] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 402, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([403] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 403, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([404] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 404, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([405] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 405, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([406] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 406, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([407] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 407, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([408] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 408, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([409] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 409, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([410] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 410, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([411] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 411, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([412] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 412, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([413] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 413, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([414] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 414, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([415] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 415, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([416] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 416, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([417] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 417, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([418] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 418, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([419] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 419, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([420] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 420, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([421] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 421, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([422] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 422, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([423] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 423, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([424] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 424, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([425] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 425, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([426] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 426, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([427] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 427, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([428] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 428, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([429] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 429, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([430] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 430, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([431] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 431, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([432] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 432, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([433] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 433, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([434] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 434, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([435] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 435, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([436] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 436, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([437] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 437, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([438] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 438, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([439] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 439, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([440] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 440, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([441] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 441, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([442] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 442, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([443] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 443, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([444] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 444, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([445] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 445, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([446] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 446, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([447] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 447, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([448] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 448, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
[] call cvLensF
triSetView ([449] call cvCamF)
logInfo format ["CVF %1 %2 %3 %4 %5 %6 %7 %8 %9", "chase_a", 449, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]
triScreenshot "chase_a"
setAccTime 1
logInfo format ["CVS %1 end %2 done %3", "chase_a", time, CHS_DONE]
triSimFrames 2
triEndTest
