// ============================================================================
//  Occupier forces, the spare cap on a base retake (OCCUPIER-FORCES.md 7.2,
//  6.5 the minimum, 16 forcesSpareCap / forcesSpareCapFloor; owner,
//  2026-10-15).  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    s1  a real camera at the default view distance in daylight; the pool
//        full (STRONG); test levers: director tick 1 s, one distinct town
//        wakes the director, the first-town roll 0, no wave strikes; the
//        pool is forced to 0
//        while Houdan rises remotely (the wake launches nothing), then
//        restored: STRONG with two tokens;
//    s2  the Airfield (garrison 12) is emptied and handed to the
//        resistance, hot (heat 100).  Its retake asks min(max(4, 12), 12) =
//        12 bodies; the Harbour spares 6, the Outpost 5, so no single base
//        can source it;
//    s3  the old one-source rule (gmForcesTune "spareCap" 0): the director
//        sends nothing to the Airfield for 20 s of director ticks;
//    s4  the spare cap (the default): a BASE operation on the Airfield
//        leaves with the largest single-base spare, at least 6 and fewer
//        than 12 bodies.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
scT0 = time
triSimUntil { time > scT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- s1: a real camera; the levers; the director wakes ----------------------
scP = gmTrafficPercept (getPos player)
triAssertEq [(scP select 0), 1]
triAssertGe [(scP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
scStart = (gmOccupierForces) select 1
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
// the Airfield's loss drops control to about 0.75: a wave may fire, but it
// sends no strike here (only the director's choice is under test)
triAssertEq [(format ["%1", gmForcesTune ["waveOps", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
scRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
// the player stands north, away from every base (no base materialized)
player setPos [7922, 8500, 0]
scV = gmZoneIndex "Village"
triAssertGe [scV, 0]
scVPos = (gmZone scV) select 8
triSimUntil { not (gmGarrisonSpawned scV) }
gmZoneSet [scV, "garrison", 0]
gmZoneSet [scV, "support", 55]
scSq = [scRes, gmFactionSquad [gmResistanceSide, 1, 1], scVPos] call GM_fnSpawnSquad
scSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units scSq)
doStop (units scSq)
triSimUntil { ((gmZone scV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units scSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", scV, 99999]]), "true"]

// ---- s2: the Airfield falls, hot and empty ----------------------------------
scA = gmZoneIndex "Airfield"
triAssertGe [scA, 0]
triAssertEq [((gmZoneSim scA) select 7), 12]
triSimUntil { not (gmGarrisonSpawned scA) }
gmZoneSet [scA, "garrison", 0]
gmZoneSet [scA, "owner", gmResistanceSide]
gmZoneSet [scA, "heatRaise", 100]
triAssertEq [((gmZone scA) select 2), gmResistanceSide]

// ---- s3: the one-source rule refuses the Airfield ---------------------------
triAssertEq [(format ["%1", gmForcesTune ["spareCap", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", scStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
scW = time
triSimUntil { time > scW + 20 }
scOp = []
{if (((_x select 1) == "BASE") and ((_x select 5) == "Airfield") and ((_x select 2) == "DIRECTOR")) then {scOp = _x}} forEach gmOccupierOps
triAssertEq [(count scOp), 0]

// ---- s4: the spare cap sends the largest single-base spare ------------------
triAssertEq [(format ["%1", gmForcesTune ["spareCap", 1]]), "true"]
triSimUntil { scOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Airfield") and ((_x select 2) == "DIRECTOR")) then {scOp = _x}} forEach gmOccupierOps; (count scOp) > 0 }
triAssertEq [(scOp select 2), "DIRECTOR"]
triAssertEq [(format ["%1", (scOp select 4) != ""]), "true"]
triAssertGe [(scOp select 7), 6]
triAssertLt [(scOp select 7), 12]
gmSimDiagLog

triEndTest
