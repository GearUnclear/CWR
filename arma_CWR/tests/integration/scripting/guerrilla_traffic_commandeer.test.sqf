// ============================================================================
//  Guerrilla Mode ambient road traffic - commandeering a civilian car.
//    The real player standing in a civ car's lane ahead (inside
//    trafficCommandeerRadius) trips the native predicate: the crew gets a
//    Stop, and after trafficCommandeerStopDelay the driver bails (unassigned,
//    AllowGetIn off, DoGetOut) and flees on foot; the hull is released from
//    the live table (still alive, boardable) and "commandeered" fires.
//  Staging: force-spawn from the Village, then teleport the ARMED player 20 m
//  ahead of the car along its heading, facing it - in the lane and inside the
//  radius on the first 0.5 s sub-tick, whether or not the car has moved yet.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- the player steps into the lane 20 m ahead, facing the car -----------------
gtD = getDir gtCar
gtCp = getPos gtCar
player setPos [(gtCp select 0) + (20 * (sin gtD)), (gtCp select 1) + (20 * (cos gtD)), 0]
player setDir (gtD + 180)

// -- the car stops, the driver bails, the hull leaves the live table -----------
triSimUntil { isNull (driver gtCar) }
triSimUntil { (speed gtCar) < 2 }
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "false"]
triAssertEq [count (gmTrafficInfo gtCar), 0]
triAssertGe [count gmEvtTrCommandeered, 1]
triAssertEq [(format ["%1", ((gmEvtTrCommandeered select 0) select 0) == gtCar]), "true"]

// -- hull alive and free; the driver alive, on foot ----------------------------
triAssertEq [(format ["%1", alive gtCar]), "true"]
triAssertEq [(format ["%1", alive gtDrv]), "true"]
triAssertEq [(format ["%1", (vehicle gtDrv) == gtDrv]), "true"]

// -- the player takes it --------------------------------------------------------
player moveInDriver gtCar
triSimFrames 3
triAssertEq [(format ["%1", (driver gtCar) == player]), "true"]

// -- gmTrafficRelease on an untracked hull is a no-op (false) -------------------
triAssertEq [(format ["%1", gmTrafficRelease gtCar]), "false"]

triEndTest
