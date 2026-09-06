triSimUntil { not (isNil "GM_CI_READY") }
triAssertEq [count GM_CI_PROFILES, 0]
triAssertEq [(triLoadGame "civilian_interaction"), "OK"]
triSimFrames 3
triAssert [not (isNull gmCiSaveBody)]
triAssertEq [count GM_CI_PROFILES, 1]
triAssertEq [((GM_CI_PROFILES select 0) select 0), gmCiSaveBody]
triAssertEq [((GM_CI_PROFILES select 0) select 1), "Village"]
triAssertEq [((GM_CI_PROFILES select 0) select 3), 60]
triAssertEq [((GM_CI_PROFILES select 0) select 4), 90]
triAssertEq [((GM_CI_PROFILES select 0) select 6), gmCiSaveNext]
triAssert [((GM_CI_PROFILES select 0) select 7)]
triAssertEq [gmResources, gmCiSaveMoney]
triAssertEq [((gmZone gmCiSaveZone) select GM_Z_SUPPORT), gmCiSaveSupport]
gmCiSaveResult = [gmCiSaveBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [gmCiSaveResult select 0, "ALREADY_PAID"]
triAssertEq [gmResources, gmCiSaveMoney]
triSimUntil { GM_CI_TICKS > (gmCiSaveTicks + 2) }
// Native event registration replaces a slot, it does not add a subscriber.
// The shared bootstrap must still deliver campaign.sqs's restore event too.
triSimUntil { not (isNil "GM_pVer") }
triAssert [not GM_CI_REMOUNT]
triAssertEq [gmCiMenuBody, gmCiSaveBody]
triAssertEq [gmCiMenuState, "ALREADY_PAID"]
triAssertEq [gmCiMenuAsk, -1]
triAssertEq [gmCiMenuExtort, -1]
triAssertGe [gmCiMenuRead, 0]
triAssert [isNull gmCiConfirmBody]
triAssertEq [count GM_CI_REQUEST, 0]
// Expiring the old cooldown does not mint another payout after reload.
gmCiSavedProfile = GM_CI_PROFILES select 0
gmCiSavedProfile set [6, 0]
GM_CI_PROFILES set [0, gmCiSavedProfile]
triAssertEq [([gmCiSaveBody, player, "SOLICIT"] call GM_CI_fnInteract) select 0, "ALREADY_PAID"]
triAssertEq [gmResources, gmCiSaveMoney]
// Delete the saved civilian: no stale personal row or dangling menu survives.
deleteVehicle gmCiSaveBody
triSimUntil { (count GM_CI_PROFILES) == 0 }
triAssertEq [gmResources, gmCiSaveMoney]
triAssertEq [((gmZone gmCiSaveZone) select GM_Z_SUPPORT), gmCiSaveSupport]
triEndTest
