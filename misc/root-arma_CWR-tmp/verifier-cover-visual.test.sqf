triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanWall = "UDLeanWall" createVehicle [7500.75,5700,0]
leanWall setPos [7500.75,5700,0]
leanWall setDir 0
triKeyDown 8
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertGt [(triLeanPose player) select 0, 0.05]
triAssertLt [(triLeanPose player) select 0, 0.4]
triScreenshot "partial_lean_first_person_cover"
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triScreenshot "partial_lean_sights_cover"
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "INTERNAL" }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanWall setPos [7500.65,5700,0]
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triScreenshot "away_from_close_cover_first_person"
triKeyDown 226
triSimFrames 3
player switchCamera "EXTERNAL"
triSetPlayerFaceView [3.2,0.9]
triScreenshot "away_from_close_cover_body"
triClearView
triKeyUp 226
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "crouch" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertLt [(triLeanPose player) select 0, -0.95]
triKeyDown 226
triSimFrames 3
triSetPlayerFaceView [3.2,0.7]
triScreenshot "crouched_away_from_cover_body"
triClearView
triKeyUp 226
player switchCamera "INTERNAL"
triScreenshot "crouched_away_from_cover_first_person"
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
triEndTest


