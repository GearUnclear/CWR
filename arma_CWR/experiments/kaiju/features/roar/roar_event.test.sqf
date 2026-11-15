// ============================================================================
//  Dedushka ROAR: the roar as an event (experiment, not a regression test).
//
//  The site is the south edge of Morton on Everon (the pipeline's photo site).
//  Dedushka stands at [5055, 3870] facing the town. Placed by the mission:
//    A  four EAST riflemen 120 m from him, in the south street   -> must break and run
//    B  three EAST riflemen 450 m from him, north of the town    -> must freeze and face him
//    C  three civilians 180 m from him                           -> must run, indoors if they can
//    D  one civilian 105 m from him, the pub 26 m behind him      -> must run (into the pub, if it qualifies)
//    a civilian Skoda, sent driving before the roar              -> must stop
//    E  two EAST riflemen 850 m from him                          -> must turn toward the sound
//    the player (EAST, captive, unarmed) 630 m from him          -> must be left alone
//  Everyone is EAST or civilian, so nobody fires. --nosound: the sound path is
//  proven by the config lookup (soundLength resolves the class through the
//  same FindSound as say), by KJ_sndLog and by the KJROAR lines in the log.
//
//  Each KJROARTEST line in game_stdout.log is a measurement.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
setViewDistance 1500
0 setFog 0
triSimFrames 30
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushka"]
// Hold him still, facing the town.
if (true) then { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 0 }
// The roar scripts load this themselves; loading it here first means every KJ_ global the test reads exists from the start.
if (true) then { call compile preprocessFile "kj\roar\init.sqf"; [ded] call KJR_fnSize }
logInfo format ["KJROARTEST height %1 m: break inside %2 m, freeze inside %3 m, civilians run inside %4 m, vehicles stop inside %5 m, heads turn inside %6 m", KJR_H, KJR_BREAK_R * KJR_H, KJR_FREEZE_R * KJR_H, KJR_CIV_R * KJR_H, KJR_VEH_R * KJR_H, KJR_ALERT_R * KJR_H]
// -- the sounds: every class the scripts use resolves in the merged config -----
reSnd = ["KJ_Roar", "KJ_RoarFar", "KJ_Step", "KJ_StepFar", "KJ_Furnace", "KJ_BeamCharge", "KJ_Beam", "KJ_BeamLoop", "KJ_Exhale", "KJ_Static", "KJ_StaticFaint", "KJ_Birds"]
if (true) then { reBad = []; { if ((soundLength _x) <= 0) then { reBad = reBad + [_x] } } forEach reSnd; logInfo format ["KJROARTEST sound classes resolved: %1 of %2, missing %3, unknown class reads %4", (count reSnd) - (count reBad), count reSnd, reBad, soundLength "KJ_NoSuchSound"] }
triAssertEq [count reBad, 0]
triAssertEq [soundLength "KJ_NoSuchSound", 0]
// -- a camera for the stills (HUD off); framing is by triSetView ---------------
reCam = "camera" camCreate [5050, 4040, 3]
reCam cameraEffect ["internal", "back"]
if (true) then { reCam camSetTarget [5055, 3870, 20]; reCam camSetFov 0.7; reCam camCommit 0 }
showCinemaBorder false
// Views are [eye x, eye height above the sea, eye z, direction x, y, z].
// From 40 m above the town north of section A, looking south over the roofs:
// the street A stands in, and him behind it. (At street level a row of houses
// is in the way.)
reViewA = [5050, 56.3, 4040, 0, -0.36, -1]
// At head height in front of the pub, looking at its door (civilian D's cover).
reViewD = [5091, 18.0, 3972, 0, 0.05, 1]
triSetView reViewA
triSimFrames 60
triScreenshot "00_warmup_discard"
// -- before -----------------------------------------------------------------------
if (true) then { reA = [kjrA1, kjrA2, kjrA3, kjrA4]; reB = [kjrB1, kjrB2, kjrB3]; reC = [kjrC1, kjrC2, kjrC3, kjrD1]; reA0 = []; { reA0 = reA0 + [_x distance ded] } forEach reA; reB0 = []; { reB0 = reB0 + [getPos _x] } forEach reB; reC0 = []; { reC0 = reC0 + [_x distance ded] } forEach reC }
logInfo format ["KJROARTEST before: A %1 m, C %2 m, B1 %3 m, car %4 m, player %5 m from him; A fleeing %6, moves %7 %8 %9", reA0, reC0, kjrB1 distance ded, kjrCar distance ded, player distance ded, fleeing kjrA1, getMove kjrA1, getMove kjrB1, getMove kjrC1]
triAssert [!(fleeing kjrA1)]
triAssert [!(fleeing kjrB1)]
triSetView reViewA
triSimFrames 6
triScreenshot "01_before_section_A_in_the_street"
// The car drives north, away from him, so it is moving when the roar comes.
if (true) then { kjrCar doMove [5030, 4700, 0]; reT = time }
triSimUntil { ((speed kjrCar) > 12) or (time > reT + 25) }
logInfo format ["KJROARTEST car before the roar: %1 km/h, %2 m from him", speed kjrCar, kjrCar distance ded]
triAssertGt [speed kjrCar, 12]
// -- the roar ----------------------------------------------------------------------
if (true) then { reT0 = time; reCarD = kjrCar distance ded; [ded] exec "kj\roar\roar.sqs" }
triSimUntil { KJ_roarCount > 0 }
// A second roar asked for while the first is still in his throat is dropped.
[ded] exec "kj\roar\roar.sqs"
triSimUntil { (count KJ_sndLog) > 0 }
// The listener is 630 m off: the near roar, heard 630 / 343 s after it is made.
logInfo format ["KJROARTEST roar sound: %1 (class, sent, heard, metres)", KJ_sndLog select 0]
triAssertEq [(KJ_sndLog select 0) select 0, "KJ_Roar"]
triAssertNear [((KJ_sndLog select 0) select 2) - ((KJ_sndLog select 0) select 1), (player distance ded) / 343, 0.15]
triAssertGt [((KJ_sndLog select 0) select 2) - ((KJ_sndLog select 0) select 1), 1.5]
// Section A has heard it after 0.35 s and is already running.
triSimUntil { time > reT0 + 4.5 }
triSetView reViewA
triSimFrames 3
triScreenshot "02_roar_plus_5s_section_A_breaks"
logInfo format ["KJROARTEST +4.5 s: A fleeing %1 %2 %3 %4, speed %5 km/h, panic log %6", fleeing kjrA1, fleeing kjrA2, fleeing kjrA3, fleeing kjrA4, speed kjrA1, KJ_panicLog]
// The roar has reached everyone 2 s after it started (B is the farthest, 450 m).
triSimUntil { (not KJ_roaring) or (time > reT0 + 15) }
logInfo format ["KJROARTEST roar log: %1", KJ_roarLog]
// Who reacted how.
reKind = { private ["_r"]; _r = "NONE"; { if ((_x select 0) == _this) then { _r = _x select 1 } } forEach KJ_roarLog; _r }
logInfo format ["KJROARTEST kinds: A %1 %2 %3 %4 | B %5 %6 %7 | C %8 %9 %10 | car %11 | player %12 | ded %13 | E %14 %15 | D %16", kjrA1 call reKind, kjrA2 call reKind, kjrA3 call reKind, kjrA4 call reKind, kjrB1 call reKind, kjrB2 call reKind, kjrB3 call reKind, kjrC1 call reKind, kjrC2 call reKind, kjrC3 call reKind, kjrCar call reKind, player call reKind, ded call reKind, kjrE1 call reKind, kjrE2 call reKind, kjrD1 call reKind]
triAssertEq [kjrA1 call reKind, "BREAK"]
triAssertEq [kjrA4 call reKind, "BREAK"]
triAssertEq [kjrB1 call reKind, "FREEZE"]
triAssertEq [kjrB3 call reKind, "FREEZE"]
triAssertEq [kjrC1 call reKind, "CIV"]
triAssertEq [kjrC3 call reKind, "CIV"]
triAssertEq [kjrD1 call reKind, "CIV"]
triAssertEq [kjrCar call reKind, "VEH"]
triAssertEq [kjrE1 call reKind, "ALERT"]
triAssertEq [kjrE2 call reKind, "ALERT"]
triAssertEq [player call reKind, "NONE"]
triAssertEq [ded call reKind, "NONE"]
// Fear travels at the speed of sound: nobody reacts before the roar reaches him,
// and nobody more than a second and a half after.
if (true) then { reEarly = 0; reLate = 0; { if ((_x select 3) < (((_x select 2) / 343) - 0.1)) then { reEarly = reEarly + 1 }; if ((_x select 3) > (((_x select 2) / 343) + 1.5)) then { reLate = reLate + 1 } } forEach KJ_roarLog; logInfo format ["KJROARTEST timing: %1 reactions, %2 too early, %3 too late", count KJ_roarLog, reEarly, reLate] }
triAssertEq [reEarly, 0]
triAssertEq [reLate, 0]
// The far section heard it later than the near one.
reWhen = { private ["_r"]; _r = -1; { if ((_x select 0) == _this) then { _r = _x select 3 } } forEach KJ_roarLog; _r }
logInfo format ["KJROARTEST heard: A1 +%1 s, C1 +%2 s, car +%3 s, B1 +%4 s", kjrA1 call reWhen, kjrC1 call reWhen, kjrCar call reWhen, kjrB1 call reWhen]
triAssertLt [kjrA1 call reWhen, kjrB1 call reWhen]
triAssertGt [kjrB1 call reWhen, 1.2]
// -- section A: broke, by the engine's own flee -----------------------------------
// rePath: how the group led by this unit is being moved, as panic.sqs wrote it down.
rePath = { private ["_r"]; _r = "NONE"; { if ((_x select 0) == _this) then { _r = _x select 2 } } forEach KJ_panicLog; _r }
triSimUntil { ((kjrA1 call rePath) != "NONE") or (time > reT0 + 12) }
logInfo format ["KJROARTEST A flee path: %1 at +%2 s, fleeing %3, combat mode %4, speed mode %5, %6 km/h", kjrA1 call rePath, time - reT0, fleeing kjrA1, combatMode kjrA1, speedMode kjrA1, speed kjrA1]
triAssert [fleeing kjrA1]
triAssertEq [kjrA1 call rePath, "native"]
triAssertEq [kjrC1 call rePath, "script"]
// -- the car: stopped ------------------------------------------------------------
triSimUntil { ((speed kjrCar) < 1) or (time > reT0 + 12) }
logInfo format ["KJROARTEST car after the roar: %1 km/h at +%2 s, stopped %3", speed kjrCar, time - reT0, stopped kjrCar]
triAssertLt [speed kjrCar, 1]
// -- section B: frozen, facing him, not fleeing ------------------------------------
triSimUntil { time > reT0 + 9 }
reFace = { private ["_b", "_d"]; _b = (((getPos ded) select 0) - ((getPos _this) select 0)) atan2 (((getPos ded) select 1) - ((getPos _this) select 1)); _d = abs ((getDir _this) - _b); if (_d > 180) then { _d = 360 - _d }; _d }
if (true) then { reBmoved = []; reI = 0; { reBmoved = reBmoved + [_x distance (reB0 select reI)]; reI = reI + 1 } forEach reB }
logInfo format ["KJROARTEST +9 s B: stopped %1 %2 %3, off his bearing by %4 %5 %6 deg, moved %7 m, fleeing %8, moves %9", stopped kjrB1, stopped kjrB2, stopped kjrB3, kjrB1 call reFace, kjrB2 call reFace, kjrB3 call reFace, reBmoved, fleeing kjrB1, getMove kjrB1]
triAssert [stopped kjrB1]
triAssert [stopped kjrB2]
triAssert [stopped kjrB3]
triAssert [!(fleeing kjrB1)]
triAssertLt [kjrB1 call reFace, 60]
triAssertLt [kjrB2 call reFace, 60]
triAssertLt [kjrB3 call reFace, 60]
triAssertLt [reBmoved select 0, 6]
// -- section E, 850 m off: heard it 2.5 s after it was made, and turned toward it ---
logInfo format ["KJROARTEST +9 s E: heard at +%1 s, off his bearing by %2 %3 deg (they stood facing north, 180 deg off), stopped %4, fleeing %5", kjrE1 call reWhen, kjrE1 call reFace, kjrE2 call reFace, stopped kjrE1, fleeing kjrE1]
triAssertGt [kjrE1 call reWhen, 2.2]
triAssert [!(stopped kjrE1)]
triAssert [!(fleeing kjrE1)]
triAssertLt [kjrE1 call reFace, 90]
// -- 30 s on: A has run from him, the civilians have run for cover -------------------
// (The leader gains 68 m in the first 20 s every time; the men behind him have to
// find their own way round the row of houses and gain 24 to 56 m by then.)
triSimUntil { time > reT0 + 30 }
if (true) then { reA1 = []; { reA1 = reA1 + [_x distance ded] } forEach reA; reC1 = []; reCgo = []; { reC1 = reC1 + [_x distance ded]; reCgo = reCgo + [_x distance (_x getVariable ["kjrFleeTo", [0, 0, 0]])] } forEach reC }
logInfo format ["KJROARTEST +30 s: A %1 m from him (was %2), speed %3 km/h; C %4 m from him (was %5), still %6 m from their cover %7; panic log %8", reA1, reA0, speed kjrA1, reC1, reC0, reCgo, kjrC1 getVariable ["kjrFleeTo", []], KJ_panicLog]
triAssertGt [(reA1 select 0) - (reA0 select 0), 60]
triAssertGt [(reA1 select 1) - (reA0 select 1), 30]
triAssertGt [(reA1 select 2) - (reA0 select 2), 30]
triAssertGt [(reA1 select 3) - (reA0 select 3), 30]
// A civilian has either put ground between himself and Dedushka or reached his cover.
reCok = { ((reC1 select _this) - (reC0 select _this) > 25) or ((reCgo select _this) < 8) }
triAssert [0 call reCok]
triAssert [1 call reCok]
triAssert [2 call reCok]
triAssert [3 call reCok]
// D: was a house chosen, and is he in it?
if (true) then { reDb = nearestBuilding kjrD1; reDto = kjrD1 getVariable ["kjrFleeTo", [0, 0, 0]] }
logInfo format ["KJROARTEST D1: cover %1, which is %2 m from the nearest enterable house (%3); he is %4 m from his cover and %5 m from him (was %6)", reDto, reDto distance reDb, typeOf reDb, kjrD1 distance reDto, kjrD1 distance ded, reC0 select 3]
triSetView reViewA
triSimFrames 3
triScreenshot "03_roar_plus_30s_square_empty"
triSetView reViewD
triSimFrames 3
triScreenshot "04_roar_plus_30s_the_pub_civilian_D_went_into"
// -- nobody was hurt by it, nobody fired --------------------------------------------
logInfo format ["KJROARTEST damage: A %1 %2 %3 %4, C %5, player %6, ded %7", getDammage kjrA1, getDammage kjrA2, getDammage kjrA3, getDammage kjrA4, getDammage kjrC1, getDammage player, getDammage ded]
triAssert [alive kjrA1]
triAssert [alive kjrC1]
triAssertLt [getDammage kjrA1, 0.05]
triAssertLt [getDammage kjrC1, 0.05]
// -- it passes: the scare is taken back and the engine ends the flee -----------------
triSimUntil { time > reT0 + 52 }
triSimUntil { (not (fleeing kjrA1)) or (time > reT0 + 80) }
logInfo format ["KJROARTEST +%9 s: A fleeing %1, damage %2 %3 %4 %5, B stopped %6, car stopped %7, errors %8", fleeing kjrA1, getDammage kjrA1, getDammage kjrA2, getDammage kjrA3, getDammage kjrA4, stopped kjrB1, stopped kjrCar, triErrorCount, time - reT0]
triAssert [!(fleeing kjrA1)]
triAssertEq [getDammage kjrA1, 0]
triAssert [!(stopped kjrB1)]
triAssert [!(stopped kjrCar)]
triAssertEq [KJ_roarCount, 1]
triEndTest
