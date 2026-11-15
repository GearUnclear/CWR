// ============================================================================
//  Dedushka COMBAT, live-fire survey (a diagnostic, not part of the gate).
//  WEAPONS ARE FIRED. Who opens fire on him, with what, from where; where the
//  rounds land on his body; what each is worth. He cannot die here (the pool
//  is set out of reach). KJP lines in game_stdout.log are the findings,
//  KJCOMBAT raw / hit lines are single reports.
//    1. an East infantry squad 210 m in front of him: rifles, a PK, a grenade
//       launcher, an RPG, an AT4, a Strela;
//    2. two T-80s 440 m in front of him and a BMP 320 m to his front right;
//    3. a Mi-24 taking off from the far end of the airfield.
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
// the lie of the land: ground heights every 50 m on four lines out from where he stands
if (true) then { kjpR = []; kjpI = 0; while { kjpI <= 900 } do { kjpR = kjpR + [[4800, 11200 - kjpI] call kjtGround]; kjpI = kjpI + 50 }; logInfo format ["KJP ground south %1", kjpR] }
if (true) then { kjpR = []; kjpI = 0; while { kjpI <= 900 } do { kjpR = kjpR + [[4800 + kjpI, 11200] call kjtGround]; kjpI = kjpI + 50 }; logInfo format ["KJP ground east %1", kjpR] }
if (true) then { kjpR = []; kjpI = 0; while { kjpI <= 900 } do { kjpR = kjpR + [[4800 - kjpI, 11200] call kjtGround]; kjpI = kjpI + 50 }; logInfo format ["KJP ground west %1", kjpR] }
if (true) then { kjpR = []; kjpI = 0; while { kjpI <= 900 } do { kjpR = kjpR + [[4800, 11200 + kjpI] call kjtGround]; kjpI = kjpI + 50 }; logInfo format ["KJP ground north %1", kjpR] }
if (true) then { kjpR = []; { kjpR = kjpR + [count (nearestObjects [[4800, 11200 - _x, 0], [], 40])] } forEach [0, 100, 200, 300, 400, 500, 600, 700, 800]; logInfo format ["KJP objects within 40 m, south line every 100 m: %1", kjpR] }
[kjtClass, 180] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
if (true) then { KJCB_POOL = 100000; KJCB_LOG = 500; KJCB_RAW = 160 }
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 40
logInfo format ["KJP creature %1 side %2 height %3 native %4 furnace %5 move %6 pos %7", typeOf kjtDed, side kjtDed, KJCB_H, KJCB_native, KJCB_furnace, getMove kjtDed, getPosASL kjtDed]
if (KJCB_native) then { logInfo format ["KJP traits: ceiling %1 report %2 farDraw %3 wade %4 blind %5", kjGet [kjtDed, "damageCeiling"], kjGet [kjtDed, "hitReport"], kjGet [kjtDed, "farDraw"], kjGet [kjtDed, "wadeDepth"], kjGet [kjtDed, "obstacleBlind"]] }
kjpCam = "camera" camCreate [4900, 11000, 30]
kjpCam cameraEffect ["internal", "back"]
if (true) then { kjpCam camSetTarget kjtDed; kjpCam camSetFov 0.7; kjpCam camCommit 0 }
triSetView ([150, 230, 14, 22] call kjtView)
triSimFrames 10
triScreenshot "p00_discard"
triScreenshot "p01_before"

