// ============================================================================
//  Guerrilla Mode - the OUTFIT cycler must not advertise a scope it lost
//  (issue #47).
//
//  THE POINT. A CHARACTER pick beats the outfit token for the PLAYER's body,
//  unconditionally: ResolvePlayerBodyClass (Game/Guerrilla/OutfitSelect.cpp)
//  returns on the pick and never falls through to the token path. That
//  precedence is deliberate and is NOT what this test guards. What was wrong
//  is that idc 153 kept reading "OUTFIT: CIVILIAN" after the pick, so the
//  screen promised a civilian player body it would not deliver - the player
//  spawned as the picked WEST rifleman while the squad went plainclothes.
//
//  The fix narrows the LABEL, not the precedence: while a pick is live the
//  row reads "OUTFIT (squad only): <token>". The cycler stays enabled and
//  keeps cycling, because the token still genuinely governs recruits,
//  companions and the captured-town hold garrison (GM_OUTFIT_CIV in
//  scripts/recruit.sqs, companions.sqs, capture.sqs) - greying it would be
//  the opposite lie and would strand the player with no way to dress their
//  fighters.
//
//  Assertions that go RED against the pre-fix engine are marked #47 below;
//  every one of them read "OUTFIT: ..." before the fix.
//
//  Flow: main menu -> GUERRILLA (120) -> Abel -> cycle OUTFIT (153) to
//  CIVILIAN -> CHARACTER (155) -> IDD 77 -> SoldierWB -> CONFIRM -> read 153
//  -> cycle 153 -> island round trip -> clear the pick back to "(match
//  outfit)" -> read 153 -> re-pick -> OK -> in-mission.
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

// == 1. UNTOUCHED SCREEN: no pick, so the token really does govern the
//       player body and the row must carry NO qualifier ======================
triAssertEq [(format ["%1 || %2", (triControlText 155), (triControlText 153)]), "CHARACTER: (match outfit) || OUTFIT: WARRIOR"]

// == 2. OUTFIT alone, still no pick: the plain label stays truthful =========
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
triScreenshot "outfit_civilian_no_pick"

// == 3. THE REPRO: pick a CHARACTER on top of OUTFIT: CIVILIAN =============
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triSelectListByData [160, "soldierwb"]), true]
triClick 1
triAssertEq [(triDisplay), 76]

// #47: the pick took the player body, so the row narrows to the squad. The
// pre-fix engine printed "OUTFIT: CIVILIAN" here - a body the launch would
// not deliver.
triAssertEq [(format ["%1 || %2", (triControlText 155), (triControlText 153)]), "CHARACTER: Soldier || OUTFIT (squad only): CIVILIAN"]
triScreenshot "outfit_squad_only_after_pick"

// == 4. THE ROW STAYS LIVE. The token is not dead - it still dresses the
//       squad - so the control must remain enabled and must keep cycling.
//       This is what rules out greying idc 153. ==============================
triAssertEq [(triGetControlEnabled 153), "1"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT (squad only): WARRIOR"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT (squad only): CIVILIAN"]

// == 5. THE QUALIFIER SURVIVES AN ISLAND SWITCH. Island changes re-seed the
//       island-scoped selections (RefreshOutfitChoices) and revalidate the
//       pick (RevalidateBodySelection, issue #45); both must re-render 153
//       with the qualifier still on. The token VALUE is deliberately not
//       pinned here - the neighbouring stock islands ship no Guerrilla
//       template, so their roster is "(mission default)", which is exactly
//       the second case the qualifier has to cover. ==========================
GOL_abelRow = triLBCurSel 101
GOL_otherRow = if (GOL_abelRow > 0) then {0} else {1}
triAssertEq [(triLBSetCurSel [101, GOL_otherRow]), true]
triAssertNe [(triLBCurSel 101), GOL_abelRow]
triAssertEq [(triControlText 155), "CHARACTER: Soldier"]
triAssertIncludes [(triControlText 153), "OUTFIT (squad only):"]
triScreenshot "outfit_squad_only_other_island"

// back to Abel: the pick is package-wide and survives (issue #45), so the
// qualifier must still be there
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triControlText 155), "CHARACTER: Soldier"]
triAssertIncludes [(triControlText 153), "OUTFIT (squad only):"]

// == 6. RECOVERY. Clearing the pick back to "(match outfit)" hands the player
//       body BACK to the token, so the qualifier must disappear. Row 0 of the
//       character list is the "(match outfit)" default (value -1) and carries
//       no row data, so it is selected by index, not by triSelectListByData. =
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triLBText [160, 0]), "(match outfit)"]
triAssertEq [(triLBSetCurSel [160, 0]), true]
triClick 1
triAssertEq [(triDisplay), 76]
// #47: the qualifier is gone and the token governs the player again. A fix
// that only ever ADDED the qualifier would go red on this line.
triAssertEq [(format ["%1 || %2", (triControlText 155), (triControlText 153)]), "CHARACTER: (match outfit) || OUTFIT: WARRIOR"]
triScreenshot "outfit_plain_after_clear"

// == 7. Re-pick and launch, so the label's claim is checked against what the
//       engine actually does with both channels. =============================
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triSelectListByData [160, "soldierwb"]), true]
triClick 1
triAssertEq [(triDisplay), 76]
triAssertEq [(triControlText 153), "OUTFIT (squad only): CIVILIAN"]

triClick 1
triSimUntil { alive player }

// -- both channels were published, unchanged by this fix --------------------
triAssertEq [(format ["%1 || %2", gmSelPlayerClass, gmSelOutfit]), "SoldierWB || CIVILIAN"]

// -- "squad only" is literally true: the PLAYER wears the pick (the token was
//    dropped for the body, which is the deliberate precedence) ...
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- ... and the SQUAD still obeys the token, which is why the row stays
//    readable and clickable instead of greyed out ---------------------------
triSimUntil { GM_LIB_READY }
triSimUntil { format ["%1", GM_OUTFIT_CIV] == "true" }
triSimUntil { format ["%1", GM_RECRUIT_FIGHTER] == "SoldierGFakeC" }
triSimUntil { format ["%1", GM_COMP_CLASS] == "SoldierGFakeC" }

triEndTest
