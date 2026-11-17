// ============================================================================
//  Cryptid prototype (experiments/cryptid/Cryptid.Eden): flees when hurt.
//  Same night set-up as cryptid_night_kill. Once the creature is stalking and
//  has closed in, the test hurts it with setDamage 0.4 (no shooting: tests stay
//  quiet). Expected: it switches to FLEE with reason "hurt", the victim lives,
//  and the creature's distance from the victim grows well past where it was
//  hurt, ending in HIDE.
// ============================================================================
// Site (probed with gmGeoGrid, see the prototype notes): the creature starts in
// a forest cell 55 m inside the north edge of the woods south of [3575, 5700];
// the victim stands on the open flat field (gradient 0) 85 m north of it, 30 m
// past the tree line, with a track further west. It faces north, away from the
// trees.
CRY_T_FOREST = [3575, 5645, 0]
CRY_T_VICTIM = [3575, 5730, 0]
CRY_T_AWAY = 0
CRY_T_PLAYER = [3425, 4950, 0]
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos CRY_T_PLAYER
ctC = createCenter east
ctC2 = createCenter civilian
ctG = createGroup east
ctV = createGroup civilian
"Civilian3" createUnit [CRY_T_FOREST, ctG, "cr = this", 0.5, "PRIVATE"]
"Civilian2" createUnit [CRY_T_VICTIM, ctV, "vic = this", 0.5, "PRIVATE"]
vic disableAI "MOVE"
vic stop true
vic setDir CRY_T_AWAY
[cr] exec "cryptid.sqs"
if (true) then { t0 = time }
triSimUntil { (cr getVariable ["cryState", ""]) == "STALK" }
// let it close in a little first: the hurt lands mid-stalk
triSimUntil { ((cr distance vic) < 40) or (time > t0 + 60) }
if (true) then { cr setDamage 0.4; tHurt = time; dHurt = cr distance vic; hurtState = cr getVariable "cryState" }
logInfo format ["TEST hurt at state=%1 d=%2 t=%3", hurtState, dHurt, tHurt - t0]
triAssert [(hurtState == "STALK") or (hurtState == "FREEZE") or (hurtState == "ATTACK")]
triSimUntil { (cr getVariable ["cryState", ""]) == "FLEE" }
triAssertEq [cr getVariable "cryWhy", "hurt"]
triSimUntil { ((cr distance vic) > dHurt + 40) or (time > tHurt + 40) }
logInfo format ["TEST fled d=%1 (hurt at %2) state=%3 victim alive=%4 dmg=%5", cr distance vic, dHurt, cr getVariable "cryState", alive vic, damage vic]
triAssertGt [cr distance vic, dHurt + 40]
triAssert [alive vic]
triAssert [alive cr]
triSimUntil { ((cr getVariable ["cryState", ""]) == "HIDE") or (time > tHurt + 70) }
triAssertEq [cr getVariable "cryState", "HIDE"]
triAssert [alive vic]
triEndTest
