// Slot block for OptionsScrollList pages.  #include INSIDE a Notebook
// class body that has `idc=105;` already set.  Declares:
//
//   - controls[] listing all Notebook sub-controls in declaration order
//   - 9 RowBg controls (idc 700+N) — one shown at a time as focus marker
//   - 9 slots × 10 sub-controls each (5N0..5N9)
//   - Hint label (idc=590) — per-row description, single line + marquee
//   - ScrollBar (idc=850) — CT_3DSCROLLBAR widget
//
// No #ifndef guard: this is intentional plain-text inclusion (multiple
// pages each include it inside their own Notebook block).  The
// containing optionsScrollList.hpp brings in the OptList* templates
// referenced here.
//
// Slot idc convention per slot N = 0..6 (verbatim from the harness):
//   500+10*N  Label              501+10*N  ValueStep
//   502+10*N  ValueBar           503+10*N  Hover (full-row click)
//   504+10*N  Track              505+10*N  Fill
//   506+10*N  Peak               507+10*N  BarClick
//   508+10*N  Prev (<)           509+10*N  Next (>)

controls[]={
	"Hint",
	"Row0Bg","Row1Bg","Row2Bg","Row3Bg","Row4Bg","Row5Bg","Row6Bg","Row7Bg","Row8Bg",
	"S0Label","S0ValueStep","S0ValueBar","S0Track","S0Fill","S0Peak",
	"S1Label","S1ValueStep","S1ValueBar","S1Track","S1Fill","S1Peak",
	"S2Label","S2ValueStep","S2ValueBar","S2Track","S2Fill","S2Peak",
	"S3Label","S3ValueStep","S3ValueBar","S3Track","S3Fill","S3Peak",
	"S4Label","S4ValueStep","S4ValueBar","S4Track","S4Fill","S4Peak",
	"S5Label","S5ValueStep","S5ValueBar","S5Track","S5Fill","S5Peak",
	"S6Label","S6ValueStep","S6ValueBar","S6Track","S6Fill","S6Peak",
	"S7Label","S7ValueStep","S7ValueBar","S7Track","S7Fill","S7Peak",
	"S8Label","S8ValueStep","S8ValueBar","S8Track","S8Fill","S8Peak",
	// Click overlays — order matters because the engine's
	// FindControl returns the FIRST visible+enabled control matching
	// (x,y), not the last.  ScrollBar / chevrons / bar zones must
	// come before the full-row Hover catch-all so the cursor lands
	// on the specific zone where it overlaps.
	"ScrollBar",
	"S0Prev","S0Next","S0BarClick","S0Hover",
	"S1Prev","S1Next","S1BarClick","S1Hover",
	"S2Prev","S2Next","S2BarClick","S2Hover",
	"S3Prev","S3Next","S3BarClick","S3Hover",
	"S4Prev","S4Next","S4BarClick","S4Hover",
	"S5Prev","S5Next","S5BarClick","S5Hover",
	"S6Prev","S6Next","S6BarClick","S6Hover",
	"S7Prev","S7Next","S7BarClick","S7Hover",
	"S8Prev","S8Next","S8BarClick","S8Hover"
};

// Per-slot bundle of 10 sub-controls.  Slot N owns idcs 5N0..5N9.
// rowY is the slot's row top in notebook UV (UV space, 0..1).  Macro is
// one-line because PreprocC's `\`-continuation handling in #define is
// not exercised elsewhere in this codebase — keep what's known to work.
#define OPT_ROWBG_BLOCK(N, rowY) class Row##N##Bg: OptListRowBgFocus { idc=(700 + N); x=OPT_LIST_ROW_X; y=rowY; w=OPT_LIST_ROW_W; h=OPT_LIST_ROW_H; };

