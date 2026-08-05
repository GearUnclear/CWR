// ============================================================================
//  Phase 2 of the BODY-browser save/reload round-trip.
//    Fresh menu launch of the SAME template with the browser UNTOUCHED: the
//    clean-slate baseline is the authored body (SoldierGB) and a nil
//    gmSelPlayerClass - proving the diff below cannot false-pass on leftover
//    state in the shared user dir. Then triLoadGame restores phase 1's save:
//    the substituted player body persists as the class it was CREATED as
//    (the world serializes the unit, no re-substitution runs on the load
//    path) and the GGameState bank brings gmSelPlayerClass back.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- same menu path, browser untouched ("(match outfit)" default) -------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triControlText 155), "BODY: (match outfit)"]
triClick 1
triSimUntil { alive player }

// -- baseline: fresh-boot defaults, NOT the sentinels -------------------------
triAssertEq [(format ["%1", isNil "gmSelPlayerClass"]), "true"]
triAssertEq [(typeOf player), "SoldierGB"]

// -- restore phase 1's binary save from the shared UserDir/Saved/Tmp/gbody.fps
triAssertEq [(triLoadGame "gbody"), "OK"]
triSimFrames 3

// -- DIFF: the cross-side player body persisted across the reload -------------
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- DIFF: the pick itself rides the GGameState bank in the save --------------
triAssertEq [gmSelPlayerClass, "SoldierWB"]

triEndTest
