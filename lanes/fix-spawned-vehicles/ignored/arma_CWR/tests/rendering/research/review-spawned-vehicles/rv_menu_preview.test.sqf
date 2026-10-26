// REVIEW (spawned-vehicles fix), not committed: the Guerrilla new-game
// mannequin (GuerrillaBodyPreview) and its weapon proxies. The body is loaded
// after the menu world's OptimizeAll, so with the fix its proxies gain vertex
// buffers and draw through the GPU path. Shots for a before/after look.
triSetLanguage "English"
triSimUntil { triGameMode == 2 }
RV_f0 = triFrameCount
triSimUntil { triFrameCount > RV_f0 + 120 }
triScreenshot "rv_main_menu"
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
RV_f1 = triFrameCount
triSimUntil { triFrameCount > RV_f1 + 300 }
triScreenshot "rv_newgame_a"
RV_f2 = triFrameCount
triSimUntil { triFrameCount > RV_f2 + 45 }
triScreenshot "rv_newgame_b"
triClick 155
RV_f3 = triFrameCount
triSimUntil { triFrameCount > RV_f3 + 240 }
triScreenshot "rv_charselect_a"
RV_f4 = triFrameCount
triSimUntil { triFrameCount > RV_f4 + 45 }
triScreenshot "rv_charselect_b"
triEndTest
