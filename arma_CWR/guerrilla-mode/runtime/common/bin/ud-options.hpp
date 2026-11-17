// Uslu dur! Options additions for the official Remaster (issue #67 follow-up).
//
// Installed to bin/ by setup-guerrilla.ps1. In Remaster mode setup keeps
// Bohemia's bin/optionsShell.hpp byte-identical and appends
// `#include "ud-options.hpp"` to Bohemia's bin/resource-extra.cpp (after its
// own optionsShell.hpp include), so everything below is parsed in the same
// ParamFile as Bohemia's Options classes and may inherit them.
//
// Why a new class name: a second `class RscOptionsPageControls` in that parse
// is a "Member already defined" error (ParamFile parse rule), the same reason
// the UD main menu is RscDisplayMainUD (see ud-mainmenu.hpp). ControlsPage
// (UI/Options/ControlsPage.cpp, SelectControlsPageResourceClass) mounts
// RscOptionsPageControlsUD when Res has it and the stock
// RscOptionsPageControls otherwise. The Classic shim's stock page already
// carries the Arma 3 row, so the Classic resource-extra.cpp does not include
// this file.
//
// controls[] is REPLACED (not merged) on inherit, so the full row list is
// repeated. The six Bohemia rows resolve through the base class (nested
// lookup walks base classes); only the UD row and the row it pushes down are
// defined here. Row macros (OPT_ROW_n_Y) and OptIdxMenuItem come from
// Bohemia's optionsUI.hpp / optionsTemplates.hpp, already included.
//
// Do NOT name this file optionsShell.hpp, mainMenu.hpp or splashLogo.hpp:
// setup treats those names as Remaster layout markers.

// Controls section index: Bohemia's six rows plus the Arma 3 controls preset
// (idc 1407, handled by ControlsPage::OnNav: confirm page, then
// ContextControlsConfig::ApplyArma3Preset). Close moves down one row.
class RscOptionsPageControlsUD: RscOptionsPageControls
{
    controls[]={"NavKbm","NavMouse","NavGamepad","NavGamepadTuning","NavResetAll","NavArma3","NavClose"};

    class NavArma3: OptIdxMenuItem
    {
        idc=1407;
        y=OPT_ROW_5_Y;
        text="$STR_DISP_OPT_CTL_ARMA3";
    };
    class NavClose: OptIdxMenuItem
    {
        idc=1404;
        y=OPT_ROW_6_Y;
        text="$STR_DISP_CLOSE";
    };
};
