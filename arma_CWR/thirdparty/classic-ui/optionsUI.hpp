// PoseidonOptionsTest — shared layout / colour / font / engine constants.
//
// Every per-display .hpp in this resource (displayOptionsTestIndex.hpp,
// displayOptionsTestAudio.hpp, …) includes this file at the top so the
// chrome looks consistent across screens.  Bump a single constant here
// to reshape every display that uses it.
//
// Coordinate space: 0..1 screen UV, except OPT_ROW_* which are 0..1
// notebook-surface UV (notebook is a 3D control, its sub-controls map
// to a UV island on the model).
//
// The param parser evaluates arithmetic at load time, so derived
// constants (e.g. OPT_STRIP_BOT_Y = 1.0 - OPT_STRIP_H) update
// automatically when the base they depend on changes.
//
// IMPORTANT: BIS PreprocC captures the rest of the line as the macro
// body, including `//` comments.  Place documentation ABOVE each
// #define, never trailing it on the same line.

#ifndef OPTIONS_TEST_UI_HPP
#define OPTIONS_TEST_UI_HPP

// ===========================================================================
// Engine constants — what the raw `type` / `style` / `access` numbers
// in the resource templates actually mean.  These come from
// engine/Poseidon/Core/resincl.hpp; redefined here because config-side
// `#include` only pulls other config files, not C++ headers.
// ===========================================================================

// ---- Control types (the `type=` field on every class) --------------------

// Plain rect / text label, non-interactive.
#define CT_STATIC               0
// 2D text that lights on hover and emits a click.
#define CT_ACTIVETEXT           11
// 3D variant — child of a ControlObject (notebook etc.).
#define CT_3DSTATIC             20
// 3D active text on a ControlObject surface.
#define CT_3DACTIVETEXT         21
// 3D scrollbar — standalone widget exposing carets / thumb / track click.
#define CT_3DSCROLLBAR          26
// 3D model host (notebook.p3d) with open/close anim.
#define CT_OBJECT_CONT_ANIM     83

// ---- Style flags (the `style=` field) ------------------------------------
//
// Flags compose with bitwise-or, but the numeric fields below cover the
// single-flag cases the Options screens actually use today.

// Default — text aligned to control left edge.
#define ST_LEFT                 0
// ST_HPOS variant: horizontally centered text.
#define ST_CENTER               2
// Filled rect with optional border (for row Bgs).
#define ST_BACKGROUND           80

// ---- Access mode (the `access=` field) -----------------------------------
//
// 3 = PAReadOnlyVerified — class is locked after parse, CRC-checked at
// runtime.  Standard for production RscClass templates so accidental
// inheritance can't mutate them.
#define ACCESS_LOCKED           3

// ===========================================================================
// Colours
// ===========================================================================

#define OPT_C_WHITE             {1,1,1,1}
#define OPT_C_TRANSPARENT       {0,0,0,0}
// Top/bottom dark bands.
#define OPT_C_STRIP_BG          {0,0,0,0.5}
// Notebook menu text.
#define OPT_C_CRT_GREEN         {0.3,1,0.3,1}
// Focused-row fill.
#define OPT_C_CRT_BG            {0.3,1,0.3,0.5}
// Focused-row outline.
#define OPT_C_CRT_BORDER        {0.3,1,0.3,0.8}
// Slider-bar palette — track is dim, fill is bright, peak is yellow.
// Differentiated from the focus-row green so the two can stack with
// the bar still legible on top of a focused row's bg fill.
#define OPT_C_BAR_TRACK         {0.18,0.40,0.18,1.0}
#define OPT_C_BAR_FILL          {0.45,1.0,0.45,0.95}
#define OPT_C_BAR_PEAK          {1.0,1.0,0.6,0.95}
// Chevron stepper — green at rest, yellow on hover (BIS active-text
// convention).  drawFocusLine=0 on the template so the colour swap
// alone signals focus.
#define OPT_C_CHEVRON           {0.3,1,0.3,0.85}
#define OPT_C_CHEVRON_ACTIVE    {1,1,0.2,1}
// Description-line tint — soft amber, reads as "tip" distinct from
// the bright-green focused row.
#define OPT_C_HINT              {0.85,0.78,0.50,0.95}
// Pending value tint — bright yellow.  Applied to the value cell of
// any settings row whose pending differs from live, so the user can
// scan the page and see at a glance what changes are queued for Apply.
#define OPT_C_PENDING           {1,1,0.2,1}
// Disabled Action row tint — desaturated dark green.  Applied to
// rows that can't be activated (e.g. Apply when nothing is pending).
// The cycle skips disabled rows; the colour is the visual cue.
#define OPT_C_DISABLED          {0.30,0.45,0.30,0.7}

