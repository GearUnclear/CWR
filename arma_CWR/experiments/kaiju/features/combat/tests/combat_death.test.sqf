// ============================================================================
//  Dedushka COMBAT: the death sequence completes and leaves a harmless body
//  (experiment, kept out of tests/integration). No weapon is fired here: the
//  last of his health is taken by script, which is exactly what the killing
//  blow of a real fight does (combat_heavy shows that end of it).
//
//  Who stands where when he dies (his frame: forward, right):
//    a man 12 m in front of his feet          inside the vent: dies of it
//    a man 30 m ahead, 3 m off his line       burnt by the vent, then under the body: dead
//    a man 10 m ahead, 40 m to his right      burnt, thrown down by the fall, lives (he stands 45 m
//                                             from the tank: a crushed T-80 blows up, and once
//                                             killed a bystander 30 m away)
//    a man 150 m ahead, 60 m to the side      untouched
//    an empty T-80 28 m ahead, 6 m to his left   the vent does not harm armour; the body crushes it
//    a jeep 20 m ahead, 30 m to the side      scorched by the vent, not crushed
//    an East rifle section 90 m off, running from him (the engine's own flee)
//  Then: the phases come in order (stagger, flare, vent, dark, fall, down,
//  body); from the first of them KJ_dead is true and the other features'
//  loops are told to stop; he is alive until the fall and dead after it; the
//  whole takes under 35 s; the running section stops running; a man put
//  right beside the body afterwards takes no harm; no light is left on him.
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
[kjtClass, 0] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 30
[KJ_combatOn, "KJ_combatOn"] call kjtCheck
[(KJ_woundStage) == (0), "KJ_woundStage == 0"] call kjtCheck
kjdCam = "camera" camCreate [4900, 11000, 30]
kjdCam cameraEffect ["internal", "back"]
if (true) then { kjdCam camSetTarget kjtDed; kjdCam camSetFov 0.7; kjdCam camCommit 0 }

// ---- the bystanders ---------------------------------------------------------------
kjdC = createCenter civilian
kjdG = createGroup civilian
kjdNear = objNull
kjdUnder = objNull
kjdBurn = objNull
kjdFar = objNull
"Civilian" createUnit [[12, 0] call kjtRel, kjdG, "kjdNear = this", 0.5, "PRIVATE"]
"Civilian2" createUnit [[30, 3] call kjtRel, kjdG, "kjdUnder = this", 0.5, "PRIVATE"]
"Civilian3" createUnit [[10, 40] call kjtRel, kjdG, "kjdBurn = this", 0.5, "PRIVATE"]
"Civilian" createUnit [[150, 60] call kjtRel, kjdG, "kjdFar = this", 0.5, "PRIVATE"]
triSimFrames 40
// createUnit puts a man in his group's formation, not on the spot asked for: each is set in place
if (true) then { kjdNear setPos ([12, 0] call kjtRel); kjdUnder setPos ([30, 3] call kjtRel); kjdBurn setPos ([10, 40] call kjtRel); kjdFar setPos ([150, 60] call kjtRel) }
{ _x stop true; _x disableAI "MOVE"; _x setBehaviour "CARELESS" } forEach [kjdNear, kjdUnder, kjdBurn, kjdFar]
// createVehicle will not put a vehicle near him: each is made far off and then set in place
kjdTank = "T80" createVehicle [4690, 11300, 0]
kjdJeep = "UAZ" createVehicle [4700, 11300, 0]
if (true) then { kjdTank setPos ([28, -6] call kjtRel); kjdTank setDir 90; kjdTank setFuel 0; kjdJeep setPos ([20, 30] call kjtRel); kjdJeep setDir 0 }
triSimFrames 60
logInfo format ["KJD staged: near %1 under %2 burn %3 far %4 tank %5 jeep %6, he %7 dir %8 furnace %9", getPos kjdNear, getPos kjdUnder, getPos kjdBurn, getPos kjdFar, getPos kjdTank, getPos kjdJeep, getPos kjtDed, getDir kjtDed, KJCB_furnace]
// the East section runs: moved off its start point, courage gone, one man hurt (the engine's own flee back to where it began)
if (true) then { kjcF1 setPos [4735, 11210, 0]; kjcF2 setPos [4737, 11206, 0]; kjcF3 setPos [4737, 11214, 0] }
triSimFrames 20
if (true) then { (group kjcF1) allowFleeing 1; kjcF2 setDammage 0.4; kjdT = time }
triSimUntil { (fleeing kjcF1) or (time > kjdT + 12) }
logInfo format ["KJD the section: fleeing %1 after %2 s", fleeing kjcF1, time - kjdT]
kjdFled = fleeing kjcF1
triSetView ([78, 100, 4, 17, 14] call kjtView)
triSimFrames 10
triScreenshot "d00_discard"
triScreenshot "d00_alive"

