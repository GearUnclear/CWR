// ============================================================================
//  Seeded bases across a save (issue #76), phase 2 of 2: a fresh process
//  boots Eden (which seeds again on its own), loads the phase 1 save and
//  finds the same zone table: the same count (nothing seeded twice), each
//  saved seeded zone with its type, owner, position, garrisonMax and the
//  seeded flag, still exactly one AIRFIELD and one SEAPORT, and the same
//  poolStart.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(triLoadGame "bases76"), "OK"]
triSimFrames 5
triAssertEq [gmZoneCount, bsZoneCount]
triAssertEq [(count bsRows), 2]
brA = bsRows select 0
brIA = gmZoneIndex (brA select 0)
triAssertGe [brIA, 0]
triAssertEq [((gmZone brIA) select 1), (brA select 1)]
triAssertEq [((gmZone brIA) select 2), (brA select 2)]
triAssertLe [([((gmZone brIA) select 8) select 0, ((gmZone brIA) select 8) select 1, 0] distance [brA select 3, brA select 4, 0]), 1]
triAssertEq [((gmZoneSim brIA) select 7), (brA select 5)]
triAssertEq [(format ["%1", (gmZone brIA) select 10]), "true"]
brB = bsRows select 1
brIB = gmZoneIndex (brB select 0)
triAssertGe [brIB, 0]
triAssertEq [((gmZone brIB) select 1), (brB select 1)]
triAssertEq [((gmZone brIB) select 2), (brB select 2)]
triAssertLe [([((gmZone brIB) select 8) select 0, ((gmZone brIB) select 8) select 1, 0] distance [brB select 3, brB select 4, 0]), 1]
triAssertEq [((gmZoneSim brIB) select 7), (brB select 5)]
triAssertEq [(format ["%1", (gmZone brIB) select 10]), "true"]
brNA = 0
brNS = 0
brI = 0
while {brI < gmZoneCount} do {if (((gmZone brI) select 1) == "AIRFIELD") then {brNA = brNA + 1}; if (((gmZone brI) select 1) == "SEAPORT") then {brNS = brNS + 1}; brI = brI + 1}
triAssertEq [brNA, 1]
triAssertEq [brNS, 1]
triAssertEq [((gmOccupierForces) select 1), bsPool]
triEndTest
