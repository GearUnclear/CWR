triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
triSimUntil { gmZoneCount >= 10 }
triSimUntil { gmGarrisonSpawned (gmZoneIndex "Wadi Checkpoint") }
triAssertEq [typeOf player, "LoBo_Egypt_FrtCrp"]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanNeutral = triLeanPose player
leanRoot = getPos player
// [pose, index, pose, index] -> distance between two triLeanPose points
leanGap = { private ["_a", "_b", "_i", "_j", "_dx", "_dy", "_dz"]; _a = _this select 0; _i = _this select 1; _b = _this select 2; _j = _this select 3; _dx = (_a select _i) - (_b select _j); _dy = (_a select (_i + 1)) - (_b select (_j + 1)); _dz = (_a select (_i + 2)) - (_b select (_j + 2)); sqrt (_dx * _dx + _dy * _dy + _dz * _dz) }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triAssertLt [(triLeanPose player) select 2, (leanNeutral select 2) - 0.1]
triAssertLt [player distance leanRoot, 0.1]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// The right lean rolls the weapon down and right; it moves with the eye
// and keeps its distance to it (one rigid upper body).
leanRight = triLeanPose player
triAssertGt [[leanRight, 5, leanNeutral, 5] call leanGap, 0.1]
triAssertNear [[leanRight, 5, leanRight, 2] call leanGap, [leanNeutral, 5, leanNeutral, 2] call leanGap, 0.01]
triAssertLt [player distance leanRoot, 0.1]
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triAssertGt [(triLeanPose player) select 0, 0.95]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triEndTest
