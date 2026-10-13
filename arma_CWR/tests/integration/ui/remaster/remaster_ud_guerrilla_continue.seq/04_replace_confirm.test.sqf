// Issue #79 B4, phase 4: confirming the prompt replaces the saved campaign
// with a new one.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "1"]
triClick 1
triAssertEq [(triDisplay), 210]
triClick 1
triSimUntil { alive player }
triSimUntil { GM_LIB_READY }

// a NEW campaign: none of the saved state came along
triAssertNe [gmResources, 4242]
triAssertNe [gmResources, 5151]
gsOut = gmZoneIndex "Outpost"
triAssertEq [((gmZone gsOut) select 2), gmOccupierSide]
triAssertEq [(format ["%1", isNil "UD_B4_POS"]), "true"]

triEndTest
