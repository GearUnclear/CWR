// ============================================================================
//  Guerrilla Mode - Gate-Zero smoke test (plan 13, "Gate-Zero" step 2):
//    "runtime-spawn ~6-8 AI in two groups (createGroup/createUnit), drive a QRF
//     with doMove, watch framerate and pathing on a real map."
//
//  GROUND-TRUTH NOTE on doMove: `doMove` is DECLARED but NOT registered in this
//  engine (confirmed absent - GameStateExt*.cpp has no "doMove" GameOperator).
//  The mode (and this test) drive movement the confirmed way the shipped
//  spawning.sqs does: group `move` (GrpMove, GameStateExt.cpp:1299) plus a MOVE
//  `addWaypoint`/`setWaypointType` chain (GameStateExt.cpp:1422/1424).
//
//  Everything here is a CONFIRMED runtime command (createCenter :1182,
//  createGroup :1184, createUnit :1339, units :979, leader :971/977,
//  getPos :938, move :1299, addWaypoint :1422, setWaypointType :1424,
//  setBehaviour :1301, deleteVehicle :1028). Player class + spawned class are
//  the stock "SoldierWB" (present in the Demo dataset), so this isolates the
//  ENGINE capability from the mode's EAST/GUER classname VERIFYs.
//
//  Assertions are framerate-agnostic: unit COUNTS (not frame timings) prove the
//  spawn; a time-bounded displacement (not a fixed frame count) proves motion.
//  Globals (gz*) are used for everything referenced across statements because
//  the harness evaluates each ';'-separated statement in its own context, so
//  `_local` vars would not survive between lines.
// ============================================================================

triSimUntil { alive player }

// -- make sure the WEST center exists so createGroup west can seat groups ------
createCenter west

// -- GROUP 1 (4 riflemen) near the player -------------------------------------
gzP  = getPos player
gzG1 = createGroup west
"SoldierWB" createUnit [[(gzP select 0) + 6,  (gzP select 1) + 6,  0], gzG1, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) + 10, (gzP select 1) + 6,  0], gzG1, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) + 6,  (gzP select 1) + 10, 0], gzG1, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) + 10, (gzP select 1) + 10, 0], gzG1, "", 0.6, "PRIVATE"]

// -- GROUP 2 (4 riflemen) - the second group proves multi-group spawn ----------
gzG2 = createGroup west
"SoldierWB" createUnit [[(gzP select 0) - 6,  (gzP select 1) + 6,  0], gzG2, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) - 10, (gzP select 1) + 6,  0], gzG2, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) - 6,  (gzP select 1) + 10, 0], gzG2, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) - 10, (gzP select 1) + 10, 0], gzG2, "", 0.6, "PRIVATE"]

// let the two groups instantiate their AI before we assert
triSimFrames 30

// -- ASSERT the spawn: 4 + 4 = 8 live AI across TWO distinct groups -----------
triAssertEq [(count units gzG1), 4]
triAssertEq [(count units gzG2), 4]
triAssertGe [(count units gzG1) + (count units gzG2), 6]

// ============================================================================
//  QRF: a third group, ordered to convoy toward a point ~110 m off. This is the
//  exact "drive a QRF" idiom spawning.sqs uses (move + MOVE waypoint), with
//  doMove routed around per the note above.
// ============================================================================
gzQRF = createGroup west
"SoldierWB" createUnit [[(gzP select 0) + 4, (gzP select 1) - 6,  0], gzQRF, "", 0.7, "SERGEANT"]
"SoldierWB" createUnit [[(gzP select 0) + 8, (gzP select 1) - 6,  0], gzQRF, "", 0.6, "PRIVATE"]
"SoldierWB" createUnit [[(gzP select 0) + 4, (gzP select 1) - 10, 0], gzQRF, "", 0.6, "PRIVATE"]
triSimFrames 20
triAssertGe [(count units gzQRF), 3]

// destination ~110 m north-east; capture the QRF leader's start position first
gzDest  = [(gzP select 0) + 110, (gzP select 1) + 40, 0]
gzLead0 = getPos (leader gzQRF)
gzQRF setBehaviour "AWARE"
gzQRF move gzDest
gzWp = gzQRF addWaypoint [gzDest, 0]
gzWp setWaypointType "MOVE"

// -- ASSERT motion: the QRF leader travels >15 m from where it spawned. This is
//    time-bounded (polled), NOT frame-count-bounded, so it is framerate-agnostic;
//    it also proves pathing did not deadlock/crash. assert_timeout in the .toml
//    gives the foot element ample real time to cover the distance.
triSimUntil { (sqrt ( ((getPos (leader gzQRF)) select 0 - (gzLead0 select 0)) ^ 2 + ((getPos (leader gzQRF)) select 1 - (gzLead0 select 1)) ^ 2 )) > 15 }

// -- everyone still alive after the move (no crash / no despawn) --------------
triAssertGe [(count units gzQRF), 1]
triAssertGe [(count units gzG1), 4]

triEndTest
