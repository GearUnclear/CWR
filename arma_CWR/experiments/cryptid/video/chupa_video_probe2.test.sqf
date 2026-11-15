// ============================================================================
//  Chupacabra video probe 2: the set, portrait window, lenses
//  Written by gen_chupa_video.py (edit that, not this file).
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
cvProps = []
cvO = "Hangar" createVehicle [4885, 11040, 0]
cvO setDir 270
cvO setPos [4885, 11040, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Hangar %1 at %2 dir %3", 0, getPosASL cvO, getDir cvO]
cvO = "Hangar_Hall" createVehicle [4892, 11125, 0]
cvO setDir 270
cvO setPos [4892, 11125, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Hangar_Hall %1 at %2 dir %3", 1, getPosASL cvO, getDir cvO]
cvO = "Su25" createVehicle [4848, 11082, 0]
cvO setDir 180
cvO setPos [4848, 11082, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Su25 %1 at %2 dir %3", 2, getPosASL cvO, getDir cvO]
cvO = "Mi17" createVehicle [4868, 11170, 0]
cvO setDir 225
cvO setPos [4868, 11170, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Mi17 %1 at %2 dir %3", 3, getPosASL cvO, getDir cvO]
cvO = "UralRefuel" createVehicle [4866, 11060, 0]
cvO setDir 0
cvO setPos [4866, 11060, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop UralRefuel %1 at %2 dir %3", 4, getPosASL cvO, getDir cvO]
cvO = "Ural" createVehicle [4938, 11092, 0]
cvO setDir 0
cvO setPos [4938, 11092, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Ural %1 at %2 dir %3", 5, getPosASL cvO, getDir cvO]
cvO = "Barrels" createVehicle [4934, 11080, 0]
cvO setDir 0
cvO setPos [4934, 11080, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Barrels %1 at %2 dir %3", 6, getPosASL cvO, getDir cvO]
cvO = "Barrel4" createVehicle [4932, 11078, 0]
cvO setDir 0
cvO setPos [4932, 11078, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Barrel4 %1 at %2 dir %3", 7, getPosASL cvO, getDir cvO]
cvO = "Barrel4" createVehicle [4933.2, 11077.3, 0]
cvO setDir 0
cvO setPos [4933.2, 11077.3, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Barrel4 %1 at %2 dir %3", 8, getPosASL cvO, getDir cvO]
cvO = "Barrel1" createVehicle [4935.5, 11077.6, 0]
cvO setDir 0
cvO setPos [4935.5, 11077.6, 0]
cvProps = cvProps + [cvO]
logInfo format ["CVP prop Barrel1 %1 at %2 dir %3", 9, getPosASL cvO, getDir cvO]
logInfo format ["CVP ground 4777 11080 %1", [4777, 11080] call cvGnd]
logInfo format ["CVP ground 4847 11080 %1", [4847, 11080] call cvGnd]
logInfo format ["CVP ground 4935 11080 %1", [4935, 11080] call cvGnd]
logInfo format ["CVP ground 4990 11080 %1", [4990, 11080] call cvGnd]
logInfo format ["CVP ground 5050 11080 %1", [5050, 11080] call cvGnd]
logInfo format ["CVP ground 5075 11080 %1", [5075, 11080] call cvGnd]
logInfo format ["CVP ground 4935 11000 %1", [4935, 11000] call cvGnd]
logInfo format ["CVP ground 4935 11160 %1", [4935, 11160] call cvGnd]
cvG0 = createGroup civilian
victim = objNull
chupa = objNull
"Civilian" createUnit [[4936, 11082, 0], cvG0, "victim = this", 0.5, "PRIVATE"]
"UDChupacabraAnim" createUnit [[5055, 11080, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x disableAI "MOVE" } forEach [victim, chupa]
cvG0 setBehaviour "CARELESS"
victim setPos [4936, 11082, 0]
victim setDir 90
chupa setPos [5055, 11080, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
triSimFrames 90
chupa switchMove "ChupaFreeze"
victim switchMove "Civil"
triSimFrames 20
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.7
cvHud camCommit 0
cvGe = [4900, 11240] call cvGnd
cvGa = [4900, 11080] call cvGnd
triSetView [4900, cvGe + 120, 11240, 0, (cvGa + 0) - (cvGe + 120), -160]
triSimFrames 8
triScreenshot "p2_top"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.7
cvHud camCommit 0
cvGe = [4790, 11000] call cvGnd
cvGa = [4930, 11100] call cvGnd
triSetView [4790, cvGe + 14, 11000, 140, (cvGa + 2) - (cvGe + 14), 100]
triSimFrames 8
triScreenshot "p2_est_wide"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.45
cvHud camCommit 0
cvGe = [4790, 11000] call cvGnd
cvGa = [4930, 11100] call cvGnd
triSetView [4790, cvGe + 14, 11000, 140, (cvGa + 2) - (cvGe + 14), 100]
triSimFrames 8
triScreenshot "p2_est_45"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.7
cvHud camCommit 0
cvGe = [4925, 11079] call cvGnd
cvGa = [5055, 11080] call cvGnd
triSetView [4925, cvGe + 1.8, 11079, 130, (cvGa + 1) - (cvGe + 1.8), 1]
triSimFrames 8
triScreenshot "p2_ots_70"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.2
cvHud camCommit 0
cvGe = [4925, 11079] call cvGnd
cvGa = [5055, 11080] call cvGnd
triSetView [4925, cvGe + 1.8, 11079, 130, (cvGa + 1) - (cvGe + 1.8), 1]
triSimFrames 8
triScreenshot "p2_ots_20"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.06
cvHud camCommit 0
cvGe = [4937, 11081] call cvGnd
cvGa = [5055, 11080] call cvGnd
triSetView [4937, cvGe + 1.7, 11081, 118, (cvGa + 0.8) - (cvGe + 1.7), -1]
triSimFrames 8
triScreenshot "p2_tight_06"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
cvGe = [4950, 11070] call cvGnd
cvGa = [4936, 11082] call cvGnd
triSetView [4950, cvGe + 1.2, 11070, -14, (cvGa + 1.4) - (cvGe + 1.2), 12]
triSimFrames 8
triScreenshot "p2_man_med"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
cvGe = [4842, 11095] call cvGnd
cvGa = [4930, 11070] call cvGnd
triSetView [4842, cvGe + 0.6, 11095, 88, (cvGa + 1.5) - (cvGe + 0.6), -25]
triSimFrames 8
triScreenshot "p2_apron_low"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.35
cvHud camCommit 0
cvGe = [5052, 11092] call cvGnd
cvGa = [4936, 11082] call cvGnd
triSetView [5052, cvGe + 0.4, 11092, -116, (cvGa + 1) - (cvGe + 0.4), -10]
triSimFrames 8
triScreenshot "p2_from_tree_line"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
cvGe = [4870, 11010] call cvGnd
cvGa = [4848, 11082] call cvGnd
triSetView [4870, cvGe + 1, 11010, -22, (cvGa + 1.5) - (cvGe + 1), 72]
triSimFrames 8
triScreenshot "p2_su25"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.6
cvHud camCommit 0
cvGe = [4950, 11115] call cvGnd
cvGa = [4890, 11120] call cvGnd
triSetView [4950, cvGe + 5, 11115, -60, (cvGa + 4) - (cvGe + 5), 5]
triSimFrames 8
triScreenshot "p2_hall"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.6
cvHud camCommit 0
cvGe = [4930, 11025] call cvGnd
cvGa = [4885, 11040] call cvGnd
triSetView [4930, cvGe + 4, 11025, -45, (cvGa + 3) - (cvGe + 4), 15]
triSimFrames 8
triScreenshot "p2_hangar"
triEndTest
