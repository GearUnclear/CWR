// ============================================================================
//  Guerrilla Mode ambient road traffic (native Traffic service) - civilian
//  cars driving town-to-town.  Proven here on full CWA data (Abel):
//    1. gmTrafficForceSpawn ["civ", Village] places a car ON the road net
//       near the town (gmRoadNearest within 10 m), driven by a CIV-side
//       driver, registered in the live table (count / vehicles / info) and
//       announced through the init.sqs "spawned" handler;
//    2. the car actually drives (>= 15 m displacement) - the ACMOVE leg to
//       another CITY under CARELESS behaviour (road pathing);
//    3. the player far offshore (> trafficRadius + hysteresis from the car)
//       drains the live table to 0 and fires "despawned".
//  Determinism: the force-spawn bypasses the chance roll and the caps; the
//  road placement is the real one (farthest band point from the player).
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8

// -- stand 400 m south of the Village centre: the town's road net then lies
//    inside the spawn band [trafficMinSpawnDist, trafficRadius] -------------
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

// -- force-spawn a civilian car from the Village -------------------------------
gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]

// -- on the road: gmRoadNearest finds a road point within 10 m (2D) ------------
gtRoad = gmRoadNearest (getPos gtCar)
triAssertGe [count gtRoad, 3]
gtDx = ((getPos gtCar) select 0) - (gtRoad select 0)
gtDy = ((getPos gtCar) select 1) - (gtRoad select 1)
triAssertGe [10, sqrt ((gtDx * gtDx) + (gtDy * gtDy))]

// -- nearestRoads / gmRoadsNear see segments around the car ---------------------
triAssertGe [count ((getPos gtCar) nearestRoads 50), 1]
triAssertGe [count (gmRoadsNear [getPos gtCar, 50]), 1]

// -- CIV driver, live table, info tuple, spawned event -------------------------
triAssertEq [(format ["%1", isNull (driver gtCar)]), "false"]
triAssertEq [(format ["%1", side (driver gtCar)]), "CIV"]
triAssertGe [gmTrafficCount "civ", 1]
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]
gtInfo = gmTrafficInfo gtCar
triAssertEq [count gtInfo, 4]
triAssertEq [gtInfo select 0, "civ"]
triAssertEq [gtInfo select 1, gtVil]
triAssertGe [count gmEvtTrSpawned, 1]

// -- it drives ------------------------------------------------------------------
gtP0 = getPos gtCar
triSimUntil { ((getPos gtCar) distance gtP0) >= 15 }

// -- despawn: the SW offshore corner is far beyond the despawn edge from any
//    Abel road ------------------------------------------------------------------
player setPos [500, 500, 0]
triSimUntil { (gmTrafficCount "all") == 0 }
triAssertGe [count gmEvtTrDespawned, 1]
triAssertEq [(format ["%1", isNull gtCar]), "true"]

triEndTest
