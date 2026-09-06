// Exercise real mounted actions, including free choices and stale clicks.
triSimUntil { not (isNil "GM_CI_READY") }
// Wait text rounds up even tiny positive waits, without adding a second at
// the maximum supported campaign cooldown (single-precision game scalars).
triAssertIncludes [(["COOLDOWN", 0, "Village", 0, 0, "INSPECT", 0, 0.0001] call GM_CI_fnDescribe), "1 s"]
triAssertIncludes [(["COOLDOWN", 0, "Village", 0, 0, "INSPECT", 0, 86400] call GM_CI_fnDescribe), "86400 s"]
triSimUntil { not (isNil "gmShkTicks") }
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
ciZone = gmZoneIndex "Village"
ciPos = (gmZone ciZone) select GM_Z_POS
player setPos ciPos
triSimUntil { (count GM_CIV_GROUPS) > 0 }
ciBody = leader (GM_CIV_GROUPS select 0)
ciOther = (units (group ciBody)) select 1
{_x disableAI "MOVE"; _x setPos [(ciPos select 0) + 220, ciPos select 1, 0]} forEach (units (group ciBody))
ciBody setPos [(ciPos select 0) + 200, ciPos select 1, 0]
player setPos [(ciPos select 0) + 202, ciPos select 1, 0]
triSimUntil { gmCiMenuBody == ciBody }
triAssertEq [count GM_CI_PROFILES, 0]
ciMoney = gmResources
ciSupport = (gmZone ciZone) select GM_Z_SUPPORT
ciRolls = 0
ciEvents = 0
GM_CI_ROLL = {ciRolls = ciRolls + 1; 0.25}
GM_CI_ON_RESULT = {ciEvents = ciEvents + 1}

// Assessment allocates once, preserves the same scores, and spends nothing.
[ciBody, player, gmCiMenuRead] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { (count GM_CI_PROFILES) == 1 }
ciProfile = +(GM_CI_PROFILES select 0)
[ciBody, player, gmCiMenuRead] exec "\gmcore\scripts\civilian_interaction_action.sqs"
ciTick = GM_CI_TICKS
triSimUntil { GM_CI_TICKS > (ciTick + 4) }
triAssertEq [GM_CI_PROFILES select 0, ciProfile]
triAssertEq [gmResources, ciMoney]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport]
triAssertEq [ciRolls, 0]
triAssertEq [ciEvents, 0]
triAssertEq [count GM_CI_LAST, 0]
GM_CI_PROFILES set [0, [ciBody, "Village", 100, 100, 90, time, 0, false]]

// Preview explicitly states consequences; cancelling is not a threat.
ciOldExtort = gmCiMenuExtort
[ciBody, player, ciOldExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
triAssertIncludes [GM_CI_FEEDBACK, "Sympathetic"]
triAssertIncludes [GM_CI_FEEDBACK, "Frightened"]
triAssertIncludes [GM_CI_FEEDBACK, "Even refusal"]
triAssertEq [gmResources, ciMoney]
triAssertEq [ciRolls, 0]
ciConfirmId = gmCiMenuExtort
// Replaying the original preview ID cannot become a confirmed threat.
[ciBody, player, ciOldExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
ciTick = GM_CI_TICKS
triSimUntil { GM_CI_TICKS > (ciTick + 2) }
triAssertEq [ciRolls, 0]
[ciBody, player, gmCiMenuAsk] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "READY" }
triAssertIncludes [GM_CI_FEEDBACK, "No money taken"]
[ciBody, player, ciConfirmId] exec "\gmcore\scripts\civilian_interaction_action.sqs"
ciTick = GM_CI_TICKS
triSimUntil { GM_CI_TICKS > (ciTick + 2) }
triAssertEq [ciRolls, 0]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport]

// Expired confirmation is rejected even before the next proximity scan.
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
gmCiConfirmUntil = time - 1
[ciBody, player, "EXTORT"] call GM_CI_fnRequest
triAssertIncludes [GM_CI_FEEDBACK, "hesitated"]
triAssertEq [ciRolls, 0]
triSimUntil { gmCiMenuState == "READY" }

