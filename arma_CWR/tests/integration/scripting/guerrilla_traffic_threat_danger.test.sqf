// ============================================================================
//  Guerrilla Mode ambient road traffic - gunfire during a hold-up (issue
//  #66).
//    A driver who chose to surrender (chance pinned to 1) is braking for the
//    gunman when the gunman fires a round: fresh relevant danger takes the
//    encounter over in the next 0.5 s sub-tick (before the pending bail), the
//    shared danger response runs (cower / U-turn / rush / bail, the roll is
//    the danger tier's and stays random) and the surrender never completes:
//    no "commandeered" event.  The encounter keeps its history (choice
//    "surrender", dangerOwned 1) but has left TSStopping, and aiming at the
//    car again starts no new roll and no second reaction.
//  Staging: the player stands 20 m ahead of the freshly forced car and 6 m to
//  the side of its lane (a "rush" reaction drives straight on, so a gunman
//  in the lane would be run down), turned to face it until it stops; then he
//  turns to the car's own heading (the round cannot hit the driver) and
//  force-fires (the guerrilla_traffic_danger idiom).
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

triAssertEq [(gmTrafficSurrenderChance 1), 1]
guWeap = primaryWeapon player
triAssertNe [guWeap, ""]
guAmmo0 = player ammo guWeap
triAssertGt [guAmmo0, 0]

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- the gunman: 20 m ahead, 6 m off the lane, facing the car -------------------
gtD = getDir gtCar
gtCp = getPos gtCar
player setPos [(gtCp select 0) + (20 * (sin gtD)) + (6 * (sin (gtD + 90))), (gtCp select 1) + (20 * (cos gtD)) + (6 * (cos (gtD + 90))), 0]
player setDir ((((getPos gtCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gtCar) select 1) - ((getPos player) select 1)))
// armed precondition (weapon in hands), read through the live probe
triSimUntil { ((gmTrafficEncounter gtCar) select 5) == 1 }

// -- the surrender is pending (TSStopping = 1) ----------------------------------
triSimUntil { player setDir ((((getPos gtCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gtCar) select 1) - ((getPos player) select 1))); ((gmTrafficInfo gtCar) select 3) == 1 }
// -- ...and the gunman fires a round along the road, away from the car ---------
player setDir (getDir gtCar)
player fire guWeap
triSimUntil { (player ammo guWeap) < guAmmo0 }

// -- danger takes over: one reaction for THIS car, no bail at gunpoint ----------
triSimUntil { ({(_x select 0) == gtCar} count gmEvtTrPanicked) >= 1 }
gtReact = ""; gtI = 0; while {gtI < count gmEvtTrPanicked} do {if (gtReact == "") then {if (((gmEvtTrPanicked select gtI) select 0) == gtCar) then {gtReact = (gmEvtTrPanicked select gtI) select 2}}; gtI = gtI + 1}
triAssertEq [(format ["%1", (gtReact == "cower") or (gtReact == "uturn") or (gtReact == "rush") or (gtReact == "bail")]), "true"]
triAssertEq [count gmEvtTrCommandeered, 0]
// one snapshot of the row, compared against the outcome the reaction implies
// (the SQS "? cond : stmt" form is not valid in a harness eval):
//   non-bail: tracked | dangerOwned 1 | choice "surrender" (history) |
//             left TSStopping | driver at the wheel
//   bail:     released (encounter []) | driver on foot
triSimUntil { (gtReact != "bail") or ((vehicle gtDrv) == gtDrv) }
gtEnc = gmTrafficEncounter gtCar
gtEncDanger = -1
gtEncChoice = "-"
gtStopping = -1
if ((count gtEnc) > 0) then {gtEncDanger = gtEnc select 1; gtEncChoice = gtEnc select 0; gtStopping = ((gmTrafficInfo gtCar) select 3) == 1}
gtSnap = format ["%1|%2|%3|%4|%5|%6", gtCar in gmTrafficVehicles, gtEncDanger, gtEncChoice, gtStopping, (driver gtCar) == gtDrv, (vehicle gtDrv) == gtDrv]
gtExpect = "true|1|surrender|false|true|false"
if (gtReact == "bail") then {gtExpect = "false|-1|-|-1|false|true"}
triAssertEq [gtSnap, gtExpect]

// -- past the 2.5 s stop delay: the surrender never completed -------------------
gtT1 = time
triSimUntil { time > gtT1 + 5 }
triAssertEq [count gmEvtTrCommandeered, 0]

// -- aim at the car again for 3 s: no new roll, no second reaction --------------
gtR1 = gmTrafficDiag select 16
gtT2 = time
triSimUntil { player setDir ((((getPos gtCar) select 0) - ((getPos player) select 0)) atan2 (((getPos gtCar) select 1) - ((getPos player) select 1))); time > gtT2 + 3 }
triAssertEq [count gmEvtTrCommandeered, 0]
triAssertEq [({(_x select 0) == gtCar} count gmEvtTrPanicked), 1]
triAssertEq [(gmTrafficDiag select 16), gtR1]

triEndTest
