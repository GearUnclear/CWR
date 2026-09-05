// ============================================================================
//  Guerrilla Mode - island/faction swap test (issue #3 item 4).
//    Boots the REAL Guerrilla.Sinai template (guerrilla-mode/mission/) on the
//    @LoBo Sinai world with the sides deliberately flipped vs Guerrilla.Demo:
//    occupier = IDF (WEST), resistance = Egyptian Frontier Corps (EAST).
//    Proves the migrated script core + native systems are island- and
//    faction-agnostic: the same ONE shared core (\gmcore\init.sqs), all data
//    from description.ext.
//
//  No gmSel* selections are published in a direct --test-mission launch, so
//  the engine resolves sides from the mission's defaultOccupier="IDF" /
//  defaultResistance="EgyptFrontier" keys (ZoneRegistry::LoadFromParams).
//
//  Zone seed: 4 explicit zones (Camp, Wadi Checkpoint, Ras Nasrani Outpost,
//  El Tor) + seedCities over southern Sinai's ~34 type-less Names entries
//  (El Tor's seed deduped by the explicit zone within 300 m).
//
//  The player spawns at the Camp, 552 m from the Wadi Checkpoint - inside
//  the native garrison cache radius (800 m) - so the WEST garrison
//  (LoBoGolaniWB tiers + LoBoGolaniWBo officer) must spawn without any
//  scripted nudge. gc* globals persist across per-statement evaluation.
// ============================================================================

// -- mode fully booted: native registry populated + init.sqs helper table ----
triSimUntil { GM_LIB_READY }

// -- sides resolved from the config defaults: the WHOLE point of this test ---
triAssertEq [(gmOccupierSide), "WEST"]
triAssertEq [(gmResistanceSide), "EAST"]

// -- zone seed: 4 explicit + the seeded towns (>= 10 proves seedCities ran) --
triSimUntil { gmZoneCount >= 10 }

// -- the near outpost: occupier-owned, reserve garrison from config ----------
gcIdx = gmZoneIndex "Wadi Checkpoint"
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
