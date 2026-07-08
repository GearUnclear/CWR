// ============================================================================
//  Projectile damage vs THIRD-PARTY (@LoBo) unit models on Sinai.
//
//  bullet_damage_headless proves the collision->damage funnel against stock
//  CWA soldier models; this repeats it against LoBo P3Ds, where a geometry/
//  hit-LOD that failed to load would make bullets pass straight through -
//  "projectiles do not damage enemies or teammates" - while stock units on
//  Abel still take hits. Victims: a live IDF garrison soldier spawned by the
//  native GarrisonCache (occupier) and a created Egyptian Frontier Corps
//  soldier (resistance = the player's own side).
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// native cache spawns the Wadi Checkpoint garrison (IDF, WEST) near the Camp
gcIdx = gmZoneIndex "Wadi Checkpoint"
triAssertGe [gcIdx, 0]
triSimUntil { gmGarrisonSpawned gcIdx }
triSimUntil { (gmGarrisonLive gcIdx) > 0 }

udUnits = units ((gmGarrisonGroups gcIdx) select 0)
triAssertGe [(count udUnits), 1]
udEnemy = udUnits select 0
triSimUntil { alive udEnemy }

// resistance-side (player-side) LoBo soldier next to the player
udGrpR = createGroup east
"LoBo_Egypt_FrtCrp" createUnit [[(getPos player select 0) + 15, getPos player select 1, 0], udGrpR, "udBuddy = this", 0.2, "PRIVATE"]
triSimUntil { !(isNil "udBuddy") }
triSimUntil { alive udBuddy }
udBuddy stop true

triSimFrames 20

// --- bullet vs LoBo enemy (IDF garrison unit) ---
// threshold 0.35: real body damage, not the glass-path component chip
// (see bullet_damage_headless)
udD0 = getDammage udEnemy
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
triSimUntil { (getDammage udEnemy) > (udD0 + 0.35) }

// --- bullet vs LoBo teammate (Egyptian resistance unit) ---
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

triEndTest
