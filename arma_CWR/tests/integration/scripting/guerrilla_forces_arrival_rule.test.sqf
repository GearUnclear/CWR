// ============================================================================
//  Occupier forces, the arrival rule in the package choice (OCCUPIER-FORCES.md
//  6.5, 16 forcesSlowEta / forcesSlowCostShare / forcesAirEta; owner,
//  2026-10-15).  Boots the SHIPPED Guerrilla.Abel template (occupierForces
//  = 1, occupier EAST: Ural truck, Mi17 transport helicopter) on the full
//  Remaster:
//    a1  a real camera at the default view distance in daylight; the pool
//        full (STRONG); test levers: director tick 1 s, one distinct town
//        wakes the director, the first-town roll 0; the pool is forced to 0
//        while Houdan rises remotely (the wake launches nothing), Houdan
//        rests on a long cooldown and the pool is restored: STRONG with two
//        tokens;
//    a2  two towns are picked by their straight distance to the nearest
//        ground base (Outpost, Le Port Harbour, Airfield), more than 850 m
//        from the player: a near one at 1.4 to 2.25 km, the nearest to
//        1.8 km (La Trinite, 1.7 km: on foot about 1140 s, past
//        forcesSlowEta 900, under forcesAirEta 1500), and a far one past
//        2.7 km (Goisse, 4.6 km: on foot over 3000 s);
//    a3  the near town is handed to the resistance, hot: the director's
//        operation on it is a TRUCK (the truck costs 2 x foot, inside the
//        3 x slow share; the helicopter, 4 x, is not);
//    a4  the far town is handed over, hot: the director's operation on it
//        is a TRANSPORT_HELI insertion (past forcesAirEta at any price).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
arT0 = time
triSimUntil { time > arT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: a real camera; the levers; the director wakes ----------------------
arP = gmTrafficPercept (getPos player)
triAssertEq [(arP select 0), 1]
triAssertGe [(arP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
arStart = (gmOccupierForces) select 1
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
arRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
// the player stands north, away from every base (no base materialized)
player setPos [7922, 8500, 0]
arV = gmZoneIndex "Village"
triAssertGe [arV, 0]
arVPos = (gmZone arV) select 8
triSimUntil { not (gmGarrisonSpawned arV) }
gmZoneSet [arV, "garrison", 0]
gmZoneSet [arV, "support", 55]
arSq = [arRes, gmFactionSquad [gmResistanceSide, 1, 1], arVPos] call GM_fnSpawnSquad
arSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units arSq)
doStop (units arSq)
triSimUntil { ((gmZone arV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units arSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", arV, 99999]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", arStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 3), 2]

// ---- a2: a near town and a far town by the nearest ground base --------------
arBases = [((gmZone (gmZoneIndex "Outpost")) select 8), ((gmZone (gmZoneIndex "Le Port Harbour")) select 8), ((gmZone (gmZoneIndex "Airfield")) select 8)]
arNear = -1
arFar = -1
arBest = 99999
arI = 0
while {arI < gmZoneCount} do {arZ = gmZone arI; arD = 99999; {if ((_x distance (arZ select 8)) < arD) then {arD = _x distance (arZ select 8)}} forEach arBases; if (((arZ select 1) == "CITY") and ((arZ select 2) != gmResistanceSide) and (not ((gmZoneSim arI) select 10)) and (((gmZoneSim arI) select 7) <= 4) and (((arZ select 8) distance (getPos player)) > 850)) then {if ((arD > 1400) and (arD < 2250) and ((abs (arD - 1800)) < arBest)) then {arNear = arI; arBest = abs (arD - 1800)}; if ((arD > 2700) and (arFar < 0)) then {arFar = arI}}; arI = arI + 1}
triAssertGe [arNear, 0]
triAssertGe [arFar, 0]
arNearName = (gmZone arNear) select 0
arFarName = (gmZone arFar) select 0
triSimUntil { (not (gmGarrisonSpawned arNear)) and (not (gmGarrisonSpawned arFar)) }

// ---- a3: the near town gets a truck ------------------------------------------
gmZoneSet [arNear, "garrison", 0]
gmZoneSet [arNear, "owner", gmResistanceSide]
gmZoneSet [arNear, "heatRaise", 100]
arOp = []
triSimUntil { arOp = []; {if ((_x select 5) == arNearName) then {arOp = _x}} forEach gmOccupierOps; (count arOp) > 0 }
triAssertEq [(arOp select 1), "RETAKE"]
triAssertEq [(arOp select 2), "DIRECTOR"]
triAssertEq [(arOp select 3), "TRUCK"]

// ---- a4: the far town gets a helicopter insertion ----------------------------
gmZoneSet [arFar, "garrison", 0]
gmZoneSet [arFar, "owner", gmResistanceSide]
gmZoneSet [arFar, "heatRaise", 100]
arOp2 = []
triSimUntil { arOp2 = []; {if ((_x select 5) == arFarName) then {arOp2 = _x}} forEach gmOccupierOps; (count arOp2) > 0 }
triAssertEq [(arOp2 select 1), "RETAKE"]
triAssertEq [(arOp2 select 2), "DIRECTOR"]
triAssertEq [(arOp2 select 3), "TRANSPORT_HELI"]
gmSimDiagLog

triEndTest
