// ============================================================================
//  Guerrilla Mode - a third-party faction pack on an island that never
//  authored it (issue #54 E2, the definition-of-done for the faction half).
//    Boots tests/integration/missions/guerrilla_factionpack.abel: the Abel
//    native-parity mission with defaultResistance = "UDFaction", a class its
//    description.ext never declares. The class comes from the GPL-clean
//    synthetic pack tests/fixtures/mods-factionpack/@udfaction mounted as a
//    mod (bin/config.cpp -> Pars) and reaches the registry only through the
//    global-U-island union (FactionSources). Zero engine or template edits.
//
//    UDFaction is authored side WEST; the template pins the resistance to
//    the player's side (GUER), the pack ships no sideTwin, so the registry
//    rebases the roster onto GUER - the roster moves, the side does not.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// -- the pack's descriptor is the bound resistance ---------------------------
triAssertEq [(gmResistanceSide), "GUER"]
triAssertEq [(gmOccupierSide), "EAST"]
triAssertEq [(gmFactionValue ["GUER", "officer"]), "UDOfficer"]
triAssertEq [(gmFactionTierClass ["GUER", 1]), "UDSoldier"]
triAssertEq [(gmFactionTierClass ["GUER", 9]), "UDOfficer"]
triAssertEq [(gmFactionCivTier ["GUER", 1]), "UDCivFighter"]
triAssertEq [(gmClassExists "UDSoldier"), true]

// -- side identity: a pack body created into the player's group fights as
//    the resistance side, whatever its config side says ---------------------
gfpBefore = count units group player
"UDSoldier" createUnit [getPos player, group player, "gfpUnit = this", 0.5, "PRIVATE"]
triSimUntil { (count units group player) > gfpBefore }
triAssertEq [(typeOf gfpUnit), "UDSoldier"]
triAssertEq [(format ["%1", side gfpUnit]), "GUER"]
triAssertEq [(alive gfpUnit), true]

triEndTest
