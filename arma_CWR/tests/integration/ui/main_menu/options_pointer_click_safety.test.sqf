// Options shell pointer-click safety (owner crash 2026-10-14 20:24:45:
// OptionsScrollList::PollPointerActionClick wrote through a freed `this` while
// the owner clicked around the Options screens from the main menu).
//
// Root cause: OptionsShell::PopPage destroyed the popped page (and its scroll
// list) synchronously, but PopPage is reached from code running ON that page:
// a pointer click on the pinned Close row (list -> WithCloseRow -> IDC_CANCEL
// -> PopPage), Esc, a modal's Resolve, the index's Close nav row. That code
// kept writing members of the freed object after PopPage returned.
//
// The scroll list's pointer poll treats a press that lands on no control as a
// click on the focused row when that row is an Action. With the pinned Close
// row focused, a click past the last row closes the page from inside
// PollPointerActionClick, the frame in the owner's stack; the poll then wrote
// m_pointerLeftWasDown into the list it had just freed.
//
// This walk drives every scroll-list page with the real pointer path (synthetic
// cursor + level-held left button, the same InputSubsystem state a mouse sets):
// wheel and PageDown through the list, scrollbar press and hold, a click past
// the last row (on the hint line under the list), the Close row by both hit-test paths (the centred action
// text 5N1, which the list's own pointer poll handles, and the hover overlay
// 5N3, which the notebook dispatches), Esc while the button is still held, and
// a press in the same frame as a page switch. It must finish with the shell
// intact and the main menu back.
//
// Slot IDCs: 500 + slot*10 + digit; slot 8 is the pinned Close row; 590 is the
// hint line under the list; 850 is the scrollbar. Scancodes: Esc 41, PageDown 78.

#include "../../helpers/options_preamble.sqf"
triSimFrames 3

// ---- Audio ----
triClick 1101
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1101
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1101
triSimFrames 3
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]

// ---- Display ----
triClick 1102
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1102
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1102
triSimFrames 3
triCursorMoveControl 583
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]

// ---- Graphics ----
triClick 1103
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1103
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1103
triSimFrames 3
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]

// ---- Game ----
triClick 1104
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1104
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1104
triSimFrames 3
triCursorMoveControl 583
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]

// ---- Difficulty ----
triClick 1108
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1108
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]
triClick 1108
triSimFrames 3
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Controls"]

// ---- Controls sub-pages ----
triClick 1107
triSimFrames 3
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- Keyboard & Mouse ----
triClick 1401
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1401
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1401
triSimFrames 3
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- Mouse ----
triClick 1402
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1402
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1402
triSimFrames 3
triCursorMoveControl 583
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- Gamepad ----
triClick 1405
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1405
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1405
triSimFrames 3
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- Gamepad Tuning ----
triClick 1406
triSimFrames 3
// Click past the last row with Close focused: hover Close (focus follows),
// then press and release on the hint line under the list (no control takes it).
// Hover focus follows cursor MOVES only, so step off the Close row first.
triCursorMoveControl 523
triSimFrames 2
triCursorMoveControl 581
triSimFrames 2
triCursorMoveControl 590
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1406
triSimFrames 3
triCursorMoveControl 523
triSimFrames 2
triCursorScroll -6
triSimFrames 2
triSendKey 78
triSendKey 78
triSendKey 78
triSendKey 78
triSimFrames 2
triCursorMoveControl 850
triSimFrames 2
triMouseLeft 1
triSimFrames 4
triMouseLeft 0
triSimFrames 2
// Press on the Close row, Esc while held (page switch mid-click), release over the parent.
triCursorMoveControl 581
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triSendKey 41
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triClick 1406
triSimFrames 3
triCursorMoveControl 583
triSimFrames 2
triMouseLeft 1
triSimFrames 2
triMouseLeft 0
triSimFrames 3
triAssertDisplay 9099
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- a press that lands in the same frame as a page switch ----
triClick 1401
triMouseLeft 1
triSimFrames 2
triSendKey 41
triMouseLeft 0
triSimFrames 3
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// ---- back out to the main menu: the shell must still be whole ----
triSendKey 41
triSimFrames 3
triAssertIncludes [(triVisibleTexts), "Controls"]
triSendKey 41
triSimFrames 30
triAssertEq [(triDisplay), 0]

triEndTest