// ===========================================================================
// Fonts — three roles, one font per role.
// ===========================================================================

// Display titles ("Options", "Audio", …).
#define OPT_FONT_TITLE          "cwrTitleB128"
// Bottom-strip 2D actions (Close, Back, …).
#define OPT_FONT_ACTION         "cwrTitleB64"
// Notebook-surface menu items.
#define OPT_FONT_CRT            "cwrMonoB64"

// ===========================================================================
// Sound triplets {clip, volume, pitch} — used by every active control's
// soundEnter / soundClick / soundEscape / soundPush slots.  Defined as
// array literals so they expand directly into `soundEnter[]=…`.
// ===========================================================================

#define UI_SOUND_VOL            0.2
#define UI_SOUND_HOVER          {"ui\ui_over", UI_SOUND_VOL, 1}
#define UI_SOUND_CLICK          {"ui\ui_ok",   UI_SOUND_VOL, 1}
#define UI_SOUND_CANCEL         {"ui\ui_cc",   UI_SOUND_VOL, 1}
#define UI_SOUND_SILENT         {"",           UI_SOUND_VOL, 1}

// ===========================================================================
// Layout — top/bottom chrome strips
// ===========================================================================

// 12.5% bands top + bottom; bottom strip Y derives from the height so
// changing OPT_STRIP_H reshapes both ends symmetrically.
#define OPT_STRIP_X             0
#define OPT_STRIP_W             1.0
#define OPT_STRIP_H             0.125
#define OPT_STRIP_TOP_Y         0
#define OPT_STRIP_BOT_Y         (1.0 - OPT_STRIP_H)

// ===========================================================================
// Layout — title (centered across the top strip)
// ===========================================================================

// 20% margin each side; W and X derive from the inset so a single
// constant change recenters everything.
#define OPT_TITLE_INSET_X       0.20
#define OPT_TITLE_X             OPT_TITLE_INSET_X
#define OPT_TITLE_W             (1.0 - 2 * OPT_TITLE_INSET_X)
#define OPT_TITLE_Y             0.02
#define OPT_TITLE_H             0.10
#define OPT_TITLE_SIZE          0.12

// ===========================================================================
// Layout — close button (bottom right corner of the bottom strip)
// ===========================================================================

// Y placed inside the OPT_STRIP_BOT_Y band; H tight to the glyphs so
// the engine's IsFocused underline (drawn at y + h) sits right under
// them.  X anchors flush-right with a 2% gutter.
#define OPT_CLOSE_GUTTER        0.02
#define OPT_CLOSE_W             0.15
#define OPT_CLOSE_X             (1.0 - OPT_CLOSE_W - OPT_CLOSE_GUTTER)
#define OPT_CLOSE_Y             0.92
#define OPT_CLOSE_H             0.062
#define OPT_CLOSE_SIZE          0.06

// ===========================================================================
// Layout — notebook menu rows (notebook UV space)
// ===========================================================================