// ---- the killing blow ---------------------------------------------------------------
if (true) then { kjdT0 = time; [kjtDed, 10000, objNull] call KJCB_fnWound }
triSimUntil { KJ_deathPhase == "stagger" }
[KJ_dead, "KJ_dead"] call kjtCheck
[KJ_dying, "KJ_dying"] call kjtCheck
[alive kjtDed, "alive kjtDed"] call kjtCheck
[!KJ_combatOn, "!KJ_combatOn"] call kjtCheck
triSimUntil { time > kjdT0 + 2.2 }
triScreenshot "d01_stagger"
triSimUntil { KJ_deathPhase == "flare" }
[alive kjtDed, "alive kjtDed"] call kjtCheck
[(getDammage kjdNear) == (0), "getDammage kjdNear == 0"] call kjtCheck
triSimUntil { (KJ_deathPhase != "flare") or (time > ((KJ_deathLog select 1) select 1) + 0.6 * KJCB_D_FLARE) }
triScreenshot "d02_flare_rising"
triSimUntil { (KJ_deathPhase != "flare") or (time > ((KJ_deathLog select 1) select 1) + 0.93 * KJCB_D_FLARE) }
triScreenshot "d03_flare_white"
triSimUntil { KJ_deathPhase == "vent" }
triSimFrames 2
triScreenshot "d04_vent"
triSimUntil { KJ_deathPhase == "dark" }
logInfo format ["KJD after the vent: near alive %1, under %2 dmg %3, burn %4 dmg %5, far dmg %6, tank dmg %7, jeep dmg %8, stats %9", alive kjdNear, alive kjdUnder, getDammage kjdUnder, alive kjdBurn, getDammage kjdBurn, getDammage kjdFar, getDammage kjdTank, getDammage kjdJeep, KJ_deathStats]
[!(alive kjdNear), "!(alive kjdNear)"] call kjtCheck
[alive kjdUnder, "alive kjdUnder"] call kjtCheck
[(getDammage kjdUnder) > (0.2), "getDammage kjdUnder > 0.2"] call kjtCheck
[alive kjdBurn, "alive kjdBurn"] call kjtCheck
[(getDammage kjdBurn) > (0.05), "getDammage kjdBurn > 0.05"] call kjtCheck
[(getDammage kjdFar) == (0), "getDammage kjdFar == 0"] call kjtCheck
[(getDammage kjdTank) == (0), "getDammage kjdTank == 0"] call kjtCheck
[(getDammage kjdJeep) >= (0.45), "getDammage kjdJeep >= 0.45"] call kjtCheck
[alive kjtDed, "alive kjtDed"] call kjtCheck
triSimUntil { (KJ_deathPhase != "dark") or (time > ((KJ_deathLog select 3) select 1) + 1.2) }
triScreenshot "d05_dark"
triSimUntil { KJ_deathPhase == "fall" }
kjdBurn0 = getDammage kjdBurn
triSimFrames 3
[!(alive kjtDed), "!(alive kjtDed)"] call kjtCheck
[(KJ_woundStage) == (5), "KJ_woundStage == 5"] call kjtCheck
triSimUntil { (KJ_deathPhase != "fall") or (time > ((KJ_deathLog select 4) select 1) + 0.45 * KJCB_D_FALL) }
triScreenshot "d06_falling"
triSimUntil { KJ_deathPhase == "down" }
triSimFrames 3
triScreenshot "d07_down"
if (KJCB_native) then { logInfo format ["KJD shake when he lands: %1", kjShakeGain]; kjdShake = kjShakeGain } else { kjdShake = 1 }
triSimUntil { time > ((KJ_deathLog select 5) select 1) + 1.6 }
triScreenshot "d08_dust"
triSimUntil { KJ_deathPhase == "body" }
kjdTb = time
logInfo format ["KJD death log %1, took %2 s, stats %3", KJ_deathLog, kjdTb - kjdT0, KJ_deathStats]
logInfo format ["KJD after the fall: under alive %1, burn alive %2 dmg %3 (was %4), far dmg %5, tank dmg %6 alive %7, jeep dmg %8, he %9 alive %10 move %11", alive kjdUnder, alive kjdBurn, getDammage kjdBurn, kjdBurn0, getDammage kjdFar, getDammage kjdTank, alive kjdTank, getDammage kjdJeep, getPos kjtDed, alive kjtDed, getMove kjtDed]
[(count KJ_deathLog) == (7), "count KJ_deathLog == 7"] call kjtCheck
[((KJ_deathLog select 0) select 0) == ("stagger"), "(KJ_deathLog select 0) select 0 == 'stagger'"] call kjtCheck
[((KJ_deathLog select 1) select 0) == ("flare"), "(KJ_deathLog select 1) select 0 == 'flare'"] call kjtCheck
[((KJ_deathLog select 2) select 0) == ("vent"), "(KJ_deathLog select 2) select 0 == 'vent'"] call kjtCheck
[((KJ_deathLog select 3) select 0) == ("dark"), "(KJ_deathLog select 3) select 0 == 'dark'"] call kjtCheck
[((KJ_deathLog select 4) select 0) == ("fall"), "(KJ_deathLog select 4) select 0 == 'fall'"] call kjtCheck
[((KJ_deathLog select 5) select 0) == ("down"), "(KJ_deathLog select 5) select 0 == 'down'"] call kjtCheck
[((KJ_deathLog select 6) select 0) == ("body"), "(KJ_deathLog select 6) select 0 == 'body'"] call kjtCheck
[(kjdTb - kjdT0) < (35), "kjdTb - kjdT0 < 35"] call kjtCheck
[(kjdTb - kjdT0) > (15), "kjdTb - kjdT0 > 15"] call kjtCheck
[!(alive kjtDed), "!(alive kjtDed)"] call kjtCheck
[KJ_dead, "KJ_dead"] call kjtCheck
[!KJ_dying, "!KJ_dying"] call kjtCheck
[(KJ_woundHealth) == (0), "KJ_woundHealth == 0"] call kjtCheck
[!(alive kjdUnder), "!(alive kjdUnder)"] call kjtCheck
[alive kjdBurn, "alive kjdBurn"] call kjtCheck
[(getDammage kjdBurn) > (kjdBurn0), "getDammage kjdBurn > kjdBurn0"] call kjtCheck
[(getDammage kjdFar) == (0), "getDammage kjdFar == 0"] call kjtCheck
[(getDammage kjdTank) >= (1), "getDammage kjdTank >= 1"] call kjtCheck
[(kjdShake) > (0.2), "kjdShake > 0.2"] call kjtCheck
// every other feature's loop was told to stop
[!KJ_crushOn, "!KJ_crushOn"] call kjtCheck
[!KJ_presenceOn, "!KJ_presenceOn"] call kjtCheck
[!KJ_furnaceOn, "!KJ_furnaceOn"] call kjtCheck
[!KJ_walkOn, "!KJ_walkOn"] call kjtCheck
[KJ_beamAbort, "KJ_beamAbort"] call kjtCheck

