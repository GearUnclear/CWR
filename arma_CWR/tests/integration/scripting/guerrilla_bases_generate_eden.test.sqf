// ============================================================================
//  Generated occupier bases (OCCUPIER-FORCES.md 5.7, steps 1 and 3), Eden.
//  Ground truth (2026-10-17, agent shadow, random seeds): the scorer adds two
//  OUTPOSTs (Le Moule Outpost about [3575, 4875], Tyrone Outpost about
//  [5425, 9225]) and, for Regina, a minor kind: no 150 m square within
//  2.5 km of Regina is dry, flat (under 0.10) and forest-free (the coast,
//  the slopes and the woods refuse all of them), so step 3 lets a minor
//  kind's single 50 m cell take the gap (Morton Coastal Post about [5625,
//  3775], deep water within 400 m).  The detected Levie Post stands within
//  60 m of its road.
//    e1  gmGeneratedBases lists 2 to 4 rows; the occupier's military zones
//        number 5 + that, at most ceil(K / 2) = 9 (K 18).
//    e2  each row is an occupier zone of the table with the seeded flag: an
//        OUTPOST (garrisonMax 6, countsForWin) or a minor kind (CHECKPOINT
//        or COASTAL, garrisonMax 5, countsForWin false).
//    e3  every town, Regina included, is within 2.5 km of an occupier
//        military zone on its landmass; at least one generated base is of a
//        minor kind (the Regina gap); gmBaseGenNotes carries the notes (the unit test pins their wording).
//    e4  the placement rules, as guerrilla_bases_generate_abel g4 (a
//        checkpoint's road band is 60 to 100 m, the others 60 to 250 m).
//    e5  conservation (OCCUPIER-FORCES.md 5.7): R over the authored and
//        seeded roster is 18 + the seeded garrisons, poolStart
//        max(24, R + 2K) less the generated garrisons (Rgen).
//    e6  the win roster takes every military zone but the minor posts.
//    e7  each generated base on empty ground has no solid map object
//        (building, wall, fence, gate, rock) within 60 m of its centre.
//    e8  every minor post has its own layout: 6 or more props, all within
//        60 m, none in a road's 6 m, none in its yard.
// ============================================================================

triSimUntil { GM_LIB_READY }
bgGenNames = []
{bgGenNames = bgGenNames + [_x select 0]} forEach gmGeneratedBases
bgFnMil = {bgMI = 0; bgML = []; while {bgMI < gmZoneCount} do {bgMZ = gmZone bgMI; if (((bgMZ select 1) != "CAMP") and ((bgMZ select 1) != "CITY") and ((bgMZ select 2) == gmOccupierSide)) then {bgML = bgML + [bgMI]}; bgMI = bgMI + 1}; bgML}
bgFnD = {bgDA = _this select 0; bgDB = _this select 1; [bgDA select 0, bgDA select 1, 0] distance [bgDB select 0, bgDB select 1, 0]}
bgFnR = {bgRS = 0; {if (not (((gmZone _x) select 0) in bgGenNames)) then {bgRS = bgRS + ((gmZoneSim _x) select 7)}} forEach (call bgFnMil); bgRS}
bgFnRgen = {bgGS = 0; {bgGI = gmZoneIndex (_x select 0); if (bgGI >= 0) then {bgGS = bgGS + ((gmZoneSim bgGI) select 7)}} forEach gmGeneratedBases; bgGS}
bgFnSeededR = {bgQS = 0; {bgQI = gmZoneIndex (_x select 0); if (bgQI >= 0) then {bgQS = bgQS + ((gmZoneSim bgQI) select 7)}} forEach gmSeededBases; bgQS}
bgMinorTypes = ["CHECKPOINT", "COASTAL", "HELIPAD", "RADAR"]

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
bgMinor = 0
{bgI = gmZoneIndex (_x select 0); if (bgI < 0) then {bgBad = bgBad + format ["[%1]nozone ", _x select 0]} else {bgZ = gmZone bgI; bgT = bgZ select 1; if (bgT in bgMinorTypes) then {bgMinor = bgMinor + 1; if (not (bgT in ["CHECKPOINT", "COASTAL"])) then {bgBad = bgBad + format ["[%1]type=%2 ", _x select 0, bgT]}; if (((gmZoneSim bgI) select 7) != 5) then {bgBad = bgBad + format ["[%1]max=%2 ", _x select 0, (gmZoneSim bgI) select 7]}; if (_x select 4) then {bgBad = bgBad + format ["[%1]counts ", _x select 0]}} else {if (bgT != "OUTPOST") then {bgBad = bgBad + format ["[%1]type=%2 ", _x select 0, bgT]}; if (((gmZoneSim bgI) select 7) != 6) then {bgBad = bgBad + format ["[%1]max=%2 ", _x select 0, (gmZoneSim bgI) select 7]}; if (not (_x select 4)) then {bgBad = bgBad + format ["[%1]minor ", _x select 0]}}; if ((bgZ select 2) != gmOccupierSide) then {bgBad = bgBad + format ["[%1]owner=%2 ", _x select 0, bgZ select 2]}; if (not (bgZ select 10)) then {bgBad = bgBad + format ["[%1]notseeded ", _x select 0]}}} forEach bgGen
triAssertEq [bgBad, ""]

