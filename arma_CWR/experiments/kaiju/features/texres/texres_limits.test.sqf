// ============================================================================
//  Texture limits: one 8 m panel per format and size (experiment, not a regression test).
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
pan = objNull
logInfo format ["TEXRES aspect [leftFOV,topFOV,ui...] %1 window %2 cutscene lens %3", triGetAspectSettings, triGetWindowSize, triCamFov]
triSimFrames 120
triSetView ([5055.00, 3810.00, 1.70, 5055.00, 3870.00, 20.00] call tvView)
triSimFrames 8
triScreenshot "00_warmup_discard"
"00_warmup_discard" call tvNote
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 60
triPerfReset 0
triSimFrames 60
logInfo format ["TEXRES baseline, no panel: %1 memory %2 MB", triPerfStats 0, triMemoryMB]
// ---- d1_1024
if (true) then { pan = "KJTxp_d1_1024" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_1024_a_overview"
"l_d1_1024_a_overview" call tvNote
logInfo format ["TEXRES d1_1024 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_1024_b_close_left"
"l_d1_1024_b_close_left" call tvNote
logInfo format ["TEXRES d1_1024 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d1_1024_c_close_right"
"l_d1_1024_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d1_1024_d_ramps"
"l_d1_1024_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- d1_2048
if (true) then { pan = "KJTxp_d1_2048" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_2048_a_overview"
"l_d1_2048_a_overview" call tvNote
logInfo format ["TEXRES d1_2048 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_2048_b_close_left"
"l_d1_2048_b_close_left" call tvNote
logInfo format ["TEXRES d1_2048 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d1_2048_c_close_right"
"l_d1_2048_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d1_2048_d_ramps"
"l_d1_2048_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- d1_4096
if (true) then { pan = "KJTxp_d1_4096" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_4096_a_overview"
"l_d1_4096_a_overview" call tvNote
logInfo format ["TEXRES d1_4096 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_4096_b_close_left"
"l_d1_4096_b_close_left" call tvNote
logInfo format ["TEXRES d1_4096 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d1_4096_c_close_right"
"l_d1_4096_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d1_4096_d_ramps"
"l_d1_4096_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- d5_2048
if (true) then { pan = "KJTxp_d5_2048" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d5_2048_a_overview"
"l_d5_2048_a_overview" call tvNote
logInfo format ["TEXRES d5_2048 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d5_2048_b_close_left"
"l_d5_2048_b_close_left" call tvNote
logInfo format ["TEXRES d5_2048 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d5_2048_c_close_right"
"l_d5_2048_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d5_2048_d_ramps"
"l_d5_2048_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- a8_1024
if (true) then { pan = "KJTxp_a8_1024" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_a8_1024_a_overview"
"l_a8_1024_a_overview" call tvNote
logInfo format ["TEXRES a8_1024 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_a8_1024_b_close_left"
"l_a8_1024_b_close_left" call tvNote
logInfo format ["TEXRES a8_1024 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_a8_1024_c_close_right"
"l_a8_1024_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_a8_1024_d_ramps"
"l_a8_1024_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- png_2048
if (true) then { pan = "KJTxp_png_2048" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_png_2048_a_overview"
"l_png_2048_a_overview" call tvNote
logInfo format ["TEXRES png_2048 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_png_2048_b_close_left"
"l_png_2048_b_close_left" call tvNote
logInfo format ["TEXRES png_2048 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_png_2048_c_close_right"
"l_png_2048_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_png_2048_d_ramps"
"l_png_2048_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- png_4096
if (true) then { pan = "KJTxp_png_4096" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_png_4096_a_overview"
"l_png_4096_a_overview" call tvNote
logInfo format ["TEXRES png_4096 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_png_4096_b_close_left"
"l_png_4096_b_close_left" call tvNote
logInfo format ["TEXRES png_4096 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_png_4096_c_close_right"
"l_png_4096_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_png_4096_d_ramps"
"l_png_4096_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- tga_1024
if (true) then { pan = "KJTxp_tga_1024" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_tga_1024_a_overview"
"l_tga_1024_a_overview" call tvNote
logInfo format ["TEXRES tga_1024 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_tga_1024_b_close_left"
"l_tga_1024_b_close_left" call tvNote
logInfo format ["TEXRES tga_1024 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_tga_1024_c_close_right"
"l_tga_1024_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_tga_1024_d_ramps"
"l_tga_1024_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- jpg_2048
if (true) then { pan = "KJTxp_jpg_2048" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_jpg_2048_a_overview"
"l_jpg_2048_a_overview" call tvNote
logInfo format ["TEXRES jpg_2048 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_jpg_2048_b_close_left"
"l_jpg_2048_b_close_left" call tvNote
logInfo format ["TEXRES jpg_2048 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_jpg_2048_c_close_right"
"l_jpg_2048_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_jpg_2048_d_ramps"
"l_jpg_2048_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- d1m_4096
if (true) then { pan = "KJTxp_d1m_4096" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d1m_4096_a_overview"
"l_d1m_4096_a_overview" call tvNote
logInfo format ["TEXRES d1m_4096 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d1m_4096_b_close_left"
"l_d1m_4096_b_close_left" call tvNote
logInfo format ["TEXRES d1m_4096 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_c_close_right"
"l_d1m_4096_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_d_ramps"
"l_d1m_4096_d_ramps" call tvNote
triSetView ([5055.00, 3820.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_50m"
"l_d1m_4096_far_50m" call tvNote
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_150m"
"l_d1m_4096_far_150m" call tvNote
triSetView ([5055.00, 3470.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_400m"
"l_d1m_4096_far_400m" call tvNote
triSetView ([5055.00, 3070.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_800m"
"l_d1m_4096_far_800m" call tvNote
0.175 call tvFov
triSetView ([5055.00, 3720.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_150m_lens4"
"l_d1m_4096_far_150m_lens4" call tvNote
triSetView ([5055.00, 3470.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_400m_lens4"
"l_d1m_4096_far_400m_lens4" call tvNote
triSetView ([5055.00, 3070.00, 1.70, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 8
triScreenshot "l_d1m_4096_far_800m_lens4"
"l_d1m_4096_far_800m_lens4" call tvNote
0.7 call tvFov
deleteVehicle pan
triSimFrames 5
// ---- a4_1024
if (true) then { pan = "KJTxp_a4_1024" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_a4_1024_a_overview"
"l_a4_1024_a_overview" call tvNote
logInfo format ["TEXRES a4_1024 overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_a4_1024_b_close_left"
"l_a4_1024_b_close_left" call tvNote
logInfo format ["TEXRES a4_1024 close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_a4_1024_c_close_right"
"l_a4_1024_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_a4_1024_d_ramps"
"l_a4_1024_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// ---- d1_8192w
if (true) then { pan = "KJTxp_d1_8192w" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }
triPerfReset 0
triSetView ([5055.00, 3856.00, 4.50, 5055.00, 3870.00, 4.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_8192w_a_overview"
"l_d1_8192w_a_overview" call tvNote
logInfo format ["TEXRES d1_8192w overview, first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triPerfReset 0
triSetView ([5053.00, 3869.55, 1.50, 5053.00, 3870.00, 1.50] call tvView)
triSimFrames 20
triScreenshot "l_d1_8192w_b_close_left"
"l_d1_8192w_b_close_left" call tvNote
logInfo format ["TEXRES d1_8192w close-up (full size level wanted), first 20 frames: %1 memory %2 MB errors %3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]
triSetView ([5057.00, 3869.55, 1.50, 5057.00, 3870.00, 1.50] call tvView)
triSimFrames 8
triScreenshot "l_d1_8192w_c_close_right"
"l_d1_8192w_c_close_right" call tvNote
triSetView ([5055.00, 3867.00, 2.80, 5055.00, 3870.00, 2.80] call tvView)
triSimFrames 8
triScreenshot "l_d1_8192w_d_ramps"
"l_d1_8192w_d_ramps" call tvNote
deleteVehicle pan
triSimFrames 5
// The player's own lens, for the pixels per metre a walking player gets.
tvCam cameraEffect ["terminate", "back"]
triClearView
triSimFrames 30
logInfo format ["TEXRES player view: lens %1 camera %2", triCamFov, triCamView]
logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]
triEndTest
