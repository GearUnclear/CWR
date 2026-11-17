// ============================================================================
//  Cryptid prototype (experiments/cryptid/Cryptid.Eden): fire scares it off.
//  Same night set-up as cryptid_night_kill. Once the creature is stalking and
//  has closed in, the test lights a camp fire 8 m from it (sound is off).
//  Expected: FLEE with reason "light", the creature unhurt, the distance from
//  the fire and the victim growing, the victim alive.
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
triSimUntil { ((cr distance vic) < 40) or (time > t0 + 60) }
if (true) then { fp = getPos cr; fire1 = "Fire" createVehicle [(fp select 0) + 8, fp select 1, 0]; fire1 inflame true; tLit = time; dLit = cr distance vic }
logInfo format ["TEST fire lit at d=%1 t=%2 inflamed=%3 fireDist=%4", dLit, tLit - t0, inflamed fire1, cr distance fire1]
triAssert [inflamed fire1]
triSimUntil { (cr getVariable ["cryState", ""]) == "FLEE" }
triAssertEq [cr getVariable "cryWhy", "light"]
triSimUntil { ((cr distance vic) > dLit + 40) or (time > tLit + 40) }
logInfo format ["TEST fled d=%1 (lit at %2) fireDist=%3 state=%4 dmg=%5", cr distance vic, dLit, cr distance fire1, cr getVariable "cryState", damage cr]
triAssertGt [cr distance vic, dLit + 40]
triAssertGt [cr distance fire1, 40]
triAssertEq [damage cr, 0]
triAssert [alive vic]
triEndTest
