// Issue #79 B4, phase 2: a FRESH process. The GUERRILLA screen offers
// CONTINUE for Abel; a new campaign asks before replacing the save, Cancel
// keeps it and the screen; CONTINUE resumes exactly what phase 1 saved.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
// the screen opens on the island with the newest campaign save
triAssertEq [(triGetControlVisible 156), "1"]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "1"]
triAssertIncludes [(triControlText 156), "Continue ("]
triScreenshot "02_new_game_with_continue"

// OK on an island with a save asks first; Cancel keeps save and screen
triClick 1
triAssertEq [(triDisplay), 210]
// the message text is a disabled static, which triVisibleTexts skips; the
// screenshot below shows it
triAssertIncludes [(triVisibleTexts), "Cancel"]
triScreenshot "02_replace_prompt"
triClick 2
triAssertEq [(triDisplay), 76]
triAssertEq [(triGetControlVisible 156), "1"]

// CONTINUE resumes the saved campaign
triClick 156
triSimUntil { alive player }
triSimFrames 5
triAssertEq [gmSelIsland, "Abel"]
triAssertEq [gmResources, 4242]
gsOut = gmZoneIndex "Outpost"
triAssertEq [((gmZone gsOut) select 2), gmResistanceSide]
triAssertLt [(player distance UD_B4_POS), 3]
triScreenshot "02_resumed"

triEndTest
