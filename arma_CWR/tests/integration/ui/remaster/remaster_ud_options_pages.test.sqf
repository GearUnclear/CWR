// Every Options page on the official Remaster (packages/RemasterUD), each
// scroll-list page left through its pinned Close row with the REAL pointer
// path.  Regression for the 2026-10-14 main-menu crash: an access violation in
// OptionsScrollList::PollPointerActionClick (OptionsScrollList.cpp) while the
// owner browsed the Options pages.  A pointer release over the pinned Close
// row dispatched OnRowAction -> IDC_CANCEL -> ScrollListPage -> PopPage, which
// destroyed the page and the list running the poll; the poll then wrote
// m_pointerLeftWasDown into the freed object.  triClick / triClickText never
// reached that path (they dispatch OnLButtonDown/Up on the control, which ends
// in OptionsScrollList::OnButtonClicked), so the footer is left here with
// triCursorMoveControl + triMouseLeft, the way a human clicks it.
//
// The cursor is parked on slot 0 (hover 503) before each footer move: the
// hover-focus poll only re-targets on cursor motion, and after a pop the
// cursor would otherwise already sit on the next page's footer.

triSetLanguage "English"
triAssertEq [(triDisplay), 0]

triClickText "OPTIONS"
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]
// Let the notebook finish swinging open: the pointer path hit-tests the 3D
// quads, and a cursor placed on a still-moving row drifts off it before the
// release lands (slot 0 sat at y=0.18 mid-animation, y=-0.65 settled).
triSimFrames 120

// -- Audio ------------------------------------------------------------------
triClickText "Audio"
triAssertEq [(triControlText 100), "Audio"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:audio_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Display ----------------------------------------------------------------
triClickText "Display"
triAssertEq [(triControlText 100), "Display"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:display_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Graphics ---------------------------------------------------------------
triClickText "Graphics"
triAssertEq [(triControlText 100), "Graphics"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:graphics_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Game -------------------------------------------------------------------
triClickText "Game"
triAssertEq [(triControlText 100), "Game"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:game_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Difficulty -------------------------------------------------------------
triClickText "Difficulty"
triAssertEq [(triControlText 100), "Difficulty"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:difficulty_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Controls and its four sub-pages ----------------------------------------
triClickText "Controls"
triAssertEq [(triControlText 100), "Controls"]
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

triClickText "Keyboard & Mouse"
triAssertEq [(triControlText 100), "Keyboard & Mouse"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:kbm_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triControlText 100), "Controls"]

triClickText "Mouse"
triAssertEq [(triControlText 100), "Mouse"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:mouse_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triControlText 100), "Controls"]

triClickText "Gamepad"
triAssertEq [(triControlText 100), "Gamepad"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:gamepad_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triControlText 100), "Controls"]

triClickText "Gamepad Tuning"
triAssertEq [(triControlText 100), "Gamepad Tuning"]
triCursorMoveControl 503
triSimFrames 3
triCursorMoveControl 583
triSimFrames 6
if ((triAssert [(triGetControlFocused 583)]) != "OK") exitWith { "FAIL:gamepad_tuning_footer_not_focused" };
triMouseLeft 1
triSimFrames 6
triMouseLeft 0
triSimFrames 3
triAssertEq [(triControlText 100), "Controls"]

// Controls is a flat menu page: its Close row (nav idc 1404) is a plain nav
// control whose label is Bohemia's on the Remaster, so click it by idc.
triClick 1404
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 100), "Options"]

// -- Esc on the index closes the shell -----------------------------------
triSendKey 41
triAssertEq [(triDisplay), 0]

triEndTest
