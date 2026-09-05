triSimUntil { not (isNil "GM_CI_READY") }
triSimUntil { not (isNil "gmShkTicks") }
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
gmCiSaveZone = gmZoneIndex "Village"
gmCiSavePos = (gmZone gmCiSaveZone) select GM_Z_POS
player setPos gmCiSavePos
triSimUntil { (count GM_CIV_GROUPS) > 0 }
gmCiSaveBody = leader (GM_CIV_GROUPS select 0)
{_x disableAI "MOVE"} forEach (units (group gmCiSaveBody))
// Keep the native presence-based support accrual out of this ledger test.
gmCiSaveBody setPos [(gmCiSavePos select 0) + 200, gmCiSavePos select 1, 0]
player setPos [(gmCiSavePos select 0) + 201, gmCiSavePos select 1, 0]
triSimUntil { gmCiMenuBody == gmCiSaveBody }
gmZoneSet [gmCiSaveZone, "support", 70]
GM_CI_PROFILES = [[gmCiSaveBody, "Village", 90, 90, 90, time, 0]]
gmCiSaveResult = [gmCiSaveBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [gmCiSaveResult select 0, "EXTORTED"]
gmCiSaveMoney = gmResources
gmCiSaveSupport = (gmZone gmCiSaveZone) select GM_Z_SUPPORT
gmCiSaveNext = (GM_CI_PROFILES select 0) select 6
gmCiSaveTicks = GM_CI_TICKS
triAssertEq [(triSaveGame "civilian_interaction"), "OK"]
triEndTest
