// ============================================================================
//  Detail layer demo frames: the AFTER body up close (experiment, not a regression test).
//  Written by gen_tests.py: edit the shot tables there, not this file.
//
//  Needs @kj_texres (features/texres/build_texres.py), mounted in place of the
//  shared @uddedushka. Run it through run-texres.ps1 (the pipeline's run-tri.ps1
//  with this feature's folders): one game, hidden desktop, --render gl33, quiet.
//  Nobody is armed against anybody and nobody fires.
//
//  Camera rules from tools/screenshots/README.md and the pipeline's photo test:
//  a cutscene camera only to hide the HUD and set the lens, triSetView for
//  framing, a throwaway first frame, a triSimFrames between change and shot.
//  Every TEXSHOT / TEXRES line in game_stdout.log is a measurement.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5060, 4120, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 5000
showCinemaBorder false
// A tri run starts with object shadows off; the props and the town should cast theirs.
// (The test bodies have one visual LOD marked lodnoshadow, so they cast none.)
triEnableShadows
tvC = createCenter civilian
tvG = createGroup civilian
tvE = createCenter east
tvEG = createGroup east
tvAt = [5055, 3870, 0]
tvProbe = "HeliHEmpty" createVehicle [5055, 4200, 0]
tvCam = "camera" camCreate [5055, 3700, 30]
tvCam cameraEffect ["internal", "back"]
tvFov = { tvCam camSetTarget [5055, 3900, 30]; tvCam camSetFov _this; tvCam camCommit 0; tvLens = _this }
0.7 call tvFov
// tvView: [camera x, camera z, eye height above the ground or sea there, target x, target z,
// target height above the creature's feet]. The feet are at tvFeet metres above sea level.
tvFeet = 16.335
tvView = { private ["_cx", "_cz", "_g", "_e"]; _cx = _this select 0; _cz = _this select 1; tvProbe setPos [_cx, _cz, 0]; _g = (getPosASL tvProbe) select 2; _e = _g + (_this select 2); tvLast = [_cx, _e, _cz, _this select 3, tvFeet + (_this select 5), _this select 4]; [_cx, _e, _cz, (_this select 3) - _cx, tvFeet + (_this select 5) - _e, (_this select 4) - _cz] }
tvNote = { logInfo format ["TEXSHOT %1 cam [x,asl,z] to [x,asl,z] %2 lens %3 window %4", _this, tvLast, tvLens, triGetWindowSize] }
ded = objNull
tvHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 180; ded setPos tvAt }
tvTank = "T80" createVehicle [5067.5, 3868.0, 0]
tvTank setDir 180
tvMan = objNull
"SoldierEB" createUnit [[5063.8, 3862.5, 0], tvEG, "tvMan = this", 0.5, "PRIVATE"]
triSimFrames 5
if (true) then { removeAllWeapons tvMan; tvMan setCaptive true; tvMan disableAI "MOVE"; tvMan setCombatMode "BLUE"; tvMan setBehaviour "CARELESS"; tvMan stop true; tvMan setDir 180 }
"UDDedushkaM" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaM"]
call tvHold
triSimFrames 120
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "00_warmup_discard"
"00_warmup_discard" call tvNote
triSetView ([5060.00, 3862.50, 1.70, 5060.00, 3864.00, 1.20] call tvView)
triSimFrames 20
triScreenshot "dd_01_touch_toes_1m5"
"dd_01_touch_toes_1m5" call tvNote
triSetView ([5065.50, 3858.50, 1.70, 5062.00, 3864.50, 1.40] call tvView)
triSimFrames 12
triScreenshot "dd_02_toes_and_soldier"
"dd_02_toes_and_soldier" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3868.00, 5.00] call tvView)
triSimFrames 12
triScreenshot "dd_03_shin_from_feet"
"dd_03_shin_from_feet" call tvNote
triSetView ([5070.00, 3851.00, 1.70, 5062.00, 3868.00, 4.00] call tvView)
triSimFrames 12
triScreenshot "dd_04_foot_and_tank"
"dd_04_foot_and_tank" call tvNote
triSetView ([5055.00, 3856.00, 1.70, 5055.00, 3870.00, 34.00] call tvView)
triSimFrames 12
triScreenshot "dd_05_feet_up"
"dd_05_feet_up" call tvNote
triSetView ([5055.00, 3869.00, 1.70, 5055.00, 3870.00, 18.50] call tvView)
triSimFrames 12
triScreenshot "dd_06_under_pelvis"
"dd_06_under_pelvis" call tvNote
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "dd_07_60m"
"dd_07_60m" call tvNote
triPerfReset 0
triSimFrames 150
logInfo format ["TEXRES detail demo, 150 frames at 60 m: %1", triPerfStats 0]
logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]
triEndTest
