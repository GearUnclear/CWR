// ============================================================================
//  Phase 2 of the hold-up ESCAPE save/reload (issue #66).
//    Fresh launch, load phase 1's save.  The serialized choice comes back as
//    "escape" (read on the first frame, before any sub-tick could clear it),
//    the deferred load restore puts the car back on its escape leg at full
//    speed, and NO reroll happens: the surrender chance (reset to the config
//    value by the load) is set to 1, so a reroll would now surrender, and the
//    armed player aims at the car from inside the radius for 2 s.  The live
//    probe must see the threat over a span longer than one 0.5 s sub-tick,
//    so at least one encounter check really ran with the gun on the car.
// ============================================================================

triSimUntil { GM_LIB_READY }

triAssertEq [(triLoadGame "trescape"), "OK"]
triSimFrames 1

// -- the row and its choice came back (handle via GGameState) -------------------
triAssertEq [(format ["%1", isNull gsCar]), "false"]
triAssertEq [(format ["%1", gsCar in gmTrafficVehicles]), "true"]
triAssertEq [((gmTrafficEncounter gsCar) select 0), "escape"]
triAssertEq [((gmTrafficInfo gsCar) select 2), gsDestIdx]
gsR0 = gmTrafficDiag select 16

// -- a reroll would now surrender ------------------------------------------------
triAssertEq [(gmTrafficSurrenderChance 1), 1]

// -- the gunman again: 12 m ahead, 5 m off the lane, tracking the car for 2 s ---
gsD = getDir gsCar
gsCp = getPos gsCar
player setPos [(gsCp select 0) + (12 * (sin gsD)) + (5 * (sin (gsD + 90))), (gsCp select 1) + (12 * (cos gsD)) + (5 * (cos (gsD + 90))), 0]
player setDir ((((getPos gsCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gsCar) select 1) - ((getPos player) select 1)))
triSimUntil { ((gmTrafficEncounter gsCar) select 5) == 1 }
gsThrA = -1
gsThrB = -1
gsT0 = time
triSimUntil { player setDir ((((getPos gsCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gsCar) select 1) - ((getPos player) select 1))); if (((gmTrafficEncounter gsCar) select 6) == 1) then {if (gsThrA < 0) then {gsThrA = time}; gsThrB = time}; time > gsT0 + 2 }
// the threat was live for longer than one sub-tick period
triAssertGe [gsThrA, 0]
triAssertGe [gsThrB - gsThrA, 0.6]

// -- the choice stands: no roll, no surrender, the driver still at the wheel ----
triAssertEq [((gmTrafficEncounter gsCar) select 0), "escape"]
triAssertEq [(gmTrafficDiag select 16), gsR0]
triAssertEq [count gmEvtTrCommandeered, 0]
triAssertEq [(format ["%1", (driver gsCar) == gsDrv]), "true"]
triAssertEq [(format ["%1", gsCar in gmTrafficVehicles]), "true"]

triEndTest
