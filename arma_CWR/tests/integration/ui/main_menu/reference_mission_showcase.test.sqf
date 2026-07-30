// ============================================================================
//  Main-menu reference-mission launch: SHOWCASE button (idc 123).
//
//  The human-playable test slices (guerrilla-mode/mission/Showcase.Abel,
//  Undercover.Abel) are surfaced on the main menu as direct-launch buttons
//  (kReferenceMissions, UI/OptionsUIApp.cpp): resource-provided controls on
//  the Classic package's RscDisplayMainRemaster, runtime-injected clones of
//  the Quit class anywhere the menu resource lacks them. Either way the
//  button only shows while the mission is installed, and a click launches it
//  through the single-mission path with no island/faction screen (the
//  mission's own description.ext defaults apply).
//
//  PRECONDITION (documented in the .toml): guerrilla-mode/install-missions.ps1
//  has been run against the data dir, so the unbanked template sits at
//  <OFPR_DATA_DIR>\Missions\Showcase.Abel\.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- both reference-mission entries are on the menu -------------------------
triAssertIncludes [(triVisibleTexts), "SHOWCASE"]
triAssertIncludes [(triVisibleTexts), "UNDERCOVER"]

// -- SHOWCASE launches straight into the mission (no selection screen) ------
triAssertEq [(triClick 123), true]
triSimUntil { alive player }

// -- it is the real Showcase.Abel: shared core boots and the authored zones
//    are live in the native registry --------------------------------------
triSimUntil { not (isNil "GM_LIB_READY") }
triSimUntil { GM_LIB_READY }
triAssertGe [(gmZoneIndex "Village"), 0]
triAssertGe [(gmZoneIndex "Outpost"), 0]

triEndTest
