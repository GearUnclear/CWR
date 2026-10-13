triSimUntil { alive player }
triAssertConfigClass "CfgWeapons/Beretta"
triAssertConfigClass "CfgWeapons/BerettaMag"
player setPos [7500,5700,0]
player setDir 0
player addMagazine "BerettaMag"
player addWeapon "Beretta"
player selectWeapon "Beretta"
leanSettle = time
triSimUntil { time > leanSettle + 4 }
weapons player
triAssertEq [getMove player, "handgunstand"]
triLeanPose player
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanAmmo = triPlayerCurrentMagazineAmmo
triFirePlayerWeapon
triAssertLt [triPlayerCurrentMagazineAmmo, leanAmmo]
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyDown 75
triSimFrames 3
triKeyUp 75
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertEq [getMove player, "handguncrouch"]
triLeanPose player
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.95 }
triKeyUp 20
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triEndTest
