// UD-only Options controls on the official Remaster, no mod mounted.
// setup-guerrilla.ps1 (Remaster mode) kept Bohemia's optionsShell.hpp (a
// six-row Controls page) and appended #include "ud-options.hpp" to Bohemia's
// bin/resource-extra.cpp. That shard defines RscOptionsPageControlsUD, which
// inherits Bohemia's page and adds the Arma 3 controls preset row (idc 1407);
// ControlsPage mounts it in preference to the stock page.

triSetLanguage "English"
triAssertEq [(triDisplay), 0]

// -- Options shell (Bohemia's chain) -> Controls page --------------------------
triClickText "OPTIONS"
triAssertEq [(triDisplay), 9099]
triAssertIncludes [(triVisibleTexts), "Controls"]
triClickText "Controls"
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]

// -- the UD row exists with its localized text, above Bohemia's Close row ------
triAssertEq [(triControlText 1407), "Arma 3 controls"]
triAssertIncludes [(triVisibleTexts), "Arma 3 controls"]
// Bohemia's six rows still mount through the inherited base class.
triAssertEq [(triControlText 1401), "Keyboard & Mouse"]
triAssertEq [(triControlText 1403), "Reset all to defaults"]
triAssertEq [(triControlText 1404), "Back"]
triAssertIncludes [(triVisibleTexts), "Gamepad Tuning"]
// Let the notebook lid finish opening so the capture shows the rows.
triWaitFrames 90
triScreenshot "01_remaster_ud_controls_page"

// -- pressing the row opens the confirm page with the UD texts -----------------
triClick 1407
triAssertEq [(triDisplay), 9099]
triAssertEq [(triControlText 9180), "Apply Arma 3 controls?"]
triAssertEq [(triControlText 9181), "Set aim, watch and Q/E lean?"]
triAssertEq [(triControlText 9101), "Apply preset"]
triAssertEq [(triControlText 9102), "Cancel"]
triAssertIncludes [(triVisibleTexts), "Apply preset"]
triScreenshot "02_remaster_ud_arma3_confirm"

// -- Cancel returns to the Controls page with the row still mounted -------------
triClick 9102
triAssertEq [(triControlText 1407), "Arma 3 controls"]
triAssertIncludes [(triVisibleTexts), "Keyboard & Mouse"]
triAssertExcludes [(triVisibleTexts), "Apply preset"]

// -- back out: Controls -> Options index -> main menu ---------------------------
triClick 1404
triAssertIncludes [(triVisibleTexts), "Audio"]
triSendKey 41
triAssertEq [(triDisplay), 0]

triEndTest
