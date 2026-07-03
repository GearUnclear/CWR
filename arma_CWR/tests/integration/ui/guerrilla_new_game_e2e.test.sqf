// ============================================================================
//  Guerrilla Mode - new-game selection flow, end to end (issue #3 item 6).
//
//  Drives the REAL player path: main menu -> GUERRILLA (injected idc 120,
//  cloned from the community menu's Quit class - @LoBo ships its own
//  RscDisplayMain) -> IDD_GUERRILLA_NEW_GAME (76, reusing the
//  RscDisplaySelectIsland layout) -> pick the Sinai island in the C3DListBox
//  (IDC_SELECT_ISLAND=101; row data = CfgWorldList class name) -> cycle the
//  injected occupier/resistance buttons (idc 150/151) -> OK (IDC_OK=1) ->
//  DisplayMain::OnChildDestroyed resolves the installed template
//  "missions\Guerrilla.sinai" (OptionsUIApp.cpp, IDD_GUERRILLA_NEW_GAME
//  case), publishes gmSelIsland/gmSelOccupier/gmSelResistance, and launches
//  it through the single-mission path (empty Intro -> no briefing file ->
//  straight into DisplayMission).
//
//  PRECONDITION (documented in the .toml): the template must be installed in
//  the game data dir - run guerrilla-mode/install-missions.ps1 first.
//
//  Faction cyclers: the menu lists candidates from the GLOBAL config's
//  CfgGuerrillaFactions (the fixture mod @lobofixup provides IDF +
//  EgyptFrontier; a mission's description.ext is not parsed at menu time).
//  The occupier list starts on IDF; two clicks cycle EgyptFrontier -> back
//  to IDF, proving the cycler actually cycles. In-mission, the published
//  class-name strings resolve against the template's own faction table:
//  IDF -> WEST occupier, EgyptFrontier -> EAST resistance.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry is present on the (community) main menu and clickable --
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]

// -- island list: select Sinai by its CfgWorldList class name ---------------
triAssertEq [(triSelectListByData [101, "sinai"]), true]

// -- occupier cycler: two clicks = EgyptFrontier -> IDF (wraps, 2 entries) --
triAssertEq [(triControlText 150), "OCCUPIER: IDF"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: EgyptFrontier"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: IDF"]
triAssertEq [(triControlText 151), "RESISTANCE: EgyptFrontier"]

// -- OK launches the installed Guerrilla.Sinai template ----------------------
triClick 1
triSimUntil { alive player }

// -- the UI selections were published for the mission scripts ---------------
triAssertEq [gmSelIsland, "sinai"]
triAssertEq [gmSelOccupier, "IDF"]
triAssertEq [gmSelResistance, "EgyptFrontier"]

// -- and the native registry resolved them to the flipped sides -------------
triAssertEq [(gmOccupierSide), "WEST"]
triAssertEq [(gmResistanceSide), "EAST"]
triSimUntil { gmZoneCount >= 10 }

// -- shared core booted on top (init.sqs ran, camp marker painted) ----------
triSimUntil { GM_LIB_READY }
triSimUntil { (getMarkerColor "gmZoneMarker_0") == "ColorGreen" }

triEndTest
