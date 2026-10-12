// ============================================================================
//  Phase 1 of the hold-up SURRENDER save/reload (issue #66).
//    Same staging as guerrilla_traffic_commandeer: surrender chance 1, the
//    armed player 20 m ahead of a freshly forced Village car, in its lane and
//    facing it.  The save is written the moment the row reads TSStopping (1),
//    inside the 2.5 s stop delay, so the bail has not happened yet.
// ============================================================================

triSimUntil { GM_LIB_READY }

gsVil = gmZoneIndex "Village"
triAssertGe [gsVil, 0]
gsPos = (gmZone gsVil) select 8
player setPos [gsPos select 0, (gsPos select 1) - 400, 0]
triSimFrames 5

triAssertEq [(gmTrafficSurrenderChance 1), 1]

gsCar = gmTrafficForceSpawn ["civ", gsVil]
triAssertEq [(format ["%1", isNull gsCar]), "false"]
gsDrv = driver gsCar
triAssertEq [(format ["%1", isNull gsDrv]), "false"]

gsD = getDir gsCar
gsCp = getPos gsCar
player setPos [(gsCp select 0) + (20 * (sin gsD)), (gsCp select 1) + (20 * (cos gsD)), 0]
player setDir (gsD + 180)
triSimUntil { ((gmTrafficEncounter gsCar) select 5) == 1 }

// -- the surrender is pending: save at once -------------------------------------
triSimUntil { ((gmTrafficInfo gsCar) select 3) == 1 }
gsSurrT = (gmTrafficEncounter gsCar) select 3
triAssertEq [(triSaveGame "trsurr"), "OK"]

triEndTest
