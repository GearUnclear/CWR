// ============================================================================
//  Dedushka COMBAT: where a hit lands and what it is worth; the wound stages
//  (experiment, kept out of tests/integration). ONE TANK FIRES in part 2.
//
//  1. The same round, by script, into four places of his body: four tank HEAT
//     rounds into the furnace chest cost him more than six times what four
//     into a leg do; head and plain body fall in between, in that order. Two
//     hundred rifle rounds into the furnace cost nothing and raise no signal.
//     Fifty 30 mm shells sting (a signal, a little health) and never make a
//     hard hit.
//  2. Live: a T-80 440 m in front of him opens fire by itself. Its rounds are
//     read from the engine's hit report: they land in the furnace (it aims at
//     his chest), the hit signal names the tank and its bearing, each is a
//     hard hit.
//  3. The stages: he is taken down through stung, wounded, badly hurt and
//     dying, a screenshot of each; the globals the brain and the beam read
//     follow (stage, pace); he roars in pain on the way; stings do nothing
//     more once he is past stung; badly hurt he turns for the sea (the point
//     is found).
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
KJCB_LOG = 60
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 30
kjwCam = "camera" camCreate [4900, 11000, 30]
kjwCam cameraEffect ["internal", "back"]
if (true) then { kjwCam camSetTarget kjtDed; kjwCam camSetFov 0.7; kjwCam camCommit 0 }
triSetView ([25, 88, 3, 23] call kjtView)
triSimFrames 10
triScreenshot "w00_discard"
triScreenshot "w00_unhurt"

