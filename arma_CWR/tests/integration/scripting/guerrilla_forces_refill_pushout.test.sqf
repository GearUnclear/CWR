// ============================================================================
//  Occupier forces, the passive refill and the quiet player's push-out
//  (OCCUPIER-FORCES.md 5.2 / D8 / D15 the refill, 5.6 the bands, 6.4 push-out
//  on towns and bases).  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    q1  a real camera at the default view distance in daylight; test levers:
//        director tick 1 s, one distinct town wakes the director, the
//        first-town roll 0; the refill compressed to 0.05 h
//        (gmForcesForce "refillHours");
//    q2  the pool is forced to 0 (SPENT: nothing can launch); Houdan rises
//        remotely and wakes the director (two tokens), then rests on a long
//        cooldown so it is no heat candidate; a cold, far town is handed to
//        the resistance (heat 0: never a heat candidate, quiet: no sighting);
//    q3  the refill brings the pool back up through THIN and STRETCHED to
//        STRONG (D2 regrowth draws first);
//    q4  STRONG with a token and no heat candidate: the director sends a
//        push-out (gmOccupierOps origin "PUSHOUT") on the quiet town.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
rpT0 = time
triSimUntil { time > rpT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- q1: a real camera; the levers -------------------------------------------
rpP = gmTrafficPercept (getPos player)
triAssertEq [(rpP select 0), 1]
triAssertGe [(rpP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
rpStart = (gmOccupierForces) select 1
triAssertGe [rpStart, 24]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.05]]), "true"]
rpRes = gmResistanceSide call GM_fnSideFromString

// ---- q2: the pool to 0; Houdan wakes the director; a quiet held town ---------
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]
player setPos [7922, 8500, 0]
rpV = gmZoneIndex "Village"
triAssertGe [rpV, 0]
rpVPos = (gmZone rpV) select 8
triSimUntil { not (gmGarrisonSpawned rpV) }
gmZoneSet [rpV, "garrison", 0]
gmZoneSet [rpV, "support", 55]
rpSq = [rpRes, gmFactionSquad [gmResistanceSide, 1, 1], rpVPos] call GM_fnSpawnSquad
rpSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units rpSq)
doStop (units rpSq)
triSimUntil { ((gmZone rpV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units rpSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", rpV, 99999]]), "true"]
triAssertEq [(count gmOccupierOps), 0]
// the quiet town: a cold, non-exempt neutral CITY away from the player and
// from Houdan, handed to the resistance by script (heat 0)
rpQ = -1
rpI = 0
while {(rpI < gmZoneCount) and (rpQ < 0)} do {rpZ = gmZone rpI; if (((rpZ select 1) == "CITY") and ((rpZ select 2) == "NEUTRAL") and (not ((gmZoneSim rpI) select 10)) and ((rpZ select 6) < 1) and (((rpZ select 8) distance (getPos player)) > 900) and (((rpZ select 8) distance rpVPos) > 300)) then {rpQ = rpI}; rpI = rpI + 1}
triAssertGe [rpQ, 0]
rpQName = (gmZone rpQ) select 0
gmZoneSet [rpQ, "garrison", 0]
gmZoneSet [rpQ, "owner", gmResistanceSide]

// ---- q3: the refill: SPENT -> ... -> STRONG ----------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "SPENT" }
triSimUntil { ((gmOccupierForces) select 2) == "STRONG" }
triAssertGe [((gmOccupierForces) select 0) * 100, 66 * rpStart]

// ---- q4: the push-out on the quiet town --------------------------------------
rpOp = []
triSimUntil { rpOp = []; {if ((_x select 2) == "PUSHOUT") then {rpOp = _x}} forEach gmOccupierOps; (count rpOp) > 0 }
triAssertEq [(rpOp select 5), rpQName]
triAssertEq [(rpOp select 1), "RETAKE"]
triAssertEq [(rpOp select 3), "FOOT"]
triAssertEq [(format ["%1", (rpOp select 4) != ""]), "true"]
gmSimDiagLog

triEndTest
