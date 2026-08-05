// ============================================================================
//  Phase 1 of the CIVILIAN-outfit save/reload round-trip (issue #25, M2.3).
//    The REAL player path: main menu -> GUERRILLA -> Abel -> cycle the
//    outfit cycler (idc 153) to CIVILIAN -> OK. The engine substitutes the
//    authored SoldierGB for the descriptor's playerClassCiv=SoldierGFakeC at
//    InitVehicles; the managers fold gmSelOutfit into their class globals.
//    Once the plainclothes companion body is up too, write the binary save
//    into the shared UserDir/Saved/Tmp/gout.fps for phase 2 to diff.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Abel ------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- outfit cycler: WARRIOR default, one click = CIVILIAN ---------------------
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]

// -- launch -------------------------------------------------------------------
triClick 1
triSimUntil { alive player }

// -- the substitution + fold landed (pinned in depth by the e2e; asserted
//    here so the SAVE provably snapshots the civilian-outfit state) -----------
triAssertEq [gmSelOutfit, "CIVILIAN"]
triAssertEq [(typeOf player), "SoldierGFakeC"]
triSimUntil { GM_LIB_READY }
triSimUntil { format ["%1", GM_OUTFIT_CIV] == "true" }
triSimUntil { format ["%1", GM_RECRUIT_FIGHTER] == "SoldierGFakeC" }

// -- the companion body must be up (and plainclothes) before the snapshot:
//    phase 2 diffs her class after the reload ---------------------------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(typeOf (GM_COMP_OBJ select 0)), "SoldierGFakeC"]

// -- write the binary save into the shared UserDir/Saved/Tmp/gout.fps ---------
triAssertEq [(triSaveGame "gout"), "OK"]

triEndTest
