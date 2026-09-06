// ============================================================================
//  Guerrilla field journal on the REAL map screen (guerrilla_native.abel).
//    The native Journal (Game/Guerrilla/Journal) is rendered into the
//    in-mission map's briefing notepad by UI/Guerrilla/GuerrillaJournalPages
//    whenever CfgGuerrillaZones is active. The unit suite proves the renderer
//    against a parser-only HTML container; this test opens the actual
//    DisplayMainMap (triOpenMap -> DisplayMap::Init -> ReloadBriefingContent)
//    and walks the pages through the briefing test hooks:
//      * the Notes page ("Main", aliased __BRIEFING) is the SITUATION page:
//        masthead, Strength (incl. the companions status line), Territory;
//      * the Plan page (__PLAN, copied from "Plan") carries the standing
//        objectives and the scripted starters;
//      * the ledgers (GM_ZONES index + GM_ZONE_<i> per zone, GM_CELL,
//        GM_FACTION), the diary (GM_LOG) and the handbook (GM_MAN_INDEX +
//        GM_MAN_*) exist and the in-page links route between them;
//      * a journal write while the map is open repaints the pages
//        (revision compare in DisplayMap::OnSimulate).
//    IDC_BRIEFING = 56 is the notes HTML control (resincl.hpp).
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- the managers' boot writes: opening diary line (campaign.sqs, tagged
//    Camp) + the starter objectives --------------------------------
triSimUntil { gmJournalCount >= 1 }
triSimUntil { (gmJournalObjectiveState "firstRecruit") == "ACTIVE" }
triSimUntil { (gmJournalObjectiveState "firstZone") == "ACTIVE" }
triSimUntil { (gmJournalObjectiveState "firstUnlock") == "ACTIVE" }
triAssertIncludes [((gmJournalEntry 0) select 1), "Reached the Camp"]
triAssertEq [((gmJournalEntry 0) select 2), "Camp"]
triAssertEq [gmIslandName, "Malden"]
// the companions / loot managers publish their status lines at boot
triSimUntil { (gmJournalStatusText "Companions") != "" }
triAssertIncludes [(gmJournalStatusText "Companions"), "Petra"]
triAssertEq [(gmJournalStatusText "Standard issue"), "the faction rifle only"]

// -- open the real map display the way the player does. DisplayMission::InitUI
//    created the DisplayMainMap at mission start (journal still empty); the
//    map KEY (UAMap, scancode 16 = M) makes World::Simulate call
//    DisplayMap::ResetHUD (the journal rebuild seam) and then SimulateHUD ->
//    OnSimulate each sim frame (the revision-compare repaint). triOpenMap
//    only forces the draw; triSimFrames runs World::Simulate --------------------
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10

// -- Notes page (Main / __BRIEFING) = SITUATION. The notepad paginates long
//    sections (SplitSection: Main, Main/0, ...) and triControlText reads only
//    the CURRENT page, so assert what sits on page 1: masthead, Strength
//    (incl. the companions status line), Territory --------------------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
gjNotes = triControlText 56
triAssertIncludes [gjNotes, "SITUATION"]
triAssertIncludes [gjNotes, "Malden"]
triAssertIncludes [gjNotes, "STRENGTH"]
triAssertIncludes [gjNotes, "Treasury"]
triAssertIncludes [gjNotes, "Petra"]
triAssertIncludes [gjNotes, "TERRITORY"]
triAssertIncludes [gjNotes, "Bases held"]
triAssertIncludes [gjNotes, "Towns risen"]
triAssertIncludes [gjNotes, "War level"]
// the Notes tab alias resolves to the same page
triAssertEq [(triBriefingSwitch "__BRIEFING"), "Main"]

// -- Plan page (__PLAN <- "Plan"): standing objectives + scripted starters ----
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
gjPlan = triControlText 56
triScreenshot "journal_plan_objectives"
triAssertIncludes [gjPlan, "OBJECTIVES"]
triAssertIncludes [gjPlan, "Hold every base"]
triAssertIncludes [gjPlan, "Raise every town"]
triAssertIncludes [gjPlan, "Take a first recruit"]
triAssertIncludes [gjPlan, "NEXT MOVES"]

// -- the ledgers: Zones index + one page per zone, Cell, Resistance, Diary ---
triAssertEq [(triBriefingSwitch "GM_ZONES"), "GM_ZONES"]
triAssertIncludes [(triControlText 56), "ZONES"]
triAssertIncludes [(triControlText 56), "Camp"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triAssertIncludes [(triControlText 56), "RECORD"]
triAssertEq [(triBriefingSwitch "GM_CELL"), "GM_CELL"]
triAssertIncludes [(triControlText 56), "ROSTER"]
triAssertIncludes [(triControlText 56), "Standard issue"]
triAssertEq [(triBriefingSwitch "GM_FACTION"), "GM_FACTION"]
triAssertIncludes [(triControlText 56), "Island held"]
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_LOG"]
triAssertIncludes [(triControlText 56), "Reached the Camp"]

// -- handbook: index + chapters exist; in-page links route between pages ----
triAssertEq [(triBriefingSwitch "GM_MAN_INDEX"), "GM_MAN_INDEX"]
triAssertIncludes [(triControlText 56), "HANDBOOK"]
triAssertEq [(triBriefingSwitch "GM_MAN_MODE"), "GM_MAN_MODE"]
triAssertIncludes [(triControlText 56), "THE CAMPAIGN"]
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triAssertIncludes [(triControlText 56), "UNDERCOVER"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
// the nav row on Situation links to the Zones index, the index links to a
// zone page, and every page links back to Situation
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triAssertIncludes [(triClickBriefingLink "#GM_ZONES"), "OK:section=GM_ZONES"]
triAssertIncludes [(triClickBriefingLink "#GM_ZONE_0"), "OK:section=GM_ZONE_0"]
triAssertIncludes [(triClickBriefingLink "#Main"), "OK:section=Main"]
triAssertIncludes [(triClickBriefingLink "#GM_MAN_INDEX"), "OK:section=GM_MAN_INDEX"]
triAssertIncludes [(triClickBriefingLink "#GM_MAN_CAPTURE"), "OK:section=GM_MAN_CAPTURE"]

// -- live repaint: a tagged note while the map is open shows up on the diary
//    AND on its zone's page ------------------------------------------------------
gmJournalNote ["JOURNAL-REPAINT-SENTINEL", "Camp", "good"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_LOG"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
// gmJournalEntry now carries the zone and the kind
triAssertEq [((gmJournalEntry (gmJournalCount - 1)) select 2), "Camp"]
triAssertEq [((gmJournalEntry (gmJournalCount - 1)) select 3), 1]
// an objective flip repaints the Plan page too (the row moves to DONE)
gmJournalObjective ["firstRecruit", "", "DONE"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
triAssertIncludes [(triControlText 56), "DONE"]
triAssertIncludes [(triControlText 56), "Take a first recruit"]

triEndTest
