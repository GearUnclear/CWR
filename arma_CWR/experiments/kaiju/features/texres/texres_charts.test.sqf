// ============================================================================
//  Texture resolution charts on the Dedushka proxy (experiment, not a regression test).
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
// Scale props beside the left foot: an empty T-80 and an unarmed soldier.
tvTank = "T80" createVehicle [5067.5, 3868.0, 0]
tvTank setDir 180
tvMan = objNull
"SoldierEB" createUnit [[5063.8, 3862.5, 0], tvEG, "tvMan = this", 0.5, "PRIVATE"]
triSimFrames 5
if (true) then { removeAllWeapons tvMan; tvMan setCaptive true; tvMan disableAI "MOVE"; tvMan setCombatMode "BLUE"; tvMan setBehaviour "CARELESS"; tvMan stop true; tvMan setDir 180 }
// ---- the chart creature ---------------------------------------------------
"UDDedushka" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushka"]
call tvHold
triSimFrames 120
logInfo format ["TEXRES placed %1 asl %2 dir %3 move %4", typeOf ded, getPosASL ded, getDir ded, getMove ded]
// First look at all: level with the chest from 10 m, 35 m from the origin at the feet.
triSetView ([5055.00, 3855.00, 32.00, 5055.00, 3865.00, 32.00] call tvView)
triSimFrames 20
triScreenshot "c00_first_look_chest_10m"
"c00_first_look_chest_10m" call tvNote
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "00_warmup_discard"
"00_warmup_discard" call tvNote
// Walking up to it: 60 m, then 26 m.
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "c12_60m"
"c12_60m" call tvNote
triSetView ([5055.00, 3844.00, 1.70, 5055.00, 3870.00, 14.00] call tvView)
triSimFrames 8
triScreenshot "c11_20m"
"c11_20m" call tvNote
// From a man's eye height at its feet: 4 m in front of the toes.
triSetView ([5050.00, 3860.00, 1.70, 5050.00, 3864.00, 1.00] call tvView)
triSimFrames 20
triScreenshot "c01a_feet_right_toes"
"c01a_feet_right_toes" call tvNote
triSetView ([5050.00, 3860.00, 1.70, 5050.00, 3868.00, 6.50] call tvView)
triSimFrames 8
triScreenshot "c01_feet_right_shin"
"c01_feet_right_shin" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3864.00, 1.00] call tvView)
triSimFrames 8
triScreenshot "c02a_feet_left_toes"
"c02a_feet_left_toes" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3868.00, 6.50] call tvView)
triSimFrames 8
triScreenshot "c02_feet_left_shin"
"c02_feet_left_shin" call tvNote
triSetView ([5050.00, 3860.00, 1.70, 5048.00, 3868.00, 14.00] call tvView)
triSimFrames 8
triScreenshot "c03_feet_right_thigh"
"c03_feet_right_thigh" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5062.00, 3868.00, 14.00] call tvView)
triSimFrames 8
triScreenshot "c04_feet_left_thigh"
"c04_feet_left_thigh" call tvNote
triSetView ([5055.00, 3860.00, 1.70, 5055.00, 3865.80, 24.00] call tvView)
triSimFrames 8
triScreenshot "c05_feet_up_belly"
"c05_feet_up_belly" call tvNote
triSetView ([5055.00, 3860.00, 1.70, 5055.00, 3865.20, 31.00] call tvView)
triSimFrames 8
triScreenshot "c06_feet_up_chest"
"c06_feet_up_chest" call tvNote
triSetView ([5055.00, 3860.00, 1.70, 5055.00, 3865.80, 38.00] call tvView)
triSimFrames 8
triScreenshot "c07_feet_up_head"
"c07_feet_up_head" call tvNote
// Close enough to touch: 1.5 m from the toes. Then straight up from between the feet.
triSetView ([5050.00, 3862.50, 1.70, 5050.00, 3864.00, 1.20] call tvView)
triSimFrames 8
triScreenshot "c09_touch_right_toes"
"c09_touch_right_toes" call tvNote
triSetView ([5060.00, 3862.50, 1.70, 5060.00, 3864.00, 1.20] call tvView)
triSimFrames 8
triScreenshot "c10_touch_left_toes"
"c10_touch_left_toes" call tvNote
triSetView ([5055.00, 3869.00, 1.70, 5055.00, 3870.00, 18.50] call tvView)
triSimFrames 8
triScreenshot "c08_under_pelvis"
"c08_under_pelvis" call tvNote
// Backing away: 150 m, 300 m, 600 m and 1 km.
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "c13_150m"
"c13_150m" call tvNote
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "c14_300m"
"c14_300m" call tvNote
triSetView ([5055.00, 3270.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "c15_600m"
"c15_600m" call tvNote
triSetView ([5055.00, 2870.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "c16_1km"
"c16_1km" call tvNote
// The same far cameras through a 4 times lens (the hold-zoom, binoculars or a scope).
0.175 call tvFov
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 12.00] call tvView)
triSimFrames 8
triScreenshot "z12_60m_lens4"
"z12_60m_lens4" call tvNote
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 30.00] call tvView)
triSimFrames 8
triScreenshot "z12b_60m_lens4_up"
"z12b_60m_lens4_up" call tvNote
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "z14_300m_lens4"
"z14_300m_lens4" call tvNote
triSetView ([5055.00, 2870.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "z16_1km_lens4"
"z16_1km_lens4" call tvNote
0.7 call tvFov
// ---- BEFORE: one 1024 sheet ----------------------------------------------------
deleteVehicle ded
triSimFrames 5
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_00_empty_60m"
"pb_00_empty_60m" call tvNote
triPerfReset 0
"UDDedushkaB" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaB"]
call tvHold
triSimFrames 40
logInfo format ["TEXRES first 45 frames with UDDedushkaB in view at 60 m: %1", triPerfStats 0]
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 20
triScreenshot "pb_01_60m"
"pb_01_60m" call tvNote
triSetView ([5077.50, 3831.00, 2.50, 5055.00, 3870.00, 6.00] call tvView)
triSimFrames 12
triScreenshot "pb_02_scale_props"
"pb_02_scale_props" call tvNote
triSetView ([5070.00, 3851.00, 1.70, 5062.00, 3868.00, 4.00] call tvView)
triSimFrames 12
triScreenshot "pb_03_foot_and_tank"
"pb_03_foot_and_tank" call tvNote
triSetView ([5065.50, 3858.50, 1.70, 5062.00, 3864.50, 1.40] call tvView)
triSimFrames 12
triScreenshot "pb_04_toes_and_soldier"
"pb_04_toes_and_soldier" call tvNote
triSetView ([5060.00, 3862.50, 1.70, 5060.00, 3864.00, 1.20] call tvView)
triSimFrames 12
triScreenshot "pb_04b_touch_toes"
"pb_04b_touch_toes" call tvNote
triSetView ([5055.00, 3856.00, 1.70, 5055.00, 3870.00, 34.00] call tvView)
triSimFrames 12
triScreenshot "pb_05_feet_up"
"pb_05_feet_up" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3868.00, 5.00] call tvView)
triSimFrames 12
triScreenshot "pb_06_shin_from_feet"
"pb_06_shin_from_feet" call tvNote
triSetView ([5055.00, 3869.00, 1.70, 5055.00, 3870.00, 18.50] call tvView)
triSimFrames 12
triScreenshot "pb_07_under_pelvis"
"pb_07_under_pelvis" call tvNote
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_08_150m"
"pb_08_150m" call tvNote
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_09_300m"
"pb_09_300m" call tvNote
triSetView ([5055.00, 3270.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_10_600m"
"pb_10_600m" call tvNote
triSetView ([5055.40, 3270.00, 1.70, 5055.40, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_10s_600m_shifted"
"pb_10s_600m_shifted" call tvNote
triSetView ([5055.00, 2870.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_11_1km"
"pb_11_1km" call tvNote
triSetView ([5055.60, 2870.00, 1.70, 5055.60, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_11s_1km_shifted"
"pb_11s_1km_shifted" call tvNote
0.175 call tvFov
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_12_300m_lens4"
"pb_12_300m_lens4" call tvNote
triSetView ([5055.05, 3570.00, 1.70, 5055.05, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pb_12s_300m_lens4_shifted"
"pb_12s_300m_lens4_shifted" call tvNote
0.7 call tvFov
// ---- AFTER: 4096 sheets by height band ----------------------------------------------------
deleteVehicle ded
triSimFrames 5
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_00_empty_60m"
"pa_00_empty_60m" call tvNote
triPerfReset 0
"UDDedushkaA" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaA"]
call tvHold
triSimFrames 40
logInfo format ["TEXRES first 45 frames with UDDedushkaA in view at 60 m: %1", triPerfStats 0]
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 20
triScreenshot "pa_01_60m"
"pa_01_60m" call tvNote
triSetView ([5077.50, 3831.00, 2.50, 5055.00, 3870.00, 6.00] call tvView)
triSimFrames 12
triScreenshot "pa_02_scale_props"
"pa_02_scale_props" call tvNote
triSetView ([5070.00, 3851.00, 1.70, 5062.00, 3868.00, 4.00] call tvView)
triSimFrames 12
triScreenshot "pa_03_foot_and_tank"
"pa_03_foot_and_tank" call tvNote
triSetView ([5065.50, 3858.50, 1.70, 5062.00, 3864.50, 1.40] call tvView)
triSimFrames 12
triScreenshot "pa_04_toes_and_soldier"
"pa_04_toes_and_soldier" call tvNote
triSetView ([5060.00, 3862.50, 1.70, 5060.00, 3864.00, 1.20] call tvView)
triSimFrames 12
triScreenshot "pa_04b_touch_toes"
"pa_04b_touch_toes" call tvNote
triSetView ([5055.00, 3856.00, 1.70, 5055.00, 3870.00, 34.00] call tvView)
triSimFrames 12
triScreenshot "pa_05_feet_up"
"pa_05_feet_up" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3868.00, 5.00] call tvView)
triSimFrames 12
triScreenshot "pa_06_shin_from_feet"
"pa_06_shin_from_feet" call tvNote
triSetView ([5055.00, 3869.00, 1.70, 5055.00, 3870.00, 18.50] call tvView)
triSimFrames 12
triScreenshot "pa_07_under_pelvis"
"pa_07_under_pelvis" call tvNote
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_08_150m"
"pa_08_150m" call tvNote
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_09_300m"
"pa_09_300m" call tvNote
triSetView ([5055.00, 3270.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_10_600m"
"pa_10_600m" call tvNote
triSetView ([5055.40, 3270.00, 1.70, 5055.40, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_10s_600m_shifted"
"pa_10s_600m_shifted" call tvNote
triSetView ([5055.00, 2870.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_11_1km"
"pa_11_1km" call tvNote
triSetView ([5055.60, 2870.00, 1.70, 5055.60, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_11s_1km_shifted"
"pa_11s_1km_shifted" call tvNote
0.175 call tvFov
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_12_300m_lens4"
"pa_12_300m_lens4" call tvNote
triSetView ([5055.05, 3570.00, 1.70, 5055.05, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pa_12s_300m_lens4_shifted"
"pa_12s_300m_lens4_shifted" call tvNote
0.7 call tvFov
// ---- AFTER with a mip anchor face per sheet ----------------------------------------------------
deleteVehicle ded
triSimFrames 5
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_00_empty_60m"
"pm_00_empty_60m" call tvNote
triPerfReset 0
"UDDedushkaM" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaM"]
call tvHold
triSimFrames 40
logInfo format ["TEXRES first 45 frames with UDDedushkaM in view at 60 m: %1", triPerfStats 0]
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 20
triScreenshot "pm_01_60m"
"pm_01_60m" call tvNote
triSetView ([5077.50, 3831.00, 2.50, 5055.00, 3870.00, 6.00] call tvView)
triSimFrames 12
triScreenshot "pm_02_scale_props"
"pm_02_scale_props" call tvNote
triSetView ([5070.00, 3851.00, 1.70, 5062.00, 3868.00, 4.00] call tvView)
triSimFrames 12
triScreenshot "pm_03_foot_and_tank"
"pm_03_foot_and_tank" call tvNote
triSetView ([5065.50, 3858.50, 1.70, 5062.00, 3864.50, 1.40] call tvView)
triSimFrames 12
triScreenshot "pm_04_toes_and_soldier"
"pm_04_toes_and_soldier" call tvNote
triSetView ([5060.00, 3862.50, 1.70, 5060.00, 3864.00, 1.20] call tvView)
triSimFrames 12
triScreenshot "pm_04b_touch_toes"
"pm_04b_touch_toes" call tvNote
triSetView ([5055.00, 3856.00, 1.70, 5055.00, 3870.00, 34.00] call tvView)
triSimFrames 12
triScreenshot "pm_05_feet_up"
"pm_05_feet_up" call tvNote
triSetView ([5060.00, 3860.00, 1.70, 5060.00, 3868.00, 5.00] call tvView)
triSimFrames 12
triScreenshot "pm_06_shin_from_feet"
"pm_06_shin_from_feet" call tvNote
triSetView ([5055.00, 3869.00, 1.70, 5055.00, 3870.00, 18.50] call tvView)
triSimFrames 12
triScreenshot "pm_07_under_pelvis"
"pm_07_under_pelvis" call tvNote
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_08_150m"
"pm_08_150m" call tvNote
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_09_300m"
"pm_09_300m" call tvNote
triSetView ([5055.00, 3270.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_10_600m"
"pm_10_600m" call tvNote
triSetView ([5055.40, 3270.00, 1.70, 5055.40, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_10s_600m_shifted"
"pm_10s_600m_shifted" call tvNote
triSetView ([5055.00, 2870.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_11_1km"
"pm_11_1km" call tvNote
triSetView ([5055.60, 2870.00, 1.70, 5055.60, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_11s_1km_shifted"
"pm_11s_1km_shifted" call tvNote
0.175 call tvFov
triSetView ([5055.00, 3570.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_12_300m_lens4"
"pm_12_300m_lens4" call tvNote
triSetView ([5055.05, 3570.00, 1.70, 5055.05, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "pm_12s_300m_lens4_shifted"
"pm_12s_300m_lens4_shifted" call tvNote
0.7 call tvFov
// ---- sections: the AFTER body in 5 runs of faces, then in 101 ------------------
// Same sheets, same faces, same camera (60 m). Two rounds, to see the noise.
deleteVehicle ded
triSimFrames 5
"UDDedushkaA" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaA"]
call tvHold
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 200
logInfo format ["TEXRES sections round 1 UDDedushkaA: %1", triPerfStats 0]
deleteVehicle ded
triSimFrames 5
"UDDedushkaS" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaS"]
call tvHold
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 200
logInfo format ["TEXRES sections round 1 UDDedushkaS: %1", triPerfStats 0]
deleteVehicle ded
triSimFrames 5
"UDDedushkaA" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaA"]
call tvHold
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 200
logInfo format ["TEXRES sections round 2 UDDedushkaA: %1", triPerfStats 0]
deleteVehicle ded
triSimFrames 5
"UDDedushkaS" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushkaS"]
call tvHold
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 200
logInfo format ["TEXRES sections round 2 UDDedushkaS: %1", triPerfStats 0]
deleteVehicle ded
triSimFrames 5
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 200
logInfo format ["TEXRES sections no creature: %1", triPerfStats 0]
logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]
triEndTest
