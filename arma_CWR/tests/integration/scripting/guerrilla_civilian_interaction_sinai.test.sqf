// Real island pack, opposite war sides, shared core; no class/side literals
// in the production interaction layer. @LoBo setup matches sinai_swap.
triSimUntil { not (isNil "GM_CI_READY") }
triSimUntil { not (isNil "gmShkTicks") }
triAssertEq [gmOccupierSide, "WEST"]
triAssertEq [gmResistanceSide, "EAST"]
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
GM_CI_SPREAD = 0
ciSwapZone = "CITY" call GM_fnZoneOfType
triAssertGe [ciSwapZone, 0]
ciSwapPos = (gmZone ciSwapZone) select GM_Z_POS
player setPos [ciSwapPos select 0, ciSwapPos select 1, 0]
triSimUntil { (count GM_CIV_GROUPS) > 0 }
ciSwapBody = leader (GM_CIV_GROUPS select 0)
{_x disableAI "MOVE"} forEach (units (group ciSwapBody))
ciSwapBody setPos [ciSwapPos select 0, ciSwapPos select 1, 0]
player setPos [(ciSwapPos select 0) + 1, ciSwapPos select 1, 0]
triSimUntil { not (isNull gmCiMenuBody) }
ciSwapBody = gmCiMenuBody
ciSwapZone = [ciSwapBody, player] call GM_CI_fnEligible
triAssertGe [ciSwapZone, 0]
gmZoneSet [ciSwapZone, "owner", gmOccupierSide]
gmZoneSet [ciSwapZone, "support", 80]
ciSwapIdx = [ciSwapBody, ciSwapZone] call GM_CI_fnProfile
triAssertEq [((GM_CI_PROFILES select ciSwapIdx) select 2), 40]
ciSwapName = (gmZone ciSwapZone) select GM_Z_NAME
GM_CI_PROFILES set [ciSwapIdx, [ciSwapBody, ciSwapName, 100, 100, 90, time, 0]]
ciSwapMoney = gmResources
ciSwapResult = [ciSwapBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciSwapResult select 0, "EXTORTED"]
triAssertEq [ciSwapResult select 2, ciSwapName]
triAssertEq [gmResources, ciSwapMoney + GM_CI_EXTORTION]
triAssertEq [(format ["%1", side ciSwapBody]), "CIV"]
triAssertEq [count GM_CI_PROFILES, 1]
triEndTest
