// ============================================================================
//  Main-menu reference-mission launch: MARKET button (idc 126).
//
//  Fourth of the kReferenceMissions direct launches (UI/OptionsUIApp.cpp),
//  driving the money-loop slice: Market.Abel on the Houdan plain. Like
//  Qrf.Abel it has its own bootstrap (MKT_BOOTED) and runs the campaign's
//  REAL \gmcore\scripts\market.sqs + \gmcore\scripts\lib.sqs (the ONE shared
//  core, pinned by test_mission_script_core.cpp), so the post-launch probes also
//  check that the shared-core handshake, the market manager's own state and
//  the native market came up.
//
//  PRECONDITION (documented in the .toml): guerrilla-mode/install-missions.ps1
//  has been run against the data dir, so the unbanked template sits at
//  <OFPR_DATA_DIR>\Missions\Market.Abel\.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- MARKET launches straight into the mission --------------------------------
triAssertEq [(triClick 126), true]
triSimUntil { alive player }

// -- it is the real Market.Abel: its own bootstrap ran, the Camp zone is live
//    in the native registry and the towns were seeded ----------------------
triSimUntil { not (isNil "MKT_BOOTED") }
triAssertGe [(gmZoneIndex "Camp"), 0]
triSimUntil { gmZoneCount >= 4 }

// -- the shared-core subset booted: lib handshake + market.sqs manager state ---
triSimUntil { not (isNil "GM_LIB_READY") }
triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_MKT_READY") }
triAssert [gmMarketActive]
triAssert [not gmHqEstablished]

triEndTest
