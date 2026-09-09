// ============================================================================
//  Guerrilla resistance dossier on the REAL map screen (guerrilla_native.abel).
//    The native Journal (Game/Guerrilla/Journal) is composed into pages
//    (UI/Guerrilla/JournalCompose*) and laid onto the in-mission map's
//    briefing notepad (UI/Guerrilla/JournalRender) whenever CfgGuerrillaZones
//    is active, in the notepad's stock look (Courier type, Garamond titles,
//    the hand in cwrpen). The unit suite proves Compose and Render against
//    parser-only HTML containers; this test opens the actual DisplayMainMap
//    (triOpenMap -> DisplayMap::Init -> ReloadBriefingContent) and walks the
//    pages through the briefing test hooks, capturing each page:
//      * Contents ("Main", the Notes tab = __BRIEFING): the six sections;
//      * Dispatches (GM_DISPATCH): threat, top objective, latest development;
//      * Operations ("Plan", copied into __PLAN) and its four pages
//        (GM_OBJECTIVES, GM_ACTIONS, GM_SUPPLY, GM_FACTION);
//      * People (GM_PEOPLE) + The roster (GM_ROSTER);
//      * Places (GM_PLACES) + GM_ZONE_<i> per zone;
//      * Chronicles (GM_CHRONICLES) + The record (GM_RECORD);
//      * Reference (GM_REFERENCE) + the GM_MAN_* handbook chapters;
//      * the legacy anchors (GM_CONTENTS, GM_OPERATIONS, GM_ZONES, GM_CELL,
//        GM_LOG, GM_MAN_INDEX) still resolve, as section aliases;
//      * the in-page links route between the pages;
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
// the companions / loot managers publish their status lines at boot (the
// Companions line is no longer rendered on a page; this is the boot net)
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

// -- reading a page. triControlText reads only the CURRENT section, and
//    triBriefingSwitch is a no-op on an unknown name that returns the current
//    section's FIRST name, so every probe starts from a sentinel page that is
//    not itself an alias target (GM_MAN_SAVE): a probe that lands answers the
//    probed page, a probe that misses answers the sentinel.
//    Two page-chaining schemes exist:
//      * gjReadAll walks SplitSection's safety-net sub-pages <name>/1, /2 ...
//        (every sub-page keeps names[0] == <name>, so the probe compares
//        against the base name). Render budgets every page under the paper,
//        so these never exist unless a single block outgrows a page;
//      * gjReadChain walks Render's continuation pages <name>_2, _3 ... (five
//        list entries a page, or a height split): each carries its OWN
//        names[0], so the probe compares against the full continuation name.
//    Resolution-independent: a page holds the same lines at any resolution,
//    only the split point between _n pages may move ---------------------------
gjReadAll = {triBriefingSwitch _this; gjAll = triControlText 56; gjN = 1; gjMore = true; while {gjMore and (gjN < 20)} do {triBriefingSwitch "GM_MAN_SAVE"; gjMore = (triBriefingSwitch format ["%1/%2", _this, gjN]) == _this; if (gjMore) then {gjAll = gjAll + " " + (triControlText 56)}; gjN = gjN + 1}; gjAll}
gjReadChain = {triBriefingSwitch _this; gjAll = triControlText 56; gjN = 2; gjMore = true; while {gjMore and (gjN < 20)} do {triBriefingSwitch "GM_MAN_SAVE"; gjMore = (triBriefingSwitch format ["%1_%2", _this, gjN]) == format ["%1_%2", _this, gjN]; if (gjMore) then {gjAll = gjAll + " " + (triControlText 56)}; gjN = gjN + 1}; gjAll}

// -- Contents (Main / __BRIEFING): the title, the campaign line with the
//    island and the date, the six sections. None of the old Notes roll-ups ----
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triScreenshot "journal_contents"
gjContents = "Main" call gjReadAll
triAssertIncludes [gjContents, "Resistance Dossier"]
triAssertIncludes [gjContents, "Malden"]
triAssertIncludes [gjContents, "Day 1"]
triAssertIncludes [gjContents, "Dispatches"]
triAssertIncludes [gjContents, "Operations"]
triAssertIncludes [gjContents, "People"]
triAssertIncludes [gjContents, "Places"]
triAssertIncludes [gjContents, "Chronicles"]
triAssertIncludes [gjContents, "Reference"]
triAssertExcludes [gjContents, "Treasury"]
triAssertExcludes [gjContents, "SITUATION"]
// Contents is the one page whose footer is the bare pencil word: the parent
// link is suppressed on it, so the footer must never read "Contents - Contents"
triAssertExcludes [gjContents, "Contents - Contents"]
// the Notes tab alias resolves to Contents
triAssertEq [(triBriefingSwitch "__BRIEFING"), "Main"]

