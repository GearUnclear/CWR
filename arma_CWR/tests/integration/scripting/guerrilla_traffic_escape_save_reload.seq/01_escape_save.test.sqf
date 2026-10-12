// ============================================================================
//  Phase 1 of the hold-up ESCAPE save/reload (issue #66).
//    Same staging as guerrilla_traffic_escape: surrender chance 0, a forced
//    Village car 15 m into its first leg, the armed player 15 m ahead and 6 m
//    off its lane, turned to face it until the one roll lands.  The save is
//    written the moment the choice reads "escape", well before the 5 s
//    encounter clear hold could start.
// ============================================================================

triSimUntil { GM_LIB_READY }

gsVil = gmZoneIndex "Village"
triAssertGe [gsVil, 0]
gsPos = (gmZone gsVil) select 8
player setPos [gsPos select 0, (gsPos select 1) - 400, 0]
triSimFrames 5

triAssertEq [(gmTrafficSurrenderChance 0), 0]

gsCar = gmTrafficForceSpawn ["civ", gsVil]
triAssertEq [(format ["%1", isNull gsCar]), "false"]
gsDrv = driver gsCar
triAssertEq [(format ["%1", isNull gsDrv]), "false"]

gsP0 = getPos gsCar
triSimUntil { ((getPos gsCar) distance gsP0) >= 15 }
gsDestIdx = (gmTrafficInfo gsCar) select 2

gsD = getDir gsCar
gsCp = getPos gsCar
player setPos [(gsCp select 0) + (15 * (sin gsD)) + (6 * (sin (gsD + 90))), (gsCp select 1) + (15 * (cos gsD)) + (6 * (cos (gsD + 90))), 0]
player setDir ((((getPos gsCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gsCar) select 1) - ((getPos player) select 1)))
triSimUntil { ((gmTrafficEncounter gsCar) select 5) == 1 }
triSimUntil { player setDir ((((getPos gsCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gsCar) select 1) - ((getPos player) select 1))); ((gmTrafficEncounter gsCar) select 0) != "none" }
triAssertEq [((gmTrafficEncounter gsCar) select 0), "escape"]
gsRolls = gmTrafficDiag select 16
triAssertEq [gsRolls, 1]

// -- write UserDir/Saved/Tmp/trescape.fps while the escape owns the car --------
triAssertEq [(triSaveGame "trescape"), "OK"]

triEndTest
