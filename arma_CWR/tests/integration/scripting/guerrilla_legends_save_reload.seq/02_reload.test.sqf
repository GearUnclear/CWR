// Enemy Legends across a save: part two, load it into a DIFFERENT campaign.
//
// This instance boots its own campaign first, which draws its own seed, rolls
// its own three commanders and places and spawns them somewhere else entirely.
// The load then has to replace all of it. Anything still matching the fresh
// campaign means the block was not read; anything failing to match the capture
// means something regenerated on load, and both are the same bug class.
//
// The load-pass work this half exists to measure, beyond plain field equality:
//   * the DAMove pin is re-asserted (a boss re-created by the world load does
//     not carry the pin unless the registry puts it back), so a living
//     commander is still standing sixty seconds later;
//   * the defeated marker is repainted on the FIRST boss tick after the load,
//     never inside ReconcileAfterLoad: growing markersMap between the Legends
//     block and the marker array leaves the archive walking one Item past the
//     end;
//   * the defeat latch is persisted, so nothing re-fires the campaign line or
//     re-flips the objective, and no commander is rebuilt.

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }

// NOTE for anyone extending this file: `!=` refuses a Bool in this evaluator
// (Number, String, Object, Side, Group only), and a script error in test mode
// aborts the run rather than failing one assertion. Bools are compared through
// format ["%1", ...] below for that reason.
//
// the fresh campaign: none of the save's globals exist yet
triAssert [(isNil "gmLegSaveIds")]
glFresh = []
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 1) then {glFresh = glFresh + [(gmLegendInfo glI) select 1]}; glI = glI + 1}
triAssertEq [(count glFresh), 3]
glFreshHist = (gmLegendHistory) select 2
glFreshName0 = glFresh select 0

triAssertEq [(triLoadGame "legends"), "OK"]
triSimFrames 3
triAssert [not (isNil "gmLegSaveIds")]

// -- the same rows, in the same order, with the same everything ---------------
triAssertEq [gmLegendCount, gmLegSaveCount]
glBoss = []
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 1) then {glBoss = glBoss + [glI]}; glI = glI + 1}
triAssertEq [(count glBoss), 3]
glBad = ""
glI = 0
while {glI < 3} do {glRow = gmLegendInfo (glBoss select glI); glP = gmLegendPos (glBoss select glI); glTag = format ["[%1]", glI]; if ((glRow select 0) != (gmLegSaveIds select glI)) then {glBad = glBad + glTag + "id=" + (glRow select 0) + " "}; if ((glRow select 1) != (gmLegSaveNames select glI)) then {glBad = glBad + glTag + "name=" + (glRow select 1) + " "}; if ((glRow select 3) != (gmLegSaveRoles select glI)) then {glBad = glBad + glTag + "role=" + (glRow select 3) + " "}; if ((glRow select 4) != (gmLegSaveZones select glI)) then {glBad = glBad + glTag + "zone=" + (glRow select 4) + " "}; if ((glP select 0) != (gmLegSaveX select glI)) then {glBad = glBad + glTag + "x=" + (format ["%1", glP select 0]) + " "}; if ((glP select 1) != (gmLegSaveY select glI)) then {glBad = glBad + glTag + "y=" + (format ["%1", glP select 1]) + " "}; if ((format ["%1", gmLegendDefeated (glBoss select glI)]) != (format ["%1", gmLegSaveDefeated select glI])) then {glBad = glBad + glTag + "defeated "}; if ((gmJournalObjectiveState ("legend_" + (glRow select 0))) != (gmLegSaveStates select glI)) then {glBad = glBad + glTag + "obj=" + (gmJournalObjectiveState ("legend_" + (glRow select 0))) + " "}; glI = glI + 1}
triAssertEq [glBad, ""]

// it really is the SAVED campaign, not the one this instance booted
triAssertNe [((gmLegendInfo (glBoss select 0)) select 1), glFreshName0]
triAssertNe [((gmLegendHistory) select 2), glFreshHist]
// ... and the history prose came back byte for byte
triAssertEq [((gmLegendHistory) select 2), gmLegSaveHist]
triAssertEq [((gmLegendHistory) select 1), gmLegSaveSeed]

