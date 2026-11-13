// ============================================================================
//  Occupation-network detection (issue #76), Lebanon80: no AIRFIELD and no
//  SEAPORT in the theatre, by design (OFFSCREEN-CONTRACT.md section 5,
//  Lebanon80 row; airpower is off-map, #35).  Boots the SHIPPED
//  Guerrilla.Lebanon80 template on the @LoBo Lebanon80 world.  Ground truth
//  (surveyed 2026-10-14 with PoseidonTools terrain objects / pbo show and a
//  tri probe): LoBo_Leb.pbo's CfgWorlds >> Lebanon80 carries ilsPosition[]
//  {8628, 2524} (Ramat David Airbase, Israel, about 7 km south of the
//  theatre, whose zones span x 11500..14071, z 9800..13000), and Tyre has a
//  quay of four nabrezi / nabrezi_najezd pieces at x 8588, z 9739..9916 on
//  the waterline; the big ports (Haifa [6120, 5415] / [6996, 5716], Beirut
//  [12913, 18076]) lie outside the theatre.  The template authors no
//  AIRFIELD / SEAPORT, so seedBases would default on: it must keep these out
//  (playRegion[] or seedBases = 0).
//    l1  no AIRFIELD and no SEAPORT zone, gmSeededBases empty;
//    l2  the authored zones are untouched: the two OUTPOSTs (R = 6 + 8 = 14),
//        poolStart = max(24, R + 2K), and no zone carries the seeded flag
//        (seedCities = 0 here).
//    l3  no generated bases either (generateBases = 0 by design,
//        OCCUPIER-FORCES.md 5.7).
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }

// ---- l1: nothing seeded ------------------------------------------------------
lbNA = 0
lbNS = 0
lbNSeed = 0
lbI = 0
while {lbI < gmZoneCount} do {lbZ = gmZone lbI; if ((lbZ select 1) == "AIRFIELD") then {lbNA = lbNA + 1}; if ((lbZ select 1) == "SEAPORT") then {lbNS = lbNS + 1}; if ((format ["%1", lbZ select 10]) == "true") then {lbNSeed = lbNSeed + 1}; lbI = lbI + 1}
triAssertEq [lbNA, 0]
triAssertEq [lbNS, 0]
triAssertEq [(count gmSeededBases), 0]

// ---- l2: the authored campaign as it was -------------------------------------
triAssertEq [lbNSeed, 0]
lbR = 0
lbNO = 0
lbI = 0
while {lbI < gmZoneCount} do {lbZ = gmZone lbI; if (((lbZ select 1) == "OUTPOST") and ((lbZ select 2) == gmOccupierSide)) then {lbR = lbR + ((gmZoneSim lbI) select 7); lbNO = lbNO + 1}; lbI = lbI + 1}
triAssertEq [lbNO, 2]
triAssertEq [lbR, 14]
lbExp = lbR + 2 * ((gmOccupierForces) select 7)
if (lbExp < 24) then {lbExp = 24}
triAssertEq [((gmOccupierForces) select 1), lbExp]

// ---- l3: no generated bases --------------------------------------------------
triAssertEq [(count gmGeneratedBases), 0]

triEndTest
