// The dummy renderer bypasses roster preparation but still enters gameplay.
triSimUntil { GM_LIB_READY }
triAssertEq [(triPortraitStats), [0, 0, 0, 0]]
triAssertGe [gmLegendCount, 3]
triEndTest
