// Capture the real action menu and hint overlay, not a standalone mockup.
triSimUntil { not (isNil "GM_CI_READY") }
triSimUntil { not (isNil "gmShkTicks") }
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
GM_CI_ROLL = {0.25}
ciZone = gmZoneIndex "Village"
ciPos = (gmZone ciZone) select GM_Z_POS
player setPos ciPos
triSimUntil { (count GM_CIV_GROUPS) > 0 }
ciBody = leader (GM_CIV_GROUPS select 0)
{_x disableAI "MOVE"; _x setPos [(ciPos select 0) + 220, ciPos select 1, 0]} forEach (units (group ciBody))
ciBody setPos [(ciPos select 0) + 200, ciPos select 1, 0]
ciBody setDir 90
player setPos [(ciPos select 0) + 202, ciPos select 1, 0]
player setDir 270
triSimUntil { gmCiMenuBody == ciBody }
GM_CI_PROFILES = [[ciBody, "Village", 100, 100, 90, time, 0, false]]
triBindAction ["NextAction", 48]
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "Read their mood"]
triAssertIncludes [(triActionMenuText), "Ask for 25 R (costs goodwill)"]
triAssertIncludes [(triActionMenuText), "Consider extortion"]
triScreenshot "01_civilian_choices"
[ciBody, player, gmCiMenuRead] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { GM_CI_FEEDBACK != "" }
triSimFrames 5
triScreenshot "02_civilian_mood"
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
triAssertIncludes [GM_CI_FEEDBACK, "s to choose"]
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "Leave them be"]
triAssertIncludes [(triActionMenuText), "Threaten for"]
triScreenshot "03_civilian_extortion_choice"
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "ALREADY_PAID" }
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "Already paid"]
triScreenshot "04_civilian_payment"
GM_CI_PROFILES = [[ciBody, "Village", 40, 40, 40, time, 0, false]]
GM_CI_COOLDOWN = 30
triSimUntil { gmCiMenuState == "READY" }
[ciBody, player, gmCiMenuAsk] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "COOLDOWN" }
triAssertIncludes [GM_CI_FEEDBACK, "Try again in 30 s"]
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "Give them space"]
triScreenshot "05_civilian_refusal"
GM_CI_PROFILES = [[ciBody, "Village", 40, 40, 40, time, 0, false]]
triSimUntil { gmCiMenuState == "READY" }
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
gmCiConfirmUntil = time - 1
triSimUntil { gmCiMenuState == "READY" }
triAssertIncludes [GM_CI_FEEDBACK, "Choice expired"]
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "Consider extortion"]
triScreenshot "06_civilian_choice_expired"
triEndTest