// Small distance changes do not churn the selected person's menu or IDs.
ciReadId = gmCiMenuRead
ciOther setPos [(ciPos select 0) + 200.5, ciPos select 1, 0]
ciTick = GM_CI_TICKS
triSimUntil { GM_CI_TICKS > (ciTick + 8) }
triAssertEq [gmCiMenuBody, ciBody]
triAssertEq [gmCiMenuRead, ciReadId]
// Walking out of reach clears intent. Returning never auto-confirms it.
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
player setPos [(ciPos select 0) + 210, ciPos select 1, 0]
[ciBody, player, "EXTORT"] call GM_CI_fnRequest
triAssertEq [ciRolls, 0]
triSimUntil { isNull gmCiMenuBody }
triAssert [isNull gmCiConfirmBody]
ciOther setPos [(ciPos select 0) + 220, ciPos select 1, 0]
player setPos [(ciPos select 0) + 202, ciPos select 1, 0]
triSimUntil { gmCiMenuBody == ciBody }
triAssertEq [gmCiMenuState, "READY"]

// Commit once, then protect the payment across both verbs and elapsed time.
gmZoneSet [ciZone, "support", 0.25]
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "CONFIRM" }
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "ALREADY_PAID" }
triAssertEq [gmResources, ciMoney + GM_CI_EXTORTION]
triAssertEq [ciRolls, 1]
triAssertEq [ciEvents, 1]
triAssertEq [GM_CI_LAST select 6, -0.25]
triAssertIncludes [GM_CI_FEEDBACK, "Village support: -0.25"]
triAssertEq [gmCiMenuAsk, -1]
triAssertEq [gmCiMenuExtort, -1]
ciProfile = GM_CI_PROFILES select 0
ciProfile set [6, 0]
GM_CI_PROFILES set [0, ciProfile]
triAssertEq [([ciBody, player, "EXTORT"] call GM_CI_fnInteract) select 0, "ALREADY_PAID"]
triAssertEq [([ciBody, player, "SOLICIT"] call GM_CI_fnInteract) select 0, "ALREADY_PAID"]
triAssertEq [ciEvents, 1]
triAssertEq [ciRolls, 1]

// A refusal preserves retry, displays the wait, and restores actions at expiry.
GM_CI_PROFILES set [0, [ciBody, "Village", 40, 40, 40, time, 0, false]]
GM_CI_COOLDOWN = 5
triSimUntil { gmCiMenuState == "READY" }
[ciBody, player, gmCiMenuAsk] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { gmCiMenuState == "COOLDOWN" }
triAssertEq [GM_CI_LAST select 0, "REFUSED"]
triAssertIncludes [GM_CI_FEEDBACK, "Donation refused"]
triAssertEq [gmCiMenuAsk, -1]
triAssertEq [gmCiMenuExtort, -1]
[ciBody, player, gmCiMenuRead] exec "\gmcore\scripts\civilian_interaction_action.sqs"
ciTick = GM_CI_TICKS
triSimUntil { GM_CI_TICKS > (ciTick + 1) }
triAssertIncludes [GM_CI_FEEDBACK, "Try again in"]
triAssertEq [ciEvents, 2]
triSimUntil { gmCiMenuState == "READY" }
triAssertGe [gmCiMenuAsk, 0]
triAssertGe [gmCiMenuExtort, 0]
// The same refusal must identify the chosen verb in player feedback.
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "REFUSED"]
triAssertIncludes [(ciResult call GM_CI_fnDescribe), "Threat refused"]
// A campaign can tune rewards to zero without showing a false paid state.
GM_CI_EXTORTION = 0
GM_CI_PROFILES set [0, [ciBody, "Village", 100, 100, 90, time, 0, false]]
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 1, 0]
triAssertIncludes [(ciResult call GM_CI_fnDescribe), "no resources"]
triAssertEq [([ciBody] call GM_CI_fnAvailability), "COOLDOWN"]
triEndTest
