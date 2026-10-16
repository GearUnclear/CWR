// ============================================================================
//  Generated occupier bases (OCCUPIER-FORCES.md 5.7, step 1), Abel.
//  Ground truth (2026-10-16, two runs on the agent shadow, random seeds):
//  the scorer adds two OUTPOSTs, one south-west (La Riviere Outpost
//  [3975, 4775]) and one west on a track (Vigny Outpost [4175, 7025]); the
//  names and spots move a little with the seed, the rules below do not.
//    g1  gmGeneratedBases lists 2 rows; the occupier holds 6 military zones
//        (4 authored + 2), inside the density rule's ceil(K / 3) .. ceil(K / 2)
//        (K 14: 5 .. 7) and the asked 5 .. 6.
//    g2  each row is an occupier OUTPOST zone of the table with garrisonMax 6,
//        a full reserve (far from the Camp), the seeded flag, countsForWin.
//    g3  every CITY is within 2500 m (straight line) of an occupier military
//        zone on its own landmass (gmLandmass).
//    g4  each generated base: 1600 m or more from every other occupier base
//        and from the Camp, 500 m or more from every town, 300 m or more from
//        every zone; no road within 40 m and a road within 300 m (the
//        60..250 m band, with the road-piece tolerance); dry ground over its
//        spawn square (centre 2 m, corners 0.5 m up).
//    g5  at least one generated base sits on a track (a third, rounded up).
//    g6  conservation (OCCUPIER-FORCES.md 5.7, the owner's rule): R over the
//        authored roster is 32, the generated garrisons Rgen 12, and
//        poolStart max(24, R + 2K) - Rgen = 60 - 12 = 48.
//    g7  the win roster takes all six.
// ============================================================================

triSimUntil { GM_LIB_READY }
// ---- helpers (globals, the gmcore lib.sqs idiom: no locals in called code) --
bgGenNames = []
{bgGenNames = bgGenNames + [_x select 0]} forEach gmGeneratedBases
// military occupier zones: [indices]
bgFnMil = {bgMI = 0; bgML = []; while {bgMI < gmZoneCount} do {bgMZ = gmZone bgMI; if (((bgMZ select 1) != "CAMP") and ((bgMZ select 1) != "CITY") and ((bgMZ select 2) == gmOccupierSide)) then {bgML = bgML + [bgMI]}; bgMI = bgMI + 1}; bgML}
// 2D distance between two gmZone positions: [p, q] call bgFnD
bgFnD = {bgDA = _this select 0; bgDB = _this select 1; [bgDA select 0, bgDA select 1, 0] distance [bgDB select 0, bgDB select 1, 0]}
// R over the authored roster (generated bases excluded)
bgFnR = {bgRS = 0; {if (not (((gmZone _x) select 0) in bgGenNames)) then {bgRS = bgRS + ((gmZoneSim _x) select 7)}} forEach (call bgFnMil); bgRS}
bgFnRgen = {bgGS = 0; {bgGI = gmZoneIndex (_x select 0); if (bgGI >= 0) then {bgGS = bgGS + ((gmZoneSim bgGI) select 7)}} forEach gmGeneratedBases; bgGS}

triSimUntil { ((gmOccupierForces) select 2) != "OFF" }

// ---- g1: two generated bases, six military zones --------------------------
bgGen = gmGeneratedBases
triAssertEq [(count bgGen), 2]
bgMil = call bgFnMil
triAssertEq [(count bgMil), 6]
triAssertGe [(count bgMil), 5]
triAssertLe [(count bgMil), 6]

// ---- g2: each row is an occupier OUTPOST zone ------------------------------
bgBad = ""
{bgI = gmZoneIndex (_x select 0); if (bgI < 0) then {bgBad = bgBad + format ["[%1]nozone ", _x select 0]} else {bgZ = gmZone bgI; if ((bgZ select 1) != "OUTPOST") then {bgBad = bgBad + format ["[%1]type=%2 ", _x select 0, bgZ select 1]}; if ((bgZ select 2) != gmOccupierSide) then {bgBad = bgBad + format ["[%1]owner=%2 ", _x select 0, bgZ select 2]}; if (((gmZoneSim bgI) select 7) != 6) then {bgBad = bgBad + format ["[%1]max=%2 ", _x select 0, (gmZoneSim bgI) select 7]}; if ((bgZ select 3) != 6) then {bgBad = bgBad + format ["[%1]reserve=%2 ", _x select 0, bgZ select 3]}; if (not (bgZ select 10)) then {bgBad = bgBad + format ["[%1]notseeded ", _x select 0]}; if (not (_x select 4)) then {bgBad = bgBad + format ["[%1]minor ", _x select 0]}; if (([(bgZ select 8), (_x select 2)] call bgFnD) > 1) then {bgBad = bgBad + format ["[%1]moved ", _x select 0]}}} forEach bgGen
triAssertEq [bgBad, ""]

