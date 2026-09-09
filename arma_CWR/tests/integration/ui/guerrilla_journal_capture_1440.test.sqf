// ============================================================================
//  Guerrilla dossier capture lane, 1440x1080 (guerrilla_native.abel).
//    The second, higher-resolution 4:3 capture (it fits a 1920x1080 desktop,
//    where 1600x1200 would be clamped by the compositor). Same checks as the
//    800x600 lane: the captured window shape, the notepad's page budget
//    (triBriefingMetrics, page units), the typography slot binding
//    (triBriefingSlot: H3/H5 garamond, H4 couriernewb, H6 cwrpen, P
//    untouched), a screenshot of every representative page, and the one-page
//    acceptance for Contents, Dispatches, Operations, People, Reference, a
//    zone page and the record
//    (neither "<name>/0" nor "<name>_2" exists; the handbook chapters are
//    authored prose and are exempt, captured rather than failed). Wrap points
//    may differ between the two resolutions (FreeType pixel-size bucketing),
//    which is why the acceptance is held at both.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmJournalCount >= 1 }
triSimUntil { (gmJournalObjectiveState "firstRecruit") == "ACTIVE" }

// -- the captured shape: at least the requested height and a 4:3 aspect, so a
//    compositor clamp (a desktop smaller than the window) fails the lane
//    instead of quietly shooting a smaller page. There is no log verb, so the
//    live "WxH" is pinned by an assertion whose FAIL payload carries it; the
//    aspect goes through triAssertNear (the 1.99 script core has no `round`) --
triAssertEq [(triGetWindowSize), "1440x1080"]
triAssertGe [triGetWindowHeight, 1080]
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

// -- the map screen's group bar (RscInGameUI >> GroupInfo) keeps drawing over
//    the open map, across the bottom tenth of the screen, and the notepad's
//    page bottom sits inside that band: the journal's footer clears it only
//    because Render reserves rows under it (JournalRender.hpp, "The map
//    screen's group bar"). The bar appears once the player leads a squad, so
//    every capture below waits for one: with a lone player the PNGs would
//    prove nothing about the overlap
triSimUntil { (count (units (group player))) >= 2 }

// -- a capture of each representative page ------------------------------------
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triScreenshot "c1440_contents"
triAssertEq [(triBriefingSwitch "GM_DISPATCH"), "GM_DISPATCH"]
triScreenshot "c1440_dispatch"
triAssertEq [(triBriefingSwitch "Plan"), "Plan"]
triScreenshot "c1440_operations"
triAssertEq [(triBriefingSwitch "GM_ZONE_0"), "GM_ZONE_0"]
triScreenshot "c1440_place"
triAssertEq [(triBriefingSwitch "GM_PEOPLE"), "GM_PEOPLE"]
triScreenshot "c1440_people"
triAssertEq [(triBriefingSwitch "GM_RECORD"), "GM_RECORD"]
triScreenshot "c1440_record"
triAssertEq [(triBriefingSwitch "GM_REFERENCE"), "GM_REFERENCE"]
triScreenshot "c1440_reference"
// A character dossier: the one page whose acceptance depends on an ASSET.  The
// portrait box is reserved at an exact size whether or not a photograph exists,
// so a dossier that fits with the pencil "Photograph unavailable" line in it is
// still not the page that ships.  The anchor is built from the registry rather
// than pinned, because the row id is seeded per campaign.
triSimUntil { (count GM_COMP_OBJ) >= 1 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
c1440Who = format ["GM_WHO_%1", (gmLegendInfo 0) select 0]
triAssertEq [(triBriefingSwitch c1440Who), c1440Who]
triScreenshot "c1440_dossier"
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER"), "GM_MAN_UNDERCOVER"]
triScreenshot "c1440_handbook"
// The handbook's Undercover chapter is the longest authored page and the only
// page whose length is not capped by the five-entry list rule: it is prose,
// not a list, so Render's height budget legitimately continues it onto
// GM_MAN_UNDERCOVER_2, which the one-page acceptance below therefore does not
// cover. The continuation is real at both resolutions, so it is pinned and
// captured rather than probed. The switch and the shot must stay SEPARATE
// statements: triScreenshot grabs the frame that is already on screen, so a
// switch in the same statement is captured one page late
triAssertEq [(triBriefingSwitch "GM_MAN_UNDERCOVER_2"), "GM_MAN_UNDERCOVER_2"]
triScreenshot "c1440_handbook_2"

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
// and the dossier: the page carrying the photograph, so the one a portrait
// sized wrong would break first
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch (c1440Who + "/0")), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch (c1440Who + "_2")), "GM_MAN_SAVE"]

triEndTest
