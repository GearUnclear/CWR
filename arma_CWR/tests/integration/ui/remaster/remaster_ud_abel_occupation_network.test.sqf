// ============================================================================
//  Malden occupation network (issue #76 Abel part, #79 blocker B1) on the full
//  Remaster with UD installed beside Bohemia's files (issue #67).
//  Boots GUERRILLA -> Abel through the menu with the default (camp) start and
//  checks the shipped Guerrilla.Abel zone table end to end:
//    * four occupier military zones load with their authored garrisons, a
//      ladder of 6 / 6 / 8 / 12 with the AIRFIELD strongest (contract C4);
//      the three far hubs are still banked at full strength (D1);
//    * the four garrisoned hub centres and their whole garrison spawn square
//      (zoneArea / 3 = 50 m each way) are dry ground, with a road in reach
//      (C6);
//    * gmTrafficDiag at the Camp reports a patrol route AND a convoy route
//      inside the first minute: there are two occupier endpoints (C3);
//    * the three enemy Legend commanders hang off more than one zone and are
//      not all within 600 m of the Camp (LegendPlacement ladder).
//  Loops assert nothing themselves (the harness reads whole lines): each one
//  builds a diagnostic string that is asserted afterwards.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triControlText 150), "OCCUPIER: EAST"]
triAssertEq [(triControlText 152), "START TOWN: (camp)"]
triClick 1
triSimUntil { alive player }
triAssertEq [gmSelIsland, "Abel"]
triSimUntil { GM_LIB_READY }

// -- the zone table: four occupier military zones, authored garrisons --------
onCamp = gmZoneIndex "Camp"
onOut = gmZoneIndex "Outpost"
onCmp = gmZoneIndex "Southeast Compound"
onPort = gmZoneIndex "Le Port Harbour"
onAir = gmZoneIndex "Airfield"
triAssertGe [onCamp, 0]
triAssertGe [onOut, 0]
triAssertGe [onCmp, 0]
triAssertGe [onPort, 0]
triAssertGe [onAir, 0]
triAssertEq [((gmZone onOut) select GM_Z_TYPE), "OUTPOST"]
triAssertEq [((gmZone onCmp) select GM_Z_TYPE), "OUTPOST"]
triAssertEq [((gmZone onPort) select GM_Z_TYPE), "SEAPORT"]
triAssertEq [((gmZone onAir) select GM_Z_TYPE), "AIRFIELD"]
onBad = ""
{if (((gmZone _x) select GM_Z_OWNER) != gmOccupierSide) then {onBad = onBad + format ["[%1]owner=%2 ", (gmZone _x) select GM_Z_NAME, (gmZone _x) select GM_Z_OWNER]}} forEach [onOut, onCmp, onPort, onAir]
triAssertEq [onBad, ""]
// gmZoneSim element 7 is garrisonMax, which defaults to the authored garrison
triAssertEq [((gmZoneSim onOut) select 7), 6]
triAssertEq [((gmZoneSim onCmp) select 7), 6]
triAssertEq [((gmZoneSim onPort) select 7), 8]
triAssertEq [((gmZoneSim onAir) select 7), 12]
// the far hubs are out of the 800 m bubble: banked at full strength
triAssertEq [((gmZone onCmp) select GM_Z_GAR), 6]
triAssertEq [((gmZone onPort) select GM_Z_GAR), 8]
triAssertEq [((gmZone onAir) select GM_Z_GAR), 12]
// exactly four occupier military zones, and the AIRFIELD is the strongest
onMil = 0
onTop = 0
onI = 0
while {onI < gmZoneCount} do {onZ = gmZone onI; if (((onZ select GM_Z_TYPE) in ["OUTPOST", "AIRFIELD", "SEAPORT"]) and ((onZ select GM_Z_OWNER) == gmOccupierSide)) then {onMil = onMil + 1; if (((gmZoneSim onI) select 7) > onTop) then {onTop = (gmZoneSim onI) select 7}}; onI = onI + 1}
triAssertEq [onMil, 4]
triAssertEq [onTop, ((gmZoneSim onAir) select 7)]

// -- C6: every garrisoned hub centre and its spawn square is dry ground -----
//    (an invisible helipad seats on the surface; at sea it reads about 1.4 m,
//    so 2 m is the dry line), and a road is in reach of the centre
onBad = ""
{onP = (gmZone _x) select GM_Z_POS; onGx = -5; while {onGx <= 5} do {onGz = -5; while {onGz <= 5} do {onV = "HeliHEmpty" createVehicle [(onP select 0) + onGx * 10, (onP select 1) + onGz * 10, 0]; onH = (getPosASL onV) select 2; deleteVehicle onV; if (onH < 2) then {onBad = onBad + format ["[%1 %2,%3 h=%4]", (gmZone _x) select GM_Z_NAME, onGx * 10, onGz * 10, onH]}; onGz = onGz + 1}; onGx = onGx + 1}} forEach [onOut, onCmp, onPort, onAir]
triAssertEq [onBad, ""]
onBad = ""
{if ((count (((gmZone _x) select GM_Z_POS) nearestRoads 120)) == 0) then {onBad = onBad + format ["[%1]noroad ", (gmZone _x) select GM_Z_NAME]}} forEach [onCmp, onPort, onAir]
triAssertEq [onBad, ""]

// -- C3: patrols and convoys are admissible at the Camp from minute one -----
onCampPos = (gmZone onCamp) select GM_Z_POS
triAssertLe [((getPos player) distance [onCampPos select 0, onCampPos select 1, 0]), 60]
triSimUntil { (gmTrafficDiag select 12) >= 2 }
triAssertLe [time, 60]
// slots 7 / 8: patrolRoute / convoyRoute (0/1)
triAssertEq [(gmTrafficDiag select 7), 1]
triAssertEq [(gmTrafficDiag select 8), 1]

// -- the three enemy commanders spread out instead of clustering at the Camp -
triSimUntil { gmLegendCount >= 4 }
onBoss = []
onI = 0
while {onI < gmLegendCount} do {if (((gmLegendInfo onI) select 2) == 1) then {onBoss = onBoss + [onI]}; onI = onI + 1}
triAssertEq [(count onBoss), 3]
onNear = 0
onZones = []
onBad = ""
{onLp = gmLegendPos _x; onLz = (gmLegendInfo _x) select 4; if (((onLp select 0) == 0) and ((onLp select 1) == 0)) then {onBad = onBad + format ["[%1]nostand ", _x]}; onDx = (onLp select 0) - (onCampPos select 0); onDz = (onLp select 1) - (onCampPos select 1); if ((sqrt ((onDx * onDx) + (onDz * onDz))) < 600) then {onNear = onNear + 1}; if (not (onLz in onZones)) then {onZones = onZones + [onLz]}} forEach onBoss
triAssertEq [onBad, ""]
triAssertLt [onNear, 3]
triAssertGe [(count onZones), 2]

triEndTest
