// ============================================================================
//  Guerrilla Mode - island picker mouse-wheel scroll (new-game screen).
//
//  With @LoBo mounted the CfgWorldList roster (11 worlds) outgrows the
//  C3DListBox's 5 visible rows, so islands below the fold are only reachable
//  by scrolling.  The list lives on the laptop prop's screen inside a
//  ControlObjectContainer (the reused RscDisplaySelectIsland layout), whose
//  OnMouseZChanged only relays the wheel to a child while its hover latch
//  (_indexMove) points at one - ControlObject's own OnMouseZChanged is a
//  no-op, so a missed latch (e.g. during the laptop-open animation) silently
//  swallowed every wheel event.  GuerrillaNewGame::OnSimulate now drains the
//  wheel ahead of Display::OnSimulate and drives the list directly (the
//  OptionsScrollList::PollWheelScroll precedent); this test pins that.
//
//  The synthetic wheel (triCursorScroll) lands in the same
//  InputSubsystem/aimDeltaZ buffer as real SDL wheel events.  The input
//  pipe's smoothing re-emits scaled echo deltas for several frames after
//  each impulse, so the up-scroll assert checks net direction rather than an
//  exact row.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- main menu -> GUERRILLA new-game screen ---------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]

// -- precondition: the modded island roster overflows the 5 visible rows ----
GIS_size = triLBSize 101
triAssertGt [GIS_size, 5]
triAssertEq [(triLBTopRow 101), 0]

// -- let the laptop-open animation settle so the list is on screen, then
//    hover it like a real player would --------------------------------------
GIS_f0 = triFrameCount
triSimUntil { triFrameCount > GIS_f0 + 240 }
triCursorMoveControl 101
GIS_f1 = triFrameCount
triSimUntil { triFrameCount > GIS_f1 + 10 }
triScreenshot "island_list_before"

// -- wheel down: the viewport must move past the first page -----------------
triCursorScroll -2
GIS_f2 = triFrameCount
triSimUntil { triFrameCount > GIS_f2 + 20 }
triCursorScroll -2
GIS_f3 = triFrameCount
triSimUntil { triFrameCount > GIS_f3 + 20 }
triCursorScroll -2
GIS_f4 = triFrameCount
triSimUntil { triFrameCount > GIS_f4 + 20 }
triScreenshot "island_list_scrolled"
GIS_topDown = triLBTopRow 101
triAssertGt [GIS_topDown, 0]

// -- wheel up: net upward motion recovers the earlier rows ------------------
triCursorScroll 3
GIS_f5 = triFrameCount
triSimUntil { triFrameCount > GIS_f5 + 20 }
triCursorScroll 3
GIS_f6 = triFrameCount
triSimUntil { triFrameCount > GIS_f6 + 20 }
triCursorScroll 3
GIS_f7 = triFrameCount
triSimUntil { triFrameCount > GIS_f7 + 20 }
triAssertLt [(triLBTopRow 101), GIS_topDown]

// -- the wheel did not close or replace the display -------------------------
triAssertEq [(triDisplay), 76]

triEndTest
