// ============================================================================
//  Cryptid prototype (experiments/cryptid/Cryptid.Eden): hides by day.
//  The clock is set to noon. Same creature and lone victim as the night tests.
//  Expected: the creature never leaves HIDE, never picks a target and does not
//  approach the victim for 45 s of game time.
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
setDate [1985, 5, 10, 12, 0]
triAssert [(dayTime > 11) and (dayTime < 13)]
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
[cr, vic] exec "cry_watch.sqs"
if (true) then { t0 = time; d0 = cr distance vic }
triSimUntil { (cr getVariable ["cryState", ""]) == "HIDE" }
triSimUntil { time > t0 + 45 }
logInfo format ["TEST day d0=%1 dMin=%2 now=%3 state=%4 stalk=%5 target=%6", d0, CRY_WATCH select 0, cr distance vic, cr getVariable "cryState", cr getVariable ["cryF_STALK", -1], cr getVariable "cryTarget"]
triAssertEq [cr getVariable "cryState", "HIDE"]
triAssertEq [cr getVariable ["cryF_STALK", -1], -1]
triAssertEq [cr getVariable ["cryF_ROAM", -1], -1]
triAssert [isNull (cr getVariable "cryTarget")]
triAssertGt [CRY_WATCH select 0, d0 - 10]
triAssert [alive vic]
triEndTest
