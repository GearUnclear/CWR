// ============================================================================
//  Chupacabra texture shoot (experiment, not a regression test). Close views
//  of UDChupacabra in soft daylight for checking the painted texture: a full
//  orbit, head and claw close-ups, and a prone frame. Same mission and mod as
//  tests/integration/scripting/cryptid_model_spawn (ChupaModel.Eden, @udchupa).
//  Camera rules from tools/screenshots/README.md: a cutscene camera only to
//  hide the HUD and set a long lens, triSetView for framing, presented-frame
//  capture, a throwaway first frame, literal screenshot labels.
//  Quiet: no weapons, nobody fires, --nosound.
// ============================================================================
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
player setCaptive true
removeAllWeapons player
// Far off, so the creature's head never turns to track the player.
player setPos [4800, 10000, 0]
setDate [1985, 5, 10, 10, 30]
0 setOvercast 0.3
0 setFog 0
setViewDistance 1200
showCinemaBorder false
chC = createCenter civilian
chG = createGroup civilian
chupa = objNull
chAt = [4800, 11000, 0]
"UDChupacabra" createUnit [chAt, chG, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull chupa)]
triAssertEq [typeOf chupa, "UDChupacabra"]
// Hold it still, facing the late-morning sun (south-east), looking ahead.
chPose = { chupa setCaptive true; chupa disableAI "MOVE"; chupa disableAI "TARGET"; chupa disableAI "AUTOTARGET"; chupa setBehaviour "CARELESS"; chupa stop true; chupa setDir 120; chupa setPos chAt; chupa doWatch [(chAt select 0) + 200 * sin 120, (chAt select 1) + 200 * cos 120, 1.7] }
call chPose
// HUD off and a long lens (camSetTarget first, or camSetFov is ignored).
chCam = "camera" camCreate [4800, 10500, 30]
chCam cameraEffect ["internal", "back"]
chCam camSetTarget [4800, 10700, 30]
chCam camSetFov 0.4
chCam camCommit 0
// Orbit view: ctA degrees round from the creature's front, ctR metres out,
// eye ctH and aim point ctT metres above its feet.
ctView = { private ["_p", "_d", "_cx", "_cz"]; _p = getPosASL chupa; _d = (getDir chupa) + ctA; _cx = (_p select 0) + ctR * sin _d; _cz = (_p select 1) + ctR * cos _d; [_cx, (_p select 2) + ctH, _cz, (_p select 0) - _cx, ctT - ctH, (_p select 1) - _cz] }
// Sky and pose settle.
triSimFrames 200
if (true) then { ctA = 0; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "00_warmup_discard"
// -- full body orbit -----------------------------------------------------------
if (true) then { ctA = 0; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "01_front"
if (true) then { ctA = -40; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "02_front_34_left"
if (true) then { ctA = 40; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "03_front_34_right"
if (true) then { ctA = 90; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "04_side_right"
if (true) then { ctA = 145; ctR = 5; ctH = 1.2; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "05_back_34"
if (true) then { ctA = 180; ctR = 5; ctH = 1.2; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "06_back"
if (true) then { ctA = -90; ctR = 5; ctH = 1.1; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "07_side_left"
// -- close-ups ---------------------------------------------------------------
if (true) then { ctA = -20; ctR = 1.7; ctH = 1.75; ctT = 1.62 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "08_head_front"
if (true) then { ctA = 80; ctR = 1.7; ctH = 1.7; ctT = 1.62 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "09_head_profile"
if (true) then { ctA = 170; ctR = 1.9; ctH = 1.7; ctT = 1.35 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "10_spines_back"
if (true) then { ctA = -35; ctR = 2.0; ctH = 1.15; ctT = 0.95 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "11_torso_claws"
if (true) then { ctA = 25; ctR = 2.4; ctH = 0.35; ctT = 1.2 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "12_low_angle"
// -- prone -------------------------------------------------------------------
chupa setUnitPos "DOWN"
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA texture-shot prone move %1", getMove chupa]
if (true) then { ctA = -55; ctR = 3.2; ctH = 1.9; ctT = 0.15 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "13_prone_above"
if (true) then { ctA = 10; ctR = 2.2; ctH = 0.45; ctT = 0.25 }
triSetView ([] call ctView)
triSimFrames 6
triScreenshot "14_prone_face"
triAssert [alive chupa]
logInfo format ["CHUPA texture-shot end time %1 move %2", time, getMove chupa]
triEndTest