// -- Dispatches: three typed facts (Threat / Objective / Latest); the
//    companions roll-up is not here any more ------------------------------------
triAssertEq [(triBriefingSwitch "GM_DISPATCH"), "GM_DISPATCH"]
triScreenshot "journal_dispatch"
gjDispatch = "GM_DISPATCH" call gjReadAll
triAssertIncludes [gjDispatch, "Dispatches"]
triAssertIncludes [gjDispatch, "Threat"]
triAssertIncludes [gjDispatch, "Objective"]
triAssertIncludes [gjDispatch, "Latest"]
triAssertExcludes [gjDispatch, "Petra"]

// -- Operations hub (__PLAN <- "Plan"): four links, one page ----------------
triAssertEq [(triBriefingSwitch "__PLAN"), "__PLAN"]
triScreenshot "journal_operations"
// __PLAN is a copy of the FIRST page of "Plan" (DisplayMap::UpdatePlan copies
// one section); the hub is one page by construction, walk "Plan" all the same
triAssertEq [(triBriefingSwitch "Plan"), "Plan"]
gjPlan = "Plan" call gjReadAll
triAssertIncludes [gjPlan, "Operations"]
triAssertIncludes [gjPlan, "Objectives"]
triAssertIncludes [gjPlan, "Suggested actions"]
triAssertIncludes [gjPlan, "Supplies"]
triAssertIncludes [gjPlan, "Resistance strength"]
triAssertIncludes [gjPlan, "open"]

// -- Objectives: the standing engine objectives + the scripted starters,
//    five to a page (continuations GM_OBJECTIVES_2 ...) ------------------------
triAssertEq [(triBriefingSwitch "GM_OBJECTIVES"), "GM_OBJECTIVES"]
triScreenshot "journal_objectives"
gjObjectives = "GM_OBJECTIVES" call gjReadChain
triAssertIncludes [gjObjectives, "Objectives"]
triAssertIncludes [gjObjectives, "Open"]
triAssertIncludes [gjObjectives, "Hold every base"]
triAssertIncludes [gjObjectives, "Raise every town"]
triAssertIncludes [gjObjectives, "Take a first recruit"]
// pre-flip control for the repaint check further down: every starter objective
// is open at boot, so the "Done" group head must not exist yet
triAssertExcludes [gjObjectives, "Done"]

// -- Suggested actions: the moves ComposeActions emits for this fixture, in the
//    hand. "move" / "moves" is in the subtitle of EVERY version of this page,
//    so it proves nothing; the content pins are the two whole move sentences
//    the day-1 fixture produces (manpower in reserve and no headquarters yet:
//    JournalComposeOps.cpp ComposeActions), and the excludes catches an empty
//    move list, which would read "0 moves" in the subtitle ---------------------
triAssertEq [(triBriefingSwitch "GM_ACTIONS"), "GM_ACTIONS"]
triScreenshot "journal_actions"
gjActions = "GM_ACTIONS" call gjReadChain
triAssertIncludes [gjActions, "Suggested actions"]
triAssertExcludes [gjActions, "0 moves"]
triAssertIncludes [gjActions, "Recruit at the Camp"]
triAssertIncludes [gjActions, "Set up a headquarters"]
// footer: Suggested actions hangs off the Operations hub
triAssertIncludes [gjActions, "Operations"]

// -- Supplies: the economy roll-up moved here from Notes ----------------------
triAssertEq [(triBriefingSwitch "GM_SUPPLY"), "GM_SUPPLY"]
triScreenshot "journal_supply"
gjSupply = "GM_SUPPLY" call gjReadAll
triAssertIncludes [gjSupply, "Supplies"]
triAssertIncludes [gjSupply, "Treasury"]

// -- Resistance strength: the war-level ladder and the ground held ------------
triAssertEq [(triBriefingSwitch "GM_FACTION"), "GM_FACTION"]
triScreenshot "journal_resistance"
gjStrength = "GM_FACTION" call gjReadAll
triAssertIncludes [gjStrength, "War level"]
triAssertIncludes [gjStrength, "Island held"]

