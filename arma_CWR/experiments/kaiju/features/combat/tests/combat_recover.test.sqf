// ============================================================================
//  Dedushka COMBAT: left alone he mends, slowly, and never all the way
//  (experiment, kept out of tests/integration). No weapon is fired: the wound
//  is given by script, the mending is what is under test. Game time runs four
//  times as fast for the waits.
//
//  He is wounded to 58 percent (stage 2, wounded). Then:
//    - for the first half minute nothing mends;
//    - after 40 s without a real hit he starts to mend, awake, slowly;
//    - one new hit stops the mending for another 40 s;
//    - dormant (the brain's state) he mends three times as fast;
//    - he drops back to stage 1, and stops at 87 percent: three tenths of
//      every wound is a scar that never mends (so he is "stung" for good);
//    - a second wound of the same size leaves him lower than the first did.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.3
0 setFog 0
setViewDistance 2000
showCinemaBorder false
player setPos [4690, 11420, 0]
call compile preprocessFile "kjt.sqf"
call compile preprocessFile "kjt_class.sqf"
{ deleteVehicle _x } forEach [kjcF1, kjcF2, kjcF3]
[kjtClass, 0] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 30
kjrCam = "camera" camCreate [4900, 11000, 30]
kjrCam cameraEffect ["internal", "back"]
if (true) then { kjrCam camSetTarget kjtDed; kjrCam camSetFov 0.7; kjrCam camCommit 0 }
triSetView ([25, 88, 3, 23] call kjtView)
triSimFrames 10
triScreenshot "r00_discard"

// ---- the wound -----------------------------------------------------------------------
if (true) then { kjrW = 0.42 * KJCB_POOL; [kjtDed, kjrW, objNull] call KJCB_fnWound; kjrT0 = time }
triSimFrames 5
kjrH0 = KJ_woundHealth
logInfo format ["KJR wounded: health %1 stage %2 scar %3", kjrH0, KJ_woundStage, KJ_woundScar]
[(KJ_woundStage) == (2), "KJ_woundStage == 2"] call kjtCheck
[(abs (kjrH0 - 0.58)) < (0.002), "abs (kjrH0 - 0.58) < 0.002"] call kjtCheck
triSimUntil { time > kjrT0 + 6 }
triScreenshot "r01_wounded"
setAccTime 4
// ---- nothing mends at first
triSimUntil { time > kjrT0 + 30 }
[(KJ_woundHealth) == (kjrH0), "KJ_woundHealth == kjrH0"] call kjtCheck
[!KJ_woundMending, "!KJ_woundMending"] call kjtCheck
// ---- then it does, awake
triSimUntil { time > kjrT0 + KJCB_MEND_WAIT + 20 }
kjrH1 = KJ_woundHealth
[KJ_woundMending, "KJ_woundMending"] call kjtCheck
[(kjrH1) > (kjrH0 + 0.02), "kjrH1 > kjrH0 + 0.02"] call kjtCheck
[(kjrH1) < (kjrH0 + 0.05), "kjrH1 < kjrH0 + 0.05"] call kjtCheck
// ---- a new hit stops it for another 40 s
if (true) then { [kjtDed, "Heat120", [0, -0.3 * KJCB_H, 0.7 * KJCB_H], objNull] call KJCB_fnScriptHit; kjrT1 = time }
triSimUntil { time > kjrT1 + 5 }
kjrH2 = KJ_woundHealth
[!KJ_woundMending, "!KJ_woundMending"] call kjtCheck
triSimUntil { time > kjrT1 + 30 }
[(KJ_woundHealth) == (kjrH2), "KJ_woundHealth == kjrH2"] call kjtCheck
[!KJ_woundMending, "!KJ_woundMending"] call kjtCheck
triSimUntil { time > kjrT1 + KJCB_MEND_WAIT + 10 }
kjrH3 = KJ_woundHealth
[KJ_woundMending, "KJ_woundMending"] call kjtCheck
[(kjrH3) > (kjrH2), "kjrH3 > kjrH2"] call kjtCheck
// ---- dormant, three times as fast (the rate over 20 s, awake against dormant)
if (true) then { kjrTa = time; kjrHa = KJ_woundHealth }
triSimUntil { time > kjrTa + 20 }
if (true) then { kjrAwake = (KJ_woundHealth - kjrHa) / (time - kjrTa); KJ_state = "dormant"; kjrTa = time; kjrHa = KJ_woundHealth }
triSimUntil { time > kjrTa + 20 }
kjrDormant = (KJ_woundHealth - kjrHa) / (time - kjrTa)
logInfo format ["KJR mending: awake %1 of his health a second, dormant %2; health %3 stage %4", kjrAwake, kjrDormant, KJ_woundHealth, KJ_woundStage]
[(kjrAwake) > (0), "kjrAwake > 0"] call kjtCheck
[(kjrDormant) > (2.5 * kjrAwake), "kjrDormant > 2.5 * kjrAwake"] call kjtCheck
[(kjrDormant) < (3.5 * kjrAwake), "kjrDormant < 3.5 * kjrAwake"] call kjtCheck
// ---- to the end: stage 1, and a scar
triSimUntil { (not KJ_woundMending) or (time > kjrTa + 160) }
setAccTime 1
triSimFrames 10
kjrEnd = KJ_woundHealth
kjrScar = KJ_woundScar
logInfo format ["KJR mended: health %1, scar %2, stage %3, wound log %4, took %5 s", kjrEnd, kjrScar, KJ_woundStage, KJ_woundLog, time - kjrT0]
[!KJ_woundMending, "!KJ_woundMending"] call kjtCheck
[(abs (kjrEnd - (1 - kjrScar))) < (0.002), "abs (kjrEnd - (1 - kjrScar)) < 0.002"] call kjtCheck
[(kjrScar) > (0.12), "kjrScar > 0.12"] call kjtCheck
[(kjrScar) < (0.135), "kjrScar < 0.135"] call kjtCheck
[(KJ_woundStage) == (1), "KJ_woundStage == 1"] call kjtCheck
[(getDammage kjtDed) == (0), "getDammage kjtDed == 0"] call kjtCheck
triScreenshot "r02_mended"
// ---- the same wound again leaves him lower than the first did
[kjtDed, kjrW, objNull] call KJCB_fnWound
triSimFrames 5
logInfo format ["KJR wounded again: health %1 (first time %2) stage %3", KJ_woundHealth, kjrH0, KJ_woundStage]
[(KJ_woundHealth) < (kjrH0 - 0.1), "KJ_woundHealth < kjrH0 - 0.1"] call kjtCheck
[(KJ_woundStage) == (2), "KJ_woundStage == 2"] call kjtCheck
[alive kjtDed, "alive kjtDed"] call kjtCheck
KJ_combatOn = false
// the feature's loops end and take their light off him before the game goes down (a light still bound to him at exit crashed the game once, after the verdict)
triSimFrames 40
"combat_recover" call kjtVerdict
triEndTest
