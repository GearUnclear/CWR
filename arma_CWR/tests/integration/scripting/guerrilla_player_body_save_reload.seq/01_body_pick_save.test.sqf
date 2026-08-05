// ============================================================================
//  Phase 1 of the BODY-browser save/reload round-trip.
//    The REAL player path: main menu -> GUERRILLA -> Abel -> cycle the BODY
//    browser (idc 155) one step to "WEST SoldierWB" -> OK. The engine
//    substitutes the authored SoldierGB with the pick at InitVehicles
//    (gmSelPlayerClass beats the outfit token; the instance side stays the
//    mission side, so the WEST body fights as GUER). Then write the binary
//    save into the shared UserDir/Saved/Tmp/gbody.fps for phase 2 to diff.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Abel ------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- BODY browser: "(match outfit)" default, one click = the first WEST body -
triAssertEq [(triControlText 155), "BODY: (match outfit)"]
triClick 155
triAssertEq [(triControlText 155), "BODY: WEST SoldierWB"]

// -- launch -------------------------------------------------------------------
triClick 1
triSimUntil { alive player }

// -- the substitution landed (pinned in depth by the e2e; asserted here so
//    the SAVE provably snapshots the cross-side body) -------------------------
triAssertEq [gmSelPlayerClass, "SoldierWB"]
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- write the binary save into the shared UserDir/Saved/Tmp/gbody.fps --------
triAssertEq [(triSaveGame "gbody"), "OK"]

triEndTest
