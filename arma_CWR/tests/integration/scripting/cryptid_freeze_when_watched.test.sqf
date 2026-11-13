// ============================================================================
//  Cryptid prototype (experiments/cryptid/Cryptid.Eden): freezes when watched.
//  Same night set-up as cryptid_night_kill, but the anchored victim keeps
//  turning to face the creature (cry_face.sqs, a wary victim). Expected: inside 40 m the creature goes to FREEZE and lies still
//  while stared at; when the victim turns its back the creature resumes STALK,
//  then ATTACK, and the victim dies.
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
vic setDir (CRY_T_AWAY + 180)
[cr] exec "cryptid.sqs"
[vic, cr] exec "cry_face.sqs"
if (true) then { t0 = time }
triSimUntil { (cr getVariable ["cryState", ""]) == "STALK" }
triSimUntil { ((cr getVariable ["cryState", ""]) == "FREEZE") or (time > t0 + 70) }
if (true) then { tFz = time; pFz = getPos cr; dFz = cr distance vic }
logInfo format ["TEST froze at d=%1 t=%2 state=%3", dFz, tFz - t0, cr getVariable "cryState"]
triAssertEq [cr getVariable "cryState", "FREEZE"]
triAssertLt [dFz, 40.5]
triSimUntil { time > tFz + 8 }
logInfo format ["TEST after 8 s stared at: state=%1 moved=%2 d=%3", cr getVariable "cryState", cr distance pFz, cr distance vic]
triAssertEq [cr getVariable "cryState", "FREEZE"]
triAssertLt [cr distance pFz, 1.5]
if (true) then { CRY_FACE = false; vic setDir CRY_T_AWAY }
triSimUntil { ((cr getVariable ["cryState", ""]) == "STALK") or (time > tFz + 20) }
triAssertEq [cr getVariable "cryState", "STALK"]
triSimUntil { (not (alive vic)) or (time > tFz + 80) }
logInfo format ["TEST end alive=%1 t=%2 strikes=%3 state=%4", alive vic, time - t0, cr getVariable "cryStrikes", cr getVariable "cryState"]
triAssert [not (alive vic)]
triEndTest
