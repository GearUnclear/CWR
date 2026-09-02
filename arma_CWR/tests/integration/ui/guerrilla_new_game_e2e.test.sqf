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
//  Faction cyclers: the menu lists the MERGED faction table - the island
//  template's own CfgGuerrillaFactions unioned with the global one in Pars
//  (FactionSources, issue #54 A1), the island winning on a name collision.
//  For Sinai the eight @LoBo rosters come from @lobofixup's bin/config.cpp
//  in config order (IDF, EgyptFrontier, EgyptArmy, Syria, Jordan, Hizballah,
//  PLO, PLO_East), and the template itself declares only CIV, which the
//  cyclers exclude.
//
//  !! THE RING IS NO LONGER EIGHT ON AN INSTALLED GAME DIR (issue #54 A4).
//  install-missions.ps1 now also drops the VANILLA library into
//  <GameDir>\bin\guerrilla-factions.hpp and includes it from
//  bin\config-extra.cpp, which the engine merges into Pars LAST. So Pars
//  carries WEST, EAST and GUER after the @LoBo eight, all three resolvable
//  on this lane (full CWA 1.99 is mounted), and the occupier ring is
//  ELEVEN entries: ..., PLO_East, WEST, EAST, GUER, wrap to IDF. The click
//  sequence below still walks eight and therefore needs three more clicks
//  before the wrap assert. Update it with the run, not from this comment.
//
//  The occupier cycler seeds on the template's defaultOccupier (IDF); the
//  test walks the full ring once - proving it cycles AND wraps - and ends
//  back on IDF so the launch below behaves exactly like a no-UI default
//  launch. In-mission, the published class-name strings resolve against the
//  merged table: IDF -> WEST occupier, EgyptFrontier -> EAST resistance.
//
//  (This section previously asserted a two-entry wrap from the pre-1702080
//  global @lobofixup roster; that went stale when the per-island roster
//  landed and was caught red 2026-08-04.)
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry is present on the (community) main menu and clickable --
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]

// -- island list: select Sinai by its CfgWorldList class name ---------------
triAssertEq [(triSelectListByData [101, "sinai"]), true]

// -- occupier cycler: walk the full eight-entry Sinai ring once (see header),
//    ending back on the IDF default the launch below depends on -------------
triAssertEq [(triControlText 150), "OCCUPIER: IDF"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: EgyptFrontier"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: EgyptArmy"]
triClick 150
triClick 150
triClick 150
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: PLO"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: PLO_East"]
// -- ... then the vanilla library (bin\config-extra.cpp merges LAST into
//    Pars, so WEST/EAST/GUER follow the @LoBo eight; issue #54 A4) --------
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: WEST"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: GUER"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: IDF"]
triAssertEq [(triControlText 151), "RESISTANCE: EgyptFrontier"]

// -- outfit cycler (issue #25, idc 153): EgyptFrontier authors playerClassCiv,
//    so the pair is offered; WARRIOR default, wraps over two entries, and is
//    left on WARRIOR so the launch below behaves exactly like before ---------
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]

// -- OK launches the installed Guerrilla.Sinai template ----------------------
triClick 1
triSimUntil { alive player }

// -- the UI selections were published for the mission scripts ---------------
triAssertEq [gmSelIsland, "sinai"]
triAssertEq [gmSelOccupier, "IDF"]
triAssertEq [gmSelResistance, "EgyptFrontier"]
// WARRIOR is published (a real choice was offered) and acts as a no-op:
// the player keeps the authored mission.sqm class
triAssertEq [gmSelOutfit, "WARRIOR"]
triAssertEq [(typeOf player), "LoBo_Egypt_FrtCrp"]

// -- and the native registry resolved them to the flipped sides -------------
triAssertEq [(gmOccupierSide), "WEST"]
triAssertEq [(gmResistanceSide), "EAST"]
triSimUntil { gmZoneCount >= 10 }

// -- shared core booted on top (init.sqs ran, camp marker painted) ----------
triSimUntil { GM_LIB_READY }
triSimUntil { (getMarkerColor "gmZoneMarker_0") == "ColorGreen" }

triEndTest