// -- the markers. The defeated one is repainted by the first boss tick AFTER
//    the load, so this is the assertion that fails if anyone ever moves that
//    work back inside ReconcileAfterLoad. -----------------------------------------
glBad = ""
glI = 0
while {glI < 3} do {glMk = gmLegendMarker (glBoss select glI); glWas = gmLegSaveMarkers select glI; glTag = format ["[%1]", glI]; if ((count glMk) != 4) then {glBad = glBad + glTag + "nomarker "}; if ((count glMk) == 4) then {glJ = 0; while {glJ < 4} do {if ((glMk select glJ) != (glWas select glJ)) then {glBad = glBad + glTag + (format ["f%1=%2 ", glJ, glMk select glJ])}; glJ = glJ + 1}}; glI = glI + 1}
triAssertEq [glBad, ""]
triAssertEq [((gmLegendMarker (glBoss select 0)) select 2), "ColorGreen"]
triAssertIncludes [((gmLegendMarker (glBoss select 0)) select 3), "(defeated)"]
triAssertEq [((gmLegendMarker (glBoss select 1)) select 2), "ColorRed"]
// no duplicate marker rows: a second gmLegend_<id> would leave the map with two
// icons, and the readback would still find the first - so ask the WORLD for the
// marker's position and require it to be the stand
triAssertEq [(getMarkerPos ("gmLegend_" + (gmLegSaveIds select 2))) select 0, (gmLegSaveX select 2)]

// -- the wounded commander kept his damage; the dead one is still dead --------
triAssert [(not (isNull (gmLegendBody (glBoss select 1))))]
triAssertGt [(getDammage (gmLegendBody (glBoss select 1))), 0.3]
triAssertLt [(getDammage (gmLegendBody (glBoss select 1))), 1]
triAssertEq [(gmLegendDefeated (glBoss select 0)), true]
triAssertEq [(gmJournalObjectiveState ("legend_" + (gmLegSaveIds select 0))), "DONE"]

// -- the fallen companion is still fallen and still named ---------------------
triAssertEq [((gmLegendInfo gmLegSaveCompRow) select 2), 0]
triAssertEq [((gmLegendInfo gmLegSaveCompRow) select 1), gmLegSaveCompName]
triAssertEq [((gmLegendInfo gmLegSaveCompRow) select 5), false]

// -- THE PIN CAME BACK. The living commander is re-pinned by the load pass, so
//    sixty seconds later he is still on his stand rather than walking to the
//    waypoint his group carries. --------------------------------------------------
glLiveObj = gmLegendBody (glBoss select 2)
triAssert [(not (isNull glLiveObj))]
triAssertEq [(triUnitAIDisabled "glLiveObj"), 2]
glLiveAt = getPos glLiveObj
glJ0 = gmJournalCount
glT0 = time
triSimUntil { time > glT0 + 60 }
glLiveNow = getPos glLiveObj
glLiveMoved = sqrt ((((glLiveNow select 0) - (glLiveAt select 0)) * ((glLiveNow select 0) - (glLiveAt select 0))) + (((glLiveNow select 1) - (glLiveAt select 1)) * ((glLiveNow select 1) - (glLiveAt select 1))))
triAssertLt [glLiveMoved, 3]

// -- and a minute of the 1 Hz poll re-fired nothing: no second campaign line
//    for the dead commander, no new row, no repaint of a settled marker -------
glDeadLines = 0
glJi = 0
while {glJi < gmJournalCount} do {if ((gmJournalEntryChar glJi) == (gmLegSaveIds select 0)) then {glDeadLines = glDeadLines + 1}; glJi = glJi + 1}
triAssertEq [glDeadLines, 1]
triAssertEq [gmLegendCount, gmLegSaveCount]
triAssertEq [(gmLegendDefeated (glBoss select 1)), false]
triAssertEq [(gmLegendDefeated (glBoss select 2)), false]
triAssertEq [((gmLegendMarker (glBoss select 0)) select 2), "ColorGreen"]

// -- the dossier reads the same row after the load ---------------------------
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10
triAssertEq [(triBriefingSwitch ("GM_WHO_" + (gmLegSaveIds select 0))), ("GM_WHO_" + (gmLegSaveIds select 0))]
glDossier = triControlText 56
triAssertIncludes [glDossier, (gmLegSaveNames select 0)]
triAssertIncludes [glDossier, "defeated"]

triEndTest
