// ============================================================================
//  Seeded bases across a save (issue #76), phase 1 of 2: Guerrilla.Eden
//  seeds its AIRFIELD and SEAPORT (guerrilla_bases_detect_eden has the
//  ground truth).  The expected rows ride the save as script globals (bs*),
//  so phase 2 compares against what this phase saw.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
bsSeeded = gmSeededBases
triAssertEq [(count bsSeeded), 2]
bsZoneCount = gmZoneCount
bsRows = []
{bsI = gmZoneIndex (_x select 0); bsZ = gmZone bsI; bsRows = bsRows + [[bsZ select 0, bsZ select 1, bsZ select 2, (bsZ select 8) select 0, (bsZ select 8) select 1, (gmZoneSim bsI) select 7]]} forEach bsSeeded
triAssertEq [(count bsRows), 2]
bsPool = (gmOccupierForces) select 1
triAssertEq [(triSaveGame "bases76"), "OK"]
triEndTest
