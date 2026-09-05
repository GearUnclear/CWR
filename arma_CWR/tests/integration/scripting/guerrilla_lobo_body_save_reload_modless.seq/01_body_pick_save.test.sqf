// ============================================================================
//  Phase 1 of the MOD-BODY save/reload WITHOUT THE MOD (issue #48, #56 task 5).
//    Byte-for-byte the sibling guerrilla_lobo_body_save_reload.seq phase 1:
//    the save this writes is what phase 2 loads with @LoBo unmounted.
//    The real player path: main menu -> GUERRILLA -> Sinai -> character
//    screen -> LoBoGolaniWB (an @LoBo IDF WEST body) -> CONFIRM -> switch the
//    island to Abel, a VANILLA template whose mission.sqm authors no addOns[]
//    at all -> OK. ApplyPlayerOutfitSelection substitutes the body and
//    additively activates its owner closure (issue #45).
//
//    Before saving, hold the world past the point where anything downstream
//    could still rewrite the unit (issue #48 gap 3): the shared core to
//    completion, then several hundred further frames. Then write the binary
//    save into the shared UserDir/Saved/Tmp/globo.fps for phase 2 to diff.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Sinai -----------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "sinai"]), true]

// -- pick the @LoBo Golani body and CONFIRM ------------------------------------
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triSelectListByData [160, "lobogolaniw"]), true]
triClick 1
triAssertEq [(triDisplay), 76]
triAssertEq [(triControlText 155), "CHARACTER: Sayeret Golani Operator"]

// -- switch to Abel: the roster is package-wide so the pick is KEPT, and Abel
//    is the template that makes the grant load-bearing (no addOns[] at all) ---
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(triControlText 155), "CHARACTER: Sayeret Golani Operator"]

triClick 1
triSimUntil { alive player }

// -- the substitution and the grant both landed -------------------------------
triAssertEq [(format ["%1 || %2 || %3", (typeOf player), (side player), gmSelPlayerClass]), "LoBoGolaniWB || GUER || LoBoGolaniWB"]
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "true || true || true"]

// -- LONGER HORIZON before the snapshot: let the whole mission init chain run
//    to completion and then some, so the save captures a settled world rather
//    than the first few frames after spawn ------------------------------------
triSimUntil { GM_LIB_READY }
triSimUntil { gmZoneCount > 0 }
glF0 = triFrameCount
triSimUntil { triFrameCount > glF0 + 600 }
triAssertEq [(format ["%1 || %2", (typeOf player), (side player)]), "LoBoGolaniWB || GUER"]
triAssertEq [(primaryWeapon player), "LoBo_M4A1_LD_G_Falcon"]

// -- write the binary save into the shared UserDir/Saved/Tmp/globo.fps ---------
triAssertEq [(triSaveGame "globo"), "OK"]

triEndTest
