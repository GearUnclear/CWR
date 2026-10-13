// ============================================================================
//  Occupier forces director across a save (OCCUPIER-FORCES.md phase 3,
//  section 11), phase 2 of 2: a fresh process boots Abel (a new, DORMANT
//  campaign), loads the save and finds the director as it was: the pool and
//  its band, AWAKE with the same (spent) tokens, the two distinct towns, the
//  rest on town 2 (counted down by no more than the time since), the
//  operation counter, the same two director operations in gmOccupierOps
//  (CounterAttack's records with their ids), and the sightings table.
// ============================================================================

triSimUntil { GM_LIB_READY }
// fresh-boot baseline: a new campaign, no director state
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(count ((gmOccupierForces) select 10)), 0]

triAssertEq [(triLoadGame "forcesdir"), "OK"]
triSimFrames 5
player allowDammage false
frRow = gmOccupierForces
triAssertEq [(count frRow), 15]
triAssertEq [(frRow select 0), fsPool]
triAssertEq [(frRow select 2), fsBand]
triAssertEq [(frRow select 3), fsTokens]
triAssertEq [(frRow select 4), "AWAKE"]
frEver = frRow select 10
triAssertEq [(count frEver), 2]
triAssertEq [(format ["%1", "Village" in frEver]), "true"]
triAssertEq [(format ["%1", fsBName in frEver]), "true"]
frCool = frRow select 12
triAssertEq [(count frCool), 1]
triAssertEq [((frCool select 0) select 0), fsBName]
triAssertGe [((fsCool select 0) select 1), ((frCool select 0) select 1)]
triAssertGe [((frCool select 0) select 1), ((fsCool select 0) select 1) - 120]
triAssertEq [(frRow select 13), fsCounter]
frIds = []
{if ((_x select 2) == "DIRECTOR") then {frIds = frIds + [_x select 0]}} forEach gmOccupierOps
triAssertEq [(count frIds), 2]
triAssertEq [(format ["%1", (fsIds select 0) in frIds]), "true"]
triAssertEq [(format ["%1", (fsIds select 1) in frIds]), "true"]
triAssertEq [(count gmOccupierIntel), fsIntel]

// the reloaded director keeps running: its next launch takes the next id
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
frT0 = time
triSimUntil { time > frT0 + 5 }
triAssertGe [((gmOccupierForces) select 13), fsCounter]
triAssertEq [((gmOccupierForces) select 4), "AWAKE"]

triEndTest
