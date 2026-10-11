// Uslu dur! main menu: Guerrilla Mode's front door, not Operation
// Flashpoint's. No BIS/CWA logo, no Missions/Campaign entries, GUERRILLA is
// the prominent primary action.
//
// One file for both install targets (issue #67), installed to bin/ by
// setup-guerrilla.ps1:
//   * Classic 1.99: our own bin/resource-extra.cpp (guerrilla-mode/runtime/
//     classic) includes it.
//   * Official Remaster: setup appends `#include "ud-mainmenu.hpp"` to
//     Bohemia's bin/resource-extra.cpp and changes nothing else there.
// The engine reads exactly one bin/resource-extra.cpp (MergeBaseResourceExtra,
// Asset/Addon/ConfigParsers.cpp) and parses it as ONE ParamFile, includes and
// all. On the Remaster that parse has already seen Bohemia's mainMenu.hpp,
// which defines the `class RscDisplayMain {};` placeholder and its own
// RscDisplayMainRemaster. Defining either name a second time in the same
// parse is a "Member already defined" error, so:
//   * the menu class is RscDisplayMainUD, which DisplayMain prefers over
//     RscDisplayMainRemaster and plain RscDisplayMain
//     (SelectMainMenuResourceClass, UI/MainMenuLayout.cpp);
//   * the placeholder below is skipped when Bohemia's mainMenu.hpp (include
//     guard MAIN_MENU_HPP) has already declared it.
// Bohemia's RscDisplayMainRemaster stays in Res untouched; it is simply not
// the class the menu loads while ours is present.
//
// Do NOT rename this file to mainMenu.hpp or splashLogo.hpp: setup treats
// those names as the Remaster layout markers, so a Classic rerun would be
// taken for a Remaster install.
//
// Same "locked base, fresh top-level class" trick as Bohemia's mainMenu.hpp:
// RscDisplayMain is locked (access=PAReadOnlyVerified), so
// ParamClass::Update() silently rejects in-place edits to its nested
// classes. A fresh class inheriting it is accepted as a new Res entry, and
// the merge rebinds the placeholder base to the real (locked) RscDisplayMain.
//
// controls[] is REPLACED (not merged) on inherit, so listing only the
// entries below is sufficient to drop the rest: "CWA" (BIS/CWA logo
// picture), "Continue" (always force-disabled in C++ regardless, and only
// meaningful for campaign saves), "Game" (Campaigns), "SingleMission"
// (Missions), "Custom"/"AllMissions" (mission-editor entries, already
// hidden in shipping builds). "Line1"/"Line2"/"Player"/"Multiplayer"/
// "Options"/"Quit"/"Version" are untouched locked nested classes of
// RscDisplayMain and instantiate at their original geometry since we don't
// redefine them here. No "Mods" entry: DisplayMain injects a MODS button
// cloned from Quit whenever the loaded menu lacks one (UI/OptionsUIApp.cpp),
// which keeps Bohemia's mods screen reachable on the Remaster.
//
// The Guerrilla button itself must be fully self-specified (not inherited
// from a locked sibling like "Game") because this file is first parsed
// standalone into a temp ParamFile, where RscDisplayMain is only the empty
// placeholder, and merged into the real Res afterwards; a locked sibling
// class isn't resolvable yet at that parse pass. Geometry is hand-placed in
// the open middle of the screen (above the Multiplayer/Options/Quit stack,
// below the Player nameplate) with a larger sizeEx than the standard buttons
// so it reads as the primary call to action. Mirrors the MODS button's
// runtime-injection style/sounds (UI/OptionsUIApp.cpp) for a consistent A/V
// feel; idc matches IDC_MAIN_GUERRILLA (UI/Guerrilla/GuerrillaModule.hpp) so
// GameModuleRegistry dispatch (DisplayMain::OnButtonClicked) picks it up
// with no further wiring.
//
// No stringtable key yet (matches the TODO in OptionsUIApp.cpp's runtime
// injection): Classic's STRINGTABLE.CSV predates this menu and a raw
// literal avoids a blind edit to that CSV's mixed-codepage rows.

#ifndef UD_MAINMENU_HPP
#define UD_MAINMENU_HPP

#ifndef MAIN_MENU_HPP
class RscDisplayMain {};
#endif

class RscDisplayMainUD: RscDisplayMain
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
    // direct single-mission launch (no island/faction screen; the missions'
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

#endif
