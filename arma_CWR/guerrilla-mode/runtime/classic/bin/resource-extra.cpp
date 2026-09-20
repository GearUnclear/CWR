// Main-menu + Options-screen override for the Classic 1.99 data install,
// merged on top of the locked vanilla RESOURCE.BIN by the ParseResource hook
// (see engine/Poseidon/Asset/Addon/ConfigParsers.cpp) exactly like the
// Remaster Demo package's own BIN/resource-extra.cpp. Goal: Uslu dur!'s main
// menu is Guerrilla Mode's front door, not Operation Flashpoint's — no
// BIS/CWA logo, no Missions/Campaign entries, GUERRILLA is the prominent
// primary action.
//
// optionsShell.hpp (+ its optionsTemplates/optionsScrollList/optionsUI
// dependency chain, copied verbatim from the Demo package's BIN/) supplies
// RscOptionsShell and every RscOptionsPage* class OptionsShell::Load() and
// OptionsPage::MountFromClass() (UI/Options/OptionsShell.cpp,
// UI/Options/OptionsPage.cpp) look up by name. Classic's locked
// RESOURCE.BIN predates the remaster Options rework and never defined
// these, so without this include DisplayMain::OnButtonClicked's
// IDC_MAIN_OPTIONS case opened an OptionsShell with nothing to mount — the
// Options button appeared to do nothing. The $STR_DISP_MAIN_OPT_*/
// $STR_DISP_OPT_* keys these pull in come from the sibling
// STRINGTABLE_OPTIONS.utf8.csv shard (auto-discovered by
// UI/Locale/Stringtable/Stringtable.cpp's sibling-shard scan — same
// mechanism as the Demo's STRINGTABLE_MAINMENU.utf8.csv), not the legacy
// mixed-codepage STRINGTABLE.CSV, so this needed no edit to that file.
#include "optionsShell.hpp"
//
// Same "locked base, fresh top-level class" trick documented in the Demo
// package's BIN/mainMenu.hpp: RscDisplayMain is locked (access=
// PAReadOnlyVerified), so ParamClass::Update() would silently reject any
// in-place edits to its nested classes. Inheriting into a fresh
// RscDisplayMainRemaster class is accepted as a new Res entry, and
// DisplayMain::DisplayMain() (UI/OptionsUIApp.cpp) already prefers
// "RscDisplayMainRemaster" over "RscDisplayMain" whenever the former is
// present in the merged Res — no C++ change needed to pick this menu up.
//
// controls[] is REPLACED (not merged) on inherit, so listing only the
// entries below is sufficient to drop the rest: "CWA" (BIS/CWA logo
// picture), "Continue" (always force-disabled in C++ regardless, and only
// meaningful for campaign saves), "Game" (Campaigns), "SingleMission"
// (Missions), "Custom"/"AllMissions" (mission-editor entries, already
// hidden in shipping builds). "Line1"/"Line2"/"Player"/"Multiplayer"/
// "Options"/"Quit"/"Version" are untouched locked nested classes of
// RscDisplayMain and instantiate at their original geometry since we don't
// redefine them here.
//
// The Guerrilla button itself must be fully self-specified (not inherited
// from a locked sibling like "Game") because this file is first parsed
// standalone into a temp ParamFile — where RscDisplayMain is only the empty
// placeholder below — and merged into the real Res afterwards; a locked
// sibling class isn't resolvable yet at that parse pass. Geometry is
// hand-placed in the open middle of the screen (above the Multiplayer/
// Options/Quit stack, below the Player nameplate) with a larger sizeEx than
// the standard buttons so it reads as the primary call to action. Mirrors
// the MODS button's runtime-injection style/sounds (UI/OptionsUIApp.cpp)
// for a consistent A/V feel; idc matches IDC_MAIN_GUERRILLA (UI/Guerrilla/
// GuerrillaModule.hpp) so GameModuleRegistry dispatch (DisplayMain::
// OnButtonClicked) picks it up with no further wiring.
//
// No stringtable key yet (matches the TODO in OptionsUIApp.cpp's runtime
// injection) — Classic's STRINGTABLE.CSV predates this menu and a raw
// literal avoids a blind edit to that CSV's mixed-codepage rows.

class RscDisplayMain {};

