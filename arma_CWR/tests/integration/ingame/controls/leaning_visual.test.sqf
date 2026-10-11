triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// Freelook stops the render-camera override from steering the player toward it.
triKeyDown 226
triSimFrames 3
player setDir 0
leanHeading = getDir player
player switchCamera "EXTERNAL"
triSetPlayerFaceView [3.2,0.9]
triScreenshot "lean_neutral_front"
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triSetPlayerFaceView [3.2,0.9]
triAssertNear [getDir player, leanHeading, 1]
triScreenshot "lean_left_front"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triSetPlayerFaceView [3.2,0.9]
triAssertNear [getDir player, leanHeading, 1]
triScreenshot "lean_right_front"
triClearView
triKeyUp 226
player switchCamera "INTERNAL"
triScreenshot "lean_right_first_person"
triKeyDown 25
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triScreenshot "lean_right_sights"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triScreenshot "lean_neutral_sights"
triEndTest
