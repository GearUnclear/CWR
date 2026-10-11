// GUERRILLA -> Sinai -> launch on the official Remaster with UD installed
// beside Bohemia's files (issue #67) and @LoBo mounted. The Sinai template was
// installed because setup found sinai.wrp in @LoBo's pbos; the @LoBo war
// rosters come from the bin/config.cpp setup generated (install-lobo-factions),
// and the vanilla library from Bohemia's config-extra.cpp include.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "sinai"]), true]
// The template's defaults: IDF occupies, EgyptFrontier resists.
triAssertEq [(triControlText 150), "OCCUPIER: IDF"]
triAssertEq [(triControlText 151), "RESISTANCE: EgyptFrontier"]
triScreenshot "01_new_game_sinai"
triClick 1
triSimUntil { alive player }

triAssertEq [gmSelIsland, "sinai"]
triAssertEq [gmSelOccupier, "IDF"]
triAssertEq [(gmOccupierSide), "WEST"]
triSimUntil { gmZoneCount >= 10 }
triSimUntil { GM_LIB_READY }
triScreenshot "02_sinai_in_mission"

triEndTest
