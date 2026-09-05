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
triAssertEq [gmResources, gmCiSaveMoney]
triAssertEq [((gmZone gmCiSaveZone) select GM_Z_SUPPORT), gmCiSaveSupport]
gmCiSaveResult = [gmCiSaveBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [gmCiSaveResult select 0, "COOLDOWN"]
triAssertEq [gmResources, gmCiSaveMoney]
triSimUntil { GM_CI_TICKS > (gmCiSaveTicks + 2) }
// Native event registration replaces a slot, it does not add a subscriber.
// The shared bootstrap must still deliver campaign.sqs's restore event too.
triSimUntil { not (isNil "GM_pVer") }
triAssert [not GM_CI_REMOUNT]
triAssertEq [gmCiMenuBody, gmCiSaveBody]
triAssertGe [gmCiMenuAsk, 0]
triAssertGe [gmCiMenuExtort, 0]
// Delete the saved civilian: no stale personal row or dangling menu survives.
deleteVehicle gmCiSaveBody
triSimUntil { (count GM_CI_PROFILES) == 0 }
triAssertEq [gmResources, gmCiSaveMoney]
triAssertEq [((gmZone gmCiSaveZone) select GM_Z_SUPPORT), gmCiSaveSupport]
triEndTest
