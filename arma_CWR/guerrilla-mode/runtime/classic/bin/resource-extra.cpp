// Main-menu + Options-screen override for the Classic 1.99 data install,
// merged on top of the locked vanilla RESOURCE.BIN by the ParseResource hook
// (see engine/Poseidon/Asset/Addon/ConfigParsers.cpp) exactly like the
// official Remaster package's own BIN/resource-extra.cpp.
//
// Classic only. On the official Remaster, setup-guerrilla.ps1 keeps
// Bohemia's resource-extra.cpp and Options headers and appends the
// ud-mainmenu.hpp include to their file instead (issue #67).
//
// optionsShell.hpp (+ its optionsTemplates/optionsScrollList/optionsUI
// dependency chain, copied verbatim from the Demo package's BIN/) supplies
// RscOptionsShell and every RscOptionsPage* class OptionsShell::Load() and
// OptionsPage::MountFromClass() (UI/Options/OptionsShell.cpp,
// UI/Options/OptionsPage.cpp) look up by name. Classic's locked
// RESOURCE.BIN predates the remaster Options rework and never defined
// these, so without this include DisplayMain::OnButtonClicked's
// IDC_MAIN_OPTIONS case opened an OptionsShell with nothing to mount: the
// Options button appeared to do nothing. The $STR_DISP_MAIN_OPT_*/
// $STR_DISP_OPT_* keys these pull in came from the sibling
// STRINGTABLE_OPTIONS.utf8.csv shard (auto-discovered by
// UI/Locale/Stringtable/Stringtable.cpp's sibling-shard scan, the same
// mechanism as the Remaster's STRINGTABLE_MAINMENU.utf8.csv), not the legacy
// mixed-codepage STRINGTABLE.CSV, so this needed no edit to that file. That
// shard and the copied Options headers were retired with Classic support on
// 2026-10-26; UD's own Options strings now live in the GPL
// common/bin/STRINGTABLE_UD_OPTIONS.utf8.csv. This shim is historical.
#include "optionsShell.hpp"

// The UD main menu (RscDisplayMainUD) lives in its own file so the same
// menu installs beside Bohemia's files on the Remaster. See its header.
#include "ud-mainmenu.hpp"
