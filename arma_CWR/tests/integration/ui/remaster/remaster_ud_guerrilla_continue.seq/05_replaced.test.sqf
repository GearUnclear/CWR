// Issue #79 B4, phase 5: the confirmed replacement in phase 4 deleted the old
// campaign's saves (and the new one was never saved), so Abel offers no
// CONTINUE and a new campaign starts without a prompt.

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "0"]
triClick 1
triAssertNe [(triDisplay), 210]
triSimUntil { GM_LIB_READY }
triAssertEq [gmSelIsland, "Abel"]

triEndTest
