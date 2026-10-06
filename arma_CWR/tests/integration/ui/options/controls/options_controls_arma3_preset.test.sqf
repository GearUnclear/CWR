// Arma 3 preset is reachable by keyboard/controller and resolves through both
// cancel and apply. Binding replacement and disk round-trip are unit-tested.
#include "../../../helpers/options_preamble.sqf"
#include "../../../helpers/controls_preamble.sqf"

triAssertIncludes [(triVisibleTexts), "Arma 3 controls"]
triClickText "Arma 3 controls"
triAssert [(triGetControlFocused 9102)]
triAssertIncludes [(triVisibleTexts), "Apply Arma 3 controls?"]
triAssertIncludes [(triVisibleTexts), "Change aim / zoom / lock / watch?"]
triWaitFrames 35
triScreenshot "00_arma3_preset_confirm"
triGpadButton 1
triWaitFrames 5
triAssert [(triGetControlFocused 1407)]
triSendKey 81
triAssert [(triGetControlFocused 1404)]
triSendKey 82
triAssert [(triGetControlFocused 1407)]
triClickText "Arma 3 controls"
triClickText "Apply preset"
triAssert [(triGetControlFocused 1407)]
triWaitFrames 35
triScreenshot "01_arma3_preset_applied"
triClickText "Close"
triClickText "Close"
triClickText "OPTIONS"
triClickText "Controls"
triAssertIncludes [(triVisibleTexts), "Arma 3 controls"]
triEndTest
