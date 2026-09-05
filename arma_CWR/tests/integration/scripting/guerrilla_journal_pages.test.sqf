// ============================================================================
//  Guerrilla field journal on the REAL map screen (guerrilla_native.abel).
//    The native Journal (Game/Guerrilla/Journal) is rendered into the
//    in-mission map's briefing notepad by UI/Guerrilla/GuerrillaJournalPages
//    whenever CfgGuerrillaZones is active. The unit suite proves the renderer
//    against a parser-only HTML container; this test opens the actual
//    DisplayMainMap (triOpenMap -> DisplayMap::Init -> ReloadBriefingContent)
//    and walks the pages through the briefing test hooks:
//      * the Notes page ("Main", aliased __BRIEFING) exists and carries the
//        journal header + Situation block + manual index + diary;
//      * the Plan page (__PLAN, copied from "Plan") carries the standing goal
//        and the scripted starter objectives;
//      * the full diary (GM_LOG) and every manual page (GM_MAN_*) exist and
//        the in-page links route between them (the '#anchor' click path);
//      * a journal write while the map is open repaints the pages
//        (revision compare in DisplayMap::OnSimulate).
//    IDC_BRIEFING = 56 is the notes HTML control (resincl.hpp).
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- the managers' boot writes: opening diary line (campaign.sqs, via
//    gmIslandName) + the four starter objectives --------------------------------
triSimUntil { gmJournalCount >= 1 }
triSimUntil { (gmJournalObjectiveState "firstRecruit") == "ACTIVE" }
triSimUntil { (gmJournalObjectiveState "firstZone") == "ACTIVE" }
triSimUntil { (gmJournalObjectiveState "firstUnlock") == "ACTIVE" }
triAssertIncludes [((gmJournalEntry 0) select 1), "campaign begins"]
triAssertIncludes [((gmJournalEntry 0) select 1), gmIslandName]
triAssertEq [gmIslandName, "Malden"]
// the companions / loot managers publish their status lines at boot
triSimUntil { (gmJournalStatusText "Companions") != "" }
triAssertIncludes [(gmJournalStatusText "Companions"), "Petra"]
triAssertEq [(gmJournalStatusText "Unlocked gear"), "none yet"]

// -- open the real map display the way the player does. DisplayMission::InitUI
//    created the DisplayMainMap at mission start (journal still empty); the
//    map KEY (UAMap, scancode 16 = M) makes World::Simulate call
//    DisplayMap::ResetHUD (the journal rebuild seam) and then SimulateHUD ->
//    OnSimulate each sim frame (the revision-compare repaint). triOpenMap
//    only forces the draw; triSimFrames runs World::Simulate --------------------
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10

// -- Notes page (Main / __BRIEFING). The notepad paginates long sections
//    (SplitSection: Main, Main/0, ...) and triControlText reads only the
//    CURRENT page, so assert what sits on page 1: header, Situation block
//    (incl. the script status lines) and the manual index; the diary lines
//    are asserted on the GM_LOG page below ------------------------------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
gjNotes = triControlText 56
triAssertIncludes [gjNotes, "Field journal"]
triAssertIncludes [gjNotes, "Malden campaign"]
triAssertIncludes [gjNotes, "Situation"]
triAssertIncludes [gjNotes, "War Level"]
triAssertIncludes [gjNotes, "Military zones held"]
triAssertIncludes [gjNotes, "Towns risen"]
triAssertIncludes [gjNotes, "Petra"]
triAssertIncludes [gjNotes, "Unlocked gear"]
triAssertIncludes [gjNotes, "Field manual"]
// the Notes tab alias resolves to the same page
triAssertEq [(triBriefingSwitch "__BRIEFING"), "Main"]

// -- Plan page (__PLAN <- "Plan"): standing goal + scripted objectives ---------
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
gjPlan = triControlText 56
triAssertIncludes [gjPlan, "Liberate Malden"]
triAssertIncludes [gjPlan, "Hold every military zone"]
triAssertIncludes [gjPlan, "Raise every town"]
triAssertIncludes [gjPlan, "Recruit your first fighter"]

// -- full diary + manual pages exist; in-page links route between them --------
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_LOG"]
triAssertIncludes [(triControlText 56), "campaign begins"]
triAssertEq [(triBriefingSwitch "GM_MAN_MODE"), "GM_MAN_MODE"]
triAssertIncludes [(triControlText 56), "What this is"]
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triAssertIncludes [(triControlText 56), "Undercover"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
// the manual index on Notes links to the capture topic, and the topic links back
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triAssertIncludes [(triClickBriefingLink "#GM_MAN_CAPTURE"), "OK:section=GM_MAN_CAPTURE"]
triAssertIncludes [(triClickBriefingLink "#Main"), "OK:section=Main"]

// -- live repaint: a diary write while the map is open shows up ---------------
gmJournalLog "JOURNAL-REPAINT-SENTINEL"
triSimFrames 10
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_LOG"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
// an objective flip repaints the Plan page too
gmJournalObjective ["firstRecruit", "", "DONE"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
triAssertIncludes [(triControlText 56), "Recruit your first fighter"]

triEndTest