// -- People: the index (Change 1: the roster is the only entry) ---------------
triAssertEq [(triBriefingSwitch "GM_PEOPLE"), "GM_PEOPLE"]
triScreenshot "journal_people"
gjPeople = "GM_PEOPLE" call gjReadAll
triAssertIncludes [gjPeople, "People"]
triAssertIncludes [gjPeople, "The roster"]
triAssertIncludes [gjPeople, "under arms"]

// -- The roster: the fighters in typed rows, then the Arms block --------------
triAssertEq [(triBriefingSwitch "GM_ROSTER"), "GM_ROSTER"]
triScreenshot "journal_roster"
gjRoster = "GM_ROSTER" call gjReadChain
triAssertIncludes [gjRoster, "The roster"]
triAssertIncludes [gjRoster, "With me"]
triAssertIncludes [gjRoster, "Standard issue"]
triAssertIncludes [gjRoster, "the faction rifle only"]

// -- Places: the zone index + one page per zone (GM_ZONE_0 = Camp) ------------
triAssertEq [(triBriefingSwitch "GM_PLACES"), "GM_PLACES"]
triScreenshot "journal_places"
gjPlaces = "GM_PLACES" call gjReadChain
triAssertIncludes [gjPlaces, "Places"]
triAssertIncludes [gjPlaces, "scouted"]
triAssertIncludes [gjPlaces, "Camp"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triScreenshot "journal_zone_camp"
gjCamp = "GM_ZONE_0" call gjReadAll
triAssertIncludes [gjCamp, "Camp"]
triAssertIncludes [gjCamp, "Record"]
triAssertIncludes [gjCamp, "Reached the Camp"]
// footer: a zone page's parent is the Places index
triAssertIncludes [gjCamp, "Places"]

// -- Chronicles hub (The record, and since issue #57 the seeded History) -----
//    The hub emits the History row only while in.history.present, which the
//    LegendRegistry sets once it has seeded the campaign. This lane boots a
//    real campaign, so the row is there; a page composed with no history at
//    all is pinned in the unit suite instead.
triAssertEq [(triBriefingSwitch "GM_CHRONICLES"), "GM_CHRONICLES"]
triScreenshot "journal_chronicles"
gjChronicles = "GM_CHRONICLES" call gjReadAll
triAssertIncludes [gjChronicles, "Chronicles"]
triAssertIncludes [gjChronicles, "The record"]
triAssertIncludes [gjChronicles, "History"]
triAssertIncludes [gjChronicles, "How the struggle began"]
triAssertEq [(triBriefingSwitch "GM_RECORD"), "GM_RECORD"]
triScreenshot "journal_record"
gjRecord = "GM_RECORD" call gjReadChain
triAssertIncludes [gjRecord, "The record"]
triAssertIncludes [gjRecord, "Day 1"]
triAssertIncludes [gjRecord, "Reached the Camp"]
// footer: the record's parent is the Chronicles hub
triAssertIncludes [gjRecord, "Chronicles"]

// -- Reference: the ten-chapter index + the chapters ---------------------------
triAssertEq [(triBriefingSwitch "GM_REFERENCE"), "GM_REFERENCE"]
triScreenshot "journal_reference"
gjReference = "GM_REFERENCE" call gjReadAll
triAssertIncludes [gjReference, "Reference"]
triAssertIncludes [gjReference, "The campaign"]
triAssertIncludes [gjReference, "Undercover"]
triAssertEq [(triBriefingSwitch "GM_MAN_MODE"), "GM_MAN_MODE"]
// The Mode chapter is read with gjReadChain for the same reason Undercover is:
// a chapter is authored prose whose only page boundary is Render's height
// budget, and since the footer gained its group-bar reserve (JournalRender.hpp)
// this one runs onto GM_MAN_MODE_2. The chapter still has to read whole
gjManMode = "GM_MAN_MODE" call gjReadChain
triAssertIncludes [gjManMode, "The campaign"]
triAssertIncludes [gjManMode, "DISPATCHES"]
// Undercover is the longest authored chapter: its tables and its two closing
// lists are more content than any other. It is read with gjReadChain, not
// triControlText, because a chapter is prose, not a five-entry list, so its
// only page boundary is Render's height budget: if the chapter needs a
// GM_MAN_UNDERCOVER_2 the chain walk still sees the tail. The tail pin is the
// last authored line of the chapter, so the whole chapter has to be reachable
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triScreenshot "journal_handbook_undercover"
gjManUnder = "GM_MAN_UNDERCOVER" call gjReadChain
triAssertIncludes [gjManUnder, "Undercover"]
triAssertIncludes [gjManUnder, "Cover returns when"]
triAssertIncludes [gjManUnder, "Cover also fails when"]
triAssertIncludes [gjManUnder, "marked for those witnesses"]
// and it does continue: the tail of the chapter lives on GM_MAN_UNDERCOVER_2,
// which is why the capture lanes' one-page acceptance covers the six built
// pages and not the handbook chapters
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER_2"), "GM_MAN_UNDERCOVER_2"]
triScreenshot "journal_handbook_undercover_2"
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]

