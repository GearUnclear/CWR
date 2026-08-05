// ============================================================================
//  Phase 2 of the CIVILIAN-outfit save/reload round-trip (issue #25, M2.3).
//    Fresh menu launch of the SAME template with the outfit cycler UNTOUCHED:
//    the clean-slate baseline is the authored warrior body (SoldierGB) and a
//    WARRIOR publish - proving the diff below cannot false-pass on leftover
//    state in the shared user dir. Then triLoadGame restores phase 1's
//    civilian save: the substituted player body persists as the class it was
//    CREATED as (the world serializes the unit, no re-substitution runs on
//    the load path), the GGameState bank brings gmSelOutfit back, and the
//    resumed scripts still hold the folded civilian class globals.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- same menu path, cycler untouched (WARRIOR default) -----------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
triClick 1
triSimUntil { alive player }

// -- baseline: fresh-boot defaults, NOT the sentinels -------------------------
triAssertEq [gmSelOutfit, "WARRIOR"]
triAssertEq [(typeOf player), "SoldierGB"]
triSimUntil { GM_LIB_READY }
triAssertEq [(format ["%1", GM_OUTFIT_CIV]), "false"]

// -- restore phase 1's binary save from the shared UserDir/Saved/Tmp/gout.fps -
triAssertEq [(triLoadGame "gout"), "OK"]
triSimFrames 3

// -- DIFF: the substituted player body persisted across the reload ------------
triAssertEq [(typeOf player), "SoldierGFakeC"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- DIFF: the choice itself rides the GGameState bank in the save ------------
triAssertEq [gmSelOutfit, "CIVILIAN"]
triAssertEq [(format ["%1", GM_OUTFIT_CIV]), "true"]
triAssertEq [GM_RECRUIT_FIGHTER, "SoldierGFakeC"]

// -- DIFF: the restored companion wears the plainclothes body too -------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(typeOf (GM_COMP_OBJ select 0)), "SoldierGFakeC"]

triEndTest
