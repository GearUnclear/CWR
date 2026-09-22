// PoseidonOptionsTest — scrolling key/value list shared resource.
//
// Hosts every settings page that uses OptionsScrollList (Audio,
// Graphics, Controls, Network, ...).  Each page resource looks like:
//
//   class RscDisplayOptionsTestX
//   {
//       idd=...;
//       controls[]={"Background1","Title"};
//       objects[]={"Notebook"};
//       class Background1: OptTplBgStrip   { ... };
//       class Title:       OptTplTitleText { ... text="X"; };
//       class Notebook:    OptTplNotebook
//       {
//           idc=105;
//           #include "optionsTestScrollListSlots.hpp"
//       };
//   };
//
// The slot block (controls[] + 70 sub-control classes + RowBgs + Hint
// + ScrollBar) is identical across pages — it lives in
// optionsTestScrollListSlots.hpp.  Page-specific row data flows from
// C++ via OptionsScrollList::Provider, not from the resource.

#ifndef OPTIONS_TEST_SCROLL_LIST_HPP
#define OPTIONS_TEST_SCROLL_LIST_HPP

#include "optionsTemplates.hpp"

// ===========================================================================
// Slot widget templates — used by every list page.  Audio was the
// first consumer; templates were renamed OptAud* → OptList* once a
// second consumer (Graphics) confirmed they're page-agnostic.
// ===========================================================================

// 3D label on the notebook surface.  Default style ST_CENTER (the
// hint uses it as-is); per-row labels override to ST_LEFT, value
// readouts override to ST_LEFT with a smaller `size`, etc.
class OptListLabel
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	idc=-1;
	style=ST_CENTER;
	color[]=OPT_C_CRT_GREEN;
	font=OPT_FONT_CRT;
	angle=0;
	selection="display";
};

// Row Bg highlight — translucent green fill + matching border.  One
// instance per visible slot; the runtime ShowCtrls exactly one to mark
// the focused row.
class OptListRowBgFocus
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	style=ST_BACKGROUND;
	selection="display";
	angle=0;
	colorBackground[]=OPT_C_CRT_BG;
	color[]=OPT_C_CRT_BORDER;
	font=OPT_FONT_CRT;
	size=0.001;
	text="";
};

// Slider track — dim green ST_BACKGROUND fill, transparent outline so
// the bar's right edge doesn't draw through.  zBias pushes the fill
// slightly camera-forward to win depth tie with the row Bg's fill.
class OptListBarTrack
{
	access=ACCESS_LOCKED;
	type=CT_3DSTATIC;
	style=ST_BACKGROUND;
	selection="display";
	angle=0;
	colorBackground[]=OPT_C_BAR_TRACK;
	color[]=OPT_C_TRANSPARENT;
	font=OPT_FONT_CRT;
	size=0.001;
	text="";
	zBias=OPT_LIST_BAR_ZBIAS;
};

// Slider fill — bright green; runtime sets w from 0..track-w to
// reflect the current slider position.
class OptListBarFill: OptListBarTrack
{
	colorBackground[]=OPT_C_BAR_FILL;
};

// VU peak-hold tick — narrow yellow accent that trails the live bar.
class OptListBarPeak: OptListBarTrack
{
	colorBackground[]=OPT_C_BAR_PEAK;
};

// Invisible full-row click zone — receives mouse clicks for any row
// the cursor lands on.  Both color and colorActive transparent so the
// row content underneath always renders identically.
class OptListRowClick
{
	access=ACCESS_LOCKED;
	type=CT_3DACTIVETEXT;
	style=ST_LEFT;
	color[]=OPT_C_TRANSPARENT;
	colorActive[]=OPT_C_TRANSPARENT;
	font=OPT_FONT_CRT;
	angle=0;
	selection="display";
	soundEnter[]=UI_SOUND_SILENT;
	soundClick[]=UI_SOUND_CLICK;
	soundEscape[]=UI_SOUND_SILENT;
	soundPush[]=UI_SOUND_SILENT;
	default=0;
};

// Visible "<" / ">" chevron — green at rest, yellow on hover.
class OptListChevron
{
	access=ACCESS_LOCKED;
	type=CT_3DACTIVETEXT;
	style=ST_CENTER;
	color[]=OPT_C_CHEVRON;
	colorActive[]=OPT_C_CHEVRON_ACTIVE;
	drawFocusLine=0;
	font=OPT_FONT_CRT;
	angle=0;
	selection="display";
	soundEnter[]=UI_SOUND_HOVER;
	soundClick[]=UI_SOUND_CLICK;
	soundEscape[]=UI_SOUND_SILENT;
	soundPush[]=UI_SOUND_SILENT;
	default=0;
};

// Notebook scrollbar — single CT_3DSCROLLBAR.
class OptListScrollBar
{
	access=ACCESS_LOCKED;
	type=CT_3DSCROLLBAR;
	style=0;
	color[]=OPT_C_CRT_GREEN;
	angle=0;
	selection="display";
};

#endif
