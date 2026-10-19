// ============================================================================
//  Occupier forces across a save (OCCUPIER-FORCES.md 11, 12.2; phase 6A),
//  phase 2 of 2: a fresh process loads the mid-flight save.  The operation's
//  record comes back LIVE with its airframe id, LZ and air origin; the
//  OccupierAir record comes back with its hull, crew and cargo refs resolved
//  (the airframe rode the world's vehicle serializer) and every cargo body
//  still seated; the insertion resumes: it lands, the infantry becomes the
//  live squad, and the airframe is removed unseen, the crew first.
// ============================================================================

triSimUntil { GM_LIB_READY }
triAssertEq [(triLoadGame "airflight"), "OK"]
triSimFrames 5
player allowDammage false
player setCaptive true

// ---- the record and the airframe came back ---------------------------------
triAssertEq [(count gmCounterAttacks), 1]
rvCa = gmCounterAttacks select 0
triAssertEq [(rvCa select 10), svOp]
triAssertEq [(rvCa select 2), "LIVE"]
triAssertEq [(rvCa select 16), "Mi17"]
triAssertEq [(rvCa select 24), svId]
triAssertEq [(rvCa select 26), svA]
triAssertEq [((rvCa select 25) select 0), (svLz select 0)]
triAssertEq [((rvCa select 25) select 1), (svLz select 1)]
triAssertEq [(format ["%1", isNull (rvCa select 18)]), "false"]
triAssertGe [(count gmOccupierAir), 1]
rvR = gmOccupierAir select 0
triAssertEq [(rvR select 8), svId]
triAssertEq [(format ["%1", isNull (rvR select 18)]), "false"]
triAssertEq [(rvR select 20), svBorn]
triAssertEq [(rvR select 5), svBorn]
triAssertGe [(rvR select 3), 1]
rvCargo = rvR select 19
rvCrew = rvR select 23
triAssertEq [(count units rvCargo), svBorn]

// ---- the insertion resumes: lands, unloads, departs, removed crew first -----
triSimUntil { ((gmOccupierAir select 0) select 0) >= 3 }
rvR = gmOccupierAir select 0
triAssertEq [(rvR select 16), ""]
triAssertLe [(rvR select 6), 100]
rvNone = [[], [], "NONE", 0, 0, 0, 0, objNull, [], "", 0, 0, 0, 0, 0, 0, "", 0, objNull, 0, objNull, 0, "", objNull, 0, [], 0, false, false]
triSimUntil { ((gmCounterAttacks + [rvNone]) select 0) select 27 }
triAssertEq [(((gmCounterAttacks select 0) select 21)), svBorn]
triSimUntil { ((gmOccupierAir select 0) select 0) == 4 }
rvR = gmOccupierAir select 0
triAssertEq [(rvR select 22), 1]
rvT = time
triSimUntil { time > rvT + 2 }
rvCrewLeft = 0
"rvCrewLeft = rvCrewLeft + 1" forEach (units rvCrew)
triAssertEq [rvCrewLeft, 0]

triEndTest
