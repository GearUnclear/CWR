// Each passable 1cm fire barrier sits between the leaned eye and muzzle.
// This isolates oblique safe-origin protection without body-contact impulses;
// leaning_cover retains the solid-wall collision cases.
triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 45
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 60
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 80
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 100
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 120
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanMidX = ((getPos player) select 0) + ((leanPose select 2) + (leanPose select 5)) * 0.5
leanMidZ = ((getPos player) select 1) + ((leanPose select 4) + (leanPose select 7)) * 0.5
leanWall = "UDLeanThinWall" createVehicle [7510,5710,0]
leanWallAngle = 135
leanWall setDir leanWallAngle
leanWall setPos [leanMidX,leanMidZ,0]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertLt [player distance [7500,5700,0], 0.02]
triAssertNear [getDir player, 0, 0.01]
triAssertGt [(triLeanPose player) select 0, 0.95]
leanPose = triLeanPose player
leanPos = getPos player
leanEyeSide = ((leanPos select 0) + (leanPose select 2) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 4) - leanMidZ) * (sin leanWallAngle)
leanMuzzleSide = ((leanPos select 0) + (leanPose select 5) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 7) - leanMidZ) * (sin leanWallAngle)
leanOriginSide = ((leanPos select 0) + (leanPose select 18) - leanMidX) * (cos leanWallAngle) - ((leanPos select 1) + (leanPose select 20) - leanMidZ) * (sin leanWallAngle)
triAssertGt [leanEyeSide, 0.005]
triAssertLt [leanMuzzleSide, -0.005]
triAssertGt [leanOriginSide, 0.005]
leanDamageBefore = getDammage leanWall
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triSimUntil { (getDammage leanWall) > leanDamageBefore }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
deleteVehicle leanWall
triEndTest
