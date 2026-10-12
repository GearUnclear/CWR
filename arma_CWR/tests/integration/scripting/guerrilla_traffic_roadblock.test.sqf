// ============================================================================
//  Guerrilla Mode ambient road traffic - an UNARMED man in the road (issue
//  #66).
//    An unarmed player standing in a civ car's lane ahead is a roadblock, not
//    a hold-up: the car brakes and waits (laneWait), no surrender/escape roll
//    is drawn, and the wait is deliberate, so the stall ladder is frozen: well
//    past trafficStallTimeout (90 s) and both blocked-recovery thresholds
//    (~30 s retry, ~60 s U-turn) the car is still tracked, still waiting, its
//    driver seated, its destination unchanged and never "abandoned".  When
//    the man steps aside the car resumes (after a 1 s continuous-clear hold)
//    and drives on.
//  Staging: strip the player's weapons (the armed probe must read 0), force
//  a car from the Village and put the player 20 m ahead of it in its lane,
//  facing it.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
removeAllWeapons player
triSimFrames 5
gtR0 = gmTrafficDiag select 16

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- the unarmed player steps into the lane 20 m ahead, facing the car ---------
gtD = getDir gtCar
gtCp = getPos gtCar
player setPos [(gtCp select 0) + (20 * (sin gtD)), (gtCp select 1) + (20 * (cos gtD)), 0]
player setDir (gtD + 180)

// -- the car brakes and waits; nothing to roll ----------------------------------
triSimUntil { ((gmTrafficEncounter gtCar) select 2) == 1 }
triAssertEq [((gmTrafficEncounter gtCar) select 5), 0]
triAssertEq [((gmTrafficEncounter gtCar) select 0), "none"]
gtDest0 = gmTrafficDest gtCar
gtDestIdx0 = (gmTrafficInfo gtCar) select 2

// -- hold the road past the stall timeout and both recovery thresholds ----------
gtT0 = time
triSimUntil { time > gtT0 + 100 }
// one snapshot, so a failure prints the whole row:
//   roadblock man alive | tracked | driver at the wheel | laneWait | choice |
//   same destination zone and road point (no U-turn) | "abandoned"
//   despawns | commandeered events | rolls drawn since staging
gtSnap = format ["%1|%2|%3|%4|%5|%6|%7|%8|%9", alive player, gtCar in gmTrafficVehicles, (driver gtCar) == gtDrv, (gmTrafficEncounter gtCar) select 2, (gmTrafficEncounter gtCar) select 0, (((gmTrafficInfo gtCar) select 2) == gtDestIdx0) and (((gmTrafficDest gtCar) distance gtDest0) < 1), {(_x select 1) == "abandoned"} count gmEvtTrDespawned, count gmEvtTrCommandeered, (gmTrafficDiag select 16) - gtR0]
triAssertEq [gtSnap, "true|true|true|1|none|true|0|0|0"]

// -- step 15 m aside: the lane clears, the car resumes and drives on ------------
gtPp = getPos player
player setPos [(gtPp select 0) + (15 * (sin (gtD + 90))), (gtPp select 1) + (15 * (cos (gtD + 90))), 0]
gtPs = getPos gtCar
triSimUntil { ((gmTrafficEncounter gtCar) select 2) == 0 }
triSimUntil { ((getPos gtCar) distance gtPs) >= 15 }
triAssertEq [(format ["%1", (driver gtCar) == gtDrv]), "true"]
triAssertEq [(gmTrafficDiag select 16), gtR0]

triEndTest
