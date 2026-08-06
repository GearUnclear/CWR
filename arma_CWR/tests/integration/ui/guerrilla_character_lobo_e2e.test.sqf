// ============================================================================
//  Guerrilla Mode - character-select screen on the LOBO lane, end to end.
//
//  THE POINT: under the old 24-per-side BODY-cycler cap, WEST's roster slots
//  filled up with base-game classes before the config scan ever reached the
//  @LoBo mod's IDF bodies, so no Golani class was reachable from the UI at
//  all. The character-select screen (IDD 77) is UNCAPPED: this test proves a
//  previously starved @LoBo IDF WEST body is now reachable by row DATA and
//  actually launches as the player.
//
//  Flow: main menu -> GUERRILLA (idc 120) -> Sinai (template installed) ->
//  open the character screen (155) -> select by classname substring
//  "lobogolaniw" (row data = CfgVehicles classname; matching is
//  case-insensitive) -> pin the class it resolves to (LoBoGolaniWB, the
//  Sayeret Golani rifleman, first LoBoGolaniW* in config scan order) ->
//  side trip to a CIV mod body -> CONFIRM -> OK -> the launched player IS
//  the Golani body.
//
//  Roster size: 1541 rows on Sinai with @LoBo mounted (1 default + 4 side
//  headers + 1536 deduped bodies; deterministic config scan). The old
//  all-sides ceiling was 101 rows at most (1 + 4 + 4x24), so the count alone
//  proves the cap is gone.
//
//  PRECONDITIONS (same as guerrilla_new_game_e2e, see the .toml): templates
//  installed via install-missions.ps1 -IncludeWorld Sinai,Lebanon80; the
//  @lobofixup patched pbos generated once.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Sinai -----------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "sinai"]), true]

// -- factions stay on the template defaults (IDF occupier, EgyptFrontier
//    resistance, pinned by guerrilla_new_game_e2e); the character button on
//    its no-op default --------------------------------------------------------
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]

// -- open the character screen ------------------------------------------------
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triControlText 163), "SELECT CHARACTER"]

// -- listbox contract + the uncapped roster: 1541 rows comfortably exceeds
//    the old 101-row all-sides ceiling ----------------------------------------
triAssertEq [(triLBText [160, 0]), "(match outfit)"]
triAssertEq [(triLBValue [160, 0]), -1]
triAssertEq [(triLBText [160, 1]), "--- WEST ---"]
triAssertEq [(triLBValue [160, 1]), -2]
triAssertEq [(triLBSize 160), 1541]

// -- THE POINT: an @LoBo IDF WEST body is reachable. Selection by row DATA
//    (classname) resolves to LoBoGolaniWB, deep in the WEST block at row 287
//    (roster index 285), far beyond the old 24-per-side horizon ---------------
triAssertEq [(triSelectListByData [160, "lobogolaniw"]), true]
triAssertEq [(triLBCurSel 160), 287]
triAssertEq [(triLBValue [160, 287]), 285]
triAssertEq [(triLBText [160, 287]), "Sayeret Golani Operator"]
triAssertEq [(triControlText 162), "SIDE: WEST  CLASS: LoBoGolaniWB  SOURCE: lobois"]
triAssertEq [(triGetControlVisible 161), "1"]

// -- side trip: a CIV mod body selects too. Row DATA keeps the REAL
//    underscores (LoBo_Civ_01), only labels/info sanitize them to hyphens ------
triAssertEq [(triSelectListByData [160, "lobo_civ"]), true]
triAssertEq [(triLBText [160, (triLBCurSel 160)]), "Arab Civilian 1"]
triAssertEq [(triControlText 162), "SIDE: CIV  CLASS: LoBo-Civ-01  SOURCE: loboterror"]
triAssertEq [(triGetControlVisible 161), "1"]

// -- back to the Golani pick and CONFIRM back to the parent -------------------
triAssertEq [(triSelectListByData [160, "lobogolaniw"]), true]
triClick 1
triAssertEq [(triDisplay), 76]
triAssertEq [(triControlText 155), "CHARACTER: Sayeret Golani Operator"]
triAssertEq [(triGetControlVisible 154), "1"]

// -- OK launches the installed Guerrilla.Sinai template -----------------------
triClick 1
triSimUntil { alive player }

// -- the pick was published and the engine substituted the player body with
//    the mod class at InitVehicles --------------------------------------------
triAssertEq [gmSelPlayerClass, "LoBoGolaniWB"]
triAssertEq [(typeOf player), "LoBoGolaniWB"]

triEndTest
