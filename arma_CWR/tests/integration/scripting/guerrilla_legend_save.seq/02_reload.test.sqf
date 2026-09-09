// Legends across a save: part two, load it back into a DIFFERENT campaign.
//
// This instance boots its own fresh campaign first, which draws its own seed,
// generates its own history and pre-rolls its own three enemy Legends. The
// load then has to replace all of it: if anything here still matches the fresh
// campaign, the block was not read; if anything fails to match the captured
// values, something regenerated on load. Both are the same bug class.

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triSimUntil { (gmLegendId 0) != "" }

// the fresh campaign: no award has fired, and nothing from the save is here yet
gmLegFreshName = gmLegendName 0
gmLegFreshHist = (gmLegendHistory) select 2
triAssertEq [((gmLegendInfo 0) select 7), 0]
triAssert [not ((gmLegendInfo 0) select 6)]
triAssert [(isNil "gmLegSaveName")]

triAssertEq [(triLoadGame "legend_names"), "OK"]
triSimFrames 3
triAssert [not (isNil "gmLegSaveName")]

// every captured value, identical
triAssertEq [(gmLegendName 0), gmLegSaveName]
triAssertEq [(gmLegendFace 0), gmLegSaveFace]
triAssertEq [(gmLegendId 0), gmLegSaveId]
triAssertEq [gmLegendCount, gmLegSaveCount]
triAssertEq [((gmLegendHistory) select 1), gmLegSaveSeed]
triAssertEq [((gmLegendHistory) select 2), gmLegSaveHist]
triAssertEq [((gmLegendHistory) select 3), gmLegSaveHist2]
triAssertEq [((gmLegendHistoryEvent 1) select 1), gmLegSaveEvent]
// the award latch rode the save: both bits, still a Legend
triAssertEq [((gmLegendInfo 0) select 7), 3]
triAssert [((gmLegendInfo 0) select 6)]
// and it is the SAVED campaign, not the fresh one this instance booted
triAssertNe [(gmLegendName 0), gmLegFreshName]
triAssertNe [((gmLegendHistory) select 2), gmLegFreshHist]

// the three enemy Legends came back in the same rows, in the same order
gmLegI = 0
gmLegB = 0
while {gmLegI < gmLegendCount} do {gmLegRow = gmLegendInfo gmLegI; if ((gmLegRow select 2) == 1) then {triAssertEq [(gmLegRow select 1), (gmLegSaveBosses select gmLegB)]; gmLegB = gmLegB + 1}; gmLegI = gmLegI + 1}
triAssertEq [gmLegB, 3]

// no duplicate rows, and no further award over three hundred frames of poll
triSimFrames 300
triAssertEq [gmLegendCount, gmLegSaveCount]
triAssertEq [(gmLegendName 0), gmLegSaveName]
triAssertEq [(gmLegendFace 0), gmLegSaveFace]
triAssertEq [(gmLegendId 0), gmLegSaveId]
triAssertEq [((gmLegendInfo 0) select 7), 3]

// the rebuilt body wears the registry name again: the load path rerolls the
// pool identity onto a recreated body, and the registry re-asserts its own
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triSimUntil { (name (GM_COMP_OBJ select 0)) == gmLegSaveName }
triAssertEq [(name (GM_COMP_OBJ select 0)), gmLegSaveName]

triEndTest
