triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// The swaying hips and rolled head box reach this wall partway through a
// right lean.
leanWall = "UDLeanWall" createVehicle [7500.73,5700,0]
leanWall setPos [7500.73,5700,0]
leanWall setDir 0
triAssertEq [typeOf leanWall, "UDLeanWall"]
leanRoot = getPos player
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triLeanPose player
triAssertGt [((triLeanPose player) select 0), 0.05]
triAssertLt [((triLeanPose player) select 0), 0.4]
triAssertLt [player distance leanRoot, 0.1]
leanBlocked = (triLeanPose player) select 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertNear [(triLeanPose player) select 0, leanBlocked, 0.03]
// A neutral shoulder clearance sphere can already overlap nearby cover.
// Leaning farther inward stays blocked, while leaning away must still work.
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanWall setPos [7500.65,5700,0]
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triAssertLt [player distance leanRoot, 0.1]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// Escaping the first wall must still respect a second obstacle on the left.
// The rolled head and shoulders reach little past the body's own width on
// this side, and a full-height wall that close would sit inside the crude
// torso collision box and shove the soldier. So the obstacle hangs at head
// height, its bottom 1.4 m up, where only the leaning upper body meets it.
leanOtherWall = "UDLeanWall" createVehicle [7499.9,5700,0]
leanOtherWall setPos [7499.9,5700,1.4]
leanOtherWall setDir 0
triKeyDown 20
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [(triLeanPose player) select 0, -0.3]
triAssertGt [(triLeanPose player) select 0, -0.95]
triAssertLt [player distance leanRoot, 0.1]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanOtherWall
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
// Removing cover while holding the key safely allows the remaining travel.
deleteVehicle leanWall
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// The neutral muzzle intersects this front wall's clearance volume. A lean
// parallel to the wall may slide along it, while shots still hit the wall.
leanWall = "UDLeanWall" createVehicle [7510,5710,0]
leanWall setDir 90
leanWall setPos [7500,5700.9,0]
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanDamageBefore = getDammage leanWall
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
// A passable, bullet-blocking barrier isolates a surface fully between the
// eye and muzzle without the body contact solver moving the standing player.
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWall setDir 90
leanWall setPos [7500,5700.88,0]
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanPose = triLeanPose player
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertLt [((getPos player) select 1) + (leanPose select 4), 5700.875]
triAssertGt [((getPos player) select 1) + (leanPose select 7), 5700.885]
triAssertLt [((getPos player) select 1) + (leanPose select 20), 5700.875]
leanDamageBefore = getDammage leanWall
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
deleteVehicle leanWall
triEndTest
