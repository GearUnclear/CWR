// Issue #79 B4, phase 3: resume, play on, quit to the main menu through the
// pause menu's Abort, and resume again in the same process. The Abort writes
// the quit snapshot (continue.fps), which is now the newest slot, so CONTINUE
// comes back to where the player quit, not to the older Save Game.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "1"]
triClick 156
triSimUntil { alive player }
triSimFrames 5
triAssertEq [gmResources, 4242]

// play on, then quit to the main menu
gmResources = 5151
triSendKey 41
triAssertEq [(triDisplay), 49]
triClick 104
triSimUntil { triDisplay == 0 }
triSimUntil { triGameMode == 2 }

// resume from the main menu in the same process
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "1"]
triClick 156
triSimUntil { alive player }
triSimFrames 5
triAssertEq [gmResources, 5151]
gsOut = gmZoneIndex "Outpost"
triAssertEq [((gmZone gsOut) select 2), gmResistanceSide]

triEndTest