class RscDisplayMainRemaster: RscDisplayMain
{
    controls[]=
    {
        "Line1",
        "Line2",
        "Player",
        "Guerrilla",
        "Showcase",
        "Undercover",
        "Qrf",
        "Multiplayer",
        "Options",
        "Quit",
        "Version"
    };

    class Guerrilla
    {
        type=11;            // CT_ACTIVETEXT
        style=2;             // ST_CENTER
        color[]={1,1,1,1};
        colorActive[]={1,0,0,1};
        font="SteelfishB128";
        sizeEx=0.090000;      // larger than the standard ~0.0588 button text
        soundEnter[]={"ui\ui_over", 0.2, 1};
        soundPush[]={"", 0.2, 1};
        soundClick[]={"ui\ui_ok", 0.2, 1};
        soundEscape[]={"ui\ui_cc", 0.2, 1};
        idc=120;              // IDC_MAIN_GUERRILLA
        default=1;
        x=0.250000;
        y=0.340000;
        w=0.500000;
        h=0.110000;
        text="GUERRILLA WARFIGHTER";
    };

    // Reference-mission direct launches (the human-playable test slices from
    // guerrilla-mode/mission/, installed by install-missions.ps1). The idcs
    // match kReferenceMissions in UI/OptionsUIApp.cpp, which hides either
    // button when its mission is not installed and dispatches the click as a
    // direct single-mission launch (no island/faction screen — the missions'
    // own description.ext defaults apply). Placed as a smaller four-across
    // row tucked under the GUERRILLA WARFIGHTER banner (x 0.25..0.75, the
    // banner's own span; Options starts at x 0.48 / y 0.54, below this row).
    class Showcase
    {
        type=11;            // CT_ACTIVETEXT
        style=2;             // ST_CENTER
        color[]={1,1,1,1};
        colorActive[]={1,0,0,1};
        font="SteelfishB128";
        sizeEx=0.045000;
        soundEnter[]={"ui\ui_over", 0.2, 1};
        soundPush[]={"", 0.2, 1};
        soundClick[]={"ui\ui_ok", 0.2, 1};
        soundEscape[]={"ui\ui_cc", 0.2, 1};
        idc=123;              // IDC_MAIN_REF_SHOWCASE
        x=0.250000;
        y=0.452000;
        w=0.120000;
        h=0.050000;
        text="SHOWCASE";
    };
    class Undercover
    {
        type=11;            // CT_ACTIVETEXT
        style=2;             // ST_CENTER
        color[]={1,1,1,1};
        colorActive[]={1,0,0,1};
        font="SteelfishB128";
        sizeEx=0.045000;
        soundEnter[]={"ui\ui_over", 0.2, 1};
        soundPush[]={"", 0.2, 1};
        soundClick[]={"ui\ui_ok", 0.2, 1};
        soundEscape[]={"ui\ui_cc", 0.2, 1};
        idc=124;              // IDC_MAIN_REF_UNDERCOVER
        x=0.375000;
        y=0.452000;
        w=0.120000;
        h=0.050000;
        text="UNDERCOVER";
    };
    class Qrf
    {
        type=11;            // CT_ACTIVETEXT
        style=2;             // ST_CENTER
        color[]={1,1,1,1};
        colorActive[]={1,0,0,1};
        font="SteelfishB128";
        sizeEx=0.045000;
        soundEnter[]={"ui\ui_over", 0.2, 1};
        soundPush[]={"", 0.2, 1};
        soundClick[]={"ui\ui_ok", 0.2, 1};
        soundEscape[]={"ui\ui_cc", 0.2, 1};
        idc=125;              // IDC_MAIN_REF_QRF
        x=0.500000;
        y=0.452000;
        w=0.120000;
        h=0.050000;
        text="QRF";
    };
    class Market
    {
        type=11;            // CT_ACTIVETEXT
        style=2;             // ST_CENTER
        color[]={1,1,1,1};
        colorActive[]={1,0,0,1};
        font="SteelfishB128";
        sizeEx=0.045000;
        soundEnter[]={"ui\ui_over", 0.2, 1};
        soundPush[]={"", 0.2, 1};
        soundClick[]={"ui\ui_ok", 0.2, 1};
        soundEscape[]={"ui\ui_cc", 0.2, 1};
        idc=126;              // IDC_MAIN_REF_MARKET
        x=0.625000;
        y=0.452000;
        w=0.120000;
        h=0.050000;
        text="MARKET";
    };
};
