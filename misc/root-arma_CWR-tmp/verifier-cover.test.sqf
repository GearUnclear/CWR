triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanWall = "UDLeanWall" createVehicle [7500.75,5700,0]
leanWall setPos [7500.75,5700,0]
leanWall setDir 0
leanRoot = getPos player
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triLeanPose player
triAssertGt [((triLeanPose player) select 0), 0.05]
triAssertLt [((triLeanPose player) select 0), 0.4]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triLeanPose player
triKeyUp 20
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertGt [((triLeanPose player) select 0), 0.05]
triAssertLt [((triLeanPose player) select 0), 0.4]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanWall setPos [7500.65,5700,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triKeyDown 20
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triLeanPose player
triAssertLt [((triLeanPose player) select 0), -0.95]
triKeyUp 20
deleteVehicle leanWall
triEndTest
