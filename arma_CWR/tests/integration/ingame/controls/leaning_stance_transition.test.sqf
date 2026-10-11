triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "combat"]
// Rifle: held Q survives standing to crouching, including the transition.
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
triAssertLt [(triLeanPose player) select 0, -0.95]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "crouch"]
triAssertLt [(triLeanPose player) select 0, -0.95]
// Rifle: held E survives the return to standing.
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
triAssertGt [(triLeanPose player) select 0, 0.95]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "combat"]
triAssertGt [(triLeanPose player) select 0, 0.95]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
// The same player stance transition is supported for a handgun.
player addMagazine "BerettaMag"
player addWeapon "Beretta"
player selectWeapon "Beretta"
leanSettle = time
triSimUntil { time > leanSettle + 4 }
triAssertEq [getMove player, "handgunstand"]
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
triAssertLt [(triLeanPose player) select 0, -0.95]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "handguncrouch"]
triAssertLt [(triLeanPose player) select 0, -0.95]
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
triAssertGt [(triLeanPose player) select 0, 0.95]
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "handgunstand"]
triAssertGt [(triLeanPose player) select 0, 0.95]
// A script interrupt must not inherit the ordinary stance-change exemption.
triKeyDown 75
triSimFrames 3
triKeyUp 75
player switchMove "handgunstand"
triSimFrames 5
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
player switchMove ""
leanSettle = time
triSimUntil { time > leanSettle + 4 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyUp 8
triEndTest
