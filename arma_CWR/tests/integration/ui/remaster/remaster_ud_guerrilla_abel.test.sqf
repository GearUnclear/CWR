// GUERRILLA -> Abel -> launch on the official Remaster with UD installed
// beside Bohemia's files (issue #67), no mod mounted. Proves the front door
// dispatches, the installed Guerrilla.Abel template resolves under the
// Remaster's Missions folder, and the shared core (gmcore) boots.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
// Guerrilla.Abel's default faction pair.
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triAssertEq [(triControlText 151), "RESISTANCE: GUER"]
// Pick the authored start town so the first tick also establishes the HQ.
triAssertEq [(triControlText 152), "START TOWN: (camp)"]
triClick 152
triAssertEq [(triControlText 152), "START TOWN: Village"]
triScreenshot "01_new_game_abel"
triClick 1
triSimUntil { alive player }

triAssertEq [gmSelIsland, "Abel"]
triAssertEq [gmSelStartTown, "Village"]
triSimUntil { gmZoneCount > 0 }
triSimUntil { gmHqEstablished }
triAssertEq [gmHqZone, "Village"]
triSimUntil { GM_LIB_READY }
triScreenshot "02_abel_in_mission"

triEndTest
