// ============================================================================
//  Occupation-network detection (issue #76), Eden: the runtime seed.
//  Boots the SHIPPED Guerrilla.Eden template, which authors no AIRFIELD and
//  no SEAPORT, so seedBases defaults on.  Ground truth (surveyed 2026-10-14
//  with PoseidonTools terrain objects / config debin and a tri probe):
//  CfgWorlds >> Eden ILS anchor [4772, 10923], runway x 4772 z 10923..11801
//  (ilsTaxiOff), first taxiIn point [4841, 11323]; the pier [7392, 6835] is
//  a molo_beton + molo_krychle pair standing in deep water, the same
//  composition as Abel's authored Le Port Harbour, so it is a harbour.  The
//  three generated outposts stand on empty ground (R authored = 3 x 6 = 18).
//    e1  one seeded AIRFIELD within 300 m of the runway segment (NOT of the
//        anchor: the taxiIn apron is 406 m from it), occupier-owned, with a
//        garrison, flagged seeded (gmZone slot 10) and listed by
//        gmSeededBases with confidence > 0 and evidence;
//    e2  one seeded SEAPORT within 300 m of the pier, likewise;
//    e3  nothing else seeded: exactly one AIRFIELD, one SEAPORT, the three
//        authored OUTPOSTs plus the generated ones (generateBases = 1,
//        OCCUPIER-FORCES.md 5.7: gmGeneratedBases, never gmSeededBases;
//        guerrilla_bases_generate_eden has their rules), and the explicit
//        CITY Saint Phillippe (216 m from the ILS anchor) still stands;
//    e4  the roster: R = 18 + the seeded garrisons (the generated bases add
//        nothing to R), poolStart = max(24, R + 2K).
//  The save round trip of the seeded bases is guerrilla_bases_detect_eden_save.seq.
// ============================================================================

triSimUntil { GM_LIB_READY }
// ---- helpers (globals, the gmcore lib.sqs idiom: no locals in called code) --
// distance from [x, z] to the segment [ax, az]..[bx, bz]:
//   [[x, z], ax, az, bx, bz] call bdFnSeg
bdFnSeg = {bdSP = _this select 0; bdSDX = (_this select 3) - (_this select 1); bdSDZ = (_this select 4) - (_this select 2); bdST = (((bdSP select 0) - (_this select 1)) * bdSDX + ((bdSP select 1) - (_this select 2)) * bdSDZ) / (bdSDX * bdSDX + bdSDZ * bdSDZ); if (bdST < 0) then {bdST = 0}; if (bdST > 1) then {bdST = 1}; bdSQX = (bdSP select 0) - ((_this select 1) + bdST * bdSDX); bdSQZ = (bdSP select 1) - ((_this select 2) + bdST * bdSDZ); sqrt (bdSQX * bdSQX + bdSQZ * bdSQZ)}
// zones of a type: "AIRFIELD" call bdFnCount
bdFnCount = {bdCN = 0; bdCI = 0; while {bdCI < gmZoneCount} do {if (((gmZone bdCI) select 1) == _this) then {bdCN = bdCN + 1}; bdCI = bdCI + 1}; bdCN}
// the zone of a type nearest [x, z] (any type for ""): [type, x, z] call bdFnNear -> [index, metres] ([-1, 1e9] when none)
bdFnNear = {bdNT = _this select 0; bdNP = [_this select 1, _this select 2, 0]; bdNB = -1; bdND = 1000000000; bdNI = 0; while {bdNI < gmZoneCount} do {bdNZ = gmZone bdNI; if ((bdNT == "") or ((bdNZ select 1) == bdNT)) then {bdNQ = [(bdNZ select 8) select 0, (bdNZ select 8) select 1, 0]; if ((bdNQ distance bdNP) < bdND) then {bdND = bdNQ distance bdNP; bdNB = bdNI}}; bdNI = bdNI + 1}; [bdNB, bdND]}
// zones of any type within r of [x, z]: [x, z, r] call bdFnWithin
bdFnWithin = {bdWP = [_this select 0, _this select 1, 0]; bdWN = 0; bdWI = 0; while {bdWI < gmZoneCount} do {bdWZ = gmZone bdWI; if (([(bdWZ select 8) select 0, (bdWZ select 8) select 1, 0] distance bdWP) <= (_this select 2)) then {bdWN = bdWN + 1}; bdWI = bdWI + 1}; bdWN}
// R: the occupier roster's summed garrisonMax (OCCUPIER-FORCES.md 5.1: every
// military zone the occupier owns at boot; gmZoneSim slot 7 is garrisonMax).
// Generated bases (5.7, gmGeneratedBases) are not in R: the pool gives them
// no share (the conservation stand-in), so they are skipped here.
bdGenNames = []
{bdGenNames = bdGenNames + [_x select 0]} forEach gmGeneratedBases
bdFnR = {bdRS = 0; bdRI = 0; while {bdRI < gmZoneCount} do {bdRZ = gmZone bdRI; if (((bdRZ select 1) in ["AIRFIELD", "SEAPORT", "OUTPOST"]) and ((bdRZ select 2) == gmOccupierSide) and (not ((bdRZ select 0) in bdGenNames))) then {bdRS = bdRS + ((gmZoneSim bdRI) select 7)}; bdRI = bdRI + 1}; bdRS}
// garrisonMax summed over the gmSeededBases rows' zones
bdFnSeededR = {bdQS = 0; {bdQI = gmZoneIndex (_x select 0); if (bdQI >= 0) then {bdQS = bdQS + ((gmZoneSim bdQI) select 7)}} forEach gmSeededBases; bdQS}
// the poolStart the roster implies: max(forcesPoolMin 24, R + forcesPoolPerTown 2 * K)
bdFnPool = {bdPE = (call bdFnR) + 2 * ((gmOccupierForces) select 7); if (bdPE < 24) then {bdPE = 24}; bdPE}

triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
bdSeeded = gmSeededBases
triAssertGe [(count bdSeeded), 2]

// ---- e1: the seeded AIRFIELD -----------------------------------------------
triAssertEq [("AIRFIELD" call bdFnCount), 1]
bdNA = ["AIRFIELD", 4772, 11362] call bdFnNear
bdAir = bdNA select 0
triAssertGe [bdAir, 0]
bdAirPos = (gmZone bdAir) select 8
triAssertLe [([[(bdAirPos select 0), (bdAirPos select 1)], 4772, 10923, 4772, 11801] call bdFnSeg), 300]
triAssertEq [((gmZone bdAir) select 2), gmOccupierSide]
triAssertGe [((gmZoneSim bdAir) select 7), 1]
triAssertEq [(format ["%1", (gmZone bdAir) select 10]), "true"]
bdRowA = []
{if ((_x select 0) == ((gmZone bdAir) select 0)) then {bdRowA = _x}} forEach bdSeeded
triAssertEq [(count bdRowA), 5]
triAssertEq [(bdRowA select 1), "AIRFIELD"]
triAssertGe [(bdRowA select 3), 0.5]
triAssertGe [(count (bdRowA select 4)), 1]

// ---- e2: the seeded SEAPORT at the pier ------------------------------------
triAssertEq [("SEAPORT" call bdFnCount), 1]
bdNS = ["SEAPORT", 7392, 6835] call bdFnNear
bdSea = bdNS select 0
triAssertGe [bdSea, 0]
triAssertLe [(bdNS select 1), 300]
triAssertEq [((gmZone bdSea) select 2), gmOccupierSide]
triAssertGe [((gmZoneSim bdSea) select 7), 1]
triAssertEq [(format ["%1", (gmZone bdSea) select 10]), "true"]
bdRowS = []
{if ((_x select 0) == ((gmZone bdSea) select 0)) then {bdRowS = _x}} forEach bdSeeded
triAssertEq [(count bdRowS), 5]
triAssertEq [(bdRowS select 1), "SEAPORT"]
triAssertGe [(count (bdRowS select 4)), 2]

// ---- e3: nothing else ------------------------------------------------------
triAssertEq [(count bdSeeded), 2]
triAssertEq [("OUTPOST" call bdFnCount), 3 + (count gmGeneratedBases)]
bdSP = gmZoneIndex "Saint Phillippe"
triAssertGe [bdSP, 0]
triAssertEq [((gmZone bdSP) select 1), "CITY"]

// ---- e4: the roster counts the seeded garrisons ----------------------------
triAssertEq [(call bdFnR), 18 + (call bdFnSeededR)]
triAssertEq [((gmOccupierForces) select 1), (call bdFnPool)]

triEndTest
