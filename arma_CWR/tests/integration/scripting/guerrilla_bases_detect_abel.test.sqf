// ============================================================================
//  Occupation-network detection (issue #76), Abel: the AUTHORED hubs win.
//  Boots the SHIPPED Guerrilla.Abel template.  Ground truth (surveyed
//  2026-10-14 with PoseidonTools terrain objects / config debin and a tri
//  probe): CfgWorlds >> Abel ILS anchor [7922, 9590],
//  runway x 7922 z 9590..10450, first taxiIn point [7991, 9990] (the
//  authored Airfield [8030, 9905] is 333 m from the anchor, 108 m from the
//  runway); the Le Port quay is a molo_beton [8476, 3754] + molo_krychle
//  [8472, 3774] pair standing in deep water.
//    b1  the three authored hubs stand with their authored types and the
//        occupier as owner: Airfield (AIRFIELD), Le Port Harbour (SEAPORT),
//        Southeast Compound (OUTPOST); exactly one AIRFIELD and one SEAPORT;
//        none of the authored zones carries the seeded flag (gmZone slot 10).
//    b2  the template authors an AIRFIELD and a SEAPORT, so seedBases
//        defaults off: gmSeededBases is empty.
//    b3  the detector still ran (gmDetectedBases, the dry run): it found
//        the runway, COVERED by "Airfield" and within 300 m of the runway
//        segment, and the Le Port quay, COVERED by "Le Port Harbour" and
//        within 300 m of the pier; nothing has status "seed".
//    b4  the roster R is the authored 6 + 6 + 8 + 12 = 32 and poolStart is
//        max(24, R + 2K).
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
// military zone the occupier owns at boot; gmZoneSim slot 7 is garrisonMax)
bdFnR = {bdRS = 0; bdRI = 0; while {bdRI < gmZoneCount} do {bdRZ = gmZone bdRI; if (((bdRZ select 1) in ["AIRFIELD", "SEAPORT", "OUTPOST"]) and ((bdRZ select 2) == gmOccupierSide)) then {bdRS = bdRS + ((gmZoneSim bdRI) select 7)}; bdRI = bdRI + 1}; bdRS}
// garrisonMax summed over the gmSeededBases rows' zones
bdFnSeededR = {bdQS = 0; {bdQI = gmZoneIndex (_x select 0); if (bdQI >= 0) then {bdQS = bdQS + ((gmZoneSim bdQI) select 7)}} forEach gmSeededBases; bdQS}
// the poolStart the roster implies: max(forcesPoolMin 24, R + forcesPoolPerTown 2 * K)
bdFnPool = {bdPE = (call bdFnR) + 2 * ((gmOccupierForces) select 7); if (bdPE < 24) then {bdPE = 24}; bdPE}

triSimUntil { ((gmOccupierForces) select 2) != "OFF" }

// ---- b1: the authored hubs -------------------------------------------------
bdAir = gmZoneIndex "Airfield"
triAssertGe [bdAir, 0]
triAssertEq [((gmZone bdAir) select 1), "AIRFIELD"]
triAssertEq [((gmZone bdAir) select 2), gmOccupierSide]
bdSea = gmZoneIndex "Le Port Harbour"
triAssertGe [bdSea, 0]
triAssertEq [((gmZone bdSea) select 1), "SEAPORT"]
triAssertEq [((gmZone bdSea) select 2), gmOccupierSide]
bdCmp = gmZoneIndex "Southeast Compound"
triAssertGe [bdCmp, 0]
triAssertEq [((gmZone bdCmp) select 1), "OUTPOST"]
triAssertEq [((gmZone bdCmp) select 2), gmOccupierSide]
triAssertEq [("AIRFIELD" call bdFnCount), 1]
triAssertEq [("SEAPORT" call bdFnCount), 1]
triAssertEq [(count (gmZone bdAir)), 11]
triAssertEq [(format ["%1", (gmZone bdAir) select 10]), "false"]
triAssertEq [(format ["%1", (gmZone bdSea) select 10]), "false"]
triAssertEq [(format ["%1", (gmZone bdCmp) select 10]), "false"]

// ---- b2: nothing seeded ----------------------------------------------------
triAssertEq [(count gmSeededBases), 0]

// ---- b3: the dry run finds both hubs and calls them covered ----------------
bdDet = gmDetectedBases
triAssertGe [(count bdDet), 2]
bdDetAir = -1
bdDetSea = -1
bdDetSeed = 0
bdI = 0
while {bdI < (count bdDet)} do {bdRow = bdDet select bdI; if ((bdRow select 5) == "seed") then {bdDetSeed = bdDetSeed + 1}; if (((bdRow select 1) == "AIRFIELD") and (bdDetAir < 0)) then {bdDetAir = bdI}; if (((bdRow select 1) == "SEAPORT") and ([((bdRow select 2) select 0), ((bdRow select 2) select 1), 0] distance [8476, 3754, 0]) < 300) then {bdDetSea = bdI}; bdI = bdI + 1}
triAssertEq [bdDetSeed, 0]
triAssertGe [bdDetAir, 0]
bdRowAir = bdDet select bdDetAir
triAssertEq [(bdRowAir select 5), "covered"]
triAssertEq [(bdRowAir select 6), "Airfield"]
triAssertLe [([[((bdRowAir select 2) select 0), ((bdRowAir select 2) select 1)], 7922, 9590, 7922, 10450] call bdFnSeg), 300]
triAssertGe [bdDetSea, 0]
bdRowSea = bdDet select bdDetSea
triAssertEq [(bdRowSea select 5), "covered"]
triAssertEq [(bdRowSea select 6), "Le Port Harbour"]

// ---- b4: the roster --------------------------------------------------------
triAssertEq [(call bdFnR), 32]
triAssertEq [((gmOccupierForces) select 1), (call bdFnPool)]

triEndTest
