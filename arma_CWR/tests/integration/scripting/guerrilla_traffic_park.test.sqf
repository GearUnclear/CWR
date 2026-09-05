// ============================================================================
//  Guerrilla Mode ambient road traffic - civ PARK / DWELL / DEPART cycle.
//    A civilian car that ARRIVES at its destination rolls trafficParkChance
//    (pinned to 1 in this fixture): the crew gets a Stop, the driver
//    dismounts after the brake delay ("parked" fires, the hull STAYS a live
//    entry), dwells 10-20 s near the car (fixture-pinned; stock 60-180 s),
//    then re-boards via the stock GetIn and departs to a fresh destination
//    ("departed" fires).
//  Determinism: force-spawn from the Village, then teleport the car onto its
//  own destination road point (gmTrafficDest) so the arrival tick is
//  immediate; the player watches from 50 m (inside trafficMinSpawnDist, so
//  the arrival never despawns; outside the 25 m commandeer radius).
// ============================================================================

triSimUntil { GM_LIB_READY }

gtParked = []
gtDeparted = []
gmTrafficOnEvent ["parked", {gtParked = gtParked + [_this]}]
gmTrafficOnEvent ["departed", {gtDeparted = gtDeparted + [_this]}]

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- deterministic arrival: teleport the car onto its own destination ----------
gtDest = gmTrafficDest gtCar
triAssertGe [count gtDest, 3]
gtCar setPos gtDest
player setPos [(gtDest select 0) + 50, gtDest select 1, 0]

// -- park: TEArrived, Stop, then the driver dismounts (arrival tick + one
//    5 s tick past the 2.5 s brake delay) -------------------------------------
triSimUntil { isNull (driver gtCar) }
triAssertGe [count gtParked, 1]
triAssertEq [(format ["%1", ((gtParked select 0) select 0) == gtCar]), "true"]
// STILL a live entry, now TSDwelling (state 6)
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]
triAssertEq [(gmTrafficInfo gtCar) select 3, 6]
// driver alive, on foot, dwelling near the car
triAssertEq [(format ["%1", alive gtDrv]), "true"]
triAssertEq [(format ["%1", (vehicle gtDrv) == gtDrv]), "true"]
triAssertGe [40, gtDrv distance gtCar]

// -- depart: dwell 10-20 s, the stock GetIn re-seats the driver ----------------
triSimUntil { (driver gtCar) == gtDrv }
gtP0 = getPos gtCar
triSimUntil { ((getPos gtCar) distance gtP0) >= 15 }
triAssertGe [count gtDeparted, 1]
triAssertEq [(format ["%1", ((gtDeparted select 0) select 0) == gtCar]), "true"]

triEndTest