// Rows stack at OPT_ROW_PITCH spacing starting at OPT_ROW_BASE_Y.  The
// production index fits 8 rows (Audio / Display / Graphics / Game /
// Controls / Difficulty / Credits / Close) on the notebook without scrolling.
#define OPT_ROW_INSET_X         0.02
#define OPT_ROW_X               OPT_ROW_INSET_X
#define OPT_ROW_W               (1.0 - 2 * OPT_ROW_INSET_X)
#define OPT_ROW_H               0.10
#define OPT_ROW_PITCH           0.11
#define OPT_ROW_BASE_Y          0.05
#define OPT_ROW_0_Y             OPT_ROW_BASE_Y
#define OPT_ROW_1_Y             (OPT_ROW_BASE_Y + 1 * OPT_ROW_PITCH)
#define OPT_ROW_2_Y             (OPT_ROW_BASE_Y + 2 * OPT_ROW_PITCH)
#define OPT_ROW_3_Y             (OPT_ROW_BASE_Y + 3 * OPT_ROW_PITCH)
#define OPT_ROW_4_Y             (OPT_ROW_BASE_Y + 4 * OPT_ROW_PITCH)
#define OPT_ROW_5_Y             (OPT_ROW_BASE_Y + 5 * OPT_ROW_PITCH)
#define OPT_ROW_6_Y             (OPT_ROW_BASE_Y + 6 * OPT_ROW_PITCH)
#define OPT_ROW_7_Y             (OPT_ROW_BASE_Y + 7 * OPT_ROW_PITCH)

// ===========================================================================
// Layout — Audio settings page (notebook-surface slot grid)
// ===========================================================================
//
// 7-slot generic row pattern reused for the Audio screen's 12 logical
// rows (scroll window).  Each slot carries label + stepper text + bar
// readout + slider track/fill/peak + invisible click overlays for
// hover / bar-click / prev-chevron / next-chevron.  Layout values
// pulled verbatim from the harness page (RscDisplayHarness3DOptionsAudio
// in packages/UITest/bin/resource.cpp:20020) so the visual is 1:1.
//
// Slot Y values stack at OPT_LIST_ROW_PITCH (0.085) starting from
// OPT_LIST_ROW_BASE_Y (0.120).  12 row Bgs total — first 7 visible,
// 8..11 demonstrate scrollbar pagination.

#define OPT_LIST_ROW_X           0.02
#define OPT_LIST_ROW_W           0.935
#define OPT_LIST_ROW_H           0.075
#define OPT_LIST_ROW_PITCH       0.085
#define OPT_LIST_ROW_BASE_Y      0.120
#define OPT_LIST_ROW_0_Y         OPT_LIST_ROW_BASE_Y
#define OPT_LIST_ROW_1_Y         (OPT_LIST_ROW_BASE_Y +  1 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_2_Y         (OPT_LIST_ROW_BASE_Y +  2 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_3_Y         (OPT_LIST_ROW_BASE_Y +  3 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_4_Y         (OPT_LIST_ROW_BASE_Y +  4 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_5_Y         (OPT_LIST_ROW_BASE_Y +  5 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_6_Y         (OPT_LIST_ROW_BASE_Y +  6 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_7_Y         (OPT_LIST_ROW_BASE_Y +  7 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_8_Y         (OPT_LIST_ROW_BASE_Y +  8 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_9_Y         (OPT_LIST_ROW_BASE_Y +  9 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_10_Y        (OPT_LIST_ROW_BASE_Y + 10 * OPT_LIST_ROW_PITCH)
#define OPT_LIST_ROW_11_Y        (OPT_LIST_ROW_BASE_Y + 11 * OPT_LIST_ROW_PITCH)

