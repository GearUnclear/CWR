// ============================================================================
//  Generated bases across a save (OCCUPIER-FORCES.md 5.7), phase 1 of 2:
//  Guerrilla.Abel generates its OUTPOSTs (guerrilla_bases_generate_abel has
//  the rules).  The expected rows ride the save as script globals (gs*), so
//  phase 2 compares against what this phase saw.  One generated base is
//  liberated before the save, so its dynamic state must come back too.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
gsGen = gmGeneratedBases
triAssertEq [(count gsGen), 2]
gsZoneCount = gmZoneCount
gsRows = []
{gsI = gmZoneIndex (_x select 0); gsZ = gmZone gsI; gsRows = gsRows + [[gsZ select 0, gsZ select 1, (gsZ select 8) select 0, (gsZ select 8) select 1, (gmZoneSim gsI) select 7]]} forEach gsGen
triAssertEq [(count gsRows), 2]
gsFreed = (gsRows select 0) select 0
gmZoneSet [gmZoneIndex gsFreed, "owner", gmResistanceSide]
triSimFrames 5
triAssertEq [((gmZone (gmZoneIndex gsFreed)) select 2), gmResistanceSide]
gsPool = (gmOccupierForces) select 1
triAssertEq [(triSaveGame "gen57"), "OK"]
triEndTest
