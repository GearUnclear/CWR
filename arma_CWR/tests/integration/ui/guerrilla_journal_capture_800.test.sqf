// ============================================================================
//  Guerrilla dossier capture lane, 800x600 (guerrilla_native.abel).
//    The content/navigation lane is scripting/guerrilla_journal_pages; this
//    lane pins the LAYOUT at the spec's minimum resolution: the window shape
//    the captures were taken at, the notepad's page budget (triBriefingMetrics,
//    page units), the typography slot binding (triBriefingSlot: H3/H5 garamond,
//    H4 couriernewb, H6 cwrpen, P untouched), a screenshot of every
//    representative page, and the one-page acceptance: Contents, Dispatches,
//    Operations, People, Reference, a zone page and the record must each fit
//    ONE physical page, i.e. neither a SplitSection safety-net sub-page "<name>/0" nor a Render
//    continuation "<name>_2" may exist for them. The acceptance does NOT cover
//    the handbook chapters: a chapter is authored prose, so a height split
//    there is legitimate and is captured rather than failed.
//    Resolution is a process flag (the toml's --width/--height), so the
//    1440x1080 sibling is a separate test file.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmJournalCount >= 1 }
triSimUntil { (gmJournalObjectiveState "firstRecruit") == "ACTIVE" }

// -- the captured shape: at least the requested height and a 4:3 aspect, so a
//    compositor clamp (a desktop smaller than the window) fails the lane
//    instead of quietly shooting a smaller page. There is no log verb, so the
//    live "WxH" is pinned by an assertion whose FAIL payload carries it; the
//    aspect goes through triAssertNear (the 1.99 script core has no `round`) --
triAssertEq [(triGetWindowSize), "800x600"]
triAssertGe [triGetWindowHeight, 600]
triAssertNear [(triGetWindowWidth / triGetWindowHeight), 1.3333, 0.01]

// -- open the real map display (see guerrilla_journal_pages for the seam) ----
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10
triAssertEq [(triBriefingSwitch "Main"), "Main"]

// -- the page budget the pages were laid out for: "x,y,w,h,scale,pageW,pageH"
//    in page units (does not move with pixels); the verb LOG_INFOs itself into
//    the game log, the assertion only proves the control exists -----------------
triAssertExcludes [(triBriefingMetrics), "FAIL"]

// -- typography: every journal slot has a face and a size. Faces as
//    Font::Name() reports the CfgFonts rows (fonts\garamond, fonts\couriernewb,
//    fonts\cwrpen; P is the stock fonts\couriernewb64 and is never rebound) ----
triAssertIncludes [(triBriefingSlot 3), "garamond"]
triAssertIncludes [(triBriefingSlot 5), "garamond"]
triAssertIncludes [(triBriefingSlot 4), "couriernewb"]
triAssertIncludes [(triBriefingSlot 6), "cwrpen"]
triAssertIncludes [(triBriefingSlot 0), "fonts"]
triAssertExcludes [(triBriefingSlot 0), ",0.0000"]
triAssertExcludes [(triBriefingSlot 3), ",0.0000"]
triAssertExcludes [(triBriefingSlot 4), ",0.0000"]
triAssertExcludes [(triBriefingSlot 5), ",0.0000"]
triAssertExcludes [(triBriefingSlot 6), ",0.0000"]

// -- a capture of each representative page ------------------------------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triScreenshot "c800_contents"
triAssertEq [(triBriefingSwitch "GM_DISPATCH"), "GM_DISPATCH"]
triScreenshot "c800_dispatch"
triAssertEq [(triBriefingSwitch "Plan"), "Plan"]
triScreenshot "c800_operations"
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triScreenshot "c800_place"
triAssertEq [(triBriefingSwitch "GM_PEOPLE"), "GM_PEOPLE"]
triScreenshot "c800_people"
triAssertEq [(triBriefingSwitch "GM_RECORD"), "GM_RECORD"]
triScreenshot "c800_record"
triAssertEq [(triBriefingSwitch "GM_REFERENCE"), "GM_REFERENCE"]
triScreenshot "c800_reference"
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triScreenshot "c800_handbook"
// The handbook's Undercover chapter is the longest authored page and the only
// page whose length is not capped by the five-entry list rule: it is prose,
// not a list, so Render's height budget legitimately continues it onto
// GM_MAN_UNDERCOVER_2, which the one-page acceptance below therefore does not
// cover. The continuation is real at both resolutions, so it is pinned and
// captured rather than probed. The switch and the shot must stay SEPARATE
// statements: triScreenshot grabs the frame that is already on screen, so a
// switch in the same statement is captured one page late
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER_2"), "GM_MAN_UNDERCOVER_2"]
triScreenshot "c800_handbook_2"

// -- one physical page each. triBriefingSwitch is a no-op on an unknown name
//    that answers the CURRENT section's first name, so each probe starts from
//    the GM_MAN_SAVE sentinel: a probe that finds no such page answers the
//    sentinel. Two schemes have to be excluded, and the FIRST sub-page of each
//    is what the probe has to name:
//      * SplitSection's safety net numbers its sub-pages from ZERO
//        (UIControlsExt.cpp:2272), and with the i > 0 guard a single oversized
//        row yields exactly ONE sub-page, "<name>/0" - so "<name>/1" would
//        miss the split entirely. Every sub-page keeps names[0] == <name>, so
//        a hit answers the base name, not the probe;
//      * Render's own continuations are "<name>_2", each with its own
//        names[0], so a hit answers the full continuation name ------------------
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "Main/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "Main_2"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_DISPATCH/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_DISPATCH_2"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "Plan/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "Plan_2"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_PEOPLE/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_PEOPLE_2"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_REFERENCE/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_REFERENCE_2"), "GM_MAN_SAVE"]
// GM_ZONE_0 and GM_RECORD are the two pages a growing campaign overflows
// first (a zone page gathers its own record excerpt, the record gathers every
// entry), so they are pinned here too: they fit one page on this fixture and a
// regression that makes them spill will name itself
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_ZONE_0_2"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_RECORD/0"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_RECORD_2"), "GM_MAN_SAVE"]

triEndTest