// ---- g3: every town within 2.5 km of a base on its landmass ----------------
bgBad = ""
bgI = 0
while {bgI < gmZoneCount} do {bgZ = gmZone bgI; if ((bgZ select 1) == "CITY") then {bgP = bgZ select 8; bgL = gmLandmass [bgP select 0, bgP select 1, 0]; bgBest = 1000000; {bgQ = (gmZone _x) select 8; if ((gmLandmass [bgQ select 0, bgQ select 1, 0]) == bgL) then {bgDd = [bgP, bgQ] call bgFnD; if (bgDd < bgBest) then {bgBest = bgDd}}} forEach bgMil; if (bgBest > 2500) then {bgBad = bgBad + format ["[%1 %2 m]", bgZ select 0, bgBest]}}; bgI = bgI + 1}
triAssertEq [bgBad, ""]

// ---- g4: the placement rules -----------------------------------------------
bgCamp = (gmZone (gmZoneIndex "Camp")) select 8
bgBad = ""
{bgI = gmZoneIndex (_x select 0); bgP = (gmZone bgI) select 8; if (([bgP, bgCamp] call bgFnD) < 1600) then {bgBad = bgBad + format ["[%1]camp ", _x select 0]}; bgJ = 0; while {bgJ < gmZoneCount} do {if (bgJ != bgI) then {bgQZ = gmZone bgJ; bgDd = [bgP, bgQZ select 8] call bgFnD; if (bgDd < 300) then {bgBad = bgBad + format ["[%1]zone %2 ", _x select 0, bgQZ select 0]}; if (((bgQZ select 1) == "CITY") and (bgDd < 500)) then {bgBad = bgBad + format ["[%1]town %2 ", _x select 0, bgQZ select 0]}; if ((bgJ in bgMil) and (bgDd < 1600)) then {bgBad = bgBad + format ["[%1]base %2 ", _x select 0, bgQZ select 0]}}; bgJ = bgJ + 1}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads 40)) > 0) then {bgBad = bgBad + format ["[%1]onroad ", _x select 0]}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads 300)) == 0) then {bgBad = bgBad + format ["[%1]noroad ", _x select 0]}} forEach bgGen
triAssertEq [bgBad, ""]
bgBad = ""
{bgP = _x select 2; {bgV = "HeliHEmpty" createVehicle [(bgP select 0) + (_x select 0), (bgP select 1) + (_x select 1), 0]; bgH = (getPosASL bgV) select 2; deleteVehicle bgV; if (bgH < (_x select 2)) then {bgBad = bgBad + format ["[%1 %2,%3 h=%4]", bgP, _x select 0, _x select 1, bgH]}} forEach [[0, 0, 2], [-25, -25, 0.5], [25, -25, 0.5], [-25, 25, 0.5], [25, 25, 0.5]]} forEach bgGen
triAssertEq [bgBad, ""]

// ---- g5: the track share ---------------------------------------------------
bgTracks = 0
{if (_x select 6) then {bgTracks = bgTracks + 1}} forEach bgGen
triAssertGe [bgTracks, 1]

// ---- g6: the generated garrisons come out of poolStart ---------------------
triAssertEq [(call bgFnR), 32]
triAssertEq [(call bgFnRgen), 12]
bgPool = (call bgFnR) + 2 * ((gmOccupierForces) select 7)
if (bgPool < 24) then {bgPool = 24}
bgPool = bgPool - (call bgFnRgen)
if (bgPool < 0) then {bgPool = 0}
triAssertEq [((gmOccupierForces) select 1), bgPool]
triAssertEq [((gmOccupierForces) select 1), 48]

// ---- g7: the win roster takes all six ---------------------------------------
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
triAssertEq [((gmCampaignEnd) select 5), 6]

triEndTest
