// ============================================================================
//  Dedushka BRAIN: left to himself (experiment, kept out of tests/integration).
//  The demo mission with its timeline switched off and KJ_brainAuto on. Nobody
//  fires and nobody orders him. The only thing in the world that can draw him
//  is the garrison's armour: two crewed T-80s and a crewed BMP parked together
//  320 m east of where he stands dormant. The rules (brain NOTES.md) say:
//    - armour standing together builds interest and anger at its place;
//    - real interest within his waking range wakes him: he rises and roars;
//    - awake, he is drawn to the place with the most interest;
//    - when the anger there passes the mark and the place is in beam range,
//      he stops, turns to it, fires (the brain's stub here), and is spent.
//  This test waits for each of those, in that order, and logs the interest
//  list as it goes (KJA lines in game_stdout.log).
//
//  Run:  features\brain\run-brain.ps1 -Name brain_auto -Mission DedDemo.Eden
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJB_ready") }
triSimUntil { !(isNull kaiju) }
if (true) then { KJD_plan = []; KJB_SPENT = 15; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; KJ_brainOrder = ["auto", true]; kjT0 = time; kjP0 = getPos kaiju }
kjCam = "camera" camCreate [4990, 3985, 3]
kjCam cameraEffect ["internal", "back"]
showCinemaBorder false
if (true) then { kjCam camSetTarget [4790, 4004, 20]; kjCam camSetFov 0.7; kjCam camCommit 0 }
kjView = { private ["_p"]; _p = getPosASL kaiju; [_this select 0, 16.3 + (_this select 2), _this select 1, (_p select 0) - (_this select 0), (_p select 2) + (_this select 3) - 16.3 - (_this select 2), (_p select 1) - (_this select 1)] }
triSetView ([5090, 3955, 8, 20] call kjView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// ---- the armour wakes him --------------------------------------------------------
triSimUntil { (KJ_state == "roar") || (time > kjT0 + 100) }
logInfo format ["KJA woke and roared after %1 s; interest %2", time - kjT0, KJ_interest]
triAssertEq [KJ_state, "roar"]
triAssert [(count KJ_interest) >= 1]
triAssert [([getPos kaiju, kjP0] call KJB_fnDist) < 1]

// ---- he is drawn to it --------------------------------------------------------------
triSimUntil { (KJ_state == "drawn") || (KJ_state == "beam") || (time > kjT0 + 150) }
logInfo format ["KJA drawn after %1 s to %2 (the tanks are at %3 and %4); interest %5", time - kjT0, KJ_brainDraw, getPos kjdTank, getPos kjdBmp, KJ_interest]
triAssert [(KJ_state == "drawn") || (KJ_state == "beam")]
triAssert [(([KJ_brainDraw, getPos kjdTank] call KJB_fnDist) < 60) || (KJ_state == "beam")]

// ---- the armour earns the beam ---------------------------------------------------------
triSimUntil { (KJ_state == "beam") || (time > kjT0 + 260) }
logInfo format ["KJA beam decided after %1 s, %2 m from the armour, target %3; interest %4", time - kjT0, [getPos kaiju, getPos kjdTank] call KJB_fnDist, KJB_beamTgt, KJ_interest]
triAssertEq [KJ_state, "beam"]
triSimUntil { (KJ_brainPhase == "firing") || (time > kjT0 + 330) }
triAssertEq [KJ_brainPhase, "firing"]
triAssert [(abs ([getDir kaiju, [getPos kaiju, getPos kjdTank] call KJB_fnBear] call KJB_fnAng)) < 12]
if (true) then { kjT1 = time }
triSimUntil { time > kjT1 + 9 }
triSetView ([5090, 3955, 8, 24] call kjView)
triSimFrames 4
triScreenshot "a01_left_to_himself_he_stands_tall_at_the_armour"
triSimUntil { (KJ_state == "spent") || (time > kjT1 + 60) }
triAssertEq [KJ_state, "spent"]
logInfo format ["KJA spent after %1 s at %2; anger left %3", time - kjT0, getPos kaiju, KJ_interest]
// spent wipes the anger: nothing earns a second beam at once
triAssert [({ (_x select 3) > 0.01 } count KJ_interest) == 0]
triSimUntil { (KJ_state != "spent") || (time > kjT1 + 120) }
logInfo format ["KJA after spent: state %1 after %2 s; states %3", KJ_state, time - kjT0, KJT_states]
logInfo format ["KJA track %1 top speed %2 jumps %3 garrison alive %4", KJ_brainTrack, KJT_maxSpeed, KJT_jumps, { alive _x } count [kjdA1, kjdB1, kjdTank, kjdTank2, kjdBmp]]
triAssert [alive kaiju]
triAssert ["roar" in KJT_states]
triAssert ["drawn" in KJT_states]
triAssert [KJT_maxSpeed < 3.2]
if (true) then { KJT_on = false; KJ_brainOn = false }
triEndTest
