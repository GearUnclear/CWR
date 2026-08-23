// ============================================================================
//  Guerrilla Mode - START TOWN cycler (issue #16 M4), end to end.
//
//  The fourth injected cycler on the new-game screen (idc 152, below the
//  faction pair): it lists the CITY zones the selected island's campaign
//  will carry (the template's authored CITY zones first, then the world's
//  Names towns the registry seeds - ZoneRegistry::CollectTownNames, one rule
//  for the cycler and the zone table) and opens on "(camp)", which publishes
//  nothing. A pick publishes gmSelStartTown (VarSet + the campaign variable
//  bank, like gmSelOccupier); on the first Simulate tick GuerrillaBase
//  resolves it to a zone, establishes the headquarters there (best enterable
//  building, else the edge of town) and relocates the player beside the
//  garage ring.
//
//  On Guerrilla.Abel the first entry is the template's authored CITY zone
//  "Village" (Houdan), ahead of the seeded Names towns - so one click picks
//  it, and the launch must open the campaign AT Houdan with the HQ standing.
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

// -- the untouched cycler publishes nothing; one click picks the authored
//    CITY zone (Village = Houdan), ahead of the seeded towns ----------------
triAssertEq [(triControlText 152), "START TOWN: (camp)"]
triClick 152
triAssertEq [(triControlText 152), "START TOWN: Village"]
// the faction pair is untouched: the launch below behaves like a default one
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triAssertEq [(triControlText 151), "RESISTANCE: GUER"]

// -- OK launches the installed Guerrilla.Abel template ----------------------
triClick 1
triSimUntil { alive player }

// -- the pick was published for the engine + scripts -------------------------
triAssertEq [gmSelStartTown, "Village"]
triAssertEq [gmSelIsland, "Abel"]

// -- the first tick elected the HQ in that zone and moved the player there ---
triSimUntil { gmHqEstablished }
triAssertEq [gmHqZone, "Village"]
triAssertEq [gmHqMoveCount, 0]
triAssertLt [((getPos player) distance gmHqPos), 150]
triAssertLt [((getPos player) distance gmHqGaragePos), 60]
triSimUntil { not (isNull gmHqCache) }

// -- the shared core booted on top and noticed the HQ (objective flips DONE) --
triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_MKT_READY") }
triSimUntil { (gmJournalObjectiveState "hqEstablish") == "DONE" }

triEndTest
