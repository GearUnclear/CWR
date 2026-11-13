// ============================================================================
//  Dedushka COMBAT, do bullets reach him at all (a diagnostic, not part of the
//  gate). RIFLES FIRE. In the first small arms runs 600 forced rounds were
//  fired at him and the engine reported not one hit. This run tells a miss
//  from a hit that is not reported:
//    1. three riflemen 35 m in front of his right leg, forced, level;
//    2. the same men told to target him (aim at his chest) and forced;
//    3. the same with their combat mode RED and a doFire;
//    4. a rifleman 12 m from his leg.
//  After each step: the hit reports the engine made (raw), and whether his
//  leg stopped a round at all (a man standing behind the leg is the witness:
//  if rounds pass through the leg, he is hit).
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
player setPos [4690, 11120, 0]
call compile preprocessFile "kjt.sqf"
call compile preprocessFile "kjt_class.sqf"
{ deleteVehicle _x } forEach [kjcF1, kjcF2, kjcF3]
[kjtClass, 0] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
if (true) then { KJCB_RAW = 60; KJCB_LOG = 20 }
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 30
kjbCam = "camera" camCreate [4900, 11000, 30]
kjbCam cameraEffect ["internal", "back"]
if (true) then { kjbCam camSetTarget kjtDed; kjbCam camSetFov 0.7; kjbCam camCommit 0 }
triSetView ([20, 60, 2, 8] call kjtView)
// the witness: a captive civilian 14 m behind his right leg, on the line of fire
kjbC = createCenter civilian
kjbGc = createGroup civilian
kjbWit = objNull
"Civilian" createUnit [[-14, 5] call kjtRel, kjbGc, "kjbWit = this", 0.5, "PRIVATE"]
kjbG = east call kjtGroup
[["SoldierEB", "SoldierEB", "SoldierEMG"], [35, 5] call kjtRel, kjbG] call kjtMen
triSimFrames 40
if (true) then { kjbWit setPos ([-14, 5] call kjtRel); kjbWit stop true; kjbWit disableAI "MOVE"; { _x setPos ([35, 4 + 1.5 * (kjtMenAll find _x)] call kjtRel) } forEach kjtMenAll }
triSimFrames 20
logInfo format ["KJB staged: men %1 %2 %3, witness %4, his leg at %5, geometry probe: %6", getPos (kjtMenAll select 0), getPos (kjtMenAll select 1), getPos (kjtMenAll select 2), getPos kjbWit, [0, 5] call kjtRel, getPos kjtDed]
// 1. forced, level, at the leg
if (true) then { [kjtMenAll, [0, 5] call kjtRel, 14, 0.2] exec "kjt_fire.sqs"; kjbT = time }
triSimUntil { time > kjbT + 10 }
triScreenshot "b00_discard"
triScreenshot "b01_level_at_the_leg"
triSimUntil { (time > kjbT + 22) and (not kjtForcedOn) }
logInfo format ["KJB 1 level at the leg: rounds %1, unnoticed %2, witness dmg %3 alive %4, his dmg %5", kjtShotCount, KJ_combatStats select 0, getDammage kjbWit, alive kjbWit, getDammage kjtDed]
// 2. told to target him, then forced
if (true) then { kjbN = kjtShotCount; kjbU = KJ_combatStats select 0; { _x doTarget kjtDed } forEach kjtMenAll; kjbT = time }
triSimUntil { time > kjbT + 6 }
if (true) then { [kjtMenAll, getPos kjtDed, 10, 0.2] exec "kjt_fire.sqs"; { _x doTarget kjtDed } forEach kjtMenAll }
triSimUntil { (time > kjbT + 24) and (not kjtForcedOn) }
logInfo format ["KJB 2 targeted: rounds %1, unnoticed %2, witness dmg %3", kjtShotCount - kjbN, (KJ_combatStats select 0) - kjbU, getDammage kjbWit]
// 3. combat mode RED and doFire
if (true) then { kjbN = kjtShotCount; kjbU = KJ_combatStats select 0; kjbG setCombatMode "RED"; kjbG setBehaviour "COMBAT"; { _x doFire kjtDed } forEach kjtMenAll; kjbT = time }
triSimUntil { time > kjbT + 15 }
logInfo format ["KJB 3 doFire: rounds %1, unnoticed %2, witness dmg %3", kjtShotCount - kjbN, (KJ_combatStats select 0) - kjbU, getDammage kjbWit]
// 4. one man 12 m from the leg
if (true) then { kjbN = kjtShotCount; kjbU = KJ_combatStats select 0; (kjtMenAll select 0) setPos ([12, 5] call kjtRel); [[kjtMenAll select 0], [0, 5] call kjtRel, 8, 0.2] exec "kjt_fire.sqs"; kjbT = time }
triSimUntil { time > kjbT + 9 }
triScreenshot "b02_twelve_metres"
triSimUntil { (time > kjbT + 16) and (not kjtForcedOn) }
logInfo format ["KJB 4 at 12 m: rounds %1, unnoticed %2, witness dmg %3 alive %4; shots %5; stats %6", kjtShotCount - kjbN, (KJ_combatStats select 0) - kjbU, getDammage kjbWit, alive kjbWit, kjtShots, KJ_combatStats]
KJ_combatOn = false
triEndTest
