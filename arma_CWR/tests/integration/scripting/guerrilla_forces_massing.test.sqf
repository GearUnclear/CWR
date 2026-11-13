// ============================================================================
//  Occupier forces, massing instead of trickling (OCCUPIER-FORCES.md 6.3
//  MASSING; phase 1 A after video 5).  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    m1  test levers: director tick 1 s, one distinct town wakes the
//        director, the first-town roll 0, the refill slowed to 48 h, a long
//        hold (massingMax 20, the lever's top) so it can be watched; the pool forced to 0
//        (SPENT) so the wake launches nothing;
//    m2  Houdan (the Village) rises remotely and its three-man hold squad
//        stays (invulnerable, hold fire), the player in the western hills:
//        the director wakes; a local talks
//        (standing 100), so the three are reported (D9);
//    m3  the ladder remembers one repel on Houdan (the lever): the next
//        operation needs twice the 3 fighters reported, 6 infantry; every
//        occupier base is bled to 4 (spares of 3 or less);
//    m4  the pool comes back (STRONG): the director MASSES for Houdan
//        (gmOccupierMassing), holds its token (gmOccupierForces [3] does
//        not fall) and launches nothing;
//    m5  the Airfield regrows to 12 (spares 9): the hold ends and a
//        DIRECTOR operation goes on Houdan with at least 6 infantry.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
msT0 = time
triSimUntil { time > msT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- m1: the levers; SPENT so the wake launches nothing ----------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
msStart = (gmOccupierForces) select 1
triAssertGe [msStart, 24]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["massingMax", 20]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
msRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]

// ---- m2: Houdan held and defended; a local talks ----------------------------
msV = gmZoneIndex "Village"
triAssertGe [msV, 0]
msVPos = (gmZone msV) select 8
// the western hills: every garrison and base reserve stays abstract
msFar = [4500, 5200, 0]
if ((gmLandmass msFar) < 0) then {msFar = [7922, 8500, 0]}
player setPos msFar
triSimUntil { not (gmGarrisonSpawned msV) }
gmZoneSet [msV, "garrison", 0]
gmZoneSet [msV, "support", 55]
msSq = [msRes, gmFactionSquad [gmResistanceSide, 1, 3], msVPos] call GM_fnSpawnSquad
msSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units msSq)
doStop (units msSq)
triAssertEq [(count units msSq), 3]
triSimUntil { ((gmZone msV) select 2) == gmResistanceSide }
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(count gmOccupierOps), 0]
gmZoneSet [msV, "standing", 100]
msRep = 0
msT1 = time
triSimUntil { msRep = 0; {if ((_x select 0) == "Village") then {msRep = _x select 1}} forEach gmOccupierIntel; (msRep >= 3) or (time > msT1 + 60) }
triAssertGe [msRep, 3]

// ---- m3: one repel remembered; every base bled below the need ---------------
triAssertEq [(format ["%1", gmForcesForce ["repels", msV, 1]]), "true"]
msRow = []
{if ((_x select 0) == "Village") then {msRow = _x}} forEach gmOccupierRepels
triAssertEq [(msRow select 1), 1]
triAssertEq [(format ["%1", msRow select 4]), "false"]
msBases = []
msI = 0
while {msI < gmZoneCount} do {msZ = gmZone msI; if ((not ((msZ select 1) in ["CAMP", "CITY"])) and ((msZ select 2) == gmOccupierSide)) then {msBases = msBases + [msI]}; msI = msI + 1}
triAssertGe [(count msBases), 2]
{if (not (gmGarrisonSpawned _x)) then {gmZoneSet [_x, "garrison", 4]}} forEach msBases
{triAssertLe [((gmZone _x) select 3), 4]} forEach msBases

// ---- m4: STRONG again: the director masses, the token is held -----------------
gmZoneSet [msV, "heatRaise", 10]
triAssertEq [(format ["%1", gmForcesForce ["pool", msStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
msTok = (gmOccupierForces) select 3
triAssertGe [msTok, 1]
msT2 = time
triSimUntil { (((gmOccupierMassing) select 0) == "Village") or (time > msT2 + 30) }
triAssertEq [((gmOccupierMassing) select 0), "Village"]
msT3 = time
triSimUntil { time > msT3 + 5 }
triAssertEq [((gmOccupierMassing) select 0), "Village"]
triAssertGe [((gmOccupierMassing) select 1), 3]
triAssertEq [((gmOccupierMassing) select 2), 20]
triAssertEq [(count gmOccupierOps), 0]
triAssertGe [((gmOccupierForces) select 3), msTok]

// ---- m5: the Airfield regrows: the hold ends, the operation goes -------------
msA = gmZoneIndex "Airfield"
triAssertGe [msA, 0]
gmZoneSet [msA, "garrison", 12]
msOp = []
msT4 = time
triSimUntil { msOp = []; {if ((_x select 5) == "Village") then {msOp = _x}} forEach gmOccupierOps; ((count msOp) > 0) or (time > msT4 + 60) }
triAssertGe [(count msOp), 8]
triAssertEq [(msOp select 1), "RETAKE"]
triAssertEq [(msOp select 2), "DIRECTOR"]
triAssertGe [(msOp select 7), 6]
triAssertEq [((gmOccupierMassing) select 0), ""]
gmSimDiagLog

triEndTest
