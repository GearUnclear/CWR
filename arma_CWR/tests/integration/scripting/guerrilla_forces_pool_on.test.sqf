// ============================================================================
//  Occupier forces pool, key ON (OCCUPIER-FORCES.md phase 2; native
//  OccupierForces service).  Guerrilla.Abel opts in (occupierForces = 1,
//  phase 3), so the service sizes itself on the first active frame:
//    a1  the roster is Abel's four occupier bases (R = 6 + 6 + 8 + 12 = 32),
//        so poolStart >= 32 + 2K; the pool starts full, STRONG, DORMANT,
//        no tokens, control 1, no wave fired;
//    a2  gmForcesForce ["pool", 0] drops it to SPENT, and with the refill
//        forced to 0.01 h (36 s) and the director tick at 1 s it climbs out
//        of SPENT and back to STRONG.  (D2 is not wired to the pool in
//        phase 2; once it is, Abel's garrisons are at strength at boot, so
//        the refill still reaches the pool.)
// ============================================================================

triSimUntil { GM_LIB_READY }
ofT0 = time
triSimUntil { time > ofT0 + 3 }

// ---- a1: activation --------------------------------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
ofRow = gmOccupierForces
ofStart = ofRow select 1
ofK = ofRow select 7
triAssertGe [ofK, 1]
// generated bases (811c5c3, d5fa57b): their garrisons come out of poolStart
ofRgen = 0
{ofGI = gmZoneIndex (_x select 0); if (ofGI >= 0) then {ofRgen = ofRgen + ((gmZoneSim ofGI) select 7)}} forEach gmGeneratedBases
triAssertGe [ofStart, 32 + 2 * ofK - ofRgen]
triAssertEq [(ofRow select 0), ofStart]
triAssertEq [(ofRow select 2), "STRONG"]
triAssertEq [(ofRow select 3), 0]
triAssertEq [(ofRow select 4), "DORMANT"]
triAssertEq [(ofRow select 5), 1]
triAssertEq [(ofRow select 6), 0]
triAssertEq [(ofRow select 9), 0]

// ---- a2: forced exhaustion and the refill ----------------------------------
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]
triSimUntil { ((gmOccupierForces) select 2) == "THIN" }
triSimUntil { ((gmOccupierForces) select 2) == "STRONG" }
triSimUntil { ((gmOccupierForces) select 0) == ofStart }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]

triEndTest
