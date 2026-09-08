// Legends across a save: part one, earn both names and write the save.
//
// The registry's whole persistence contract is that NOTHING regenerates on
// load. The seed, the history prose, the row ids, the earned name slots and
// the awardMask latch are all written into the GuerrillaLegends block, so the
// reload half can compare every one of them against what was captured here.
// Both award thresholds are crossed BEFORE the save so the reload also proves
// the latch survives: a threshold already crossed must never re-award.

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triSimUntil { (gmLegendId 0) != "" }

gmLegSaveBase = GM_COMP_NAMES select 0
gmLegSavePlain = gmLegendName 0
triAssertNe [gmLegSavePlain, ""]
triAssertEq [((gmLegendInfo 0) select 7), 0]

// SERGEANT, then COLONEL: one slot each, awarded by the live 1 Hz poll
GM_COMP_XP set [0, 250]
triSimUntil { ((gmLegendInfo 0) select 7) >= 1 }
GM_COMP_XP set [0, 1900]
triSimUntil { ((gmLegendInfo 0) select 7) >= 3 }
triSimFrames 30

gmLegSaveName = gmLegendName 0
gmLegSaveFace = gmLegendFace 0
gmLegSaveId = gmLegendId 0
gmLegSaveCount = gmLegendCount
gmLegSaveHist = (gmLegendHistory) select 2
gmLegSaveHist2 = (gmLegendHistory) select 3
gmLegSaveSeed = (gmLegendHistory) select 1
gmLegSaveEvent = (gmLegendHistoryEvent 1) select 1
gmLegSaveBosses = []
gmLegI = 0
while {gmLegI < gmLegendCount} do {gmLegRow = gmLegendInfo gmLegI; if ((gmLegRow select 2) == 1) then {gmLegSaveBosses = gmLegSaveBosses + [gmLegRow select 1]}; gmLegI = gmLegI + 1}

triAssertEq [(count gmLegSaveBosses), 3]
triAssertNe [gmLegSaveName, gmLegSavePlain]
triAssertNe [gmLegSaveFace, ""]
triAssertNe [gmLegSaveId, ""]
triAssertNe [gmLegSaveHist, ""]
triAssertNe [gmLegSaveSeed, 0]
triAssert [((gmLegendInfo 0) select 6)]
triAssertIncludes [gmLegSaveName, gmLegSaveBase]
triAssertEq [(name (GM_COMP_OBJ select 0)), gmLegSaveName]

triAssertEq [(triSaveGame "legend_names"), "OK"]
triEndTest
