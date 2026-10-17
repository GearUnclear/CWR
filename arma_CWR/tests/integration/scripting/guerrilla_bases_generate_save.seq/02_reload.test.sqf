// ============================================================================
//  Generated bases across a save (OCCUPIER-FORCES.md 5.7), phase 2 of 2: a
//  fresh process boots Abel (which generates again, from a new seed), loads
//  the phase 1 save and finds the stored bases: the same zone count (nothing
//  generated twice), gmGeneratedBases naming exactly the saved rows, each at
//  its saved position with its type and garrisonMax, the liberated one still
//  the resistance's, and the same poolStart.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(triLoadGame "gen57"), "OK"]
triSimFrames 5
triAssertEq [gmZoneCount, gsZoneCount]
grGen = gmGeneratedBases
triAssertEq [(count grGen), (count gsRows)]
grBad = ""
grK = 0
while {grK < (count gsRows)} do {grRow = gsRows select grK; grLive = grGen select grK; if ((grLive select 0) != (grRow select 0)) then {grBad = grBad + format ["[row %1 named %2, saved %3]", grK, grLive select 0, grRow select 0]}; grI = gmZoneIndex (grRow select 0); if (grI < 0) then {grBad = grBad + format ["[%1]nozone ", grRow select 0]} else {grZ = gmZone grI; if ((grZ select 1) != (grRow select 1)) then {grBad = grBad + format ["[%1]type ", grRow select 0]}; if (([(grZ select 8) select 0, (grZ select 8) select 1, 0] distance [grRow select 2, grRow select 3, 0]) > 1) then {grBad = grBad + format ["[%1]moved ", grRow select 0]}; if (((gmZoneSim grI) select 7) != (grRow select 4)) then {grBad = grBad + format ["[%1]max ", grRow select 0]}}; grK = grK + 1}
triAssertEq [grBad, ""]
// nothing generated twice: exactly the saved rows carry the generated names
grN = 0
grI = 0
while {grI < gmZoneCount} do {if (((gmZone grI) select 1) == "OUTPOST") then {grN = grN + 1}; grI = grI + 1}
triAssertEq [grN, 2 + (count gsRows)]
triAssertEq [((gmZone (gmZoneIndex gsFreed)) select 2), gmResistanceSide]
triAssertEq [((gmOccupierForces) select 1), gsPool]
triEndTest
