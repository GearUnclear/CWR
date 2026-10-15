// ============================================================================
//  Occupation-network detection (issue #76), Sinai (@LoBo).  Boots the
//  SHIPPED Guerrilla.Sinai template on the @LoBo Sinai world.  Ground truth
//  (surveyed 2026-10-14 with PoseidonTools terrain objects / pbo show and a
//  tri probe):
//    * the Wadi airstrip: the fenced compound x 7687..8215, z 3167..4190,
//      hangar.p3d x2 [8079, 3767], brown_start_turm [8045, 3732], a HAS
//      [8067, 3598]; object cluster centroid [8130, 3774];
//    * the Egyptian Airbase: Names point [3443, 1479], hangar01 x5 from
//      [3573, 1583], a fenced base over x 2399..3611, z 812..2368; today it is
//      seeded as a CITY (a C7 defect);
//    * Sharm harbour: 58 AGS pier / crane / crate objects round [10729, 3488]
//      (1.47 km E of the Sharm el Sheikh Names point, which is in the sea);
//    * El Tor harbour: 10 AGS pier / crane objects round [5923, 3202], 985 m W
//      of El Tor.
//  NEITHER airfield has an ILS entry (lost.pbo's only ilsPosition is the
//  Israeli Airbase [12176, 11476]), so s1/s2 hold only when the template
//  authors them or the detector finds airfields by their objects.
//    s1  an AIRFIELD within 400 m of the Wadi airstrip centroid;
//    s2  an AIRFIELD within 700 m of the Egyptian Airbase Names point, the
//        ONLY zone there (no CITY "Egyptian Airbase" left beside it), and
//        ONE AIRFIELD for the whole fenced base (its hangar groups at
//        x 2662..2839 and x 3573 are 1 km apart inside one perimeter: no
//        second AIRFIELD within 1300 m of its centre [3005, 1590]);
//    s3  a SEAPORT within 450 m of the Sharm harbour and one within 450 m of
//        the El Tor harbour (the zone centre walks inland up to 300 m);
//    s4  every AIRFIELD / SEAPORT is occupier-owned with a garrison, and the
//        roster R (the authored 6 + 8 = 14 plus every base added) sizes
//        poolStart = max(24, R + 2K).
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

// ---- s1: the Wadi airstrip -------------------------------------------------
bdNW = ["AIRFIELD", 8130, 3774] call bdFnNear
triAssertGe [(bdNW select 0), 0]
triAssertLe [(bdNW select 1), 400]

// ---- s2: the Egyptian Airbase, one zone, an AIRFIELD -----------------------
bdNE = ["AIRFIELD", 3443, 1479] call bdFnNear
triAssertGe [(bdNE select 0), 0]
triAssertLe [(bdNE select 1), 700]
triAssertEq [([3443, 1479, 700] call bdFnWithin), 1]
bdEgC = gmZoneIndex "Egyptian Airbase"
bdEgT = "none"
if (bdEgC >= 0) then {bdEgT = (gmZone bdEgC) select 1}
triAssertNe [bdEgT, "CITY"]
bdEgN = 0
bdI = 0
while {bdI < gmZoneCount} do {bdZ = gmZone bdI; if (((bdZ select 1) == "AIRFIELD") and (([(bdZ select 8) select 0, (bdZ select 8) select 1, 0] distance [3005, 1590, 0]) <= 1300)) then {bdEgN = bdEgN + 1}; bdI = bdI + 1}
triAssertEq [bdEgN, 1]

// ---- s3: the Sharm and El Tor harbours -------------------------------------
bdNSh = ["SEAPORT", 10729, 3488] call bdFnNear
triAssertGe [(bdNSh select 0), 0]
triAssertLe [(bdNSh select 1), 450]
bdNEt = ["SEAPORT", 5923, 3202] call bdFnNear
triAssertGe [(bdNEt select 0), 0]
triAssertLe [(bdNEt select 1), 450]

// ---- s4: owners, garrisons, the roster ---------------------------------------
bdBad = 0
bdI = 0
while {bdI < gmZoneCount} do {bdZ = gmZone bdI; if ((bdZ select 1) in ["AIRFIELD", "SEAPORT"]) then {if (((bdZ select 2) != gmOccupierSide) or (((gmZoneSim bdI) select 7) < 1)) then {bdBad = bdBad + 1}}; bdI = bdI + 1}
triAssertEq [bdBad, 0]
bdAuth = 0
bdI = 0
while {bdI < gmZoneCount} do {bdZ = gmZone bdI; if (((bdZ select 1) in ["AIRFIELD", "SEAPORT", "OUTPOST"]) and ((bdZ select 2) == gmOccupierSide) and ((format ["%1", bdZ select 10]) == "false")) then {bdAuth = bdAuth + ((gmZoneSim bdI) select 7)}; bdI = bdI + 1}
triAssertEq [(call bdFnR), bdAuth + (call bdFnSeededR)]
triAssertGe [bdAuth, 14]
triAssertEq [((gmOccupierForces) select 1), (call bdFnPool)]

triEndTest
