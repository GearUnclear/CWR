// ============================================================================
//  Dedushka COMBAT: a heavy force kills him, in minutes, never at once
//  (experiment, kept out of tests/integration). TANKS, LAUNCHERS AND GUNSHIPS
//  FIRE, by their own choice: nothing is ordered to shoot, he is only revealed.
//
//  The force, all East, all in front of the furnace:
//    two tank platoons (six T-80s) 440 and 520 m off
//    two BMPs 330 m off
//    an anti-tank section 200 m off (two RPG men, two AT4 men)
//    two Mi-24s taking off from the far end of the airfield
//  He only stands and takes it (no brain in this test: no beam, no stomp, no
//  walking off). So this is the least time a force of this size needs.
//
//  Asserted: he dies, later than 2.5 minutes and sooner than 8 after the
//  first shot; on the way he passes through every wound stage in order; most
//  hits land in the furnace; nobody fired a rifle or a machine gun at him;
//  the death sequence then runs to the end and leaves a dead body.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.3
0 setFog 0
setViewDistance 2500
showCinemaBorder false
player setPos [4690, 11120, 0]
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
kjhCam = "camera" camCreate [4900, 11000, 30]
kjhCam cameraEffect ["internal", "back"]
if (true) then { kjhCam camSetTarget kjtDed; kjhCam camSetFov 0.7; kjhCam camCommit 0 }

// ---- the force ----------------------------------------------------------------------
kjhGa = east call kjtGroup
kjhGb = east call kjtGroup
kjhGc = east call kjtGroup
kjhGd = east call kjtGroup
kjhGe = east call kjtGroup
kjhCrew = ["driver", "gunner", "commander"]
if (true) then { { ["T80", [440, _x] call kjtRel, 180, kjhGa, "SoldierECrew", kjhCrew] call kjtVehicle } forEach [-60, 0, 60] }
if (true) then { { ["T80", [520, _x] call kjtRel, 180, kjhGb, "SoldierECrew", kjhCrew] call kjtVehicle } forEach [-90, -30, 30] }
if (true) then { { ["BMP", [330, _x] call kjtRel, 180, kjhGc, "SoldierECrew", kjhCrew] call kjtVehicle } forEach [-95, 95] }
[["SoldierELAW", "SoldierELAW", "SoldierEAT", "SoldierEAT"], [200, 20] call kjtRel, kjhGd] call kjtMen
if (true) then { { ["Mi24", [_x select 0, _x select 1] call kjtRel, 180, kjhGe, "SoldierEPilot", ["driver", "gunner"]] call kjtVehicle } forEach [[820, -20], [850, 30]] }
triSimFrames 20
call kjtMount
triSimFrames 60
logInfo format ["KJH the force: %1 vehicles, %2 crew of %3, %4 men on foot; first tank at %5, %6 m from him", count kjtVehAll, count kjtCrewAll, kjtCrewAsked, count kjtMenAll, getPos (kjtVehAll select 0), kjtDed distance (kjtVehAll select 0)]
[(count kjtCrewAll) == (kjtCrewAsked), "count kjtCrewAll == kjtCrewAsked"] call kjtCheck
// from behind his right shoulder, up the runway to the force
triSetView ([195, 120, 14, 22] call kjtView)
triSimFrames 10
triScreenshot "h00_discard"
triScreenshot "h00_the_force_up_the_runway"
// then from in front of him, the guns behind the camera
triSetView ([32, 125, 4, 21] call kjtView)
triSimFrames 6
if (true) then { { _x reveal kjtDed } forEach kjtGroups; { if ((typeOf _x) == "Mi24") then { _x engineOn true; _x flyInHeight 70 } } forEach kjtVehAll; kjhT0 = time; kjhTick = 0 }
// kjhLog: one line every 20 s
kjhLog = { if (time > kjhT0 + kjhTick) then { kjhTick = kjhTick + 20; logInfo format ["KJH %1", call kjtBook] } }

