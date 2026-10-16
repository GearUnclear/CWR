// ============================================================================
//  Generated occupier bases (OCCUPIER-FORCES.md 5.7, step 1), Eden.
//  Ground truth (2026-10-16, agent shadow, random seeds): the scorer adds two
//  OUTPOSTs (Le Moule Outpost [3525, 4925], Tyrone Outpost [5425, 9375]);
//  Regina stays open, because no 150 m square within 2.5 km of it is dry,
//  flat (under 0.10), free of objects and on the island's road band (the
//  "Regina is open" INFO line counts the refusals), and the detected Levie
//  Post stands within 60 m of its road.
//    e1  gmGeneratedBases lists 2 to 4 rows; the occupier's military zones
//        number 5 + that, at most ceil(K / 2) = 9 (K 18).
//    e2  each row is an occupier OUTPOST zone with garrisonMax 6, the seeded
//        flag and countsForWin.
//    e3  at most one town (of the six open before) is still more than 2.5 km
//        from an occupier military zone on its landmass.
//    e4  the placement rules, as guerrilla_bases_generate_abel g4.
//    e5  conservation (OCCUPIER-FORCES.md 5.7): R over the authored and
//        seeded roster is 18 + the seeded garrisons, poolStart
//        max(24, R + 2K) less the generated garrisons (Rgen).
//    e6  the win roster takes every one of them.
// ============================================================================

triSimUntil { GM_LIB_READY }
bgGenNames = []
{bgGenNames = bgGenNames + [_x select 0]} forEach gmGeneratedBases
bgFnMil = {bgMI = 0; bgML = []; while {bgMI < gmZoneCount} do {bgMZ = gmZone bgMI; if (((bgMZ select 1) != "CAMP") and ((bgMZ select 1) != "CITY") and ((bgMZ select 2) == gmOccupierSide)) then {bgML = bgML + [bgMI]}; bgMI = bgMI + 1}; bgML}
bgFnD = {bgDA = _this select 0; bgDB = _this select 1; [bgDA select 0, bgDA select 1, 0] distance [bgDB select 0, bgDB select 1, 0]}
bgFnR = {bgRS = 0; {if (not (((gmZone _x) select 0) in bgGenNames)) then {bgRS = bgRS + ((gmZoneSim _x) select 7)}} forEach (call bgFnMil); bgRS}
bgFnRgen = {bgGS = 0; {bgGI = gmZoneIndex (_x select 0); if (bgGI >= 0) then {bgGS = bgGS + ((gmZoneSim bgGI) select 7)}} forEach gmGeneratedBases; bgGS}
bgFnSeededR = {bgQS = 0; {bgQI = gmZoneIndex (_x select 0); if (bgQI >= 0) then {bgQS = bgQS + ((gmZoneSim bgQI) select 7)}} forEach gmSeededBases; bgQS}

triSimUntil { ((gmOccupierForces) select 2) != "OFF" }

// ---- e1 ---------------------------------------------------------------------
bgGen = gmGeneratedBases
triAssertGe [(count bgGen), 2]
triAssertLe [(count bgGen), 4]
bgMil = call bgFnMil
triAssertEq [(count bgMil), 5 + (count bgGen)]
triAssertLe [(count bgMil), 9]

// ---- e2 ---------------------------------------------------------------------
bgBad = ""
{bgI = gmZoneIndex (_x select 0); if (bgI < 0) then {bgBad = bgBad + format ["[%1]nozone ", _x select 0]} else {bgZ = gmZone bgI; if ((bgZ select 1) != "OUTPOST") then {bgBad = bgBad + format ["[%1]type=%2 ", _x select 0, bgZ select 1]}; if ((bgZ select 2) != gmOccupierSide) then {bgBad = bgBad + format ["[%1]owner=%2 ", _x select 0, bgZ select 2]}; if (((gmZoneSim bgI) select 7) != 6) then {bgBad = bgBad + format ["[%1]max=%2 ", _x select 0, (gmZoneSim bgI) select 7]}; if (not (bgZ select 10)) then {bgBad = bgBad + format ["[%1]notseeded ", _x select 0]}; if (not (_x select 4)) then {bgBad = bgBad + format ["[%1]minor ", _x select 0]}}} forEach bgGen
triAssertEq [bgBad, ""]

// ---- e3 ---------------------------------------------------------------------
bgOpen = []
bgI = 0
while {bgI < gmZoneCount} do {bgZ = gmZone bgI; if ((bgZ select 1) == "CITY") then {bgP = bgZ select 8; bgL = gmLandmass [bgP select 0, bgP select 1, 0]; bgBest = 1000000; {bgQ = (gmZone _x) select 8; if ((gmLandmass [bgQ select 0, bgQ select 1, 0]) == bgL) then {bgDd = [bgP, bgQ] call bgFnD; if (bgDd < bgBest) then {bgBest = bgDd}}} forEach bgMil; if (bgBest > 2500) then {bgOpen = bgOpen + [bgZ select 0]}}; bgI = bgI + 1}
triAssertLe [(count bgOpen), 1]

// ---- e4 ---------------------------------------------------------------------
bgCamp = (gmZone (gmZoneIndex "Camp")) select 8
bgBad = ""
{bgI = gmZoneIndex (_x select 0); bgP = (gmZone bgI) select 8; if (([bgP, bgCamp] call bgFnD) < 1600) then {bgBad = bgBad + format ["[%1]camp ", _x select 0]}; bgJ = 0; while {bgJ < gmZoneCount} do {if (bgJ != bgI) then {bgQZ = gmZone bgJ; bgDd = [bgP, bgQZ select 8] call bgFnD; if (bgDd < 300) then {bgBad = bgBad + format ["[%1]zone %2 ", _x select 0, bgQZ select 0]}; if (((bgQZ select 1) == "CITY") and (bgDd < 500)) then {bgBad = bgBad + format ["[%1]town %2 ", _x select 0, bgQZ select 0]}; if ((bgJ in bgMil) and (bgDd < 1600)) then {bgBad = bgBad + format ["[%1]base %2 ", _x select 0, bgQZ select 0]}}; bgJ = bgJ + 1}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads 40)) > 0) then {bgBad = bgBad + format ["[%1]onroad ", _x select 0]}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads 300)) == 0) then {bgBad = bgBad + format ["[%1]noroad ", _x select 0]}} forEach bgGen
triAssertEq [bgBad, ""]
bgBad = ""
{bgP = _x select 2; {bgV = "HeliHEmpty" createVehicle [(bgP select 0) + (_x select 0), (bgP select 1) + (_x select 1), 0]; bgH = (getPosASL bgV) select 2; deleteVehicle bgV; if (bgH < (_x select 2)) then {bgBad = bgBad + format ["[%1 %2,%3 h=%4]", bgP, _x select 0, _x select 1, bgH]}} forEach [[0, 0, 2], [-25, -25, 0.5], [25, -25, 0.5], [-25, 25, 0.5], [25, 25, 0.5]]} forEach bgGen
triAssertEq [bgBad, ""]

// ---- e5 ---------------------------------------------------------------------
triAssertEq [(call bgFnR), 18 + (call bgFnSeededR)]
triAssertGe [(call bgFnRgen), 12]
bgPool = (call bgFnR) + 2 * ((gmOccupierForces) select 7)
if (bgPool < 24) then {bgPool = 24}
bgPool = bgPool - (call bgFnRgen)
if (bgPool < 0) then {bgPool = 0}
triAssertEq [((gmOccupierForces) select 1), bgPool]

// ---- e6 ---------------------------------------------------------------------
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
triAssertEq [((gmCampaignEnd) select 5), count bgMil]

triEndTest
