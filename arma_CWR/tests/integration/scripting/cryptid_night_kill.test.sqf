// ============================================================================
//  Cryptid prototype (experiments/cryptid/Cryptid.Eden): night kill.
//  23:00 on Eden. The creature starts in a forest cell at the edge of a field;
//  a lone civilian stands anchored in the open about 85 m away, facing away
//  from the trees. Expected: the creature leaves HIDE for STALK, crawls in,
//  rushes (ATTACK), lands three fixed 0.4 strikes from within reach and the
//  victim dies inside a bounded game time; the body carries the kill tag and
//  the creature moves on to FEED. No gunfire anywhere in this test.
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
triAssert [(dayTime >= 20) or (dayTime < 5)]
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
if (true) then { t0 = time; d0 = cr distance vic }
[cr, vic] exec "cry_watch.sqs"
logInfo format ["TEST start d0=%1 forest=%2 victim=%3", d0, getPos cr, getPos vic]
// the site: the creature starts in a forest cell, the victim does not
triAssertEq [(gmGeoGrid [getPos cr, 0]) select 0, "F"]
triAssertNe [(gmGeoGrid [getPos vic, 0]) select 0, "F"]
triAssertGe [d0, 55]
triSimUntil { (cr getVariable ["cryState", ""]) == "STALK" }
triAssertEq [cr getVariable "cryTarget", vic]
triSimUntil { (not (alive vic)) or (time > t0 + 110) }
logInfo format ["TEST end t=%1 alive=%2 dmg=%3 strikes=%4 dMin=%5 state=%6 fS=%7 fA=%8", time - t0, alive vic, damage vic, cr getVariable "cryStrikes", CRY_WATCH select 0, cr getVariable "cryState", cr getVariable "cryF_STALK", cr getVariable "cryF_ATTACK"]
triAssert [not (alive vic)]
triAssertLe [time - t0, 110]
triAssertGe [cr getVariable ["cryF_STALK", -1], 0]
triAssertGt [cr getVariable ["cryF_ATTACK", -1], cr getVariable ["cryF_STALK", -1]]
// it physically reached the victim: the closest approach is inside strike reach
triAssertLt [CRY_WATCH select 0, 2.8]
triAssertEq [(vic getVariable ["cryKill", [-1]]) select 0, cr getVariable "cryId"]
triSimUntil { (cr getVariable ["cryState", ""]) == "FEED" }
triAssertEq [cr getVariable "cryStrikes", 3]
triAssert [alive cr]
triEndTest
