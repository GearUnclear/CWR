// PoseidonOptionsTest — chrome templates shared across every Options
// display.  The bg strips, title text, bottom-strip action buttons (Close
// / OK / Cancel) and the 3D notebook host are the same on every screen
// — they live here so per-display .hpp files declare only their
// display-specific controls (menu items, sliders, toggles, …).
//
// Each per-display file `#include`s this header and additionally pulls
// in any display-specific templates it needs.  The constants this
// header references come from `optionsUI.hpp`, included below.
//
// Same PreprocC gotcha as the constants header: trailing `//` comments
// after a class member would land in the next token, so keep
// documentation above each member or in its own line.

#ifndef OPTIONS_TEST_TEMPLATES_HPP
#define OPTIONS_TEST_TEMPLATES_HPP

#include "optionsUI.hpp"

// Background strip — semi-transparent dark band along the top or bottom
// of the screen.  Two instances per display (Background1 / Background2).
class OptTplBgStrip
{
	access=ACCESS_LOCKED;
	type=CT_STATIC;
	idc=-1;
	style=ST_LEFT;
	text="";
	colorBackground[]=OPT_C_STRIP_BG;
	colorText[]=OPT_C_TRANSPARENT;
	font=OPT_FONT_ACTION;
	sizeEx=0;
};

// Title text — centered SteelfishB128 in white on the top strip.
// Per-display files override only `text` and the placement.
class OptTplTitleText
{
	access=ACCESS_LOCKED;
	type=CT_STATIC;
	idc=-1;
	style=ST_CENTER;
	text="";
	colorBackground[]=OPT_C_TRANSPARENT;
	colorText[]=OPT_C_WHITE;
	font=OPT_FONT_TITLE;
	sizeEx=OPT_TITLE_SIZE;
};

// Bottom-strip action button — white active text matching the production
// main-menu Close / OK / Cancel style.  colorActive pinned to color so
// neither hover nor focus swaps it; the visible focus signal is the
// engine's IsFocused underline (drawn at y + h, so close-fitting h
// keeps it tight under the glyphs).  Reused for Close on the index,
// OK + Cancel on settings pages.
class OptTplActionText
{
	access=ACCESS_LOCKED;
	type=CT_ACTIVETEXT;
	style=ST_CENTER;
	color[]=OPT_C_WHITE;
	colorActive[]=OPT_C_WHITE;
	font=OPT_FONT_ACTION;
	sizeEx=OPT_CLOSE_SIZE;
	soundEnter[]=UI_SOUND_HOVER;
	soundPush[]=UI_SOUND_SILENT;
	soundClick[]=UI_SOUND_CLICK;
	soundEscape[]=UI_SOUND_CANCEL;
	default=0;
};

// Flat-menu nav item — green CRT text on the notebook surface.
// colorActive pinned to color (no hover-only colour swap) so the
// visible focus signal is the engine's colorFocusBg fill drawn behind
// the text plus the underline drawn at IsFocused.  Used by every
// flat-menu page (Index, Graphics, Controls, ...).
//
// X / W / H baked in — every flat menu uses the same row geometry
// (a centred label sweeping the notebook width).  Pages declare only
// `idc=N; y=OPT_ROW_K_Y; text="…";` per row.
class OptIdxMenuItem
{
	access=ACCESS_LOCKED;
	type=CT_3DACTIVETEXT;
	style=ST_CENTER;
	color[]=OPT_C_CRT_GREEN;
	colorActive[]=OPT_C_CRT_GREEN;
	colorFocusBg[]=OPT_C_CRT_BG;
	font=OPT_FONT_CRT;
	angle=0;
	selection="display";
	soundEnter[]=UI_SOUND_HOVER;
	soundPush[]=UI_SOUND_SILENT;
	soundClick[]=UI_SOUND_CLICK;
	soundEscape[]=UI_SOUND_CANCEL;
	default=0;
	x=OPT_ROW_X;
	w=OPT_ROW_W;
	h=OPT_ROW_H;
};

// Modal dialog chrome — rendered ON the notebook screen surface in 3D
// so the panel, border and text move with the laptop pose and read as
// part of the same green CRT UI as the rest of the page underneath.
// Shared across every dialog (PressKey, PressButton, Confirm,
// ConfirmRevert).

// Bright-green outer frame.  ST_BACKGROUND draws bgColor as a filled
// rectangle plus the four edge lines in `color`; using the same green
// for both produces a solid green block that the dark inner panel
// overlays — the visible green frame is whatever remains uncovered.
//
// Geometry baked in — every modal uses the same notebook-UV rectangle
// for its outer frame.  Modal classes drop in `class Border:
// OptTplDlgBorder {};` with no overrides.
class OptTplDlgBorder
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	idc=-1;
	style=ST_BACKGROUND;
	selection="display";
	angle=0;
	text="";
	color[]=OPT_C_CRT_GREEN;
	colorBackground[]=OPT_C_CRT_GREEN;
	font=OPT_FONT_CRT;
	size=0.001;
	zBias=OPT_DLG_BORDER_ZBIAS;
	x=OPT_DLG_X;
	y=OPT_DLG_Y;
	w=OPT_DLG_W;
	h=OPT_DLG_H;
};

