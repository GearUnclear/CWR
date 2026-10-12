// ============================================================================
//  Guerrilla Mode ambient road traffic - an armed hold-up that ends in a
//  SURRENDER (issue #66, forced outcome).
//    An armed player inside trafficCommandeerRadius aiming at a civ car makes
//    its driver choose, once per encounter, surrender or escape.  This test
//    pins the chance to 1 (gmTrafficSurrenderChance), so the one roll must be
//    a surrender: the crew gets a Stop, and after trafficCommandeerStopDelay
//    the driver bails (unassigned, AllowGetIn and OrderGetIn off, DoGetOut)
//    and flees on foot; the hull is released from the live table (still
//    alive, boardable) and "commandeered" fires exactly once.
//  Staging: force-spawn from the Village, then teleport the player 20 m ahead
//  of the car along its heading, facing it: in the lane, inside the radius
//  and aiming on the first 0.5 s sub-tick, whether or not the car has moved.
//  "Armed" is pose based (ClassifyWeaponShow == UCWInHands); the fixture
//  player stands in the "combat" move with his rifle, which counts, and the
//  precondition is asserted rather than assumed.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

// -- deterministic outcome: the one roll can only be a surrender ---------------
triAssertEq [(gmTrafficSurrenderChance 1), 1]
gtR0 = gmTrafficDiag select 16

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- the player steps into the lane 20 m ahead, facing the car -----------------
gtD = getDir gtCar
gtCp = getPos gtCar
player setPos [(gtCp select 0) + (20 * (sin gtD)), (gtCp select 1) + (20 * (cos gtD)), 0]
player setDir (gtD + 180)
// armed precondition (weapon in hands), read through the live probe
triSimUntil { ((gmTrafficEncounter gtCar) select 5) == 1 }

// -- the driver chose to surrender; the car stops, the driver bails ------------
triSimUntil { ((gmTrafficEncounter gtCar) select 0) == "surrender" }
triSimUntil { isNull (driver gtCar) }
triSimUntil { (speed gtCar) < 2 }
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "false"]
triAssertEq [count (gmTrafficInfo gtCar), 0]
triAssertEq [count gmEvtTrCommandeered, 1]
triAssertEq [(format ["%1", ((gmEvtTrCommandeered select 0) select 0) == gtCar]), "true"]
// the roll was drawn (once: the chance-1 surrender leaves nothing to reroll)
triAssertEq [(gmTrafficDiag select 16), gtR0 + 1]

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
