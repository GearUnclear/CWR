triSimUntil { alive player }
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

