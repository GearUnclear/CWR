// ============================================================================
//  Dedushka COMBAT: small arms alone never kill him and never visibly hurt him
//  (experiment, kept out of tests/integration). RIFLES AND MACHINE GUNS FIRE.
//
//  An East rifle section stands 150 m in front of him: four riflemen, two PK
//  machine gunners, a sniper.
//    1. Left to themselves, with him revealed as an enemy, for 30 s: not one
//       round. Riflemen do not waste themselves on him.
//    2. Then they are brought to 42 m and every one of them is made to fire at
//       him for 75 s (the engine's forced shot). Hundreds of rounds strike him. Nothing: his health is
//       whole, the wound stage is 0, no hit signal is raised, he does not
//       flinch or roar, the engine's own damage value is 0, he is alive and
//       still in his idle. On the kaiju binary the rounds are counted as they
//       land ("unnoticed").
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
kjsMove = getMove kjtDed
kjsCam = "camera" camCreate [4900, 11000, 30]
kjsCam cameraEffect ["internal", "back"]
if (true) then { kjsCam camSetTarget kjtDed; kjsCam camSetFov 0.7; kjsCam camCommit 0 }
triSetView ([8, 172, 2.2, 17] call kjtView)
triSimFrames 10
triScreenshot "s00_discard"

// ---- 1. left to themselves -----------------------------------------------------------
kjsG = east call kjtGroup
[["SoldierEB", "SoldierEB", "SoldierEB", "SoldierEB", "SoldierEMG", "SoldierEMG", "SoldierESniper"], [150, 6] call kjtRel, kjsG] call kjtMen
triSimFrames 40
[(count kjtMenAll) == (7), "count kjtMenAll == 7"] call kjtCheck
if (true) then { kjsG reveal kjtDed; kjsT = time }
triSimUntil { time > kjsT + 30 }
logInfo format ["KJS left alone 30 s: rounds %1 %2, knows %3, behaviour %4, hurt %5, his side %6, they %7 m off", kjtShotCount, kjtShots, (kjtMenAll select 0) knowsAbout kjtDed, behaviour (kjtMenAll select 0), KJ_hurtCount, side kjtDed, kjtDed distance (kjtMenAll select 0)]
[(kjtShotCount) == (0), "kjtShotCount == 0"] call kjtCheck
[((kjtMenAll select 0) knowsAbout kjtDed) > (1.4), "(kjtMenAll select 0) knowsAbout kjtDed > 1.4"] call kjtCheck
triScreenshot "s01_holding_fire"

// ---- 2. made to fire -----------------------------------------------------------------
// they are brought to 42 m in front of him: a man made to fire holds his rifle about 30 degrees up, so
// from there the rounds land on his belly and chest (from 150 m they pass over his head)
if (true) then { { _x setPos ([42, -9 + 3 * (kjtMenAll find _x)] call kjtRel) } forEach kjtMenAll }
triSetView ([14, 80, 2, 19] call kjtView)
triSimFrames 30
if (true) then { [kjtMenAll, getPos kjtDed, 75, 0.25, kjtDed] exec "kjt_fire.sqs"; kjsT = time }
triSimUntil { time > kjsT + 30 }
triScreenshot "s02_under_fire"
triSimUntil { time > kjsT + 60 }
triScreenshot "s03_still_firing"
triSimUntil { (time > kjsT + 80) and (not kjtForcedOn) }
triSimFrames 60
logInfo format ["KJS made to fire: rounds %1 %2", kjtShotCount, kjtShots]
logInfo format ["KJS and he: %1", call kjtBook]
logInfo format ["KJS move %1 (was %2), last felt %3, wound log %4", getMove kjtDed, kjsMove, KJ_combatLast, KJ_woundLog]
[(kjtShotCount) >= (300), "kjtShotCount >= 300"] call kjtCheck
kjsLanded = 100
if (KJCB_native) then { kjsLanded = KJ_combatStats select 0 }
[(kjsLanded) >= (100), "kjsLanded >= 100"] call kjtCheck
[(KJ_woundHealth) == (1), "KJ_woundHealth == 1"] call kjtCheck
[(KJ_woundStage) == (0), "KJ_woundStage == 0"] call kjtCheck
[(KJ_hurtCount) == (0), "KJ_hurtCount == 0"] call kjtCheck
[(KJ_combatStats select 1) == (0), "KJ_combatStats select 1 == 0"] call kjtCheck
[(KJ_combatStats select 2) == (0), "KJ_combatStats select 2 == 0"] call kjtCheck
[(KJ_combatStats select 5) == (0), "KJ_combatStats select 5 == 0"] call kjtCheck
[(KJ_combatStats select 6) == (0), "KJ_combatStats select 6 == 0"] call kjtCheck
[(count KJ_woundLog) == (0), "count KJ_woundLog == 0"] call kjtCheck
[(getDammage kjtDed) == (0), "getDammage kjtDed == 0"] call kjtCheck
[alive kjtDed, "alive kjtDed"] call kjtCheck
[!KJ_dead, "!KJ_dead"] call kjtCheck
[(getMove kjtDed) == (kjsMove), "getMove kjtDed == kjsMove"] call kjtCheck
triScreenshot "s04_after"
KJ_combatOn = false
// the feature's loops end and take their light off him before the game goes down (a light still bound to him at exit crashed the game once, after the verdict)
triSimFrames 40
"combat_smallarms" call kjtVerdict
triEndTest
