#include "../../../helpers/options_preamble.sqf"
#include "../../../helpers/controls_preamble.sqf"
triClickText "Keyboard & Mouse"
triAssertEq [triDisplay, 9099]
// The twelfth on-foot action is Lean left; navigate through the real scroll list.
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triSendKey 81
triAssertIncludes [triVisibleTexts, "Lean left"]
triSendKey 81
triAssertIncludes [triVisibleTexts, "Lean right"]
triSendKey 82
triSendKey 40
triAssertEq [triControlText 9380, "Press a key"]
triSendKey 20
triAssertEq [triControlText 9380, "Captured: Q"]
triClick 9301
triAssertIncludes [triVisibleTexts, "Lean left"]
triScreenshot "lean_bindings"
triEndTest
