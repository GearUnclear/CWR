triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "combat"]
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "crouch"]
triAssertLt [(triLeanPose player) select 0, -0.95]
triKeyDown 75
triSimFrames 3
triKeyUp 75
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "combat"]
triAssertLt [(triLeanPose player) select 0, -0.95]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triEndTest
