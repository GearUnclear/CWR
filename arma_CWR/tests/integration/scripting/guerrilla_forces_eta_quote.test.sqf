// ============================================================================
//  Occupier forces, the road quote of the arrival rule (OCCUPIER-FORCES.md
//  6.5, 16 forcesRoadFactor; round 3 D, 2026-10-16).  The director quotes a
//  ground package's arrival on the road route the counter-attack service
//  marches (RoadQuoteCache), not the straight line.  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1, occupier EAST: Ural truck,
//  Mi17 transport helicopter) on the full Remaster:
//    a1  a real camera at the default view distance in daylight; the pool
//        full (STRONG); the levers of guerrilla_forces_arrival_rule: director
//        tick 1 s, one distinct town wakes the director, the first-town roll
//        0; the pool is forced to 0 while the Village rises remotely, the
//        Village rests on a long cooldown and the pool is restored;
//    a2  Arudy: 1350 to 2250 m straight from the Outpost, its nearest ground
//        base (on foot 900 to 1500 s by the straight line: the old quote
//        sent a TRUCK), more than 850 m from the player;
//    a3  Arudy is handed to the resistance, hot: the director quotes the
//        Outpost's road (2691 m, on foot about 1794 s, past forcesAirEta
//        1500) and the operation is a TRANSPORT_HELI insertion;
//    a4  the INFO line named the road quote: gmForcesEtaNotes holds the
//        cheapest package's words, FOOT from the operation's source, kind
//        ROAD ("road N s"), N past 1500 s and above the straight-line
//        seconds; the insertion itself flies the straight line ("line N s");
//    a5  the director's plan is cached: the pair answers from the cache
//        (gmRoadQuote counts no new plan), the same length the note quoted.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
eqT0 = time
triSimUntil { time > eqT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: a real camera; the levers; the director wakes ----------------------
eqP = gmTrafficPercept (getPos player)
triAssertEq [(eqP select 0), 1]
triAssertGe [(eqP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
eqStart = (gmOccupierForces) select 1
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
eqRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
// the player stands north, away from every base (no base materialized)
player setPos [7922, 8500, 0]
eqV = gmZoneIndex "Village"
triAssertGe [eqV, 0]
eqVPos = (gmZone eqV) select 8
triSimUntil { not (gmGarrisonSpawned eqV) }
gmZoneSet [eqV, "garrison", 0]
gmZoneSet [eqV, "support", 55]
eqSq = [eqRes, gmFactionSquad [gmResistanceSide, 1, 1], eqVPos] call GM_fnSpawnSquad
eqSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units eqSq)
doStop (units eqSq)
triSimUntil { ((gmZone eqV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units eqSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", eqV, 99999]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", eqStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]

// ---- a2: Arudy, a truck by the straight line --------------------------------
eqA = gmZoneIndex "Arudy"
triAssertGe [eqA, 0]
eqAPos = (gmZone eqA) select 8
eqOut = gmZoneIndex "Outpost"
triAssertGe [eqOut, 0]
eqOutPos = (gmZone eqOut) select 8
eqLine = eqOutPos distance eqAPos
triAssertGt [eqLine, 1350]
triAssertLt [eqLine, 2250]
// the Outpost is Arudy's nearest ground base in a straight line
triAssertLt [eqLine, (((gmZone (gmZoneIndex "Le Port Harbour")) select 8) distance eqAPos)]
triAssertLt [eqLine, (((gmZone (gmZoneIndex "Airfield")) select 8) distance eqAPos)]
triAssertGt [(eqAPos distance (getPos player)), 850]
triAssertEq [((gmZone eqA) select 1), "CITY"]
triSimUntil { not (gmGarrisonSpawned eqA) }

// ---- a3: Arudy gets a helicopter insertion -----------------------------------
gmZoneSet [eqA, "garrison", 0]
gmZoneSet [eqA, "owner", gmResistanceSide]
gmZoneSet [eqA, "heatRaise", 100]
eqOp = []
triSimUntil { eqOp = []; {if ((_x select 5) == "Arudy") then {eqOp = _x}} forEach gmOccupierOps; (count eqOp) > 0 }
triAssertEq [(eqOp select 1), "RETAKE"]
triAssertEq [(eqOp select 2), "DIRECTOR"]
triAssertEq [(eqOp select 3), "TRANSPORT_HELI"]
eqSrc = gmZoneIndex (eqOp select 4)
triAssertGe [eqSrc, 0]
eqSrcPos = (gmZone eqSrc) select 8

// ---- a4: the INFO line named the road quote ----------------------------------
eqFoot = []
eqHeli = []
{if (((_x select 1) == "FOOT") and ((_x select 2) == eqSrc)) then {eqFoot = _x}; if (((_x select 1) == "TRANSPORT_HELI") and ((_x select 2) == eqSrc)) then {eqHeli = _x}} forEach gmForcesEtaNotes
triAssertEq [(count eqFoot), 5]
triAssertEq [(eqFoot select 4), "ROAD"]
triAssertGt [(eqFoot select 3), 1500]
triAssertGt [(eqFoot select 3), ((eqSrcPos distance eqAPos) / 1.5)]
triAssertIncludes [(eqFoot select 0), "road "]
triAssertEq [(count eqHeli), 5]
triAssertEq [(eqHeli select 4), "LINE"]
triAssertIncludes [(eqHeli select 0), "line "]

// ---- a5: the director's plan answers from the cache ---------------------------
// a pair far from any road: its lookup reads the counters (no route)
eqQ0 = gmRoadQuote [[10, 0, 10], [20, 0, 20]]
eqQ1 = gmRoadQuote [eqSrcPos, eqAPos]
triAssertEq [(eqQ1 select 3), (eqQ0 select 3)]
triAssertGt [(eqQ1 select 2), (eqQ0 select 2)]
triAssertGt [(eqQ1 select 0), (eqSrcPos distance eqAPos)]
triAssertLt [(abs ((eqQ1 select 0) - ((eqFoot select 3) * 1.5))), 2]
gmSimDiagLog

triEndTest
