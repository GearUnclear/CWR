// UD front door on the official Remaster (issue #67), no mod mounted.
// setup-guerrilla.ps1 (Remaster mode) kept every Bohemia file and appended one
// #include "ud-mainmenu.hpp" to Bohemia's bin/resource-extra.cpp. Both
// RscDisplayMainRemaster (theirs) and RscDisplayMainUD (ours) are in Res;
// DisplayMain loads ours (SelectMainMenuResourceClass).

triSetLanguage "English"
triAssertEq [(triDisplay), 0]

// -- the UD menu, not Bohemia's: GUERRILLA front door, no campaign entries ----
triAssertEq [(triControlText 120), "GUERRILLA WARFIGHTER"]
triAssertIncludes [(triVisibleTexts), "GUERRILLA WARFIGHTER"]
triAssertIncludes [(triVisibleTexts), "SHOWCASE"]
triAssertExcludes [(triVisibleTexts), "CAMPAIGN GAME"]
triAssertExcludes [(triVisibleTexts), "SINGLE MISSION"]
triScreenshot "01_remaster_ud_menu"

// -- Bohemia's mods screen stays reachable (MODS cloned from Quit) -----------
triAssertIncludes [(triVisibleTexts), "MODS"]
triClick 119
triAssertEq [(triDisplay), 72]
triAssertEq [(triControlText 115), "Load Mods"]
triSendKey 41
triAssertEq [(triDisplay), 0]

// -- Bohemia's Options shell (their optionsShell.hpp chain) mounts ------------
triClickText "OPTIONS"
triAssertEq [(triDisplay), 9099]
triAssertIncludes [(triVisibleTexts), "Audio"]
triScreenshot "02_remaster_options"
triSendKey 41
triAssertEq [(triDisplay), 0]

// -- UD keys Bohemia's shards lack come from bin/STRINGTABLE_UD.utf8.csv ------
triAssertEq [(localize "STR_USRACT_LEAN_LEFT"), "Lean left"]
triAssertEq [(localize "STR_DISP_OPT_CTL_ARMA3"), "Arma 3 controls"]
// ... while a key Bohemia ships still resolves from their shard (the MODS label).
triAssertEq [(localize "STR_DISP_MAIN_MODS"), "MODS"]

triEndTest
