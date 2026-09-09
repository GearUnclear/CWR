// Enemy Legends across a save: part one, three commanders in three different
// states, then write the save.
//
// The persistence contract is that NOTHING about a commander regenerates on
// load and nothing re-fires: the role, the stand, the marker paint, the
// objective state and the defeat latch are all in the GuerrillaLegends block,
// and the actors are re-pinned rather than rebuilt. So this half deliberately
// leaves one commander DEAD (his latch, his DONE objective and his repainted
// marker all have to survive), one WOUNDED (damage is the world's business,
// not the registry's), one UNTOUCHED and STANDING (the DAMove re-assert is
// what the reload half measures), plus a fallen companion so the mixed table
// is exercised, and the dossier open on a commander's own page.
//
// Assertions inside a while body are silent (the harness reads the line's
// value), so the loops below only collect.

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }

glBoss = []
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 1) then {glBoss = glBoss + [glI]}; glI = glI + 1}
triAssertEq [(count glBoss), 3]
triSimUntil { (not (isNull (gmLegendBody (glBoss select 0)))) and (not (isNull (gmLegendBody (glBoss select 1)))) and (not (isNull (gmLegendBody (glBoss select 2)))) }

// -- the three states ---------------------------------------------------------
glDead = glBoss select 0
glHurt = glBoss select 1
glLive = glBoss select 2
(gmLegendBody glDead) setDammage 1
triSimUntil { gmLegendDefeated glDead }
// wound him FIRST, then shield him: allowDammage false refuses setDammage too
(gmLegendBody glHurt) setDammage 0.6
(gmLegendBody glHurt) allowDammage false
// the untouched one keeps standing: his stand is the reload half's yardstick
triAssertEq [(gmLegendDefeated glHurt), false]
triAssertEq [(gmLegendDefeated glLive), false]

// -- a fallen companion, so the save carries a MIXED row table ----------------
glComp = -1
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 0) then {if (glComp < 0) then {glComp = glI}}; glI = glI + 1}
triAssertGe [glComp, 0]
gmLegSaveCompName = (gmLegendInfo glComp) select 1
(GM_COMP_OBJ select 0) setDammage 1
triSimUntil { not ((gmLegendInfo glComp) select 5) }

// -- everything worth comparing, captured --------------------------------------
gmLegSaveIds = []
gmLegSaveNames = []
gmLegSaveRoles = []
gmLegSaveZones = []
gmLegSaveX = []
gmLegSaveY = []
gmLegSaveMarkers = []
gmLegSaveStates = []
gmLegSaveDefeated = []
glI = 0
while {glI < 3} do {glRow = gmLegendInfo (glBoss select glI); glP = gmLegendPos (glBoss select glI); gmLegSaveIds = gmLegSaveIds + [glRow select 0]; gmLegSaveNames = gmLegSaveNames + [glRow select 1]; gmLegSaveRoles = gmLegSaveRoles + [glRow select 3]; gmLegSaveZones = gmLegSaveZones + [glRow select 4]; gmLegSaveX = gmLegSaveX + [glP select 0]; gmLegSaveY = gmLegSaveY + [glP select 1]; gmLegSaveMarkers = gmLegSaveMarkers + [gmLegendMarker (glBoss select glI)]; gmLegSaveStates = gmLegSaveStates + [gmJournalObjectiveState ("legend_" + (glRow select 0))]; gmLegSaveDefeated = gmLegSaveDefeated + [gmLegendDefeated (glBoss select glI)]; glI = glI + 1}
gmLegSaveCount = gmLegendCount
gmLegSaveHist = (gmLegendHistory) select 2
gmLegSaveSeed = (gmLegendHistory) select 1
gmLegSaveRows = glBoss + []
gmLegSaveCompRow = glComp
gmLegSaveHurt = getDammage (gmLegendBody glHurt)
gmLegSaveLivePos = gmLegendPos glLive
gmLegSaveJournal = gmJournalCount

// the states really are the three intended ones
triAssertEq [(gmLegSaveDefeated select 0), true]
triAssertEq [(gmLegSaveDefeated select 1), false]
triAssertEq [(gmLegSaveDefeated select 2), false]
triAssertEq [(gmLegSaveStates select 0), "DONE"]
triAssertEq [(gmLegSaveStates select 1), "ACTIVE"]
triAssertEq [((gmLegSaveMarkers select 0) select 2), "ColorGreen"]
triAssertEq [((gmLegSaveMarkers select 1) select 2), "ColorRed"]
triAssertGt [gmLegSaveHurt, 0.3]
triAssertLt [gmLegSaveHurt, 1]

// -- and the dossier is OPEN on a commander's own page when the save is written
// the map KEY is the journal rebuild seam (DisplayMap::ResetHUD); triOpenMap
// alone only forces the draw, so the dossier sections do not exist yet
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10
triAssertEq [(triBriefingSwitch ("GM_WHO_" + (gmLegSaveIds select 0))), ("GM_WHO_" + (gmLegSaveIds select 0))]
gmLegSaveDossier = triControlText 56
triAssertIncludes [gmLegSaveDossier, (gmLegSaveNames select 0)]
triAssertIncludes [gmLegSaveDossier, "defeated"]

triAssertEq [(triSaveGame "legends"), "OK"]
triEndTest
