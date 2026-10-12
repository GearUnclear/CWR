// ============================================================================
//  Off-screen contract (issue #71) budget soak: Malden (Guerrilla.Abel, vanilla factions) on the full Remaster
//  (UD installed beside Bohemia's files, issue #67).
//  Boots through the GUERRILLA menu with the template's shipped tuning (D1
//  remote tick on, D2 at 600 s), then tours the island: the start position,
//  every non-CITY zone and the first CITY zones in table order. At each stop
//  it simulates 400 frames for caches, civilians and traffic to settle,
//  samples 256 frames of the frame profiler and writes one [gmsim] line
//  (gmSimDiagLog) plus the [tri] triPerfStats line to the engine log, which
//  guerrilla-mode/FULL-SCALE-SESSION.md reads for the evidence table.
//  Hard asserts are the accepted live-entity budgets (issue #71 section 6):
//  at most 130 live AI bodies and 40 groups on any one side at every stop.
//  Frame time is recorded, not asserted here: a headless Trident window is
//  not the human session's 1080p hub scene.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triClick 1
triSimUntil { alive player }
triAssertEq [gmSelIsland, "Abel"]

triSimUntil { GM_LIB_READY }
player allowDammage false
bdStops = [getPos player]
bdCities = 0
bdI = 0
while {bdI < gmZoneCount} do {bdZ = gmZone bdI; bdP = bdZ select 8; if ((bdZ select 1) != "CITY") then {bdStops = bdStops + [[bdP select 0, bdP select 1, 0]]} else {if (bdCities < 4) then {bdStops = bdStops + [[bdP select 0, bdP select 1, 0]]; bdCities = bdCities + 1}}; bdI = bdI + 1}
bdMaxLive = 0
bdMaxGroups = 0
bdK = -1

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

bdK = bdK + 1
if (bdK < count bdStops) then {player setPos (bdStops select bdK)}
triSimFrames 400
triPerfReset 0
triSimFrames 256
bdPerf = triPerfStats 0
bdLog = gmSimDiagLog
bdD = gmSimDiag
if ((bdK < count bdStops) and (((bdD select 2) select 3) > bdMaxLive)) then {bdMaxLive = (bdD select 2) select 3}
if ((bdK < count bdStops) and (((bdD select 3) select 3) > bdMaxGroups)) then {bdMaxGroups = (bdD select 3) select 3}

triAssertGe [bdK + 1, count bdStops]
triAssertGe [130, bdMaxLive]
triAssertGe [40, bdMaxGroups]
triEndTest
