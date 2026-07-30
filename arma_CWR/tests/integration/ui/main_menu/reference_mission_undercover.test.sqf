// ============================================================================
//  Main-menu reference-mission launch: UNDERCOVER button (idc 124).
//
//  Companion to reference_mission_showcase - same direct-launch mechanism
//  (kReferenceMissions, UI/OptionsUIApp.cpp), driving the second slice:
//  Undercover.Abel, the deep-undercover sandbox in Houdan. Unlike the
//  Showcase it is NOT shared-core, so the post-launch probes target its own
//  bootstrap (UC_BOOTED) and the natively-authored zones.
//
//  PRECONDITION (documented in the .toml): guerrilla-mode/install-missions.ps1
//  has been run against the data dir, so the unbanked template sits at
//  <OFPR_DATA_DIR>\Missions\Undercover.Abel\.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- UNDERCOVER launches straight into the mission --------------------------
triAssertEq [(triClick 124), true]
triSimUntil { alive player }

// -- it is the real Undercover.Abel: its own bootstrap ran and the two
//    authored zones are live in the native registry ------------------------
triSimUntil { not (isNil "UC_BOOTED") }
triAssertGe [(gmZoneIndex "Houdan"), 0]
triAssertGe [(gmZoneIndex "Checkpoint"), 0]

// -- the cover baseline is in place: undercover flag up, player captive -----
triSimUntil { not (isNil "gmUndercover") }
triAssert [gmUndercover]
triAssert [captive player]

triEndTest