// Per-slot column split.  Label sits at the left, stepper text
// centred in the middle, bar readout right.  Slider track + fill +
// peak occupy the central band; click overlays sit on top.
#define OPT_LIST_LABEL_X         0.04
#define OPT_LIST_LABEL_W         0.34
#define OPT_LIST_VALUE_STEP_X    0.46
#define OPT_LIST_VALUE_STEP_W    0.44
#define OPT_LIST_VALUE_BAR_X     0.81
#define OPT_LIST_VALUE_BAR_W     0.12
#define OPT_LIST_TRACK_X         0.40
#define OPT_LIST_TRACK_W         0.40
#define OPT_LIST_TRACK_H         0.018
#define OPT_LIST_BAR_Y_OFFSET    0.0285
#define OPT_LIST_PEAK_W          0.005
#define OPT_LIST_HOVER_X         0.02
#define OPT_LIST_HOVER_W         0.96
#define OPT_LIST_PREV_X          0.40
#define OPT_LIST_PREV_W          0.05
#define OPT_LIST_NEXT_X          0.92
#define OPT_LIST_NEXT_W          0.03

// Hint line — single line of the per-row description tucked along
// the bottom of the notebook surface.  Long descriptions marquee-
// scroll horizontally (handled C++ side in OptionsScrollList::
// UpdateRowHighlight).  Left margin matches the row labels at
// OPT_LIST_LABEL_X = 0.04 so the hint reads as a visual continuation
// of the label column rather than a centred caption.
//
// `size` here is a fraction of the control's H, not absolute UV — keep
// H at the original 0.07 so the glyphs render at the legibility size
// they always did; `lines=1` on the Hint class itself enforces the
// single-line shape.
#define OPT_LIST_HINT_X          0.02
#define OPT_LIST_HINT_Y          0.89
#define OPT_LIST_HINT_W          0.96
#define OPT_LIST_HINT_H          0.07
#define OPT_LIST_HINT_SIZE       0.40

// Scrollbar (CT_3DSCROLLBAR).  Sits flush right of the row Bg edge
// (Bg ends 0.955, scrollbar starts 0.96 with a 0.005 gutter).  Spans
// 9 visible slots top-to-bottom — row 0 top to row 8 bottom (0.875).
#define OPT_LIST_SCROLLBAR_X     0.96
#define OPT_LIST_SCROLLBAR_W     0.035
#define OPT_LIST_SCROLLBAR_Y     0.1175
#define OPT_LIST_SCROLLBAR_H     0.7575

// Stepper / readout text size — smaller than the row label so the
// numeric value reads as secondary content.
#define OPT_LIST_VALUE_SIZE      0.55

// Bar zBias — pushes the slider track / fill / peak slightly forward
// of the row Bg so they win the depth tie when both render
// ST_BACKGROUND fills on the same notebook surface plane.
#define OPT_LIST_BAR_ZBIAS       0.0001

// ===========================================================================
// Layout — modal dialogs (rendered on the notebook surface, UV space)
// ===========================================================================
//
// Coordinates here are UV positions on the laptop's screen mesh, not the
// host window — the panel and its content move with the notebook pose
// and read as part of the same green CRT surface as the parent page.
// X spans 0..1 across the screen; Y spans 0..1 down the screen.
//
// Border is the green frame; Panel sits just inside it (OPT_DLG_BORDER
// inset on every side) so the visible green edge has uniform thickness.

#define OPT_DLG_X            0.05
// Y leaves a clear gap below row 1 (0.205 + 0.075 = 0.28) so the
// page row's text + CRT-bg fill don't butt against the modal panel
// edge.  H leaves the third button row fully inside the panel for
// the Replace / Retry / Cancel capture modal.
#define OPT_DLG_Y            0.30
#define OPT_DLG_W            0.90
#define OPT_DLG_H            0.55

// Slim frame — wider panels make the green border read as too heavy
// at 1% thickness; 0.4% gives a clean edge that doesn't dominate.
#define OPT_DLG_BORDER       0.004
#define OPT_DLG_PANEL_X      (OPT_DLG_X + OPT_DLG_BORDER)
#define OPT_DLG_PANEL_Y      (OPT_DLG_Y + OPT_DLG_BORDER)
#define OPT_DLG_PANEL_W      (OPT_DLG_W - 2 * OPT_DLG_BORDER)
#define OPT_DLG_PANEL_H      (OPT_DLG_H - 2 * OPT_DLG_BORDER)

