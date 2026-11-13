// ============================================================================
//  Dedushka BRAIN: the demo mission played through on its own timeline
//  (experiment, kept out of tests/integration). DedDemo.Eden: he stands dormant
//  west of Morton, rises, roars, walks east along the main row of houses, is
//  "hit" by the tank (the reaction is ordered, no shot is fired), turns to it,
//  roars, beams the armour, is spent, turns back to his line, walks out the
//  east side and goes dormant again.
//
//  Run alone (the brain's stubs for roar and beam):
//      features\brain\run-brain.ps1 -Name brain_demo -Mission DedDemo.Eden
//  With the other features' payloads copied in, the same test drives their
//  entry points (see run-brain.ps1 -Full).
//
//  To keep the run short the test moves him 100 m nearer the town while he is
//  still dormant, brings the exit point in, and shortens the spent time. The
//  mission itself plays the full length.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJB_ready") }
triSimUntil { !(isNull kaiju) }
if (true) then { KJD_exit = [5200, 4008, 0]; KJD_hold = 3; KJB_SPENT = 10; KJ_beamOverride = { KJ_beamT_spent = 12 }; kaiju setPos [4890, 4004, 0]; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs" }
kjCam = "camera" camCreate [4990, 3985, 3]
kjCam cameraEffect ["internal", "back"]
showCinemaBorder false
if (true) then { kjCam camSetTarget [4890, 4004, 20]; kjCam camSetFov 0.7; kjCam camCommit 0 }
// kjView: [camera x, camera z, eye height over the shelf, aim height over his feet]
kjView = { private ["_p"]; _p = getPosASL kaiju; [_this select 0, 16.3 + (_this select 2), _this select 1, (_p select 0) - (_this select 0), (_p select 2) + (_this select 3) - 16.3 - (_this select 2), (_p select 1) - (_this select 1)] }
triSetView ([4985, 3984, 1.7, 20] call kjView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// ---- dormant, seen from the west entrance of the town ---------------------------
if (true) then { kjT0 = time }
triSimUntil { ((KJ_state == "dormant") && (KJ_brainPhase == "still")) || (time > kjT0 + 20) }
triAssertEq [KJ_state, "dormant"]
triSimFrames 60
triSetView ([4985, 3984, 1.7, 20] call kjView)
triSimFrames 4
triScreenshot "d01_dormant_from_the_street"
logInfo format ["KJD dormant: pos %1 move %2 step %3 garrison alive %4", getPos kaiju, getMove kaiju, KJD_step, { alive _x } count [kjdA1, kjdB1, kjdTank, kjdTank2, kjdBmp]]

// ---- wake: the rise and the roar ----------------------------------------------------
triSimUntil { (KJ_state == "roar") || (time > kjT0 + 60) }
triAssertEq [KJ_state, "roar"]
if (true) then { kjT1 = time }
triSimUntil { time > kjT1 + 4.5 }
triSetView ([4985, 3984, 1.7, 26] call kjView)
triSimFrames 4
triScreenshot "d02_risen_roaring"
logInfo format ["KJD roar at %1 s, move %2", kjT1 - kjT0, getMove kaiju]

// ---- through the row ------------------------------------------------------------------
triSimUntil { (KJD_step == "through") || (time > kjT0 + 90) }
triAssertEq [KJD_step, "through"]
triSimUntil { (((getPos kaiju) select 0) > 4925) || (time > kjT0 + 130) }
triSetView ([5000, 3982, 1.7, 18] call kjView)
triSimFrames 4
triScreenshot "d03_walking_into_town"
triSimUntil { (((getPos kaiju) select 0) > 4962) || (time > kjT0 + 150) }
triSetView ([5010, 3978, 1.7, 22] call kjView)
triSimFrames 4
triScreenshot "d04_among_the_first_houses"
logInfo format ["KJD through: at %1 after %2 s, dir %3, state %4", getPos kaiju, time - kjT0, getDir kaiju, KJ_state]
triAssertEq [KJ_state, "drawn"]
triAssert [(abs (((getPos kaiju) select 1) - 4004)) < 4]

// ---- the hit: shudder, stand, turn to the tank -------------------------------------------
triSimUntil { (KJ_state == "flinch") || (time > kjT0 + 200) }
triAssertEq [KJ_state, "flinch"]
if (true) then { kjT1 = time }
triSimUntil { (KJ_brainPhase == "staring") || (time > kjT1 + 30) }
triSetView ([5090, 3955, 8, 24] call kjView)
triSimFrames 4
triScreenshot "d05_hit_he_stands_and_stares"
logInfo format ["KJD hit: staring %1 s after the hit, pos %2", time - kjT1, getPos kaiju]

// ---- the beam ----------------------------------------------------------------------------------
triSimUntil { (KJ_brainPhase == "firing") || (time > kjT1 + 90) }
triAssertEq [KJ_state, "beam"]
logInfo format ["KJD beam: firing %1 s after the hit, facing %2, tank bears %3", time - kjT1, getDir kaiju, [getPos kaiju, getPos kjdTank] call KJB_fnBear]
triAssert [(abs ([getDir kaiju, [getPos kaiju, getPos kjdTank] call KJB_fnBear] call KJB_fnAng)) < 6]
if (true) then { kjT2 = time }
triSimUntil { time > kjT2 + 9.5 }
triSetView ([4953, 3985, 1.7, 30] call kjView)
triSimFrames 4
triScreenshot "d06_beam_stands_tall_from_the_street"
triSetView ([5090, 3955, 8, 26] call kjView)
triSimFrames 4
triScreenshot "d06b_beam_stands_tall_from_the_armour"
triSimUntil { (KJ_state == "spent") || (time > kjT2 + 120) }
triAssertEq [KJ_state, "spent"]
triSimFrames 120
triSetView ([5090, 3955, 8, 22] call kjView)
triSimFrames 4
triScreenshot "d07_spent"

// ---- out the other side --------------------------------------------------------------------------
triSimUntil { (KJD_step == "out") || (time > kjT2 + 240) }
triAssertEq [KJD_step, "out"]
if (true) then { kjT3 = time }
triSimUntil { (((getPos kaiju) select 0) > 5075) || (time > kjT3 + 90) }
triSetView ([5150, 3985, 1.7, 22] call kjView)
triSimFrames 4
triScreenshot "d08_through_the_row"
triSimUntil { (((getPos kaiju) select 0) > 5150) || (time > kjT3 + 140) }
triSetView ([5100, 3978, 1.7, 22] call kjView)
triSimFrames 4
triScreenshot "d09_out_the_east_side"
triSimUntil { (KJD_step == "done") || (time > kjT3 + 220) }
logInfo format ["KJD end: step %1 state %2 phase %3 pos %4 after %5 s, track %6", KJD_step, KJ_state, KJ_brainPhase, getPos kaiju, time - kjT0, KJ_brainTrack]
logInfo format ["KJD end: states %1", KJT_states]
logInfo format ["KJT jumps (time, metres, seconds, from, to, men, vehicles): %1", KJT_jumps]
logInfo format ["KJD end: top speed %1, alive %2, damage %3, garrison alive %4", KJT_maxSpeed, alive kaiju, getDammage kaiju, { alive _x } count [kjdA1, kjdB1, kjdTank, kjdTank2, kjdBmp]]
triAssertEq [KJD_step, "done"]
triAssertEq [KJ_state, "dormant"]
triAssert [((getPos kaiju) select 0) > 5170]
triAssert [(abs (((getPos kaiju) select 1) - 4006)) < 20]
triAssert [alive kaiju]
// never faster than a walk by his own doing: a higher reading must come with a recorded push (KJT_jumps)
triAssert [(KJT_maxSpeed < 3.2) || ((count KJT_jumps) > 0)]
triAssert ["roar" in KJT_states]
triAssert ["flinch" in KJT_states]
triAssert ["beam" in KJT_states]
triAssert ["spent" in KJT_states]
triAssert [({ _x == "drawn" } count KJT_states) >= 2]
triSetView ([5150, 3985, 1.7, 20] call kjView)
triSimFrames 4
triScreenshot "d10_dormant_on_the_far_side"
if (true) then { KJT_on = false; KJ_brainOn = false }
triEndTest
