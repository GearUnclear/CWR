// ============================================================================
//  Guerrilla Mode - menu-time faction gating + a faction pack on every
//  island, end to end (issue #54 A2/E2).
//    Two GPL-clean synthetic packs are mounted as mods: @udfaction (its
//    classes inherit vanilla bodies, so the Classic package can field it)
//    and @udbroken (tiers[0] names a class nothing ships). Neither appears
//    in any island template; both reach the cyclers through the union.
//
//    Merged ring on Abel (island block = CIV only, excluded): the mod
//    configs merge earliest-listed first, then bin\config-extra.cpp's
//    vanilla library last, so the OCCUPIER cycler walks
//      EAST (default) -> GUER -> UDFaction -> UDBrokenFaction -> WEST -> EAST.
//    UDBrokenFaction must read "(not in loaded data)" and be REFUSED on OK;
//    UDFaction must be selectable as the resistance and land in the mission
//    as the bound resistance roster.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]

// -- occupier ring: the broken pack is offered greyed, the good one plain --
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: GUER"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: UDFaction"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: UDBrokenFaction (not in loaded data)"]
// (OK on that greyed pick is refused with a message box - a child
//  ControlsContainer the harness cannot address by display id, so the
//  refusal is covered by the unit tests on GuerrillaUnavailableMessage and
//  the launch below proves the non-greyed path instead)
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: WEST"]
triClick 150
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]

// -- resistance: the pack, picked on an island that never declared it ------
triAssertEq [(triControlText 151), "RESISTANCE: GUER"]
triClick 151
triAssertEq [(triControlText 151), "RESISTANCE: UDFaction"]

// -- launch --------------------------------------------------------------
triClick 1
triSimUntil { alive player }
triAssertEq [gmSelIsland, "Abel"]
triAssertEq [gmSelOccupier, "EAST"]
triAssertEq [gmSelResistance, "UDFaction"]
triSimUntil { GM_LIB_READY }
// the pack roster is the bound resistance, rebased onto the player's side;
// its playerClassWarrior became the player's body (issue #54 A3)
triAssertEq [(gmResistanceSide), "GUER"]
triAssertEq [(gmFactionTierClass ["GUER", 1]), "UDSoldier"]
triAssertEq [(typeOf player), "UDSoldier"]

triEndTest
