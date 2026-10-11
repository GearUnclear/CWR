// Exercise the actual UI exit: Lebanon80 declares an empty cutscene name, and the
// former empty lookup left the entire active mission behind the main menu.
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "lebanon80"]), true]
triClick 1
triSimUntil { alive player }
triSimUntil { GM_LIB_READY }
triAssertEq [(getWorld), "lebanon80"]
triAssertGt [(gmZoneCount), 0]
triReturnMissionSentinel = true

// Start uses the controller pause action, which also works under dummy SDL.
triGpadButton 9
triSimFrames 2
triAssertEq [(triDisplay), 49]
triAssertEq [(triClick 104), true]
triAssertEq [(triDisplay), 0]
triAssertEq [(triGameMode), 2]
triAssert [(isNil "triReturnMissionSentinel")]
triAssert [(isNil "GM_LIB_READY")]
triAssertEq [(gmZoneCount), 0]
triAssert [not ((getWorld) == "lebanon80")]
triScreenshot "lobo_lebanon_menu_restored"
triEndTest
