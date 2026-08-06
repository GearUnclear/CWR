// ============================================================================
//  Guerrilla Mode - CHARACTER select (player-body pick from ANY side), e2e.
//
//  The class-driven follow-up to issue #25's outfit axis, now a real screen:
//  the new-game display's idc 155 is no longer a flat cycler. Its default
//  label is "CHARACTER: (match outfit)" and clicking it opens the
//  character-select child display (IDD 77, GuerrillaCharacterSelect): a 2D
//  listbox (idc 160) with row 0 "(match outfit)" (value -1), then per side
//  present (WEST, EAST, GUER, CIV order) a grey header row (value -2, never
//  selectable) followed by that side's body rows (value = roster index,
//  data = the CfgVehicles classname, text = displayName-based label). The
//  roster is UNCAPPED (the old 24-per-side cap is gone), deduped by
//  displayName+model per side, config scan order. Picking a body row and
//  CONFIRM (idc 1) returns the pick to the parent; OK then publishes
//  gmSelPlayerClass and the engine substitutes the player's authored
//  mission.sqm class with the pick at InitVehicles
//  (OutfitSelect::ResolvePlayerBodyClass - the pick beats the outfit token),
//  while the INSTANCE side stays the mission side: a WEST-config body
//  fights as GUER.
//
//  Coexistence contract pinned here: the OUTFIT cycler (153) keeps governing
//  the SQUAD family (recruits/companions/hold), the character screen only
//  the player's own body, and its default "(match outfit)" publishes
//  nothing - the untouched-screen invariant is asserted from the other side
//  in guerrilla_outfit_civilian_e2e (gmSelPlayerClass stays nil there).
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

// -- the untouched screen: character button on its no-op default, mannequin
//    showing the outfit-resolved warrior body --------------------------------
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
triAssertEq [(triGetControlVisible 154), "1"]

// -- clicking 155 opens the character-select child display (IDD 77) ---------
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triControlText 163), "SELECT CHARACTER"]

// -- listbox contract: row 0 is the "(match outfit)" default (value -1),
//    row 1 the WEST header (value -2, fixed side order). On Classic Abel the
//    full uncapped roster is 96 rows (1 default + 4 side headers + 91 deduped
//    bodies; deterministic config scan). Note: 96 sits BELOW the old
//    24-per-side cap's 101-row ceiling, so this count pins determinism only;
//    the uncapping proof lives in ui/guerrilla_character_lobo_e2e, whose
//    modded roster is 1541 rows -----------------------------------------------
triAssertEq [(triLBText [160, 0]), "(match outfit)"]
triAssertEq [(triLBValue [160, 0]), -1]
triAssertEq [(triLBText [160, 1]), "--- WEST ---"]
triAssertEq [(triLBValue [160, 1]), -2]
triAssertEq [(triLBSize 160), 96]

// -- select SoldierWB by row DATA (classname, case-insensitive substring):
//    it lands on row 2 (the first WEST body, roster index 0), the info line
//    names the class exactly, and the mannequin (161) shows the body ---------
triAssertEq [(triSelectListByData [160, "soldierwb"]), true]
triAssertEq [(triLBCurSel 160), 2]
triAssertEq [(triLBValue [160, 2]), 0]
triAssertEq [(triLBText [160, 2]), "Soldier"]
triAssertEq [(triControlText 162), "SIDE: WEST  CLASS: SoldierWB  SOURCE: Base game"]
triAssertEq [(triGetControlVisible 161), "1"]

// -- CONFIRM returns the pick to the parent: 155 carries the row label and
//    the parent preview (154) tracks the pick ---------------------------------
triClick 1
triAssertEq [(triDisplay), 76]
triAssertEq [(triControlText 155), "CHARACTER: Soldier"]
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