// Dark interior panel — black-with-a-green-tint, opaque so the parent
// page's content underneath is fully covered inside the dialog area.
class OptTplDlgPanel
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	idc=-1;
	style=ST_BACKGROUND;
	selection="display";
	angle=0;
	text="";
	color[]=OPT_C_TRANSPARENT;
	colorBackground[]={0.01,0.04,0.01,1.0};
	font=OPT_FONT_CRT;
	size=0.001;
	zBias=OPT_DLG_PANEL_ZBIAS;
	x=OPT_DLG_PANEL_X;
	y=OPT_DLG_PANEL_Y;
	w=OPT_DLG_PANEL_W;
	h=OPT_DLG_PANEL_H;
};

// Title text — bright green, single-line prompt asking for the next
// input.  Short copy lets the text render larger so it doesn't look
// dwarfed by the action rows below.
class OptTplDlgTitle
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	idc=-1;
	style=ST_CENTER;
	lines=1;
	selection="display";
	angle=0;
	text="";
	color[]=OPT_C_CRT_GREEN;
	colorBackground[]=OPT_C_TRANSPARENT;
	font=OPT_FONT_CRT;
	size=OPT_DLG_TITLE_3D_SIZE;
	x=OPT_DLG_TEXT_X;
	y=OPT_DLG_TEXT_Y;
	w=OPT_DLG_TEXT_W;
	h=OPT_DLG_TEXT_H;
};

// Body / status text — one short centered line under the prompt.
// Kept generic so capture pages can stay translation-friendly and not
// depend on page-specific labels.
class OptTplDlgBody
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	idc=-1;
	style=ST_CENTER;
	lines=1;
	selection="display";
	angle=0;
	text="";
	color[]={0.45,1,0.45,0.95};
	colorBackground[]=OPT_C_TRANSPARENT;
	font=OPT_FONT_CRT;
	size=OPT_DLG_BODY_3D_SIZE;
	x=OPT_DLG_TEXT_X;
	y=OPT_DLG_COUNT_Y;
	w=OPT_DLG_TEXT_W;
	h=OPT_DLG_TEXT_H;
};

// Modal button — full-width menu-item row, same visual + interaction
// model as the flat-menu nav rows (Index, Graphics, Controls).
// Inherits everything from OptIdxMenuItem (font, sounds, colorFocusBg
// green fill on focus, ST_CENTER text); only x/w/h are overridden so
// the row sits inside the modal panel instead of spanning the full
// notebook width.
//
// Three vertical slots at OPT_DLG_BTN_Y_{0,1,2} stacked at the same
// pitch as flat-menu rows.  2-button modals use slots 0+1, 3-button
// modals use all three.
class OptTplDlgBtn: OptIdxMenuItem
{
	x=OPT_DLG_TEXT_X;
	w=OPT_DLG_TEXT_W;
	h=OPT_DLG_BTN_H;
};

class OptTplDlgBtn0: OptTplDlgBtn { y=OPT_DLG_BTN_Y_0; };
class OptTplDlgBtn1: OptTplDlgBtn { y=OPT_DLG_BTN_Y_1; };
class OptTplDlgBtn2: OptTplDlgBtn { y=OPT_DLG_BTN_Y_2; };

// 3D laptop-style notebook hosting the per-display sub-controls.
// Pose values mirror legacy RscDisplayOptions.Notebook
// (resource_orig/resource.cpp:10450) so the notebook lands in the
// same place within the production main-menu scene; but the
// auto-zoom + auto-open animation is skipped (autoOpen=0,
// autoZoom=0, inBack=0, animPhase=1) so the notebook is fully open
// immediately on Mount.  This differs from the legacy fly-in
// animation which was visually nicer but exposes a pre-existing
// engine access-violation bug during the auto-open simulate loop
// (documented in production_options_capture.test.sqf) — the bug
// crashes the game ~100 frames in, before the sub-controls render,
// which makes anything beyond the opening flourish untestable.
// Skipping the animation is a polish regression we can revisit
// once the underlying engine bug is fixed.
class OptTplNotebook
{
	access=ACCESS_LOCKED;
	type=CT_OBJECT_CONT_ANIM;
	model="notebook.p3d";
	animation="notebook.rtm";
	// Match the legacy RscDisplayLogin (profile picker) notebook setup
	// 1:1 — same model/anim, same pose, same speed.  Keeps the open/close
	// flourish consistent across every notebook screen in the game instead
	// of standing out as a faster, larger, slightly-flatter outlier.
	autoOpen=1;
	autoZoom=1;
	animSpeed=1;
	position[]={0,-0.175000,0.250000};
	direction[]={0,"sin 30","cos 30"};
	up[]={0,"cos 30","-sin 30"};
	positionBack[]={0,-0.040000,0.600000};
	inBack=1;
	enableZoom=0;
	zoomDuration=1;
	scale=1;
};

#endif
