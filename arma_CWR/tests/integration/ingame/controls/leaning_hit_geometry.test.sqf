triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanPose = triLeanPose player
leanPos = getPosASL player
leanDir = getDir player
leanHitX = (leanPos select 0) + (leanPose select 2) * cos leanDir + ((leanPose select 4) + 3) * sin leanDir
leanHitZ = (leanPos select 1) - (leanPose select 2) * sin leanDir + ((leanPose select 4) + 3) * cos leanDir
leanHitY = (leanPos select 2) + (leanPose select 3)
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanDamage = getDammage player
leanBullet = "Bullet7_6E" camCreate [leanHitX, leanHitZ, 10]; leanBullet setPosASL [leanHitX, leanHitZ, leanHitY]; leanBullet setVelocity [-300 * sin leanDir, -300 * cos leanDir, 0]
leanSettle = time
triSimUntil { time > leanSettle + 0.5 }
triAssertNear [getDammage player, leanDamage, 0.001]
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
leanBullet = "Bullet7_6E" camCreate [leanHitX, leanHitZ, 10]; leanBullet setPosASL [leanHitX, leanHitZ, leanHitY]; leanBullet setVelocity [-300 * sin leanDir, -300 * cos leanDir, 0]
triSimUntil { (getDammage player) > leanDamage }
triKeyUp 8
triEndTest