// ---- 1. the same round into four places ------------------------------------------------
// kjwDrop: wait until the wound has opened, then what it cost (share of his health)
kjwH = KJ_woundHealth
if (true) then { kjwI = 0; while { kjwI < 4 } do { [kjtDed, "Heat120", KJCB_furnace, objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 3
triSimUntil { KJCB_buf <= 0 }
if (true) then { kjwChest = kjwH - KJ_woundHealth; kjwH = KJ_woundHealth }
if (true) then { kjwI = 0; while { kjwI < 4 } do { [kjtDed, "Heat120", [0.12 * KJCB_H, 0.03 * KJCB_H, 0.2 * KJCB_H], objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 3
triSimUntil { KJCB_buf <= 0 }
if (true) then { kjwLegs = kjwH - KJ_woundHealth; kjwH = KJ_woundHealth }
if (true) then { kjwI = 0; while { kjwI < 4 } do { [kjtDed, "Heat120", [0, -0.3 * KJCB_H, 0.95 * KJCB_H], objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 3
triSimUntil { KJCB_buf <= 0 }
if (true) then { kjwHead = kjwH - KJ_woundHealth; kjwH = KJ_woundHealth }
if (true) then { kjwI = 0; while { kjwI < 4 } do { [kjtDed, "Heat120", [0, -0.3 * KJCB_H, 0.7 * KJCB_H], objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 3
triSimUntil { KJCB_buf <= 0 }
if (true) then { kjwBody = kjwH - KJ_woundHealth; kjwH = KJ_woundHealth }
logInfo format ["KJW four HEAT rounds each: furnace %1, head %2, body %3, legs %4 of his health; by zone %5; furnace point %6; hurt count %7", kjwChest, kjwHead, kjwBody, kjwLegs, KJ_combatByZone, KJCB_furnace, KJ_hurtCount]
[(kjwLegs) > (0), "kjwLegs > 0"] call kjtCheck
[(kjwChest) > (6 * kjwLegs), "kjwChest > 6 * kjwLegs"] call kjtCheck
[(kjwChest) < (6.5 * kjwLegs), "kjwChest < 6.5 * kjwLegs"] call kjtCheck
[(kjwChest) > (kjwHead), "kjwChest > kjwHead"] call kjtCheck
[(kjwHead) > (kjwBody), "kjwHead > kjwBody"] call kjtCheck
[(kjwBody) > (kjwLegs), "kjwBody > kjwLegs"] call kjtCheck
[(KJ_combatByZone select 0) == (4), "KJ_combatByZone select 0 == 4"] call kjtCheck
[(KJ_combatByZone select 1) == (4), "KJ_combatByZone select 1 == 4"] call kjtCheck
[(KJ_combatByZone select 2) == (4), "KJ_combatByZone select 2 == 4"] call kjtCheck
[(KJ_combatByZone select 3) == (4), "KJ_combatByZone select 3 == 4"] call kjtCheck
[(KJ_woundStage) == (0), "KJ_woundStage == 0"] call kjtCheck
// rifle rounds: nothing, and no signal
if (true) then { kjwHc = KJ_hurtCount; kjwI = 0; while { kjwI < 200 } do { [kjtDed, "BulletSingle", KJCB_furnace, objNull] call KJCB_fnScriptHit; [kjtDed, "Bullet7_6", KJCB_furnace, objNull] call KJCB_fnScriptHit; [kjtDed, "Bullet12_7", KJCB_furnace, objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 30
logInfo format ["KJW 600 rifle and machine gun rounds into the furnace: health %1 (was %2), hurt count %3 (was %4), unnoticed %5", KJ_woundHealth, kjwH, KJ_hurtCount, kjwHc, KJ_combatStats select 0]
[(KJ_woundHealth) == (kjwH), "KJ_woundHealth == kjwH"] call kjtCheck
[(KJ_hurtCount) == (kjwHc), "KJ_hurtCount == kjwHc"] call kjtCheck
[(KJ_combatStats select 0) >= (600), "KJ_combatStats select 0 >= 600"] call kjtCheck
// 30 mm shells and grenades: a sting
if (true) then { kjwI = 0; while { kjwI < 50 } do { [kjtDed, "Bullet30", KJCB_furnace, objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 }; kjwFl = KJ_combatStats select 5 }
triSimFrames 3
triSimUntil { KJCB_buf <= 0 }
if (true) then { kjwSting = kjwH - KJ_woundHealth; kjwH = KJ_woundHealth }
logInfo format ["KJW fifty 30 mm shells into the furnace: cost %1 of his health, hurt %2, last %3, stings %4, flinches %5", kjwSting, KJ_hurt, KJ_combatLast, KJ_combatStats select 1, KJ_combatStats select 5]
[(kjwSting) > (0), "kjwSting > 0"] call kjtCheck
[(kjwSting) < (0.25 * kjwChest), "kjwSting < 0.25 * kjwChest"] call kjtCheck
[(KJ_hurtCount) > (kjwHc), "KJ_hurtCount > kjwHc"] call kjtCheck
[(KJ_hurt select 3) < (0.5), "KJ_hurt select 3 < 0.5"] call kjtCheck
[(KJ_combatStats select 1) == (50), "KJ_combatStats select 1 == 50"] call kjtCheck
[(KJ_combatStats select 5) == (kjwFl), "KJ_combatStats select 5 == kjwFl"] call kjtCheck

// ---- 2. live: a tank in front of him ---------------------------------------------------
kjwGa = east call kjtGroup
kjwTank = ["T80", [440, 0] call kjtRel, 180, kjwGa, "SoldierECrew", ["driver", "gunner", "commander"]] call kjtVehicle
triSimFrames 20
call kjtMount
triSimFrames 40
if (true) then { kjwReal0 = KJ_combatStats select 2; kjwZ0 = KJ_combatByZone select 0; kjwGa reveal kjtDed; kjwT = time }
triSimUntil { ((KJ_combatStats select 2) >= kjwReal0 + 3) or (time > kjwT + 120) }
if (true) then { kjwBear = (((getPos kjwTank) select 0) - ((getPos kjtDed) select 0)) atan2 (((getPos kjwTank) select 1) - ((getPos kjtDed) select 1)); if (kjwBear < 0) then { kjwBear = kjwBear + 360 }; kjwHurt = KJ_hurt; kjwLast = KJ_combatLast }
call kjtCease
logInfo format ["KJW the tank: %1 rounds fired %2, real hits %3, into the furnace %4, by zone %5, hurt %6 (tank bearing %7), last %8, after %9 s, health %10", kjtShots, kjtShotCount, (KJ_combatStats select 2) - kjwReal0, (KJ_combatByZone select 0) - kjwZ0, KJ_combatByZone, kjwHurt, kjwBear, kjwLast, time - kjwT, KJ_woundHealth]
[((KJ_combatStats select 2) - kjwReal0) >= (3), "(KJ_combatStats select 2) - kjwReal0 >= 3"] call kjtCheck
// the place of a hit is known on the kaiju binary only
kjwInFurnace = 2
if (KJCB_native) then { kjwInFurnace = (KJ_combatByZone select 0) - kjwZ0 }
[(kjwInFurnace) >= (2), "kjwInFurnace >= 2"] call kjtCheck
[(kjwHurt select 3) >= (1), "kjwHurt select 3 >= 1"] call kjtCheck
// the bearing is known on the kaiju binary (the report names the shooter)
kjwErr = 0
if (KJCB_native) then { kjwErr = abs ((kjwHurt select 1) - kjwBear) }
[(kjwErr) < (10), "kjwErr < 10"] call kjtCheck
[(("Bullet12_7" call kjtFiredOf) + ("Bullet7_6" call kjtFiredOf)) == (0), "('Bullet12_7' call kjtFiredOf) + ('Bullet7_6' call kjtFiredOf) == 0"] call kjtCheck
// the tank is taken out of the test, so that no late round moves the numbers below
kjwTank setDammage 1
triSimUntil { time > kjwT + 8 }
triSimUntil { KJCB_buf <= 0 }

// ---- 3. the stages --------------------------------------------------------------------
// kjwTo: wound him by script down to that share of his health
kjwTo = { [kjtDed, (KJ_woundHealth - _this) * KJCB_POOL, kjwTank] call KJCB_fnWound }
[(KJ_woundStage) == (0), "KJ_woundStage == 0"] call kjtCheck
0.80 call kjwTo
triSimFrames 5
[(KJ_woundStage) == (1), "KJ_woundStage == 1"] call kjtCheck
[(KJ_woundPace) == (0.92), "KJ_woundPace == 0.92"] call kjtCheck
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 7 }
triScreenshot "w01_stung"
if (true) then { kjwR0 = KJ_combatStats select 6; 0.55 call kjwTo }
triSimFrames 5
[(KJ_woundStage) == (2), "KJ_woundStage == 2"] call kjtCheck
[(KJ_woundPace) == (0.78), "KJ_woundPace == 0.78"] call kjtCheck
[((KJ_combatStats select 6) - kjwR0) == (1), "(KJ_combatStats select 6) - kjwR0 == 1"] call kjtCheck
// past stung, a sting costs nothing more
if (true) then { kjwH = KJ_woundHealth; kjwI = 0; while { kjwI < 60 } do { [kjtDed, "Bullet30", KJCB_furnace, objNull] call KJCB_fnScriptHit; kjwI = kjwI + 1 } }
triSimFrames 20
[(KJ_woundHealth) == (kjwH), "KJ_woundHealth == kjwH"] call kjtCheck
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 8 }
triScreenshot "w02_wounded"
0.30 call kjwTo
triSimFrames 5
[(KJ_woundStage) == (3), "KJ_woundStage == 3"] call kjtCheck
[(KJ_woundPace) == (0.6), "KJ_woundPace == 0.6"] call kjtCheck
triSimFrames 20
logInfo format ["KJW badly hurt: retreat %1 to %2, side %3, mending %4, hurt %5", KJ_woundRetreat, KJ_woundRetreatPos, KJ_woundSide, KJ_woundMending, KJ_hurt]
[KJ_woundRetreat, "KJ_woundRetreat"] call kjtCheck
[(count KJ_woundRetreatPos) == (3), "count KJ_woundRetreatPos == 3"] call kjtCheck
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 8 }
triScreenshot "w03_badly_hurt"
0.10 call kjwTo
triSimFrames 5
[(KJ_woundStage) == (4), "KJ_woundStage == 4"] call kjtCheck
[(KJ_woundPace) == (0.45), "KJ_woundPace == 0.45"] call kjtCheck
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 8 }
triScreenshot "w04_dying"
triSetView ([8, 50, 2, 27] call kjtView)
triSimFrames 30
triScreenshot "w05_dying_from_below"
logInfo format ["KJW end: wound log %1, stats %2, health %3, scar %4, alive %5, move %6", KJ_woundLog, KJ_combatStats, KJ_woundHealth, KJ_woundScar, alive kjtDed, getMove kjtDed]
[alive kjtDed, "alive kjtDed"] call kjtCheck
[!KJ_dead, "!KJ_dead"] call kjtCheck
[(getDammage kjtDed) == (0), "getDammage kjtDed == 0"] call kjtCheck
KJ_combatOn = false
// the feature's loops end and take their light off him before the game goes down (a light still bound to him at exit crashed the game once, after the verdict)
triSimFrames 40
"combat_wounds" call kjtVerdict
triEndTest