// ---- 1. infantry ----------------------------------------------------------------
kjpGi = east call kjtGroup
[["SoldierEB", "SoldierEB", "SoldierEMG", "SoldierEG", "SoldierELAW", "SoldierEAT", "SoldierEAA"], [4790, 10990], kjpGi] call kjtMen
triSimFrames 40
logInfo format ["KJP infantry made: %1 men, side %2, at %3, ground %4", count kjtMenAll, side (kjtMenAll select 0), getPos (kjtMenAll select 0), [4790, 10990] call kjtGround]
if (true) then { kjpGi reveal kjtDed; kjpT = time; kjpTick = 0 }
triSimUntil { if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP inf t %1 knows %2 behaviour %3 shots %4 hurt %5 last %6", time - kjpT, (kjtMenAll select 0) knowsAbout kjtDed, behaviour (kjtMenAll select 0), kjtShots, KJ_hurtCount, KJ_combatLast] }; time > kjpT + 35 }
triScreenshot "p02_infantry"
triSimUntil { if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP inf t %1 knows %2 behaviour %3 shots %4 hurt %5 last %6", time - kjpT, (kjtMenAll select 0) knowsAbout kjtDed, behaviour (kjtMenAll select 0), kjtShots, KJ_hurtCount, KJ_combatLast] }; time > kjpT + 75 }
logInfo format ["KJP infantry end: %1", call kjtBook]
if (true) then { kjpR = []; { kjpR = kjpR + [[typeOf _x, kjtDed distance _x, alive _x, fleeing _x]] } forEach kjtMenAll; logInfo format ["KJP infantry where: %1", kjpR] }
call kjtCease

// ---- 2. armour ------------------------------------------------------------------
kjpGa = east call kjtGroup
kjpGb = east call kjtGroup
kjpT80a = ["T80", [4765, 10760], 0, kjpGa, "SoldierECrew", ["driver", "gunner", "commander"]] call kjtVehicle
kjpT80b = ["T80", [4835, 10760], 0, kjpGa, "SoldierECrew", ["driver", "gunner", "commander"]] call kjtVehicle
kjpBmp = ["BMP", [4580, 10970], 45, kjpGb, "SoldierECrew", ["driver", "gunner", "commander"]] call kjtVehicle
triSimFrames 20
call kjtMount
triSimFrames 40
logInfo format ["KJP armour made: crews %1 of %2, T80a %3 gunner %4, bmp %5, ground %6 %7", count kjtCrewAll, kjtCrewAsked, getPos kjpT80a, gunner kjpT80a, getPos kjpBmp, [4765, 10760] call kjtGround, [4580, 10970] call kjtGround]
if (true) then { kjpGa reveal kjtDed; kjpGb reveal kjtDed; kjpT = time; kjpTick = 0 }
triSimUntil { if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP arm t %1 knows %2 %3 shots %4 hurt %5 health %6 last %7", time - kjpT, kjpT80a knowsAbout kjtDed, kjpBmp knowsAbout kjtDed, kjtShots, KJ_hurtCount, KJ_woundHealth, KJ_combatLast] }; time > kjpT + 40 }
triScreenshot "p03_armour"
triSimUntil { if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP arm t %1 knows %2 %3 shots %4 hurt %5 health %6 last %7", time - kjpT, kjpT80a knowsAbout kjtDed, kjpBmp knowsAbout kjtDed, kjtShots, KJ_hurtCount, KJ_woundHealth, KJ_combatLast] }; time > kjpT + 100 }
logInfo format ["KJP armour end: %1", call kjtBook]
logInfo format ["KJP armour where: T80a %1 m, T80b %2 m, BMP %3 m, ammo left Gun120 %4, Gun73 %5", kjtDed distance kjpT80a, kjtDed distance kjpT80b, kjtDed distance kjpBmp, kjpT80a ammo "Gun120", kjpBmp ammo "Gun73"]
call kjtCease

// ---- 3. a gunship ----------------------------------------------------------------
kjpGh = east call kjtGroup
kjpHind = ["Mi24", [4640, 11520], 180, kjpGh, "SoldierEPilot", ["driver", "gunner"]] call kjtVehicle
triSimFrames 20
call kjtMount
triSimFrames 20
if (true) then { kjpHind engineOn true; kjpHind flyInHeight 70; kjpGh reveal kjtDed; kjpT = time; kjpTick = 0; kjpMinD = 9999; kjpMaxH = 0 }
triSimUntil { if ((kjpHind distance kjtDed) < kjpMinD) then { kjpMinD = kjpHind distance kjtDed }; if (((getPos kjpHind) select 2) > kjpMaxH) then { kjpMaxH = (getPos kjpHind) select 2 }; if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP air t %1 knows %2 pos %3 dist %4 shots %5 hurt %6 health %7 last %8 alive %9", time - kjpT, kjpHind knowsAbout kjtDed, getPos kjpHind, kjpHind distance kjtDed, kjtShots, KJ_hurtCount, KJ_woundHealth, KJ_combatLast, alive kjpHind] }; time > kjpT + 70 }
triScreenshot "p04_gunship"
triSimUntil { if ((kjpHind distance kjtDed) < kjpMinD) then { kjpMinD = kjpHind distance kjtDed }; if (((getPos kjpHind) select 2) > kjpMaxH) then { kjpMaxH = (getPos kjpHind) select 2 }; if (time > kjpT + kjpTick) then { kjpTick = kjpTick + 15; logInfo format ["KJP air t %1 knows %2 pos %3 dist %4 shots %5 hurt %6 health %7 last %8 alive %9", time - kjpT, kjpHind knowsAbout kjtDed, getPos kjpHind, kjpHind distance kjtDed, kjtShots, KJ_hurtCount, KJ_woundHealth, KJ_combatLast, alive kjpHind] }; time > kjpT + 150 }
logInfo format ["KJP gunship end: closest %1 m, highest %2 m, alive %3, %4", kjpMinD, kjpMaxH, alive kjpHind, call kjtBook]
triScreenshot "p05_end"
triAssert [alive kjtDed]
triAssertGt [kjtShotCount, 0]
KJ_combatOn = false
triEndTest