// ---- the body: dark, silent, harmless -------------------------------------------------
kjdG2 = createGroup civilian
kjdVisit = objNull
kjdVisit2 = objNull
"Civilian" createUnit [[4690, 11300, 0], kjdG2, "kjdVisit = this", 0.5, "PRIVATE"]
"Civilian2" createUnit [[4692, 11300, 0], kjdG2, "kjdVisit2 = this", 0.5, "PRIVATE"]
triSimFrames 20
// one against his flank half way along the body, one at his feet
if (true) then { kjdVisit setPos ([20, 15] call kjtRel); kjdVisit2 setPos ([-6, 0] call kjtRel); { _x stop true; _x disableAI "MOVE" } forEach [kjdVisit, kjdVisit2] }
triSetView ([70, 95, 7, 6, 20] call kjtView)
triSimUntil { time > kjdTb + 6 }
triScreenshot "d09_body"
logInfo format ["KJD the section after his death: fleeing %1 (was %2), fear off %3", fleeing kjcF1, kjdFled, KJ_fearOff]
[!(fleeing kjcF1), "!(fleeing kjcF1)"] call kjtCheck
[KJ_fearOff, "KJ_fearOff"] call kjtCheck
if (KJCB_native) then { kjdLights = kjLightCount kjtDed; kjdShake2 = kjShakeGain } else { kjdLights = 0; kjdShake2 = 0 }
triSimUntil { time > kjdTb + 22 }
triSetView ([110, 45, 1.7, 5, 14] call kjtView)
triSimFrames 6
triScreenshot "d10_body_close"
logInfo format ["KJD the body: visitors dmg %1 %2 alive %3 %4, far dmg %5, lights %6, shake %7 then %8, his pos %9 dmg %10", getDammage kjdVisit, getDammage kjdVisit2, alive kjdVisit, alive kjdVisit2, getDammage kjdFar, kjdLights, kjdShake, kjdShake2, getPos kjtDed, getDammage kjtDed]
[(getDammage kjdVisit) == (0), "getDammage kjdVisit == 0"] call kjtCheck
[(getDammage kjdVisit2) == (0), "getDammage kjdVisit2 == 0"] call kjtCheck
[alive kjdVisit, "alive kjdVisit"] call kjtCheck
[alive kjdVisit2, "alive kjdVisit2"] call kjtCheck
[(getDammage kjdFar) == (0), "getDammage kjdFar == 0"] call kjtCheck
[(kjdLights) == (0), "kjdLights == 0"] call kjtCheck
[!(alive kjtDed), "!(alive kjtDed)"] call kjtCheck
triAssert [!(isNull kjtDed)]
"combat_death" call kjtVerdict
triEndTest
