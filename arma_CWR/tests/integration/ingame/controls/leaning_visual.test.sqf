triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
// First person and sights, before any camera override touches the aim.
player switchCamera "INTERNAL"
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triScreenshot "lean_neutral_first_person"
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triScreenshot "lean_right_first_person"
triKeyUp 8
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triScreenshot "lean_left_first_person"
triKeyDown 25
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triScreenshot "lean_left_sights"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triScreenshot "lean_right_sights"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triScreenshot "lean_neutral_sights"
triKeyDown 25
triKeyUp 25
triSimUntil { triCamView == "INTERNAL" }
// The camera a player actually uses: the default third-person view behind
// the soldier, looking along his heading, with no override.
player switchCamera "EXTERNAL"
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triScreenshot "lean_behind_neutral"
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triScreenshot "lean_behind_left"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triScreenshot "lean_behind_right"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// Crouched, same camera (Classic's stance-up key, Page Up, crouches from the
// combat stand and stands back up from the crouch).
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "crouch" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triScreenshot "lean_crouch_behind_neutral"
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triScreenshot "lean_crouch_behind_left"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triScreenshot "lean_crouch_behind_right"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// Fixed views next. Freelook stops the render-camera override from steering
// the player toward it, so hold it through every override shot.
triKeyDown 226
triSimFrames 3
player setDir 0
leanHeading = getDir player
leanP = getPosASL player
// Facing north (setDir 0), model x/z map straight onto world east/north.
// Camera 1.8 m in front of the muzzle, looking back down the bore, fixed at
// the neutral pose so the lean shows as motion.
leanMuzzleView = { private ["_pose", "_bore", "_m"]; _pose = triLeanPose player; _bore = (triGetLeanRig [player, []]) select 4; _m = [(leanP select 0) + (_pose select 5), (leanP select 2) + (_pose select 6), (leanP select 1) + (_pose select 7)]; [(_m select 0) + 1.8 * (_bore select 0), (_m select 1) + 1.8 * (_bore select 1), (_m select 2) + 1.8 * (_bore select 2), -(_bore select 0), -(_bore select 1), -(_bore select 2)] }
leanMuzzle = [] call leanMuzzleView
leanFront = [leanP select 0, (leanP select 2) + 0.7, (leanP select 1) + 3.2, 0, -0.08, -1]
leanRear = [leanP select 0, (leanP select 2) + 0.8, (leanP select 1) - 3.2, 0, -0.08, 1]
triSetView leanMuzzle
triScreenshot "lean_crouch_neutral_muzzle"
triSetView leanRear
triScreenshot "lean_crouch_neutral_rear"
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triSetView leanMuzzle
triScreenshot "lean_crouch_left_muzzle"
triSetView leanFront
triScreenshot "lean_crouch_left_front"
triSetView leanRear
triScreenshot "lean_crouch_left_rear"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triSetView leanMuzzle
triScreenshot "lean_crouch_right_muzzle"
triSetView leanFront
triScreenshot "lean_crouch_right_front"
triSetView leanRear
triScreenshot "lean_crouch_right_rear"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// Back to the combat stand, still holding freelook.
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "combat" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
player setDir 0
leanP = getPosASL player
leanMuzzle = [] call leanMuzzleView
// Front views look back along the bore, rear views look down it. Both see the
// roll about the soldier's forward axis edge-on.
leanFront = [leanP select 0, (leanP select 2) + 1.0, (leanP select 1) + 3.2, 0, -0.08, -1]
leanRear = [leanP select 0, (leanP select 2) + 1.1, (leanP select 1) - 3.2, 0, -0.08, 1]
triSetView leanMuzzle
triScreenshot "lean_neutral_muzzle"
triSetView leanFront
triScreenshot "lean_neutral_front"
triSetView leanRear
triScreenshot "lean_neutral_rear"
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triAssertNear [getDir player, leanHeading, 1]
triSetView leanMuzzle
triScreenshot "lean_left_muzzle"
triSetView leanFront
triScreenshot "lean_left_front"
triSetView leanRear
triScreenshot "lean_left_rear"
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triAssertNear [getDir player, leanHeading, 1]
triSetView leanMuzzle
triScreenshot "lean_right_muzzle"
triSetView leanFront
triScreenshot "lean_right_front"
triSetView leanRear
triScreenshot "lean_right_rear"
// Three-quarter view from the front right, the original single shot.
triSetPlayerFaceView [3.2,0.9]
triScreenshot "lean_right_face"
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triClearView
triKeyUp 226
triEndTest
