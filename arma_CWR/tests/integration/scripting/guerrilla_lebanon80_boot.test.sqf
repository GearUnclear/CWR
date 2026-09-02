// ============================================================================
//  Guerrilla Mode - Lebanon80 boot smoke (third island pack).
//    Boots the REAL Guerrilla.Lebanon80 template (guerrilla-mode/mission/) on
//    the @LoBo Lebanon (80's) world: occupier = IDF (WEST), resistance =
//    Hizballah (EAST). The same ONE shared core as Guerrilla.Demo /
//    Guerrilla.Sinai (they all exec \gmcore\init.sqs); all island/faction
//    data from description.ext. Modeled
//    on guerrilla_sinai_swap - the differences are the roster (Hizballah, not
//    Egyptian Frontier Corps) and the zone seed (see below).
//
//  No gmSel* selections are published in a direct --test-mission launch, so
//  the engine resolves sides from the mission's defaultOccupier="IDF" /
//  defaultResistance="Hizballah" keys (ZoneRegistry::LoadFromParams).
//
//  Zone seed: 6 explicit zones ONLY (Camp, Litani Checkpoint, Marjayoun
//  Barracks, Tyre, Saida, Ghajar) - the template sets seedCities=0 because
//  Lebanon80's type-less Names block is a theater map ("Mediterranean Sea",
//  "Sea of Galilee", country labels...) that would seed CITY zones in open
//  water. So this test asserts the EXACT count, unlike sinai_swap's >= 10.
//
//  The player spawns at the Camp, 500 m from the Litani Checkpoint - inside
//  the native garrison cache radius (800 m) - so the WEST garrison
//  (LoBoGolaniWB tiers + LoBoGolaniWBo officer) must spawn without any
//  scripted nudge. gc* globals persist across per-statement evaluation.
// ============================================================================

// -- mode fully booted: native registry populated + init.sqs helper table ----
triSimUntil { GM_LIB_READY }

// -- sides resolved from the config defaults ---------------------------------
triAssertEq [(gmOccupierSide), "WEST"]
triAssertEq [(gmResistanceSide), "EAST"]

// -- player: alive at the Camp, welded to the resistance side ----------------
triSimUntil { alive player }
triAssertEq [(format ["%1", side player]), "EAST"]

// -- zone seed: exactly the 6 explicit zones (proves seedCities=0 held) ------
triSimUntil { gmZoneCount >= 6 }
triAssertEq [(gmZoneCount), 6]

// -- the near outpost: occupier-owned, reserve garrison from config ----------
gcIdx = gmZoneIndex "Litani Checkpoint"
triAssertEq [((gmZone gcIdx) select 2), "WEST"]

// -- camp is resistance-owned and painted with the resistance color ----------
gcCampIdx = gmZoneIndex "Camp"
triAssertEq [((gmZone gcCampIdx) select 2), "EAST"]
triSimUntil { (getMarkerColor "gmZoneMarker_0") == "ColorGreen" }

// -- native garrison cache engages near the player: IDF units spawn live -----
triSimUntil { gmGarrisonSpawned gcIdx }
triSimUntil { (gmGarrisonLive gcIdx) > 0 }

// -- occupier outpost revealed near the camp paints red (occupier color) -----
triSimUntil { (getMarkerColor "gmZoneMarker_1") == "ColorRed" }

// -- undisturbed start: the checkpoint's alert FSM sits at GREEN (0) ---------
triAssertEq [(gmZoneAlert gcIdx), 0]

triEndTest
