triSimUntil { GM_LIB_READY }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
// No map has been opened: a face outside the four rolled faces must still finish.
gpBefore = triPortraitStats
gpLate = triPortraitId [typeOf (GM_COMP_OBJ select 0), "Face11"]
triAssertEq [(triPortraitId [typeOf (GM_COMP_OBJ select 0), "Face11"]), gpLate]
triSimFrames 3
gpAfter = triPortraitStats
triAssertEq [gpAfter select 0, (gpBefore select 0) + 1]
triAssertEq [gpAfter select 1, gpAfter select 0]
triAssertEq [(gpAfter select 2) + (gpAfter select 3), (gpBefore select 2) + (gpBefore select 3) + 1]

// Add a later companion through the normal script roster/spawn/bind machinery.
gpIndex = count GM_COMP_NAMES
GM_COMP_NAMES = GM_COMP_NAMES + ["Portrait recruit"]
GM_COMP_XP = GM_COMP_XP + [0]
GM_COMP_RANK = GM_COMP_RANK + ["PRIVATE"]
GM_COMP_SKILL = GM_COMP_SKILL + [0.35]
GM_COMP_ALIVE = GM_COMP_ALIVE + [true]
GM_COMP_OBJ = GM_COMP_OBJ + [objNull]
GM_COMP_LOADOUT = GM_COMP_LOADOUT + [[[], []]]
GM_COMP_PREVRATING = GM_COMP_PREVRATING + [0]
[gpIndex, position player] call GM_fnCompSpawn
triSimUntil { (gmLegendId gpIndex) != "" }
triAssertEq [((triPortraitRow gpIndex) select 0), typeOf (GM_COMP_OBJ select gpIndex)]
triAssertEq [((triPortraitRow gpIndex) select 1), gmLegendFace gpIndex]
triAssertEq [((triPortraitRow gpIndex) select 3), 2]
gpPhoto = "portrait:" + ((triPortraitRow gpIndex) select 2)
gpAnchor = "GM_WHO_" + (gmLegendId gpIndex)
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10
triAssertEq [(triBriefingSwitch gpAnchor), gpAnchor]
triAssertIncludes [(triBriefingImages), gpPhoto]
triAssert [triBriefingFits]
triEndTest
