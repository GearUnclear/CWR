triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
triSimUntil { gmZoneCount >= 10 }
triSimUntil { gmGarrisonSpawned (gmZoneIndex "Wadi Checkpoint") }
triAssertEq [typeOf player, "LoBo_Egypt_FrtCrp"]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanNeutral = triLeanPose player
leanRoot = getPos player
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triAssertLt [(triLeanPose player) select 2, (leanNeutral select 2) - 0.1]
triAssertLt [player distance leanRoot, 0.1]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triAssertGt [(triLeanPose player) select 5, (leanNeutral select 5) + 0.1]
triAssertLt [player distance leanRoot, 0.1]
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triAssertGt [(triLeanPose player) select 0, 0.95]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triEndTest
