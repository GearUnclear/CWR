// ============================================================================
//  Phase 2 of the hold-up SURRENDER save/reload (issue #66).
//    Fresh launch, load phase 1's save.  The row comes back in TSStopping
//    with its "surrender" choice and the elapsed delay (surrenderTime is
//    serialized; it never runs backwards), with no commandeered event yet.
//    The surrender chance (reset to the config value by the load) is set to
//    0, so a reroll would now escape; instead the restored stop completes:
//    the driver bails exactly once and the hull leaves the live table, with
//    no roll drawn since the load.
// ============================================================================

triSimUntil { GM_LIB_READY }

triAssertEq [(triLoadGame "trsurr"), "OK"]
triSimFrames 1

triAssertEq [(format ["%1", isNull gsCar]), "false"]
triAssertEq [((gmTrafficInfo gsCar) select 3), 1]
triAssertEq [((gmTrafficEncounter gsCar) select 0), "surrender"]
triAssertGe [((gmTrafficEncounter gsCar) select 3), gsSurrT]
triAssertEq [count gmEvtTrCommandeered, 0]
gsR0 = gmTrafficDiag select 16

// -- a reroll would now escape ---------------------------------------------------
triAssertEq [(gmTrafficSurrenderChance 0), 0]

// -- the restored stop completes: one bail, the hull released -------------------
triSimUntil { isNull (driver gsCar) }
triAssertEq [count gmEvtTrCommandeered, 1]
triAssertEq [(format ["%1", ((gmEvtTrCommandeered select 0) select 0) == gsCar]), "true"]
triAssertEq [(format ["%1", gsCar in gmTrafficVehicles]), "false"]
triAssertEq [count (gmTrafficInfo gsCar), 0]
triAssertEq [(format ["%1", alive gsDrv]), "true"]
triAssertEq [(format ["%1", (vehicle gsDrv) == gsDrv]), "true"]
triAssertEq [(gmTrafficDiag select 16), gsR0]

triEndTest
