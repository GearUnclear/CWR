// ============================================================================
//  Phase 1 of the character-select save/reload round-trip.
//    The REAL player path: main menu -> GUERRILLA -> Abel -> open the
//    character-select screen (155 -> IDD 77) -> pick SoldierWB -> CONFIRM ->
//    OK. The engine substitutes the authored SoldierGB with the pick at
//    InitVehicles (gmSelPlayerClass beats the outfit token; the instance
//    side stays the mission side, so the WEST body fights as GUER). Then
//    write the binary save into the shared UserDir/Saved/Tmp/gbody.fps for
//    phase 2 to diff.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Abel ------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- character screen: "(match outfit)" default, open it, pick SoldierWB by
//    row DATA, CONFIRM back to the parent (row/label pins live in
//    ui/guerrilla_player_body_e2e; here just enough to prove the pick) --------
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]
triClick 155
triAssertEq [(triControlText 163), "SELECT CHARACTER"]
triAssertEq [(triSelectListByData [160, "soldierwb"]), true]
triAssertEq [(triControlText 162), "SIDE: WEST  CLASS: SoldierWB  SOURCE: Base game"]
triClick 1
triAssertEq [(triControlText 155), "CHARACTER: Soldier"]

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
