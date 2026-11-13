// ============================================================================
//  Occupier forces director, both towns (OCCUPIER-FORCES.md phase 3, 6.2 and
//  6.3; native OccupierForces + CounterAttack).  Guerrilla.Abel opts in
//  (occupierForces = 1).  The player stands 2.5 km off, so both towns rise on
//  the remote tick with nobody watching:
//    b1  the pool is sized at boot, DORMANT, no tokens;
//    b2  Houdan (Village) is emptied and raised: the first distinct town
//        keeps the director DORMANT and goes to the legacy Decide (its 0.4
//        roll tuned to 0 here, so no legacy record clutters the list);
//    b3  the nearest other neutral town is raised: the second distinct town
//        wakes the director with two tokens (invariant W), and within a few
//        director ticks (tuned to 1 s) it spends both: two operations in
//        gmOccupierOps with distinct ids, kind RETAKE, origin DIRECTOR, one
//        per town, each from an occupier base, and the tokens drop to 0.
//  Houdan's heat is raised before the second flip so it stays a candidate
//  whatever its flip roll (a fresh flip already is; the raise makes the
//  minute between the two flips irrelevant).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
bfT0 = time
triSimUntil { time > bfT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- b1: the pool, DORMANT -------------------------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [((gmOccupierForces) select 3), 0]
triAssertEq [(count gmOccupierOps), 0]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]

bfRes = gmResistanceSide call GM_fnSideFromString
bfA = gmZoneIndex "Village"
triAssertGe [bfA, 0]
bfAPos = (gmZone bfA) select 8
// the second town: the nearest other neutral, non-exempt CITY
bfB = -1
bfBest = 1000000
bfI = 0
while {bfI < gmZoneCount} do {bfZ = gmZone bfI; if (((bfZ select 1) == "CITY") and ((bfZ select 2) == "NEUTRAL") and (bfI != bfA) and (not ((gmZoneSim bfI) select 10))) then {bfD = (bfZ select 8) distance bfAPos; if (bfD < bfBest) then {bfBest = bfD; bfB = bfI}}; bfI = bfI + 1}
triAssertGe [bfB, 0]
bfBPos = (gmZone bfB) select 8
bfBName = (gmZone bfB) select 0

// leave: both towns cache their garrisons back to stored reserves
bfFar = [7922, 8500, 0]
player setPos bfFar
triSimUntil { not ((gmZoneSim bfA) select 0) }
triSimUntil { not ((gmZoneSim bfB) select 0) }
triSimUntil { not (gmGarrisonSpawned bfA) }
triSimUntil { not (gmGarrisonSpawned bfB) }

// ---- b2: town 1 rises (DORMANT, legacy) ------------------------------------
gmZoneSet [bfA, "garrison", 0]
gmZoneSet [bfA, "support", 55]
bfSqA = [bfRes, gmFactionSquad [gmResistanceSide, 1, 1], bfAPos] call GM_fnSpawnSquad
bfSqA setCombatMode "BLUE"
{_x allowDammage false} forEach (units bfSqA)
doStop (units bfSqA)
triSimUntil { ((gmZone bfA) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units bfSqA)
bfT1 = time
triSimUntil { time > bfT1 + 3 }
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [((gmOccupierForces) select 3), 0]
triAssertEq [(count gmOccupierOps), 0]
gmZoneSet [bfA, "heatRaise", 20]

// ---- b3: town 2 rises: AWAKE, two operations --------------------------------
gmZoneSet [bfB, "garrison", 0]
gmZoneSet [bfB, "support", 55]
bfSqB = [bfRes, gmFactionSquad [gmResistanceSide, 1, 1], bfBPos] call GM_fnSpawnSquad
bfSqB setCombatMode "BLUE"
{_x allowDammage false} forEach (units bfSqB)
doStop (units bfSqB)
triSimUntil { ((gmZone bfB) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units bfSqB)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
bfOps = []
triSimUntil { bfOps = []; {if ((_x select 2) == "DIRECTOR") then {bfOps = bfOps + [_x]}} forEach gmOccupierOps; (count bfOps) >= 2 }
triAssertEq [(count bfOps), 2]
bfOp1 = bfOps select 0
bfOp2 = bfOps select 1
triAssertGe [(bfOp1 select 0), 1]
triAssertGe [(bfOp2 select 0), 1]
triAssertEq [(format ["%1", (bfOp1 select 0) != (bfOp2 select 0)]), "true"]
triAssertEq [(bfOp1 select 1), "RETAKE"]
triAssertEq [(bfOp2 select 1), "RETAKE"]
triAssertEq [(format ["%1", (bfOp1 select 5) != (bfOp2 select 5)]), "true"]
bfTargets = [(bfOp1 select 5), (bfOp2 select 5)]
triAssertEq [(format ["%1", "Village" in bfTargets]), "true"]
triAssertEq [(format ["%1", bfBName in bfTargets]), "true"]
triAssertEq [(format ["%1", (bfOp1 select 4) != ""]), "true"]
triAssertGe [(bfOp1 select 7), 4]
triAssertGe [(bfOp2 select 7), 4]
triAssertEq [((gmOccupierForces) select 3), 0]
gmSimDiagLog

triEndTest