#define OPT_SLOT_BLOCK(N, rowY) class S##N##Label: OptListLabel { idc=(500 + N*10 + 0); x=OPT_LIST_LABEL_X; y=rowY; w=OPT_LIST_LABEL_W; h=OPT_LIST_ROW_H; style=ST_LEFT; text=""; }; class S##N##ValueStep: OptListLabel { idc=(500 + N*10 + 1); x=OPT_LIST_VALUE_STEP_X; y=rowY; w=OPT_LIST_VALUE_STEP_W; h=OPT_LIST_ROW_H; style=ST_CENTER; size=OPT_LIST_VALUE_SIZE; text=""; }; class S##N##ValueBar: OptListLabel { idc=(500 + N*10 + 2); x=OPT_LIST_VALUE_BAR_X; y=rowY; w=OPT_LIST_VALUE_BAR_W; h=OPT_LIST_ROW_H; style=ST_LEFT; size=OPT_LIST_VALUE_SIZE; text=""; }; class S##N##Track: OptListBarTrack { idc=(500 + N*10 + 4); x=OPT_LIST_TRACK_X; y=(rowY + OPT_LIST_BAR_Y_OFFSET); w=OPT_LIST_TRACK_W; h=OPT_LIST_TRACK_H; }; class S##N##Fill: OptListBarFill { idc=(500 + N*10 + 5); x=OPT_LIST_TRACK_X; y=(rowY + OPT_LIST_BAR_Y_OFFSET); w=0.00; h=OPT_LIST_TRACK_H; }; class S##N##Peak: OptListBarPeak { idc=(500 + N*10 + 6); x=OPT_LIST_TRACK_X; y=(rowY + OPT_LIST_BAR_Y_OFFSET); w=OPT_LIST_PEAK_W; h=OPT_LIST_TRACK_H; }; class S##N##Hover: OptListRowClick { idc=(500 + N*10 + 3); x=OPT_LIST_HOVER_X; y=rowY; w=OPT_LIST_HOVER_W; h=OPT_LIST_ROW_H; text=""; }; class S##N##BarClick: OptListRowClick { idc=(500 + N*10 + 7); x=OPT_LIST_TRACK_X; y=rowY; w=OPT_LIST_TRACK_W; h=OPT_LIST_ROW_H; text=""; }; class S##N##Prev: OptListChevron { idc=(500 + N*10 + 8); x=OPT_LIST_PREV_X; y=rowY; w=OPT_LIST_PREV_W; h=OPT_LIST_ROW_H; text="<"; }; class S##N##Next: OptListChevron { idc=(500 + N*10 + 9); x=OPT_LIST_NEXT_X; y=rowY; w=OPT_LIST_NEXT_W; h=OPT_LIST_ROW_H; text=">"; };

OPT_ROWBG_BLOCK(0, OPT_LIST_ROW_0_Y)
OPT_ROWBG_BLOCK(1, OPT_LIST_ROW_1_Y)
OPT_ROWBG_BLOCK(2, OPT_LIST_ROW_2_Y)
OPT_ROWBG_BLOCK(3, OPT_LIST_ROW_3_Y)
OPT_ROWBG_BLOCK(4, OPT_LIST_ROW_4_Y)
OPT_ROWBG_BLOCK(5, OPT_LIST_ROW_5_Y)
OPT_ROWBG_BLOCK(6, OPT_LIST_ROW_6_Y)
OPT_ROWBG_BLOCK(7, OPT_LIST_ROW_7_Y)
OPT_ROWBG_BLOCK(8, OPT_LIST_ROW_8_Y)

OPT_SLOT_BLOCK(0, OPT_LIST_ROW_0_Y)
OPT_SLOT_BLOCK(1, OPT_LIST_ROW_1_Y)
OPT_SLOT_BLOCK(2, OPT_LIST_ROW_2_Y)
OPT_SLOT_BLOCK(3, OPT_LIST_ROW_3_Y)
OPT_SLOT_BLOCK(4, OPT_LIST_ROW_4_Y)
OPT_SLOT_BLOCK(5, OPT_LIST_ROW_5_Y)
OPT_SLOT_BLOCK(6, OPT_LIST_ROW_6_Y)
OPT_SLOT_BLOCK(7, OPT_LIST_ROW_7_Y)
OPT_SLOT_BLOCK(8, OPT_LIST_ROW_8_Y)

class ScrollBar: OptListScrollBar { idc=850; x=OPT_LIST_SCROLLBAR_X; y=OPT_LIST_SCROLLBAR_Y; w=OPT_LIST_SCROLLBAR_W; h=OPT_LIST_SCROLLBAR_H; };

class Hint: OptListLabel { idc=590; x=OPT_LIST_HINT_X; y=OPT_LIST_HINT_Y; w=OPT_LIST_HINT_W; h=OPT_LIST_HINT_H; style=ST_LEFT; lines=1; size=OPT_LIST_HINT_SIZE; color[]=OPT_C_HINT; text=""; };
