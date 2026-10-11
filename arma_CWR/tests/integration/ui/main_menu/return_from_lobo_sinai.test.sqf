// The island's missing lost_anims intro used to skip the entire teardown:
// mission actors, scripts and Guerrilla state kept running behind the menu.
triSimUntil { alive player }
triSimUntil { GM_LIB_READY }
triAssertEq [(getWorld), "sinai"]
triAssertGt [(gmZoneCount), 0]
triAssertEq [(typeOf player), "LoBo_Egypt_FrtCrp"]
triReturnMissionSentinel = true

// The same transition called by the mission Abort action.
triAssertEq [(triStartRandomCutscene), "OK"]
triAssertEq [(triGameMode), 2]
triAssert [(isNil "triReturnMissionSentinel")]
triAssert [(isNil "GM_LIB_READY")]
triAssertEq [(gmZoneCount), 0]
triAssert [not ((typeOf player) == "LoBo_Egypt_FrtCrp")]
triAssert [not ((getWorld) == "sinai")]
triEndTest
