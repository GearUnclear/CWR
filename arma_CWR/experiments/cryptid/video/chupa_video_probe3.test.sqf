// ============================================================================
//  Chupacabra video probe 3: heights, the perimeter road, man moves
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
cvRow = ""
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 10980] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 10980%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11000] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11000%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11020] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11020%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11040] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11040%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11060] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11060%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11080] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11080%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11100] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11100%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11120] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11120%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11140] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11140%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11160] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11160%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11180] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11180%1", cvRow]
cvRow = ""; { cvRow = cvRow + format [" %1", [_x, 11200] call cvGnd] } forEach [4780, 4795, 4810, 4825, 4840, 4855, 4870, 4885, 4900, 4915, 4930, 4945, 4960, 4975, 4990, 5005, 5020, 5035, 5050, 5065, 5080, 5095, 5110, 5125, 5140]; logInfo format ["CVH 11200%1", cvRow]
cvO = "Hangar_Hall" createVehicle [4900, 11120, 0]
cvO setDir 90
cvO setPos [4900, 11120, 0]
cvO = "Barracks" createVehicle [4905, 11045, 0]
cvO setDir 90
cvO setPos [4905, 11045, 0]
cvO = "Su25" createVehicle [4848, 11082, 0]
cvO setDir 180
cvO setPos [4848, 11082, 0]
cvO = "Mi17" createVehicle [4870, 11175, 0]
cvO setDir 225
cvO setPos [4870, 11175, 0]
cvO = "UralRefuel" createVehicle [4872, 11060, 0]
cvO setDir 0
cvO setPos [4872, 11060, 0]
cvO = "Ural" createVehicle [4987, 11097, 0]
cvO setDir 180
cvO setPos [4987, 11097, 0]
cvO = "Barrels" createVehicle [4981, 11082, 0]
cvO setDir 0
cvO setPos [4981, 11082, 0]
cvO = "Barrel4" createVehicle [4980.5, 11079.5, 0]
cvO setDir 0
cvO setPos [4980.5, 11079.5, 0]
cvO = "Barrel4" createVehicle [4981.7, 11078.8, 0]
cvO setDir 0
cvO setPos [4981.7, 11078.8, 0]
cvO = "Barrel1" createVehicle [4982.6, 11080, 0]
cvO setDir 0
cvO setPos [4982.6, 11080, 0]
cvG0 = createGroup civilian
victim = objNull
chupa = objNull
"Civilian" createUnit [[4984, 11080, 0], cvG0, "victim = this", 0.5, "PRIVATE"]
"UDChupacabraAnim" createUnit [[5082, 11080, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x disableAI "MOVE" } forEach [victim, chupa]
cvG0 setBehaviour "CARELESS"
victim setPos [4984, 11080, 0]
victim setDir 90
chupa setPos [5082, 11080, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
triSimFrames 90
chupa switchMove "ChupaFreeze"
victim switchMove "Civil"
triSimFrames 20
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.7
cvHud camCommit 0
cvGe = [4950, 11140] call cvGnd
cvGa = [4990, 11080] call cvGnd
triSetView [4950, cvGe + 40, 11140, 40, (cvGa + 0) - (cvGe + 40), -60]
triSimFrames 8
triScreenshot "p3_top"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.5
cvHud camCommit 0
cvGe = [4975, 11079] call cvGnd
cvGa = [5082, 11080] call cvGnd
triSetView [4975, cvGe + 1.7, 11079, 107, (cvGa + 1) - (cvGe + 1.7), 1]
triSimFrames 8
triScreenshot "p3_ots_50"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.15
cvHud camCommit 0
cvGe = [4983, 11079.5] call cvGnd
cvGa = [5082, 11080] call cvGnd
triSetView [4983, cvGe + 1.75, 11079.5, 99, (cvGa + 1) - (cvGe + 1.75), 0.5]
triSimFrames 8
triScreenshot "p3_pov_15"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.05
cvHud camCommit 0
cvGe = [4985, 11080] call cvGnd
cvGa = [5082, 11080] call cvGnd
triSetView [4985, cvGe + 1.7, 11080, 97, (cvGa + 0.9) - (cvGe + 1.7), 0]
triSimFrames 8
triScreenshot "p3_pov_05"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.45
cvHud camCommit 0
cvGe = [4995, 11070] call cvGnd
cvGa = [4984, 11080] call cvGnd
triSetView [4995, cvGe + 1.4, 11070, -11, (cvGa + 1.3) - (cvGe + 1.4), 10]
triSimFrames 8
triScreenshot "p3_man"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.35
cvHud camCommit 0
cvGe = [5030, 11060] call cvGnd
cvGa = [4984, 11080] call cvGnd
triSetView [5030, cvGe + 1, 11060, -46, (cvGa + 1.3) - (cvGe + 1), 20]
triSimFrames 8
triScreenshot "p3_slope_to_man"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.6
cvHud camCommit 0
cvGe = [4960, 11050] call cvGnd
cvGa = [4890, 11110] call cvGnd
triSetView [4960, cvGe + 6, 11050, -70, (cvGa + 2) - (cvGe + 6), 60]
triSimFrames 8
triScreenshot "p3_base_from_road"
cvHud camSetTarget [4900, 14000, 30]
cvHud camSetFov 0.6
cvHud camCommit 0
cvGe = [4986, 11040] call cvGnd
cvGa = [4900, 11110] call cvGnd
triSetView [4986, cvGe + 18, 11040, -86, (cvGa + 0) - (cvGe + 18), 70]
triSimFrames 8
triScreenshot "p3_est"
victim switchMove "CivilPutDown"
logInfo format ["CVP move CivilPutDown -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilPutDown_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilPutDown_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilPutDown_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "FXStandDip"
logInfo format ["CVP move FXStandDip -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandDip_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandDip_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandDip_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "EffectStandTalk"
logInfo format ["CVP move EffectStandTalk -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_EffectStandTalk_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_EffectStandTalk_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_EffectStandTalk_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "CivilStillV1"
logInfo format ["CVP move CivilStillV1 -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilStillV1_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilStillV1_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_CivilStillV1_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "FXStandBug"
logInfo format ["CVP move FXStandBug -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandBug_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandBug_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandBug_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "FXStandSur"
logInfo format ["CVP move FXStandSur -> %1", getMove victim]
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandSur_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandSur_010"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]
triScreenshot "p3_mv_FXStandSur_020"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim switchMove "Civil"
triSimFrames 5
victim switchMove "CivilToCivilLying"
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_down_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_down_015"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_down_030"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
victim playMove "CivilLyingToCivil"
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_up_000"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_up_015"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]
triScreenshot "p3_mv_up_030"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
setAccTime 1
triEndTest
