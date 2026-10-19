// ============================================================================
//  Occupier forces, a town strike (OCCUPIER-FORCES.md 7.3 "Town strikes",
//  D17, D18, D21; 12.2).  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    t1  the pool is sized (STRONG); the director stays DORMANT (wakeTowns
//        99) and the legacy first-town roll is 0, so nothing but the forced
//        strike flies; the player stays at the Camp, captive and shielded;
//    t2  three CITY zones more than 1.5 km from the player: A (made
//        resistance-held), B within 1.7 km of A, C beyond 2.4 km of A;
//        support and standing 80 in all three;
//    t3  a real strike: gmForcesForce ["strike", A] buys a strike airframe
//        (jet or attack helicopter, the faction descriptor's forcesJet /
//        forcesAttackHeli) and flies it from an occupier airfield; when it
//        strikes, A loses 40 support and 40 standing, B 10 and 10, C
//        nothing; A is stamped struck until about time + 1200
//        (gmZoneSim slot 17, gmZoneStruck true);
//    t4  the townStruck event panics A (civilians.sqs, GM_PANIC_ZONES); the
//        panic episode is shortened so it runs out inside the test;
//    t5  D18: with nobody present, support does not climb back;
//    t6  with the panic over, the struck stamp alone halves A's Roubles on
//        an economy tick (support * GM_ECON_R_PER_SUP * 0.5); the other
//        towns are zeroed so A is the only town income;
//    t7  the stamp is cleared (struckUntil = time - 1): full income resumes.
//  Tag slow_air: not in the default run.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
tsT0 = time
triSimUntil { time > tsT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// ---- t1: the pool, the director held back ------------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triSimUntil { not (isNil "GM_fnZonePanicPhase") }
triSimUntil { not (isNil "GM_ECON_R_PER_SUP") }
tsSupI = GM_Z_SUPPORT

// ---- t2: the three towns -----------------------------------------------------
tsPl = getPos player
tsA = -1
tsB = -1
tsC = -1
tsI = 0
while {tsI < gmZoneCount} do {tsZi = gmZone tsI; if ((tsA < 0) and ((tsZi select 1) == "CITY") and (((tsZi select 8) distance tsPl) > 1500)) then {tsNb = -1; tsFar = -1; tsJ = 0; while {tsJ < gmZoneCount} do {tsZj = gmZone tsJ; if ((tsJ != tsI) and ((tsZj select 1) == "CITY") and (((tsZj select 8) distance tsPl) > 1500)) then {tsDj = (tsZj select 8) distance (tsZi select 8); if ((tsNb < 0) and (tsDj < 1700)) then {tsNb = tsJ}; if ((tsFar < 0) and (tsDj > 2400)) then {tsFar = tsJ}}; tsJ = tsJ + 1}; if ((tsNb >= 0) and (tsFar >= 0)) then {tsA = tsI; tsB = tsNb; tsC = tsFar}}; tsI = tsI + 1}
triAssertGe [tsA, 0]
triAssertGe [tsB, 0]
triAssertGe [tsC, 0]
tsAName = (gmZone tsA) select 0
gmZoneSet [tsA, "garrison", 0]
gmZoneSet [tsA, "owner", gmResistanceSide]
triAssertEq [((gmZone tsA) select 2), gmResistanceSide]
{gmZoneSet [_x, "support", 80]; gmZoneSet [_x, "standing", 80]} forEach [tsA, tsB, tsC]
triAssertEq [((gmZoneSim tsA) select 17), 0]
triAssertEq [(format ["%1", gmZoneStruck tsA]), "false"]
GM_PANIC_SCATTER_T = 5
GM_PANIC_EMPTY_T = 10

// ---- t3: a real strike -------------------------------------------------------
tsStruck0 = (gmOccupierForces) select 24
triAssertEq [(format ["%1", gmForcesForce ["strike", tsA]]), "true"]
tsOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == tsAName)) then {tsOp = _x}} forEach gmOccupierOps
triAssertGe [(count tsOp), 4]
triAssertEq [(format ["%1", (tsOp select 3) in ["JET", "ATTACK_HELI"]]), "true"]
// sample the three towns every frame until the strike lands, so the cut is
// measured against the values of the frame before it
tsPre = []
triSimUntil { if (((gmZoneSim tsA) select 17) == 0) then {tsPre = [((gmZone tsA) select tsSupI), ((gmZoneSim tsA) select 16), ((gmZone tsB) select tsSupI), ((gmZoneSim tsB) select 16), ((gmZone tsC) select tsSupI), ((gmZoneSim tsC) select 16)]}; ((gmZoneSim tsA) select 17) > 0 }
tsNow = time
tsPost = [((gmZone tsA) select tsSupI), ((gmZoneSim tsA) select 16), ((gmZone tsB) select tsSupI), ((gmZoneSim tsB) select 16), ((gmZone tsC) select tsSupI), ((gmZoneSim tsC) select 16)]
triAssertNear [(tsPost select 0), ((tsPre select 0) - 40), 0.5]
triAssertNear [(tsPost select 1), ((tsPre select 1) - 40), 0.5]
triAssertNear [(tsPost select 2), ((tsPre select 2) - 10), 0.5]
triAssertNear [(tsPost select 3), ((tsPre select 3) - 10), 0.5]
triAssertNear [(tsPost select 4), (tsPre select 4), 0.5]
triAssertNear [(tsPost select 5), (tsPre select 5), 0.5]
triAssertNear [(((gmZoneSim tsA) select 17) - tsNow), 1200, 5]
triAssertEq [(format ["%1", gmZoneStruck tsA]), "true"]
triAssertEq [((gmOccupierForces) select 24), tsStruck0 + 1]

