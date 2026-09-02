// ============================================================================
//  Guerrilla Mode - scaffolded island boots (issue #54 C2/E2).
//    Boots guerrilla-mode/mission/Guerrilla.Eden, which was NOT hand-authored:
//    it is the verbatim output of
//      PoseidonTools guerrilla scaffold --world Eden --data-dir <Classic>
//                                      --out guerrilla-mode/mission/Guerrilla.Eden
//    (18 CITY zones seeded from Everon's Names entries that sit on dry land
//    with houses around them, a resistance CAMP and 3 occupier OUTPOSTs on
//    off-road dry ground, every elevation sampled from eden.wrp; the
//    war-faction placeholders EAST/GUER resolve against the global vanilla
//    library). The template ships as generated, so this test is the proof
//    that "one tool" gets from a .wrp to a running campaign.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// -- the placeholder pair resolved through the global faction library -------
triAssertEq [(gmOccupierSide), "EAST"]
triAssertEq [(gmResistanceSide), "GUER"]
triAssertEq [(format ["%1", side player]), "GUER"]

// -- the scaffold's zone table came through the registry intact --------------
triSimUntil { gmZoneCount >= 22 }
triAssertEq [(gmZoneCount), 22]
gsCamp = gmZoneIndex "Camp"
triAssertEq [((gmZone gsCamp) select 1), "CAMP"]
triAssertEq [((gmZone gsCamp) select 2), "GUER"]
gsOut = gmZoneIndex "Outpost 1"
triAssertEq [((gmZone gsOut) select 1), "OUTPOST"]
triAssertEq [((gmZone gsOut) select 2), "EAST"]
gsTown = gmZoneIndex "Montignac"
triAssertEq [((gmZone gsTown) select 1), "CITY"]
triAssertEq [((gmZone gsTown) select 2), "NEUTRAL"]
// the sampled elevation is real ground, not the Names label size
triAssertEq [(((gmZone gsTown) select 8) select 2) > 100, true]

// -- the player stands at the camp the scaffold placed, on dry land ---------
triAssertEq [(player distance ((gmZone gsCamp) select 8)) < 60, true]
triAssertEq [((getPos player) select 2) < 5, true]

triEndTest
