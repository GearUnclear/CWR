// ============================================================================
//  Projectile damage end-to-end: a ShotBullet that intersects a Man must
//  apply damage through the Landscape::ObjectCollision ->
//  Landscape::ExplosionDammage -> DirectDammage/HitBy funnel — for enemy
//  AND friendly targets alike (HitBy has no side gate).
//
//  Deterministic by construction: no AI aiming. Each victim gets a rifle
//  bullet camCreated directly above its head with a straight-down velocity,
//  so the per-frame collision segment sweeps the whole body regardless of
//  stance or facing. A camCreated bullet has no parent/owner, which also
//  covers the owner==nullptr branches of ExplosionDammage.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// enemy victim, 15 m east of the player
udGrpE = createGroup east
"SoldierEB" createUnit [[(getPos player select 0) + 15, getPos player select 1, 0], udGrpE, "udEnemy = this", 0.2, "PRIVATE"]
triSimUntil { !(isNil "udEnemy") }
triSimUntil { alive udEnemy }
udEnemy stop true

// friendly victim (player's side), 15 m west
udGrpG = createGroup resistance
"SoldierGB" createUnit [[(getPos player select 0) - 15, getPos player select 1, 0], udGrpG, "udBuddy = this", 0.2, "PRIVATE"]
triSimUntil { !(isNil "udBuddy") }
triSimUntil { alive udBuddy }
udBuddy stop true

// let both settle into their idle pose before reading positions
triSimFrames 20

// --- bullet vs enemy ---
// threshold 0.35: a real body hit (ExplosionDammage -> HitBy) is a large
// damage step; the broken "everything is glass" path only chip-damaged
// components (getDammage crept up by hundredths) and must NOT pass
triAssertEq [(getDammage udEnemy), 0]
// several drops, re-reading the position each time - a freshly created unit
// can still be settling into formation when the first bullet arrives
udPos = getPos udEnemy
udB1 = "Bullet7_6E" camCreate [udPos select 0, udPos select 1, 10]
udB1 setVelocity [0, 0, -300]
triSimFrames 60
udPos = getPos udEnemy
udB1 = "Bullet7_6E" camCreate [udPos select 0, udPos select 1, 10]
udB1 setVelocity [0, 0, -300]
triSimFrames 60
udPos = getPos udEnemy
udB1 = "Bullet7_6E" camCreate [udPos select 0, udPos select 1, 10]
udB1 setVelocity [0, 0, -300]
triSimUntil { (getDammage udEnemy) > 0.35 }

// --- bullet vs teammate ---
triAssertEq [(getDammage udBuddy), 0]
udPos2 = getPos udBuddy
udB2 = "Bullet7_6E" camCreate [udPos2 select 0, udPos2 select 1, 10]
udB2 setVelocity [0, 0, -300]
triSimFrames 60
udPos2 = getPos udBuddy
udB2 = "Bullet7_6E" camCreate [udPos2 select 0, udPos2 select 1, 10]
udB2 setVelocity [0, 0, -300]
triSimFrames 60
udPos2 = getPos udBuddy
udB2 = "Bullet7_6E" camCreate [udPos2 select 0, udPos2 select 1, 10]
udB2 setVelocity [0, 0, -300]
triSimUntil { (getDammage udBuddy) > 0.35 }

// (no player-`fire` phase: the fire command's no-target force-fire
// deliberately discharges skyward — dir[1]=10 in Man::AimWeaponForceFire —
// so it cannot assert aimed damage; muzzle-fired-bullet damage is covered
// by point_blank_damage's AI duel instead)

// --- explosive projectile (indirect damage loop) vs a fresh victim ---
// direct hits go through DirectDammage; blast damage to bystanders goes
// through ExplosionDammage's indirect-radius pass — a separate loop that
// must be proven separately (a break here = "grenades/shells do nothing"
// while rifles still work)
udGrpE3 = createGroup east
"SoldierEB" createUnit [[(getPos player select 0) + 30, getPos player select 1, 0], udGrpE3, "udBlast = this", 0.2, "PRIVATE"]
triSimUntil { !(isNil "udBlast") }
triSimUntil { alive udBlast }
udBlast stop true
triSimFrames 20

triAssertEq [(getDammage udBlast), 0]
udPos3 = getPos udBlast
// grenade dropped 2 m up, ~2.5 m to the side: detonates on ground impact,
// victim damaged by blast radius only (no direct hit)
udG = "GrenadeHand" camCreate [(udPos3 select 0) + 2.5, udPos3 select 1, 2]
triSimUntil { (getDammage udBlast) > 0.01 }

triEndTest