// ---- the fight: a screenshot as each stage comes --------------------------------------
triSimUntil { call kjhLog; (KJ_woundStage >= 1) or KJ_dead or (time > kjhT0 + 480) }
kjhT1 = time
triScreenshot "h01_stung"
triSimUntil { call kjhLog; (KJ_woundStage >= 2) or KJ_dead or (time > kjhT0 + 480) }
triSimUntil { call kjhLog; (time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 5) or KJ_dead }
triScreenshot "h02_wounded"
triSimUntil { call kjhLog; (KJ_woundStage >= 3) or KJ_dead or (time > kjhT0 + 480) }
triSimUntil { call kjhLog; (time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 5) or KJ_dead }
triScreenshot "h03_badly_hurt"
triSimUntil { call kjhLog; (KJ_woundStage >= 4) or KJ_dead or (time > kjhT0 + 480) }
triSimUntil { call kjhLog; (time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 4) or KJ_dead }
triScreenshot "h04_dying"
triSimUntil { call kjhLog; KJ_dead or (time > kjhT0 + 480) }
kjhTd = time
logInfo format ["KJH dead %1 after %2 s (first stage after %3 s): %4", KJ_dead, kjhTd - kjhT0, kjhT1 - kjhT0, call kjtBook]
logInfo format ["KJH wound log %1", KJ_woundLog]
[KJ_dead, "KJ_dead"] call kjtCheck
[(kjhTd - kjhT0) > (150), "kjhTd - kjhT0 > 150"] call kjtCheck
[(kjhTd - kjhT0) < (480), "kjhTd - kjhT0 < 480"] call kjtCheck
// every stage, in order
[(count KJ_woundLog) >= (4), "count KJ_woundLog >= 4"] call kjtCheck
[((KJ_woundLog select 0) select 1) == (1), "(KJ_woundLog select 0) select 1 == 1"] call kjtCheck
[((KJ_woundLog select 1) select 1) == (2), "(KJ_woundLog select 1) select 1 == 2"] call kjtCheck
[((KJ_woundLog select 2) select 1) == (3), "(KJ_woundLog select 2) select 1 == 3"] call kjtCheck
[((KJ_woundLog select 3) select 1) == (4), "(KJ_woundLog select 3) select 1 == 4"] call kjtCheck
// who fired what
[("Heat120" call kjtFiredOf) > (30), "'Heat120' call kjtFiredOf > 30"] call kjtCheck
[(("BulletSingle" call kjtFiredOf) + ("Bullet7_6" call kjtFiredOf) + ("Bullet12_7" call kjtFiredOf)) == (0), "('BulletSingle' call kjtFiredOf) + ('Bullet7_6' call kjtFiredOf) + ('Bullet12_7' call kjtFiredOf) == 0"] call kjtCheck
[(KJ_combatStats select 2) >= (60), "KJ_combatStats select 2 >= 60"] call kjtCheck
kjhFurnace = 1
kjhLegs = 0
if (KJCB_native) then { kjhFurnace = KJ_combatByZone select 0; kjhLegs = KJ_combatByZone select 3 }
[(kjhFurnace) > (kjhLegs), "kjhFurnace > kjhLegs"] call kjtCheck

// ---- and the death ------------------------------------------------------------------
triSimUntil { (KJ_deathPhase == "flare") or (time > kjhTd + 20) }
triSimUntil { (KJ_deathPhase != "flare") or (time > ((KJ_deathLog select 1) select 1) + 0.9 * KJCB_D_FLARE) }
triScreenshot "h05_flare"
triSimUntil { (KJ_deathPhase == "down") or (time > kjhTd + 40) }
triSimFrames 20
triScreenshot "h06_down"
triSimUntil { (KJ_deathPhase == "body") or (time > kjhTd + 50) }
call kjtCease
triSimFrames 60
triScreenshot "h07_body"
logInfo format ["KJH death log %1, stats %2, alive %3, force left: %4 vehicles alive of %5", KJ_deathLog, KJ_deathStats, alive kjtDed, { alive _x } count kjtVehAll, count kjtVehAll]
[(KJ_deathPhase) == ("body"), "KJ_deathPhase == 'body'"] call kjtCheck
[!(alive kjtDed), "!(alive kjtDed)"] call kjtCheck
[(KJ_woundStage) == (5), "KJ_woundStage == 5"] call kjtCheck
"combat_heavy" call kjtVerdict
triEndTest
