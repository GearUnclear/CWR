triSetLanguage "English"
triSimUntil { alive player }
player setPos [7500, 5700, 0]
player setDir 0
leanStart = time
triSimUntil { time > leanStart + 2 }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triLeanPose player
leanNeutral = triLeanPose player
leanRoot = getPos player
// [pose, index, pose, index] -> distance between two triLeanPose points
leanGap = { private ["_a", "_b", "_i", "_j", "_dx", "_dy", "_dz"]; _a = _this select 0; _i = _this select 1; _b = _this select 2; _j = _this select 3; _dx = (_a select _i) - (_b select _j); _dy = (_a select (_i + 1)) - (_b select (_j + 1)); _dz = (_a select (_i + 2)) - (_b select (_j + 2)); sqrt (_dx * _dx + _dy * _dy + _dz * _dz) }
// SDL scancodes Q=20, E=8. Exercise production events and default bindings.
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
leanLeft = triLeanPose player
triLeanPose player
triAssertLt [leanLeft select 2, (leanNeutral select 2) - 0.1]
triAssertLt [leanLeft select 5, (leanNeutral select 5) - 0.1]
triAssertLt [player distance leanRoot, 0.1]
triKeyDown 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyUp 20
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanRight = triLeanPose player
triAssertGt [leanRight select 2, (leanNeutral select 2) + 0.1]
// The right lean rolls the weapon down and right; it moves with the eye
// and keeps its distance to it (one rigid upper body).
triAssertGt [[leanRight, 5, leanNeutral, 5] call leanGap, 0.1]
triAssertNear [[leanRight, 5, leanRight, 2] call leanGap, [leanNeutral, 5, leanNeutral, 2] call leanGap, 0.01]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triAssertLt [player distance leanRoot, 0.1]
// First person and sights share that pose: the internal camera is the eye,
// and the sight camera moves with it, keeping its distance to the eye.
triAssertLt [[leanRight, 12, leanRight, 2] call leanGap, 0.01]
triAssertGt [[leanRight, 15, leanNeutral, 15] call leanGap, 0.1]
triAssertNear [[leanRight, 15, leanRight, 12] call leanGap, [leanNeutral, 15, leanNeutral, 12] call leanGap, 0.01]
triKeyDown 25
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 25
triKeyUp 25
triSimUntil { triCamView == "INTERNAL" }
// Classic's stance-up key (Page Up) crouches from the combat stand.
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "crouch" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyDown 78
triSimFrames 3
triKeyUp 78
triSimUntil { ((triLeanPose player) select 1) == 4 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "crouch" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// Held lean cannot reappear after an unsupported stance until released.
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// Fire Geometry must follow the leaned head, not leave an invisible neutral body.
triKeyDown 75
triSimFrames 3
triKeyUp 75
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanPos = getPosASL player
leanDir = getDir player
leanHitX = (leanPos select 0) + (leanPose select 2) * cos leanDir + ((leanPose select 4) + 3) * sin leanDir
leanHitZ = (leanPos select 1) - (leanPose select 2) * sin leanDir + ((leanPose select 4) + 3) * cos leanDir
leanDamage = getDammage player
// ASL keeps the shot at eye height across sloping ground.
leanBullet = "Bullet7_6E" camCreate [leanHitX, leanHitZ, 10]; leanBullet setPosASL [leanHitX, leanHitZ, (leanPos select 2) + (leanPose select 3)]; leanBullet setVelocity [-300 * sin leanDir, -300 * cos leanDir, 0]
triSimUntil { (getDammage player) > leanDamage }
triKeyUp 8
triEndTest
