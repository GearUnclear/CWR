// ============================================================================
//  Cryptid prototype: screenshots of a stalk at dusk-night (20:40) for the
//  owner. Not a regression test (it lives outside tests/integration on
//  purpose). Same site as the cryptid_* tests. Quiet: no gunfire, no sound.
//  Shots: 01 the crawl seen from behind the creature, 02 close in behind the
//  victim, 03 the rush, 04 the strike.
// ============================================================================
CRY_T_FOREST = [3575, 5645, 0]
CRY_T_VICTIM = [3575, 5730, 0]
CRY_T_AWAY = 0
CRY_T_PLAYER = [3425, 4950, 0]
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos CRY_T_PLAYER
setDate [1985, 5, 10, 20, 40]
showCinemaBorder false
ssCam = "camera" camCreate [3575, 5300, 30]
ssCam cameraEffect ["internal", "back"]
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
// camera: ssBack metres behind the creature on the victim line, ssUp high, aimed at the victim's chest
ssAim = { ssC = getPosASL cr; ssV = getPosASL vic; ssDx = (ssV select 0) - (ssC select 0); ssDz = (ssV select 1) - (ssC select 1); ssL = sqrt ((ssDx * ssDx) + (ssDz * ssDz)) + 0.01; ssCx = (ssC select 0) - (ssDx / ssL) * ssBack + (ssDz / ssL) * ssSide; ssCz = (ssC select 1) - (ssDz / ssL) * ssBack - (ssDx / ssL) * ssSide; ssCy = (ssC select 2) + ssUp; triSetView [ssCx, ssCy, ssCz, (ssV select 0) - ssCx, ((ssV select 2) + 1.2) - ssCy, (ssV select 1) - ssCz] }
triSimUntil { ((cr getVariable ["cryState", ""]) == "STALK") and ((cr distance vic) < 32) }
if (true) then { ssBack = 3.5; ssSide = 0.8; ssUp = 0.6; call ssAim }
triSimFrames 10
triScreenshot "01_crawl_behind_creature"
triSimUntil { ((cr distance vic) < 16) or ((cr getVariable ["cryState", ""]) == "ATTACK") }
if (true) then { ssBack = 2.5; ssSide = -1.2; ssUp = 0.4; call ssAim }
triSimFrames 10
triScreenshot "02_close_behind_victim"
triSimUntil { ((cr getVariable ["cryState", ""]) == "ATTACK") and ((cr distance vic) < 7) }
if (true) then { ssBack = 4; ssSide = 2.5; ssUp = 1.2; call ssAim }
triSimFrames 5
triScreenshot "03_rush"
triSimUntil { (cr getVariable ["cryStrikes", 0]) >= 1 }
if (true) then { ssBack = 3; ssSide = 3; ssUp = 1.0; call ssAim }
triSimFrames 5
triScreenshot "04_strike"
triSimUntil { not (alive vic) }
triEndTest