// ---- e3: every town covered, Regina by a minor kind --------------------------
bgOpen = []
bgI = 0
while {bgI < gmZoneCount} do {bgZ = gmZone bgI; if ((bgZ select 1) == "CITY") then {bgP = bgZ select 8; bgL = gmLandmass [bgP select 0, bgP select 1, 0]; bgBest = 1000000; {bgQ = (gmZone _x) select 8; if ((gmLandmass [bgQ select 0, bgQ select 1, 0]) == bgL) then {bgDd = [bgP, bgQ] call bgFnD; if (bgDd < bgBest) then {bgBest = bgDd}}} forEach bgMil; if (bgBest > 2500) then {bgOpen = bgOpen + [format ["%1 %2 m", bgZ select 0, bgBest]]}}; bgI = bgI + 1}
triAssertEq [(format ["%1", bgOpen]), "[]"]
triAssertGe [bgMinor, 1]
triAssertGt [(count gmBaseGenNotes), 0]

// ---- e4 ---------------------------------------------------------------------
bgCamp = (gmZone (gmZoneIndex "Camp")) select 8
bgBad = ""
{bgI = gmZoneIndex (_x select 0); bgP = (gmZone bgI) select 8; bgBand = 300; if ((_x select 1) == "CHECKPOINT") then {bgBand = 140}; if (([bgP, bgCamp] call bgFnD) < 1600) then {bgBad = bgBad + format ["[%1]camp ", _x select 0]}; bgJ = 0; while {bgJ < gmZoneCount} do {if (bgJ != bgI) then {bgQZ = gmZone bgJ; bgDd = [bgP, bgQZ select 8] call bgFnD; if (bgDd < 300) then {bgBad = bgBad + format ["[%1]zone %2 ", _x select 0, bgQZ select 0]}; if (((bgQZ select 1) == "CITY") and (bgDd < 500)) then {bgBad = bgBad + format ["[%1]town %2 ", _x select 0, bgQZ select 0]}; if ((bgJ in bgMil) and (bgDd < 1600)) then {bgBad = bgBad + format ["[%1]base %2 ", _x select 0, bgQZ select 0]}}; bgJ = bgJ + 1}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads 40)) > 0) then {bgBad = bgBad + format ["[%1]onroad ", _x select 0]}; if ((count ([bgP select 0, bgP select 1, 0] nearestRoads bgBand)) == 0) then {bgBad = bgBad + format ["[%1]noroad ", _x select 0]}} forEach bgGen
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

// ---- e6: the win roster leaves the minor posts out ----------------------------
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
triAssertEq [((gmCampaignEnd) select 5), (count bgMil) - bgMinor]

// ---- e7: object-free footprints ----------------------------------------------
bgBad = ""
{if (not (_x select 5)) then {bgName = _x select 0; bgP = _x select 2; {if (_x select 3) then {bgBad = bgBad + format ["[%1: %2 at %3]", bgName, _x select 0, _x select 1]}} forEach (gmBaseSiteObjects [bgP select 0, bgP select 1, 60])}} forEach bgGen
triAssertEq [bgBad, ""]

// ---- e8: the minor posts' own layouts ----------------------------------------
bgProps = gmBaseProps
bgBad = ""
{bgT = _x select 1; if (bgT in bgMinorTypes) then {bgName = _x select 0; bgC = _x select 2; bgN = 0; {if ((_x select 0) == bgName) then {bgN = bgN + 1; bgPos = _x select 2; bgDd = [bgPos, bgC] call bgFnD; if (bgDd > 60) then {bgBad = bgBad + format ["[%1 %2]far ", bgName, _x select 1]}; if ((count ([bgPos select 0, bgPos select 1, 0] nearestRoads 6)) > 0) then {bgBad = bgBad + format ["[%1 %2]road ", bgName, _x select 1]}; if ((bgDd - (_x select 5)) < 15) then {bgBad = bgBad + format ["[%1 %2]yard ", bgName, _x select 1]}; if (not (_x select 4)) then {bgBad = bgBad + format ["[%1 %2]gone ", bgName, _x select 1]}}} forEach bgProps; if (bgN < 6) then {bgBad = bgBad + format ["[%1]only %2 ", bgName, bgN]}}} forEach bgGen
triAssertEq [bgBad, ""]

triEndTest
