triSetLanguage "English"
gpId = triPortraitId ["SoldierGB", "Face10"]
triSimFrames 3
triAssertEq [((triPortraitStats) select 1), 1]
triClickText "OPTIONS"
triClickText "Game"
triAssertEq [(triDisplay), 9099]
triSimFrames 3
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 81
triSimFrames 2
triSendKey 40
triSimFrames 2
triAssertEq [(triPortraitStats), [0, 0, 0, 0]]
// A subsequent request regenerates: neither a retained CPU image nor a disk hit.
triAssertEq [(triPortraitId ["SoldierGB", "Face10"]), gpId]
triSimFrames 3
triAssertEq [(triPortraitStats), [1, 1, 1, 0]]
triEndTest
