// ============================================================================
//  Dedushka BRAIN: the state machine and the walk, alone (experiment, kept out
//  of tests/integration). No other feature's files are in the mission, so the
//  roar, the beam and the hit signal are the brain's stubs. Quiet: nobody is
//  armed against anybody, nobody fires, the one hit is a scripted setDammage.
//
//  Run:  features\brain\run-brain.ps1 -Name brain_states
//  Needs the mount tmp\@kj_brain (features\brain\build-kj-brain.ps1).
//
//  What it proves, in order (each KJT line in game_stdout.log is a measurement):
//   1. he starts dormant and stays put in the dormant clip
//   2. ordered awake he takes his time, roars, and stands
//   3. ordered to a point 110 m ahead he walks there in whole strides, on a
//      straight line, at a walk, and stops within a stride of it
//   4. ordered to a point behind him he turns on the spot first, slowly,
//      without leaving the spot, then walks
//   5. a hard hit: he shudders, stands, turns to where it came from and is
//      drawn there
//   6. ordered to beam he stops, turns to it, roars, stands tall, and is spent
//   7. left to himself, a scripted rise in damage stops him and is healed
//   8. sent at the sea he stops on dry land
//  Site: the flat shelf of Morton (Everon), 16.3 m above the sea, north of the
//  main row of houses (measured with a grid probe, see the brain NOTES.md).
// ============================================================================
triSimUntil { alive player }
player allowDammage false
setViewDistance 2500
kjG = createGroup east
ded = objNull
"UDDedushka" createUnit [[4880, 4050, 0], kjG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setCaptive true; ded setDir 90; ded setPos [4880, 4050, 0] }
kjTank = "T80" createVehicle [5100, 4060, 0]
// HUD off. kjView: [camera x, camera z, eye height over the shelf, aim height over his feet].
kjCam = "camera" camCreate [4950, 3980, 3]
kjCam cameraEffect ["internal", "back"]
showCinemaBorder false
if (true) then { kjCam camSetTarget [4880, 4050, 20]; kjCam camSetFov 0.7; kjCam camCommit 0 }
kjView = { private ["_p"]; _p = getPosASL ded; [_this select 0, 16.3 + (_this select 2), _this select 1, (_p select 0) - (_this select 0), (_p select 2) + (_this select 3) - 16.3 - (_this select 2), (_p select 1) - (_this select 1)] }
// The brain in director mode: he does only what he is told. Short spent time for the test.
if (true) then { KJ_brainAuto = false; KJB_SPENT = 8; KJT_on = true; [ded] exec "kj\brain\start.sqs" }
triSimUntil { !(isNil "KJB_ready") }
[ded] exec "kj\brain\watch.sqs"
triSetView ([4930, 3990, 1.7, 22] call kjView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// ---- 1. dormant --------------------------------------------------------------
if (true) then { kjT0 = time }
triSimUntil { ((KJ_state == "dormant") && (KJ_brainPhase == "still")) || (time > kjT0 + 20) }
triSimFrames 120
logInfo format ["KJT 1 dormant: state %1 phase %2 move %3 pos %4 moves found: idle %5 walk %6 turn %7", KJ_state, KJ_brainPhase, getMove ded, getPos ded, KJB_mIdle, KJB_mWalk, KJB_mTurnL]
triAssertEq [KJ_state, "dormant"]
triAssertEq [KJ_brainPhase, "still"]
triAssert [(getMove ded) == KJB_mDormant]
triAssert [([getPos ded, [4880, 4050, 0]] call KJB_fnDist) < 1]
triSetView ([4945, 4080, 1.7, 22] call kjView)
triSimFrames 4
triScreenshot "01_dormant"

// ---- 2. wake: the rise, the roar, awake ----------------------------------------
if (true) then { kjT0 = time; KJ_brainOrder = ["wake"] }
triSimUntil { (KJ_state == "roar") || (time > kjT0 + 45) }
logInfo format ["KJT 2 wake: roar after %1 s, move %2, phases %3", time - kjT0, getMove ded, KJT_phases]
triAssertEq [KJ_state, "roar"]
triAssert [(time - kjT0) > 6]
triSimFrames 240
triSetView ([4945, 4080, 1.7, 26] call kjView)
triSimFrames 4
triScreenshot "02_risen_roar"
triSimUntil { (KJ_state == "idle") || (time > kjT0 + 70) }
logInfo format ["KJT 2 awake after %1 s: state %2 move %3 moved %4 m", time - kjT0, KJ_state, getMove ded, [getPos ded, [4880, 4050, 0]] call KJB_fnDist]
triAssertEq [KJ_state, "idle"]
triAssert [([getPos ded, [4880, 4050, 0]] call KJB_fnDist) < 1]

// ---- 3. drawn: 110 m straight ahead --------------------------------------------
if (true) then { kjT0 = time; kjDev = 0; kjS0 = KJ_brainTrack select 0; KJ_brainOrder = ["drawn", [4990, 4050, 0]] }
triSimUntil { (KJ_state == "drawn") || (time > kjT0 + 5) }
triAssertEq [KJ_state, "drawn"]
triSimUntil { (((getPos ded) select 0) > 4912) || (time > kjT0 + 40) }
triSetView ([4960, 3975, 1.7, 18] call kjView)
triSimFrames 4
triScreenshot "03_walk_stride_a"
triSimUntil { (((getPos ded) select 0) > 4930) || (time > kjT0 + 50) }
triSetView ([4960, 3975, 1.7, 18] call kjView)
triSimFrames 4
triScreenshot "04_walk_stride_b"
triSimUntil { kjD = abs (((getPos ded) select 1) - 4050); if (kjD > kjDev) then { kjDev = kjD }; (KJ_state == "idle") || (time > kjT0 + 100) }
logInfo format ["KJT 3 drawn: %1 s, pos %2, off the line at most %3 m, dir %4, strides %5, top speed %6 m/s, stride now %7 m", time - kjT0, getPos ded, kjDev, getDir ded, (KJ_brainTrack select 0) - kjS0, KJT_maxSpeed, KJB_stride]
triAssertEq [KJ_state, "idle"]
triAssert [kjDev < 2]
triAssert [([getPos ded, [4990, 4050, 0]] call KJB_fnDist) < 20]
triAssert [((KJ_brainTrack select 0) - kjS0) >= 6]
triAssert [KJT_maxSpeed < 3.2]
triAssert [KJT_maxSpeed > 1.2]
triAssert [(abs ([getDir ded, 90] call KJB_fnAng)) < 4]

// ---- 4. a goal behind him: turn on the spot, then walk --------------------------
if (true) then { kjT0 = time; kjTurns = KJ_brainTrack select 1; kjP0 = getPos ded; kjGoal = [(kjP0 select 0) - 33, (kjP0 select 1) + 60, 0]; kjBear = [kjP0, kjGoal] call KJB_fnBear; KJ_brainOrder = ["drawn", kjGoal] }
triSimUntil { (KJB_mv == "turn") || (time > kjT0 + 10) }
triAssertEq [KJB_mv, "turn"]
triSimUntil { time > kjT0 + 6 }
triSetView ([(kjP0 select 0) + 10, 4112, 10, 20] call kjView)
triSimFrames 4
triScreenshot "05_turning_on_the_spot"
triSimUntil { (KJB_mv != "turn") || (time > kjT0 + 45) }
logInfo format ["KJT 4 turn: %1 s, dir %2 (wanted %3), shifted %4 m, turns %5, move %6", time - kjT0, getDir ded, kjBear, [getPos ded, kjP0] call KJB_fnDist, (KJ_brainTrack select 1) - kjTurns, getMove ded]
triAssert [([getPos ded, kjP0] call KJB_fnDist) < 1.5]
triAssert [(abs ([getDir ded, kjBear] call KJB_fnAng)) < 3]
triAssert [(time - kjT0) > 8]
triAssertEq [(KJ_brainTrack select 1) - kjTurns, 1]
triSimUntil { (KJ_state == "idle") || (time > kjT0 + 100) }
logInfo format ["KJT 4 arrived: %1 s, %2 m from the goal", time - kjT0, [getPos ded, kjGoal] call KJB_fnDist]
triAssertEq [KJ_state, "idle"]
triAssert [([getPos ded, kjGoal] call KJB_fnDist) < 20]

// ---- 5. a hard hit from the tank's side -----------------------------------------
if (true) then { kjT0 = time; KJ_brainOrder = ["hit", kjTank] }
triSimUntil { (KJ_state == "flinch") || (time > kjT0 + 5) }
triAssertEq [KJ_state, "flinch"]
triSimUntil { (KJ_state == "drawn") || (time > kjT0 + 70) }
logInfo format ["KJT 5 hit: drawn after %1 s to %2 (tank at %3), phases %4, dir %5", time - kjT0, KJ_brainDraw, getPos kjTank, KJT_phases, getDir ded]
triAssertEq [KJ_state, "drawn"]
triAssert [([KJ_brainDraw, getPos kjTank] call KJB_fnDist) < 5]
triAssert ["shudder" in KJT_phases]
triAssert ["staring" in KJT_phases]
triAssert [(time - kjT0) > 6]

// ---- 6. the beam (the brain's stub: stands tall, then spent) ---------------------
if (true) then { kjT0 = time; KJ_brainOrder = ["beam", kjTank] }
triSimUntil { (KJ_brainPhase == "firing") || (time > kjT0 + 70) }
logInfo format ["KJT 6 beam: firing after %1 s, state %2, facing %3 (tank bears %4)", time - kjT0, KJ_state, getDir ded, [getPos ded, getPos kjTank] call KJB_fnBear]
triAssertEq [KJ_state, "beam"]
triAssertEq [KJ_brainPhase, "firing"]
triAssert [(abs ([getDir ded, [getPos ded, getPos kjTank] call KJB_fnBear] call KJB_fnAng)) < 6]
if (true) then { kjT1 = time }
triSimUntil { time > kjT1 + 9 }
logInfo format ["KJT 6 beam: standing tall, move %1, KJ_beamActive %2", getMove ded, KJ_beamActive]
triAssert [KJ_beamActive]
triAssert [(getMove ded) == "DedBeamHold"]
triSetView ([((getPos ded) select 0) - 30, ((getPos ded) select 1) + 70, 12, 24] call kjView)
triSimFrames 4
triScreenshot "06_beam_stands_tall"
triSimUntil { (KJ_state == "spent") || (time > kjT0 + 120) }
triAssertEq [KJ_state, "spent"]
triSimUntil { (KJ_state == "idle") || (time > kjT0 + 150) }
logInfo format ["KJT 6 spent over after %1 s, state %2, move %3, states so far %4", time - kjT0, KJ_state, getMove ded, KJT_states]
triAssertEq [KJ_state, "idle"]

// ---- 7. left to himself: a scripted rise in damage -------------------------------
if (true) then { kjT0 = time; KJ_brainOrder = ["auto", true] }
triSimFrames 20
ded setDammage 0.02
triSimUntil { (KJ_state == "flinch") || (time > kjT0 + 10) }
logInfo format ["KJT 7 scripted hit: state %1 after %2 s, damage now %3, hits seen %4", KJ_state, time - kjT0, getDammage ded, KJ_hurtCount]
triAssertEq [KJ_state, "flinch"]
triAssert [(getDammage ded) < 0.001]
triAssert [alive ded]
if (true) then { KJ_brainOrder = ["auto", false] }
triSimFrames 10
if (true) then { kjT0 = time; KJ_brainOrder = ["idle"] }
triSimUntil { ((KJ_state == "idle") && (KJB_mv == "stand") && (KJ_brainPhase == "standing")) || (time > kjT0 + 30) }
triAssertEq [KJ_state, "idle"]

// ---- 8. the sea: he stops on dry land ---------------------------------------------
triAssert [[[5060, 4000, 0], 0, 60] call KJB_fnDryLine]
triAssert [!([[5060, 3800, 0], 180, 120] call KJB_fnDryLine)]
if (true) then { KJT_skip = true; ded setPos [5060, 3850, 0]; ded setDir 180 }
triSimFrames 80
if (true) then { KJT_skip = false; kjT0 = time; kjW0 = KJ_brainTrack select 3; kjH0 = KJ_hurtCount; KJ_brainOrder = ["drawn", [5060, 3650, 0]] }
triSimUntil { (KJ_state == "drawn") || (time > kjT0 + 5) }
// he decides to stop in the middle of a stride: wait until that stride is over and he stands, then 6 s more
triSimUntil { ((KJ_state == "idle") && (KJB_mv == "stand") && (time > kjT0 + 3)) || (time > kjT0 + 110) }
if (true) then { kjT1 = time }
triSimUntil { time > kjT1 + 6 }
logInfo format ["KJT 8 sea: standing after %1 s at %2, feet %3 m above sea level 0 (the sea surface is at 2.7), water stops %4, alive %5, damage %6, damage rises seen %7", kjT1 - kjT0, getPos ded, (getPosASL ded) select 2, (KJ_brainTrack select 3) - kjW0, alive ded, getDammage ded, KJ_hurtCount - kjH0]
triAssertEq [KJ_state, "idle"]
triAssertEq [KJB_mv, "stand"]
triAssert [((KJ_brainTrack select 3) - kjW0) >= 1]
triAssert [((getPosASL ded) select 2) > 2.9]
triAssert [alive ded]
triAssertEq [KJ_hurtCount - kjH0, 0]
triSetView ([5128, 3812, 1.7, 14] call kjView)
triSimFrames 4
triScreenshot "07_stopped_at_the_sea"
logInfo format ["KJT end: states %1", KJT_states]
logInfo format ["KJT jumps (time, metres, seconds, from, to, men, vehicles): %1", KJT_jumps]
logInfo format ["KJT end: track %1 log entries %2 top speed %3", KJ_brainTrack, count KJ_brainLog, KJT_maxSpeed]
if (true) then { KJT_on = false; KJ_brainOn = false }
triEndTest
