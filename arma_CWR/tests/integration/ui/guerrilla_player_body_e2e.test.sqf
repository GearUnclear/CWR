// ============================================================================
//  Guerrilla Mode - BODY browser (player-body pick from ANY side), end to end.
//
//  The class-driven follow-up to issue #25's outfit axis: the new-game screen
//  grew a BODY cycler (idc 155) enumerating every creatable Man-derived
//  CfgVehicles class of the loaded package, grouped WEST/EAST/GUER/CIV
//  (GuerrillaListPlayerBodies - deduped by displayName+model, capped per
//  side, config scan order). Picking an entry publishes gmSelPlayerClass and
//  the engine substitutes the player's authored mission.sqm class with the
//  pick at InitVehicles (OutfitSelect::ResolvePlayerBodyClass - the pick
//  beats the outfit token), while the INSTANCE side stays the mission side:
//  a WEST-config body fights as GUER.
//
//  Coexistence contract pinned here: the OUTFIT cycler (153) keeps governing
//  the SQUAD family (recruits/companions/hold), the BODY browser only the
//  player's own body, and its default "(match outfit)" publishes nothing -
//  the untouched-screen invariant is asserted from the other side in
//  guerrilla_outfit_civilian_e2e (gmSelPlayerClass stays nil there).
//
//  PRECONDITION (same as guerrilla_new_game_e2e): the templates must be
//  installed in the game data dir - run guerrilla-mode/install-missions.ps1.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display ------------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]

// -- island: Abel (full CWA 1.99 stock island; template installed) ----------
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- the untouched screen: browser on its no-op default, mannequin showing
//    the outfit-resolved warrior body --------------------------------------
triAssertEq [(triControlText 155), "BODY: (match outfit)"]
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
triAssertEq [(triGetControlVisible 154), "1"]

// -- one click into the roster: WEST comes first (fixed side order), and on
//    Classic data the first WEST body in config scan order is SoldierWB -----
triClick 155
triAssertEq [(triControlText 155), "BODY: WEST SoldierWB"]
// the mannequin tracks the browser pick immediately (SoldierWB's model
// resolves on Classic, so the preview must stay visible)
triAssertEq [(triGetControlVisible 154), "1"]

// -- launch ------------------------------------------------------------------
triClick 1
triSimUntil { alive player }

// -- the pick was published; the outfit channel stayed on its no-op ----------
triAssertEq [gmSelPlayerClass, "SoldierWB"]
triAssertEq [gmSelOutfit, "WARRIOR"]

// -- the player wears the WEST body, on the resistance side: config side is
//    for the eyes only (distant side-resolve ladder), instance side is the
//    mission side field ------------------------------------------------------
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- the squad family stayed on the outfit axis: warrior default, recruits
//    keep the descriptor's warrior classes (no per-class squad matching) -----
triSimUntil { GM_LIB_READY }
triAssertEq [(format ["%1", GM_OUTFIT_CIV]), "false"]

triEndTest
