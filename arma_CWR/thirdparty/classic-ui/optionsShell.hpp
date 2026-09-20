#include "optionsTemplates.hpp"
#include "optionsScrollList.hpp"

class RscOptionsShell
{
    idd=9099;
    movingEnable=0;
    objects[]={"Notebook"};
    controls[]={"Background1","Title"};

    class Background1: OptTplBgStrip
    {
        x=OPT_STRIP_X;
        y=OPT_STRIP_TOP_Y;
        w=OPT_STRIP_W;
        h=OPT_STRIP_H;
    };
    class Title: OptTplTitleText
    {
        idc=100;
        x=OPT_TITLE_X;
        y=OPT_TITLE_Y;
        w=OPT_TITLE_W;
        h=OPT_TITLE_H;
        text="";
    };

    class Notebook: OptTplNotebook
    {
        idc=105;
        controls[]={};
    };
};

class RscOptionsPageIndex
{
    controls[]={"NavAudio","NavDisplay","NavGraphics","NavGame","NavControls","NavDifficulty","NavCredits","NavClose"};

    class NavAudio: OptIdxMenuItem
    {
        idc=1101;
        y=OPT_ROW_0_Y;
        text="$STR_DISP_MAIN_OPT_AUDIO";
    };
    class NavDisplay: OptIdxMenuItem
    {
        idc=1102;
        y=OPT_ROW_1_Y;
        text="$STR_DISP_MAIN_OPT_DISPLAY";
    };
    class NavGraphics: OptIdxMenuItem
    {
        idc=1103;
        y=OPT_ROW_2_Y;
        text="$STR_DISP_MAIN_OPT_GRAPHICS";
    };
    class NavGame: OptIdxMenuItem
    {
        idc=1104;
        y=OPT_ROW_3_Y;
        text="$STR_DISP_MAIN_OPT_GAME";
    };
    class NavControls: OptIdxMenuItem
    {
        idc=1107;
        y=OPT_ROW_4_Y;
        text="$STR_DISP_OPT_CONTROLS";
    };
    class NavDifficulty: OptIdxMenuItem
    {
        idc=1108;
        y=OPT_ROW_5_Y;
        text="$STR_DISP_OPTIONS_DIFFICULTY";
    };
    class NavCredits: OptIdxMenuItem
    {
        idc=1105;
        y=OPT_ROW_6_Y;
        text="$STR_DISP_MAIN_CREDITS";
    };
    class NavClose: OptIdxMenuItem
    {
        idc=1106;
        y=OPT_ROW_7_Y;
        text="$STR_DISP_CLOSE";
    };
};

class RscOptionsPageControls
{
    controls[]={"NavKbm","NavMouse","NavGamepad","NavGamepadTuning","NavResetAll","NavClose"};

    class NavKbm: OptIdxMenuItem
    {
        idc=1401;
        y=OPT_ROW_0_Y;
        text="$STR_DISP_OPT_CTL_KBM";
    };
    class NavMouse: OptIdxMenuItem
    {
        idc=1402;
        y=OPT_ROW_1_Y;
        text="$STR_DISP_OPT_CTL_MOUSE";
    };
    class NavGamepad: OptIdxMenuItem
    {
        idc=1405;
        y=OPT_ROW_2_Y;
        text="$STR_DISP_OPT_CTL_GAMEPAD";
    };
    class NavGamepadTuning: OptIdxMenuItem
    {
        idc=1406;
        y=OPT_ROW_3_Y;
        text="$STR_DISP_OPT_CTL_GAMEPAD_TUNING";
    };
    class NavResetAll: OptIdxMenuItem
    {
        idc=1403;
        y=OPT_ROW_4_Y;
        text="$STR_DISP_OPT_CTL_RESET_ALL";
    };
    class NavClose: OptIdxMenuItem
    {
        idc=1404;
        y=OPT_ROW_5_Y;
        text="$STR_DISP_CLOSE";
    };
};

class RscOptionsPageDifficulty
{
    controls[]={"HdrName","HdrCadet","HdrVeteran","ValueDifficulties","ButtonDefault","ButtonClose"};

    class DiffHeader: OptListLabel
    {
        color[]=OPT_C_HINT;
        size=0.060;
        h=0.05;
    };
    class DiffHeaderCenter: DiffHeader
    {
        style=ST_CENTER;
        color[]=OPT_C_CRT_GREEN;
    };
    class DiffAction: OptIdxMenuItem
    {
        h=0.05;
    };

    class HdrName: DiffHeader
    {
        idc=120;
        style=ST_LEFT;
        x=0.03;
        y=0.08;
        w=0.54;
        text="$STR_DISP_DIFF_NAME";
    };
    class HdrCadet: DiffHeaderCenter
    {
        idc=121;
        x=0.60;
        y=0.08;
        w=0.17;
        text="$STR_DISP_DIFF_CADET";
    };
    class HdrVeteran: DiffHeaderCenter
    {
        idc=122;
        x=0.79;
        y=0.08;
        w=0.17;
        text="$STR_DISP_DIFF_VETERAN";
    };
    class ValueDifficulties
    {
        access=ACCESS_LOCKED;
        type=CT_3DLISTBOX;
        idc=101;
        style=0;
        selection="display";
        angle=0;
        rows=11.2;
        size=0.90;
        colorSelect[]=OPT_C_CRT_GREEN;
        colorSelectBackground[]=OPT_C_CRT_BG;
        colorText[]=OPT_C_CRT_GREEN;
        font=OPT_FONT_CRT;
        x=0.03;
        y=0.14;
        w=0.94;
        h=0.56;
    };
    class ButtonDefault: DiffAction
    {
        idc=104;
        y=0.77;
        text="$STR_DISP_DEFAULT";
    };
    class ButtonClose: DiffAction
    {
        idc=2;
        y=0.85;
        text="$STR_DISP_CLOSE";
    };
};

