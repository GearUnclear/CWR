// ============================================================================
//  Guerrilla Mode ambient road traffic - an armed hold-up that ends in an
//  ESCAPE (issue #66, forced outcome).
//    With the surrender chance pinned to 0 the one roll must be an escape:
//    the driver stays seated and floors it along his CURRENT route (EscCurrent:
//    mid-trip the destination does not change; only the ordinary blocked
//    recovery may turn him round).  No "commandeered" event, the car stays a
//    live civ entry, exactly one roll is drawn.
//  Staging: let the forced car drive 15 m, then put the armed player 15 m
//  ahead and 6 m to the side (outside the 4 m lane half-width, inside the
//  25 m radius) and keep him turned to face the car until the roll lands, so
//  the aim cone (15 deg) holds on the sub-tick whatever the car does.
//
//  ESCAPE_KMH calibration (2026-10-11, Abel, Classic 1.99 data, build
//  ud66-rwdi): a forced civ car from the Village at its ordinary pace
//  (SpeedLimited) peaked at 27.26 km/h over 30 s of driving in two separate
//  probe runs (27.27 and 27.26); after an escape roll the same staging
//  peaked at 93.8 and 123.7 km/h within 20 s.  ESCAPE_KMH = 1.5 x 27.3,
//  rounded: 41 km/h.  The car must beat it WHILE the escape still owns
//  movement (the encounter clears 5 s after the player is 60 m behind it,
//  and the pace then drops back to SpeedLimited).
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

// -- deterministic outcome: the one roll can only be an escape -----------------
triAssertEq [(gmTrafficSurrenderChance 0), 0]
gtR0 = gmTrafficDiag select 16

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- it drives off on its first leg ---------------------------------------------
gtP0 = getPos gtCar
triSimUntil { ((getPos gtCar) distance gtP0) >= 15 }
gtDest0 = gmTrafficDest gtCar
gtDestIdx0 = (gmTrafficInfo gtCar) select 2

// -- the gunman: 15 m ahead, 6 m off the lane, facing the car -------------------
gtD = getDir gtCar
gtCp = getPos gtCar
player setPos [(gtCp select 0) + (15 * (sin gtD)) + (6 * (sin (gtD + 90))), (gtCp select 1) + (15 * (cos gtD)) + (6 * (cos (gtD + 90))), 0]
player setDir ((((getPos gtCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gtCar) select 1) - ((getPos player) select 1)))
// armed precondition (weapon in hands), read through the live probe
triSimUntil { ((gmTrafficEncounter gtCar) select 5) == 1 }

// -- keep aiming until the one roll lands; it must be an escape -----------------
triSimUntil { player setDir ((((getPos gtCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gtCar) select 1) - ((getPos player) select 1))); ((gmTrafficEncounter gtCar) select 0) != "none" }
triAssertEq [((gmTrafficEncounter gtCar) select 0), "escape"]
gtP1 = getPos gtCar

// -- it floors it: past ESCAPE_KMH (41) while the escape owns movement ----------
triSimUntil { ((speed gtCar) > 41) and (((gmTrafficEncounter gtCar) select 0) == "escape") }

// -- and gets away along its current route --------------------------------------
triSimUntil { ((getPos gtCar) distance gtP1) >= 60 }
triAssertEq [(format ["%1", (driver gtCar) == gtDrv]), "true"]
triAssertEq [(format ["%1", alive gtDrv]), "true"]
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]
triAssertEq [count gmEvtTrCommandeered, 0]
// EscCurrent: same destination zone and road point as before the hold-up
triAssertEq [((gmTrafficInfo gtCar) select 2), gtDestIdx0]
triAssertLt [((gmTrafficDest gtCar) distance gtDest0), 1]
// exactly one roll for the whole encounter
triAssertEq [(gmTrafficDiag select 16), gtR0 + 1]

triEndTest