// ---- t4: the struck town panics ------------------------------------------------
triSimUntil { tsAName in GM_PANIC_ZONES }
triAssertGe [([tsAName] call GM_fnZonePanicPhase), 1]
triSimUntil { not (tsAName in GM_PANIC_ZONES) }

// ---- t5: D18, no presence, no recovery -----------------------------------------
tsT1 = time
triSimUntil { time > tsT1 + 12 }
triAssertLe [((gmZone tsA) select tsSupI), (tsPost select 0) + 0.01]
triAssertLe [((gmZone tsB) select tsSupI), (tsPost select 2) + 0.01]
triAssertEq [(format ["%1", gmZoneStruck tsA]), "true"]

// ---- t6: half income while struck ---------------------------------------------
tsI = 0
while {tsI < gmZoneCount} do {if ((tsI != tsA) and (((gmZone tsI) select 1) == "CITY")) then {gmZoneSet [tsI, "support", 0]}; tsI = tsI + 1}
tsMil = 0
tsI = 0
while {tsI < gmZoneCount} do {tsZi = gmZone tsI; if (((tsZi select 2) == gmResistanceSide) and ((tsZi select 1) in ["OUTPOST", "AIRFIELD", "SEAPORT"])) then {tsMil = tsMil + (tsZi select GM_Z_INCOME)}; tsI = tsI + 1}
GM_ECON_TICK = 10
gmEcoR = -1
tsR0 = gmResources
triSimUntil { gmEcoR >= 0 }
tsSupA = (gmZone tsA) select tsSupI
triAssertGt [tsSupA, 1]
triAssertNear [gmEcoR, tsMil + (tsSupA * GM_ECON_R_PER_SUP * 0.5), 0.6]
triAssertNear [(gmResources - tsR0), gmEcoR, 0.01]

// ---- t7: the stamp cleared, full income -----------------------------------------
gmZoneSet [tsA, "struckUntil", time - 1]
triAssertEq [(format ["%1", gmZoneStruck tsA]), "false"]
triAssertEq [(format ["%1", tsAName in GM_PANIC_ZONES]), "false"]
gmEcoR = -1
tsR0 = gmResources
triSimUntil { gmEcoR >= 0 }
tsSupA = (gmZone tsA) select tsSupI
triAssertNear [gmEcoR, tsMil + (tsSupA * GM_ECON_R_PER_SUP), 0.6]
triAssertNear [(gmResources - tsR0), gmEcoR, 0.01]
GM_ECON_TICK = 600
gmSimDiagLog

triEndTest