class RscOptionsPageScrollList
{
    #include "optionsScrollListSlots.hpp"
};

class RscOptionsPageMicTest
{
    controls[]={"Border","Panel","Title","Body","MeterTrack","MeterFill","MeterPeak","Close"};

    class Border: OptTplDlgBorder
    {
        idc=9582;
    };
    class Panel: OptTplDlgPanel
    {
        idc=9583;
    };
    class Title: OptTplDlgTitle
    {
        idc=9580;
        text="$STR_DISP_MAIN_OPT_MIC_TEST_TITLE";
    };
    class Body: OptTplDlgBody
    {
        idc=9581;
        text="$STR_DISP_MAIN_OPT_MIC_TEST_BODY";
    };

    class MeterTrack: OptListBarTrack
    {
        idc=9510;
        x=OPT_MICTEST_METER_X;
        y=OPT_MICTEST_METER_Y;
        w=OPT_MICTEST_METER_W;
        h=OPT_MICTEST_METER_H;
        zBias=OPT_MICTEST_METER_ZBIAS;
    };
    class MeterFill: OptListBarFill
    {
        idc=9511;
        x=OPT_MICTEST_METER_X;
        y=OPT_MICTEST_METER_Y;
        w=0.00;
        h=OPT_MICTEST_METER_H;
        zBias=OPT_MICTEST_METER_ZBIAS;
    };
    class MeterPeak: OptListBarPeak
    {
        idc=9512;
        x=OPT_MICTEST_METER_X;
        y=OPT_MICTEST_METER_Y;
        w=OPT_MICTEST_PEAK_W;
        h=OPT_MICTEST_METER_H;
        zBias=OPT_MICTEST_METER_ZBIAS;
    };

    class Close: OptTplDlgBtn1
    {
        idc=9501;
        default=1;
        text="$STR_DISP_CLOSE";
    };
};

// Generic Yes/No confirm modal — used by destructive actions like
// "Reset all to defaults" and per-category Reset.  Default focus on No
// so a stuck Enter doesn't trigger the destructive action.
class RscOptionsPageConfirm
{
    controls[]={"Border","Panel","Title","Body","Yes","No"};

    class Border: OptTplDlgBorder {};
    class Panel:  OptTplDlgPanel  {};
    class Title:  OptTplDlgTitle  { idc=9180; };
    class Body:   OptTplDlgBody   { idc=9181; };

    class Yes: OptTplDlgBtn0 { idc=9101; default=0; text="$STR_DISP_OPT_CONFIRM_YES"; };
    class No:  OptTplDlgBtn1 { idc=9102; default=1; text="$STR_DISP_OPT_CONFIRM_NO";  };
};

// Press-key capture modal — pushed by KbmPage when a binding row is
// clicked.  Title shows the action being rebound; Status shows the
// "waiting / captured / conflict" hint.  Save / Retry are hidden in
// the Listening state and shown after a key is captured.
//
// Identical sizing to OptionsTest's tuned RscOptionsPagePressKey —
// title at OPT_DLG_TITLE_3D_SIZE (0.70), status at OPT_DLG_BODY_3D_SIZE
// (0.52); fits one short line each at OPT_DLG_TEXT_W (0.86 of screen).
// Strings come from the stringtable so localizers can adjust per locale.
class RscOptionsPagePressKey
{
    controls[]={"Border","Panel","Title","Status","Save","Retry","Cancel"};

    class Border: OptTplDlgBorder { idc=9382; };
    class Panel:  OptTplDlgPanel  { idc=9383; };
    class Title:  OptTplDlgTitle  { idc=9380; };
    class Status: OptTplDlgBody   { idc=9381; };

    class Save:   OptTplDlgBtn0 { idc=9301; default=0; text="$STR_DISP_OPT_CAP_SAVE";   };
    class Retry:  OptTplDlgBtn1 { idc=9303; default=0; text="$STR_DISP_OPT_CAP_RETRY";  };
    class Cancel: OptTplDlgBtn2 { idc=9302; default=1; text="$STR_DISP_OPT_CAP_CANCEL"; };
};

class RscOptionsPageConfirmRevert
{
    controls[]={"Border","Panel","Title","Countdown","Keep","Revert"};

    class Border: OptTplDlgBorder {};
    class Panel: OptTplDlgPanel {};
    class Title: OptTplDlgTitle
    {
        idc=9280;
        text="$STR_DISP_MAIN_OPT_CONFIRM_REVERT_TITLE";
    };
    class Countdown: OptTplDlgBody
    {
        idc=9281;
        text="";
    };
    class Keep: OptTplDlgBtn0
    {
        idc=9201;
        default=0;
        text="$STR_DISP_MAIN_OPT_CONFIRM_REVERT_KEEP";
    };
    class Revert: OptTplDlgBtn1
    {
        idc=9202;
        default=1;
        text="$STR_DISP_MAIN_OPT_CONFIRM_REVERT_REVERT";
    };
};