// Tight 2 % text padding inside the panel maximises the line width for
// the prompt + status rows.
#define OPT_DLG_TEXT_X       (OPT_DLG_X + 0.02)
#define OPT_DLG_TEXT_Y       (OPT_DLG_Y + 0.04)
#define OPT_DLG_TEXT_W       (OPT_DLG_W - 0.04)
// Title + status each hold a single short line.
#define OPT_DLG_TEXT_H       0.08

#define OPT_DLG_COUNT_Y      (OPT_DLG_Y + 0.13)

// Button rows — full-width menu-item style, same pitch + height as
// flat-menu nav rows so a Save / Retry / Cancel modal reads as the
// same kind of selectable list as Index / Graphics / Controls.
// Three slots, top-down; 2-button modals use slots 0 and 1 only.
#define OPT_DLG_BTN_H        OPT_ROW_H
#define OPT_DLG_BTN_PITCH    OPT_ROW_PITCH
#define OPT_DLG_BTN_Y_0      (OPT_DLG_Y + 0.22)
#define OPT_DLG_BTN_Y_1      (OPT_DLG_BTN_Y_0 + OPT_DLG_BTN_PITCH)
#define OPT_DLG_BTN_Y_2      (OPT_DLG_BTN_Y_0 + 2 * OPT_DLG_BTN_PITCH)

// 3D text size — larger than before because capture prompts now fit on
// one short line instead of wrapping through a multi-line hint block.
// Button text still inherits OptIdxMenuItem so it matches flat-menu
// nav rows pixel-for-pixel.
#define OPT_DLG_TITLE_3D_SIZE  0.70
#define OPT_DLG_BODY_3D_SIZE   0.52

// Border + Panel z-bias — both fills are coplanar with the underlying
// page's row highlights on the notebook surface; without a camera-
// forward push, depth ties make the page's bright row fills bleed
// through the modal panel.  zBias is camera-forward distance, so
// LARGER values draw IN FRONT.  Order from back to front:
//   page row Bg / hover    (zBias 0.0001 — OPT_LIST_BAR_ZBIAS)
//   modal Border           (zBias 0.0006)
//   modal Panel            (zBias 0.0008)
//   button focus highlight (z = -0.001, hardcoded in C3DActiveText)
//   text + lines           (z = -0.002, hardcoded)
// The panel must stay BELOW 0.001 so the menu-item button's focus
// background — drawn by C3DActiveText at exactly -0.001 — lands ON
// TOP of the panel and is visible.
#define OPT_DLG_BORDER_ZBIAS   0.0006
#define OPT_DLG_PANEL_ZBIAS    0.0008

// ===========================================================================
// Layout — Mic Test modal meter (a single horizontal bar centred in the panel)
// ===========================================================================

// Sits below the body line, above the Close button (which uses
// OPT_DLG_BTN_Y_1 so the meter has room).  Centred 80% of the panel width.
#define OPT_MICTEST_METER_X    (OPT_DLG_X + 0.10)
#define OPT_MICTEST_METER_W    (OPT_DLG_W - 0.20)
// Sits in the gap between body (ends ~0.51) and Close button (starts at
// OPT_DLG_BTN_Y_1 = 0.61).
#define OPT_MICTEST_METER_Y    (OPT_DLG_Y + 0.23)
#define OPT_MICTEST_METER_H    0.04
#define OPT_MICTEST_PEAK_W     0.005
// Track/Fill/Peak inherit OptList*'s zBias = OPT_LIST_BAR_ZBIAS (0.0001),
// designed for the bare notebook.  Inside a modal, the panel sits at
// OPT_DLG_PANEL_ZBIAS (0.0008) and would cover the bar.  Push the meter
// in front of the panel — same idea as the modal Border/Panel zBias
// staircase.
#define OPT_MICTEST_METER_ZBIAS 0.0010

#endif
