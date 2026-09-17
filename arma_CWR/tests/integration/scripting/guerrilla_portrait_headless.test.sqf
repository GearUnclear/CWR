// The dummy renderer bypasses roster preparation but still enters gameplay.
triSimUntil { GM_LIB_READY }
triAssertEq [(triPortraitStats), [0, 0, 0, 0]]
triAssertGe [gmLegendCount, 3]
// Even an explicit late request must not construct/render a headless mannequin.
gpHeadless = triPortraitId ["SoldierGB", "Face10"]
triSimFrames 3
triAssertEq [(triPortraitStats), [1, 1, 0, 0]]
triEndTest
