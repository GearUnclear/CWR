// ============================================================================
//  Occupier forces, the playable cadence (OCCUPIER-FORCES.md 6.3 tokens, 16
//  forcesTokenInterval[]; owner, 2026-10-15).  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    c1  test levers: director tick 1 s, one distinct town wakes the
//        director, the first-town roll 0, the refill slowed to 48 h so the
//        band holds still; the pool is forced to 0 so the wake launches
//        nothing, Houdan rises remotely and wakes the director (two
//        tokens), then rests on a long cooldown (no heat candidate, no
//        quiet town for a push-out) and the pool is restored: STRONG;
//    c2  STRONG at the default intervals: with the purse raised to three the
//        third token comes after about 600 s (it was 1800 s);
//    c3  STRETCHED (pool at half): the next token comes after about 1050 s
//        (it was 2700 s);
//    c4  SPENT: no token comes and the token clock stands still.
//  Game time runs at 4x (setAccTime) to keep the wait short.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
cdT0 = time
triSimUntil { time > cdT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- c1: the levers; the director wakes with two tokens and nothing to do --
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
cdStart = (gmOccupierForces) select 1
triAssertGe [cdStart, 24]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
cdRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]
player setPos [7922, 8500, 0]
cdV = gmZoneIndex "Village"
triAssertGe [cdV, 0]
cdVPos = (gmZone cdV) select 8
triSimUntil { not (gmGarrisonSpawned cdV) }
gmZoneSet [cdV, "garrison", 0]
gmZoneSet [cdV, "support", 55]
cdSq = [cdRes, gmFactionSquad [gmResistanceSide, 1, 1], cdVPos] call GM_fnSpawnSquad
cdSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units cdSq)
doStop (units cdSq)
triSimUntil { ((gmZone cdV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units cdSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", cdV, 99999]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", cdStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 3), 2]
cdW = time
triSimUntil { time > cdW + 5 }
triAssertEq [(count gmOccupierOps), 0]
triAssertEq [((gmOccupierForces) select 3), 2]

// ---- c2: STRONG, one token per 600 s -----------------------------------------
setAccTime 4
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 3]]), "true"]
cdA = time
triSimUntil { ((gmOccupierForces) select 3) == 3 }
cdStrong = time - cdA
triAssertGe [cdStrong, 590]
triAssertLe [cdStrong, 640]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]

// ---- c3: STRETCHED, one token per 1050 s -------------------------------------
cdHalf = cdStart / 2
triAssertEq [(format ["%1", gmForcesForce ["pool", cdHalf]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRETCHED"]
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 4]]), "true"]
cdB = time
triSimUntil { ((gmOccupierForces) select 3) == 4 }
cdStretched = time - cdB
triAssertGe [cdStretched, 1040]
triAssertLe [cdStretched, 1090]
triAssertEq [((gmOccupierForces) select 2), "STRETCHED"]

// ---- c4: SPENT, no tokens ----------------------------------------------------
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 5]]), "true"]
cdC = time
triSimUntil { time > cdC + 240 }
triAssertEq [((gmOccupierForces) select 3), 4]
triAssertEq [((gmOccupierForces) select 14), 0]
triAssertEq [(count gmOccupierOps), 0]
setAccTime 1
gmSimDiagLog

triEndTest
