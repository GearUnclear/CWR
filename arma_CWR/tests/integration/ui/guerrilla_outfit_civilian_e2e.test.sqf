// ============================================================================
//  Guerrilla Mode - character-select CIVILIAN outfit, end to end (issue #25).
//
//  Drives the real player path on full CWA data: main menu -> GUERRILLA ->
//  IDD_GUERRILLA_NEW_GAME (76) -> pick Abel -> cycle the outfit cycler
//  (idc 153, injected next to the occupier/resistance pair) to CIVILIAN ->
//  OK. Then proves BOTH halves of the outfit axis in the launched campaign:
//
//    M1 (player substitution): the engine rewrote the authored mission.sqm
//       SoldierGB to the GUER descriptor's playerClassCiv=SoldierGFakeC at
//       InitVehicles (WorldInit.cpp / Game/Guerrilla/OutfitSelect.cpp) -
//       typeOf player is the plainclothes body, side still resistance
//       (instance side comes from the mission side field, never the class).
//
//    M2 (recruit matching): the manager scripts folded gmSelOutfit into
//       GM_OUTFIT_CIV and swapped their body classes through the *Civ
//       descriptor keys (asserted on the boot-time class globals), AND the
//       spawn paths actually produce plainclothes bodies (M2.3): the
//       auto-spawned companion and two real recruit-queue spawns (fighter +
//       specialist) are asserted by typeOf on the live units. The hold-squad
//       civilian branch (capture.sqs gmFactionSquad bypass) is covered by
//       scripting/guerrilla_outfit_hold_spawn on the native fixture, and
//       save/reload body persistence by scripting/guerrilla_outfit_save_reload.
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
// (triSelectListByData is a case-sensitive substring match on the row data;
// Classic's CfgWorldList class is "Abel")
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- the faction cyclers open on the template's default* keys ----------------
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triAssertEq [(triControlText 151), "RESISTANCE: GUER"]

// -- outfit cycler: WARRIOR default (the no-op), one click = CIVILIAN --------
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
// preview mannequin (idc 154, issue #25 M4): SoldierGB's model resolves on
// Classic 1.99, so the injected CT_OBJECT must be visible. Visibility IS the
// assertion - a missing class/model hides the control instead of crashing.
triAssertEq [(triGetControlVisible 154), "1"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
// still visible after the body swap to playerClassCiv=SoldierGFakeC
triAssertEq [(triGetControlVisible 154), "1"]

// -- launch ------------------------------------------------------------------
triClick 1
triSimUntil { alive player }

// -- the selection was published for engine and scripts ----------------------
triAssertEq [gmSelOutfit, "CIVILIAN"]

// -- M1: the player wears the plainclothes body, on the resistance side ------
triAssertEq [(typeOf player), "SoldierGFakeC"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- M2: the shared core folded the choice and matched the body classes ------
triSimUntil { GM_LIB_READY }
triSimUntil { format ["%1", GM_OUTFIT_CIV] == "true" }
// recruit.sqs / companions.sqs / capture.sqs boot after lib - poll each
// class global until its manager has run its outfit-conditional read
triSimUntil { format ["%1", GM_RECRUIT_FIGHTER] == "SoldierGFakeC" }
triSimUntil { format ["%1", GM_RECRUIT_SPEC] == "SoldierGFakeC2" }
triSimUntil { format ["%1", GM_COMP_CLASS] == "SoldierGFakeC" }
triSimUntil { format ["%1", GM_HOLD_CIV] == "SoldierGFakeC" }

// -- the native registry served the civTier[] ladder too (M3 surface) --------
triAssertEq [(gmFactionCivTier [gmResistanceSide, 1]), "SoldierGFakeC"]

// ============================================================================
//  M2.3: REAL spawns, not just the folded class globals.
// ============================================================================

// -- companion: companions.sqs auto-spawns Petra near the player; her body
//    must be the plainclothes companionClassCiv class -------------------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(typeOf (GM_COMP_OBJ select 0)), "SoldierGFakeC"]

// -- recruit a fighter through the real request queue. The player spawns AT
//    the Camp anchor (mission.sqm player pos == the CAMP zone pos), so
//    recruit.sqs mounts the addAction menu; the test enqueues the request
//    exactly as recruit_action.sqs does (gmReqId + gmReqPending) ---------------
gmManpower = 5
triSimUntil { gmRecruitActive }
gmReqId = gmActRecruit
gmReqPending = true
triSimUntil { not (isNil "gmNewUnit") }
triAssertEq [(format ["%1", isNull gmNewUnit]), "false"]
triAssertEq [(typeOf gmNewUnit), "SoldierGFakeC"]
// side weld: the civilian-class body fights as resistance, in the player group
triAssertEq [(format ["%1", side gmNewUnit]), "GUER"]
triAssertEq [(format ["%1", gmNewUnit in (units (group player))]), "true"]

// -- and a specialist: the second recruit rung wears the FakeC2 body ----------
gmReqId = gmActSpec
gmReqPending = true
triSimUntil { (typeOf gmNewUnit) == "SoldierGFakeC2" }
triAssertEq [(format ["%1", side gmNewUnit]), "GUER"]

triEndTest
