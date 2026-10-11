triSetLanguage "English"
triSimUntil { alive player }
player setPos [7500, 5700, 0]
player setDir 0
leanStart = time
triSimUntil { time > leanStart + 2 }
triLeanPose player
leanNeutral = triLeanPose player
leanRoot = getPos player
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
triAssertGt [leanRight select 5, (leanNeutral select 5) + 0.1]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triAssertLt [player distance leanRoot, 0.1]
// Both first-person and sights use the same upper-body displacement.
triAssertNear [(leanRight select 12) - (leanNeutral select 12), 0.28, 0.01]
triAssertNear [(leanRight select 15) - (leanNeutral select 15), 0.28, 0.01]
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
