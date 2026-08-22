// ============================================================================
//  Main-menu reference-mission launch: QRF button (idc 125).
//
//  Third of the kReferenceMissions direct launches (UI/OptionsUIApp.cpp),
//  driving the alert->QRF slice: Qrf.Abel on the Houdan slope. Like
//  Undercover.Abel it has its own bootstrap (QRF_BOOTED), but unlike it the
//  mission runs the campaign's REAL scripts/qrf.sqs + scripts/lib.sqs
//  (byte-identical core subset, pinned by test_mission_script_core.cpp), so
//  the post-launch probes also check that the shared-core handshake and the
//  QRF manager's own state came up.
//
//  PRECONDITION (documented in the .toml): guerrilla-mode/install-missions.ps1
//  has been run against the data dir, so the unbanked template sits at
//  <OFPR_DATA_DIR>\Missions\Qrf.Abel\.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- QRF launches straight into the mission ----------------------------------
triAssertEq [(triClick 125), true]
triSimUntil { alive player }

// -- it is the real Qrf.Abel: its own bootstrap ran and the three authored
//    zones are live in the native registry ---------------------------------
triSimUntil { not (isNil "QRF_BOOTED") }
triAssertGe [(gmZoneIndex "Camp"), 0]
triAssertGe [(gmZoneIndex "Outpost"), 0]
triAssertGe [(gmZoneIndex "Depot"), 0]

// -- the shared-core subset booted: lib handshake + qrf.sqs manager state -----
triSimUntil { not (isNil "GM_LIB_READY") }
triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_QRF_ACTIVE") }
triAssert [not GM_QRF_ACTIVE]

triEndTest