// -- legacy anchors are section ALIASES (names[1..] of the new page), not
//    links: assert them at the switch level. Each probe bounces off the
//    GM_MAN_SAVE sentinel (not an alias target) and expects the NEW page's
//    first name back --------------------------------------------------------------
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_CONTENTS"), "Main"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_OPERATIONS"), "Plan"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_ZONES"), "GM_PLACES"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_CELL"), "GM_PEOPLE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_LOG"), "GM_RECORD"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_INDEX"), "GM_REFERENCE"]

// -- every href the new pages emit resolves to a real section. This is a
//    DEAD-ANCHOR check over the whole document, not a routing walk: CHTML::
//    ActivateHRef scans EVERY section for the anchor, so landing on
//    GM_ZONE_0 proves the target exists, not that the link was on the page we
//    were reading. The per-page half of the claim is the label pins above -
//    Contents carries "Places" / "People" / "Chronicles" / "Reference"
//    (gjContents), the Places index carries the "Camp" zone entry (gjPlaces),
//    Chronicles carries "The record" (gjChronicles), People carries "The
//    roster" (gjPeople) - so label and anchor are pinned separately -------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triAssertIncludes [(triClickBriefingLink "#GM_PLACES"), "OK:section=GM_PLACES"]
triAssertIncludes [(triClickBriefingLink "#GM_ZONE_0"), "OK:section=GM_ZONE_0"]
triAssertIncludes [(triClickBriefingLink "#Main"), "OK:section=Main"]
triAssertIncludes [(triClickBriefingLink "#GM_REFERENCE"), "OK:section=GM_REFERENCE"]
triAssertIncludes [(triClickBriefingLink "#GM_MAN_CAPTURE"), "OK:section=GM_MAN_CAPTURE"]
triAssertIncludes [(triClickBriefingLink "#GM_PEOPLE"), "OK:section=GM_PEOPLE"]
triAssertIncludes [(triClickBriefingLink "#GM_ROSTER"), "OK:section=GM_ROSTER"]
triAssertIncludes [(triClickBriefingLink "#Plan"), "OK:section=Plan"]
triAssertIncludes [(triClickBriefingLink "#GM_OBJECTIVES"), "OK:section=GM_OBJECTIVES"]
triAssertIncludes [(triClickBriefingLink "#GM_CHRONICLES"), "OK:section=GM_CHRONICLES"]
triAssertIncludes [(triClickBriefingLink "#GM_RECORD"), "OK:section=GM_RECORD"]

// -- live repaint: a tagged note while the map is open shows up on the record
//    (newest first, so on its first page), on its zone's page and as the
//    Dispatches "Latest" row (the newest entry with a kind) ----------------------
gmJournalNote ["JOURNAL-REPAINT-SENTINEL", "Camp", "good"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "GM_RECORD"), "GM_RECORD"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
triAssertEq [(triBriefingSwitch "GM_DISPATCH"), "GM_DISPATCH"]
triAssertIncludes [(triControlText 56), "JOURNAL-REPAINT-SENTINEL"]
// gmJournalEntry now carries the zone and the kind
triAssertEq [((gmJournalEntry (gmJournalCount - 1)) select 2), "Camp"]
triAssertEq [((gmJournalEntry (gmJournalCount - 1)) select 3), 1]
// an objective flip repaints the Objectives chain too (the row moves to Done)
gmJournalObjective ["firstRecruit", "", "DONE"]
triSimFrames 10
triAssertEq [(triBriefingSwitch "GM_OBJECTIVES"), "GM_OBJECTIVES"]
triScreenshot "journal_objectives_done"
gjObjectivesDone = "GM_OBJECTIVES" call gjReadChain
triAssertIncludes [gjObjectivesDone, "Done"]
triAssertIncludes [gjObjectivesDone, "Take a first recruit"]

triEndTest
