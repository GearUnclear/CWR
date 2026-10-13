// ============================================================================
//  Occupier forces pool, key OFF (OCCUPIER-FORCES.md phase 2; native
//  OccupierForces service).  The guerrilla_offscreen.abel fixture pins
//  occupierForces = 0 (the shipped Guerrilla.Abel opts in from phase 3), so
//  the service must stay inert: gmOccupierForces reports band and posture
//  "OFF" with an unsized pool and an empty director (the ten documented
//  slots plus the five phase-3 director slots), and the test levers answer
//  without moving anything.
// ============================================================================

triSimUntil { GM_LIB_READY }
ofT0 = time
triSimUntil { time > ofT0 + 3 }

ofRow = gmOccupierForces
triAssertEq [(count ofRow), 15]
triAssertEq [(ofRow select 2), "OFF"]
triAssertEq [(ofRow select 4), "OFF"]
triAssertEq [(ofRow select 0), 0]
triAssertEq [(ofRow select 1), 0]
triAssertEq [(ofRow select 3), 0]
triAssertEq [(ofRow select 6), 0]
triAssertEq [(ofRow select 7), 0]
triAssertEq [(count (ofRow select 8)), 0]
triAssertEq [(ofRow select 9), 0]
triAssertEq [(count (ofRow select 10)), 0]
triAssertEq [(count (ofRow select 12)), 0]
triAssertEq [(ofRow select 13), 0]
triAssertEq [(count gmOccupierOps), 0]
triAssertEq [(count gmOccupierIntel), 0]

// the levers are known but inert while the key is off
triAssertEq [(format ["%1", gmForcesForce ["pool", 10]]), "true"]
triAssertEq [((gmOccupierForces) select 0), 0]
triAssertEq [(format ["%1", gmForcesForce ["wave", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["bogus", 1]]), "false"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 0.01]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["bogus", 1]]), "false"]
ofT1 = time
triSimUntil { time > ofT1 + 3 }
triAssertEq [((gmOccupierForces) select 2), "OFF"]

triEndTest
