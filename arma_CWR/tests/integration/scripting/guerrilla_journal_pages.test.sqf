// ============================================================================
//  Guerrilla field journal on the REAL map screen (guerrilla_native.abel).
//    The native Journal (Game/Guerrilla/Journal) is rendered into the
//    in-mission map's briefing notepad by UI/Guerrilla/GuerrillaJournalPages
//    whenever CfgGuerrillaZones is active, in the notepad's stock look (its
//    own Courier / Garamond / handwriting slots on the paper). The unit suite
//    proves the renderer against a parser-only HTML container; this test
//    opens the actual DisplayMainMap (triOpenMap -> DisplayMap::Init ->
//    ReloadBriefingContent) and walks the pages through the briefing test
//    hooks, capturing each page:
//      * the Notes page ("Main", aliased __BRIEFING) is the day's page:
//        dated title, the campaign line, the threat, the cell (incl. the
//        companions status line), the ground held;
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

// -- the notepad paginates a long section (SplitSection: every sub-page is
//    named <name> AND <name>/<n>, with prev/next arrows) and triControlText
//    reads only the CURRENT page, so a page's text is read by walking every
//    sub-page: the section itself first, then <name>/1, <name>/2 ... Each
//    probe starts from a sentinel page because triBriefingSwitch is a no-op
//    on an unknown name and returns the current section's FIRST name, which
//    is <name> on every sub-page: a probe that lands answers <name>, a probe
//    that misses answers the sentinel. Resolution-independent: a page holds
//    the same lines at any resolution -----------------------------------------
gjReadAll = {triBriefingSwitch _this; gjAll = triControlText 56; gjN = 1; gjMore = true; while {gjMore and (gjN < 20)} do {triBriefingSwitch "GM_MAN_INDEX"; gjMore = (triBriefingSwitch format ["%1/%2", _this, gjN]) == _this; if (gjMore) then {gjAll = gjAll + " " + (triControlText 56)}; gjN = gjN + 1}; gjAll}

// -- Notes page (Main / __BRIEFING) = the day's page: the dated title, the
//    campaign line, the cell paragraph (incl. the companions status line),
//    the ground held, the latest entries ------------------------------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triScreenshot "journal_notes"
gjNotes = "Main" call gjReadAll
triAssertIncludes [gjNotes, "Day 1"]
triAssertIncludes [gjNotes, "Malden"]
triAssertIncludes [gjNotes, "Treasury"]
triAssertIncludes [gjNotes, "Petra"]
triAssertIncludes [gjNotes, "We hold"]
triAssertIncludes [gjNotes, "War level"]
triAssertIncludes [gjNotes, "Latest"]
triAssertExcludes [gjNotes, "SITUATION"]
// the Notes tab alias resolves to the same page
triAssertEq [(triBriefingSwitch "__BRIEFING"), "Main"]

// -- Plan page (__PLAN <- "Plan"): standing objectives + scripted starters ----
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
triScreenshot "journal_plan_objectives"
// __PLAN is a copy of the FIRST page of "Plan" (DisplayMap::UpdatePlan copies
// one section), so the continuation pages keep the source name: walk "Plan"
gjPlan = "Plan" call gjReadAll
triAssertIncludes [gjPlan, "Objectives"]
triAssertIncludes [gjPlan, "Hold every base"]
triAssertIncludes [gjPlan, "Raise every town"]
triAssertIncludes [gjPlan, "Take a first recruit"]
triAssertIncludes [gjPlan, "Next"]

// -- the ledgers: Zones index + one page per zone, Cell, Resistance, Diary ---
triAssertEq [(triBriefingSwitch "GM_ZONES"), "GM_ZONES"]
triScreenshot "journal_zones"
triAssertIncludes [(triControlText 56), "Zones"]
triAssertIncludes [(triControlText 56), "Camp"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triScreenshot "journal_zone_camp"
triAssertIncludes [(triControlText 56), "Record"]
triAssertEq [(triBriefingSwitch "GM_CELL"), "GM_CELL"]
triScreenshot "journal_cell"
triAssertIncludes [(triControlText 56), "Roster"]
triAssertIncludes [(triControlText 56), "Standard issue"]
triAssertEq [(triBriefingSwitch "GM_FACTION"), "GM_FACTION"]
triScreenshot "journal_resistance"
triAssertIncludes [(triControlText 56), "Island held"]
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_LOG"]
triScreenshot "journal_diary"
triAssertIncludes [(triControlText 56), "Reached the Camp"]

// -- handbook: index + chapters exist; in-page links route between pages ----
triAssertEq [(triBriefingSwitch "GM_MAN_INDEX"), "GM_MAN_INDEX"]
triScreenshot "journal_handbook"
triAssertIncludes [(triControlText 56), "Handbook"]
triAssertEq [(triBriefingSwitch "GM_MAN_MODE"), "GM_MAN_MODE"]
triAssertIncludes [(triControlText 56), "The campaign"]
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triScreenshot "journal_handbook_undercover"
triAssertIncludes [(triControlText 56), "Undercover"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
// the footer on Notes links to the Zones index, the index links to a zone
// page, and every page links back to Notes
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
// an objective flip repaints the Plan page too (the row moves to Done)
gmJournalObjective ["firstRecruit", "", "DONE"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
triScreenshot "journal_plan_done"
gjPlanDone = "Plan" call gjReadAll
triAssertIncludes [gjPlanDone, "Done"]
triAssertIncludes [gjPlanDone, "Take a first recruit"]

triEndTest
