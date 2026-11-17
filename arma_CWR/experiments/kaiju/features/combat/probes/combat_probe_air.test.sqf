// ============================================================================
//  Dedushka COMBAT, live-fire survey of aircraft (a diagnostic, not part of
//  the gate). WEAPONS ARE FIRED. Which weapon a gunship chooses against him,
//  how close it flies, what its ordnance is worth. He cannot die here.
//    1. a Mi-24 (30 mm, AT-6, 57 mm rockets) and a Mi-17 (57 mm rockets),
//       taking off 800 m up the runway, for three minutes;
//    2. then a West Cobra (30 mm, Hellfire, Zuni) from the same place, alone,
//       for two and a half.
//  Run once with the addon's armor at 120 (the 30 mm passes the AI's gates)
//  and once at 500 (it does not): the difference is the finding.
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
if (true) then { KJCB_POOL = 100000; KJCB_LOG = 300; KJCB_RAW = 40 }
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 40
logInfo format ["KJA creature %1 armor %2 x %3, rise factor %4, glow %5", typeOf kjtDed, (typeOf kjtDed) GetVehicleParam "armor", (typeOf kjtDed) GetVehicleParam "armorStructural", KJCB_riseK, KJCB_GLOW]
kjaCam = "camera" camCreate [4900, 11000, 30]
kjaCam cameraEffect ["internal", "back"]
if (true) then { kjaCam camSetTarget kjtDed; kjaCam camSetFov 0.7; kjaCam camCommit 0 }
triSetView ([30, 110, 4, 24] call kjtView)
triSimFrames 10
triScreenshot "a00_discard"

// ---- 1. Mi-24 and Mi-17 -----------------------------------------------------------------
kjaG1 = east call kjtGroup
kjaG2 = east call kjtGroup
kjaHind = ["Mi24", [800, -20] call kjtRel, 180, kjaG1, "SoldierEPilot", ["driver", "gunner"]] call kjtVehicle
kjaHip = ["Mi17", [840, 30] call kjtRel, 180, kjaG2, "SoldierEPilot", ["driver", "gunner"]] call kjtVehicle
triSimFrames 20
call kjtMount
triSimFrames 20
if (true) then { { _x engineOn true; _x flyInHeight 70 } forEach [kjaHind, kjaHip]; kjaG1 reveal kjtDed; kjaG2 reveal kjtDed; kjaT = time; kjaTick = 0; kjaMin = 9999 }
kjaLog = { if ((kjaHind distance kjtDed) < kjaMin) then { kjaMin = kjaHind distance kjtDed }; if (time > kjaT + kjaTick) then { kjaTick = kjaTick + 15; logInfo format ["KJA t %1 hind %2 d %3 alive %4, hip %5 d %6 alive %7, shots %8, real %9 stings %10 last %11", time - kjaT, getPos kjaHind, kjaHind distance kjtDed, alive kjaHind, getPos kjaHip, kjaHip distance kjtDed, alive kjaHip, kjtShots, KJ_combatStats select 2, KJ_combatStats select 1, KJ_combatLast] } }
triSimUntil { call kjaLog; ((KJ_combatStats select 2) >= 1) or (time > kjaT + 90) }
triSimFrames 4
triScreenshot "a01_first_real_hit"
triSimUntil { call kjaLog; time > kjaT + 100 }
triScreenshot "a02_under_air_attack"
triSimUntil { call kjaLog; time > kjaT + 180 }
logInfo format ["KJA east end: hind closest %1 m; %2", kjaMin, call kjtBook]
if (true) then { kjaHind setDammage 1; kjaHip setDammage 1 }
triSimFrames 200

// ---- 2. a Cobra --------------------------------------------------------------------------
kjaG3 = west call kjtGroup
kjaCobra = ["Cobra", [800, 0] call kjtRel, 180, kjaG3, "SoldierWPilot", ["driver", "gunner"]] call kjtVehicle
triSimFrames 20
call kjtMount
triSimFrames 20
if (true) then { kjaCobra engineOn true; kjaCobra flyInHeight 70; kjaG3 reveal kjtDed; kjaT = time; kjaTick = 0; kjaMin = 9999 }
kjaLog = { if ((kjaCobra distance kjtDed) < kjaMin) then { kjaMin = kjaCobra distance kjtDed }; if (time > kjaT + kjaTick) then { kjaTick = kjaTick + 15; logInfo format ["KJA t %1 cobra %2 d %3 alive %4 knows %5, shots %6, real %7 stings %8 last %9", time - kjaT, getPos kjaCobra, kjaCobra distance kjtDed, alive kjaCobra, kjaCobra knowsAbout kjtDed, kjtShots, KJ_combatStats select 2, KJ_combatStats select 1, KJ_combatLast] } }
triSimUntil { call kjaLog; time > kjaT + 150 }
logInfo format ["KJA west end: cobra closest %1 m; %2", kjaMin, call kjtBook]
triScreenshot "a03_end"
triAssert [alive kjtDed]
KJ_combatOn = false
triEndTest
