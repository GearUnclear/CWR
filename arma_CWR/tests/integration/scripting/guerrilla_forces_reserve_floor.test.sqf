// ============================================================================
//  Occupier forces, the reserve floor (OCCUPIER-FORCES.md 6.3 THE RESERVE
//  FLOOR, owner decision 2026-10-17).  Video 7 (tmp/of-director-video-7) at
//  2884 s: "[op 4] APC (6, BMP) with T72 from Le Port Harbour to Sainte
//  Marie (... the pool pays 28 ...)", the strongest package after three
//  ticks of massing on a town that had repelled one attack, took a pool of
//  32 of 48 to 4, under the 9.6 floor.  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    r1  test levers: director tick 1 s, one distinct town wakes the
//        director, the first-town roll 0, the refill slowed to 48 h, no
//        urgency (so no strike rule joins in); the pool forced to 0 (SPENT)
//        so the wake launches nothing;
//    r2  Le Port rises remotely: six resistance fighters, invulnerable and
//        holding fire, take it and stay; the player waits in the western
//        hills; a local talks (standing 100), so the six are reported;
//    r3  the ladder remembers one repel on Le Port (the lever): the next
//        attack needs twice the 6 fighters as infantry (12, above every
//        base's spare) and lethality x1.5; every base is bled to 4, the
//        Harbour left at 8 (it spares 6, the video's Le Port Harbour);
//    r4  the pool at 32 (the video's): the director masses three ticks,
//        then the strongest package one base can bring, the Harbour's APC
//        with a T72 escort (the pool pays 28), WAITS: no operation, the
//        token kept, gmOccupierReserve names the plan and its one line;
//    r5  the pool at 38 (28 + 9.6 rounded up): the same package goes and
//        the pool keeps 10.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
rfT0 = time
triSimUntil { time > rfT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- r1: the levers; SPENT so the wake launches nothing ----------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
rfStart = (gmOccupierForces) select 1
triAssertEq [rfStart, 48]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["urgentHeat", 100]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
rfRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]
rfR = gmOccupierReserve
triAssertEq [(format ["%1", (rfR select 0) > 9.5]), "true"]
triAssertEq [(format ["%1", (rfR select 0) < 9.7]), "true"]
triAssertEq [(count (rfR select 3)), 0]

// ---- r2: Le Port held by six fighters who hold fire; a local talks ------------
rfV = gmZoneIndex "Le Port"
triAssertGe [rfV, 0]
rfVPos = (gmZone rfV) select 8
rfFar = [4500, 5200, 0]
if ((gmLandmass rfFar) < 0) then {rfFar = [7922, 8500, 0]}
triAssertGe [(rfFar distance rfVPos), 1500]
player setPos rfFar
triSimUntil { not (gmGarrisonSpawned rfV) }
gmZoneSet [rfV, "garrison", 0]
gmZoneSet [rfV, "support", 55]
rfSq = [rfRes, gmFactionSquad [gmResistanceSide, 1, 6], rfVPos] call GM_fnSpawnSquad
rfSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units rfSq)
doStop (units rfSq)
triAssertEq [(count units rfSq), 6]
triSimUntil { ((gmZone rfV) select 2) == gmResistanceSide }
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(count gmOccupierOps), 0]
gmZoneSet [rfV, "standing", 100]
rfRep = 0
rfT1 = time
triSimUntil { rfRep = 0; {if ((_x select 0) == "Le Port") then {rfRep = _x select 1}} forEach gmOccupierIntel; (rfRep >= 6) or (time > rfT1 + 90) }
triAssertGe [rfRep, 6]

// ---- r3: one repel remembered; the bases bled, the Harbour spares 6 ------------
triAssertEq [(format ["%1", gmForcesForce ["repels", rfV, 1]]), "true"]
rfBases = []
rfI = 0
while {rfI < gmZoneCount} do {rfZ = gmZone rfI; if ((not ((rfZ select 1) in ["CAMP", "CITY"])) and ((rfZ select 2) == gmOccupierSide)) then {rfBases = rfBases + [rfI]}; rfI = rfI + 1}
triAssertGe [(count rfBases), 2]
{if (not (gmGarrisonSpawned _x)) then {gmZoneSet [_x, "garrison", 4]}} forEach rfBases
rfH = gmZoneIndex "Le Port Harbour"
triAssertGe [rfH, 0]
triAssertEq [(format ["%1", gmGarrisonSpawned rfH]), "false"]
gmZoneSet [rfH, "garrison", 8]

// ---- r4: the pool at 32: the strongest package waits for the reserve ----------
gmZoneSet [rfV, "heatRaise", 10]
triAssertEq [(format ["%1", gmForcesForce ["pool", 32]]), "true"]
rfTok = (gmOccupierForces) select 3
triAssertGe [rfTok, 1]
rfRow = []
rfT2 = time
triSimUntil { rfRow = []; {if ((_x select 0) == "Le Port") then {rfRow = _x}} forEach ((gmOccupierReserve) select 3); ((count rfRow) > 0) or (time > rfT2 + 60) }
triAssertEq [(count rfRow), 8]
triAssertEq [(rfRow select 1), "escalation retake after massing"]
triAssertEq [(rfRow select 2), "APC (6, BMP) with T72"]
triAssertEq [(rfRow select 3), "APC"]
triAssertEq [(rfRow select 4), 28]
triAssertEq [(rfRow select 5), 32]
triAssertEq [(format ["%1", rfRow select 6]), "true"]
triAssertEq [(rfRow select 7), "OccupierForces: Le Port: escalation retake after massing (APC (6, BMP) with T72, the pool pays 28, lethality 25.50 of 32.40, the most lethal package) waits, the pool keeps its reserve (pool 32 of 48, floor 9.6: the launch would leave 4)"]
// several director ticks later: still waiting, nothing launched, the token kept,
// the town not rested
rfT3 = time
triSimUntil { time > rfT3 + 8 }
rfOps = 0
{if ((_x select 5) == "Le Port") then {rfOps = rfOps + 1}} forEach gmOccupierOps
triAssertEq [rfOps, 0]
triAssertGe [((gmOccupierForces) select 3), rfTok]
triAssertEq [((gmOccupierForces) select 0), 32]
triAssertEq [((gmOccupierMassing) select 0), ""]
rfRested = false
{if ((_x select 0) == "Le Port") then {rfRested = true}} forEach ((gmOccupierForces) select 12)
triAssertEq [(format ["%1", rfRested]), "false"]
triAssertEq [(count ((gmOccupierReserve) select 3)), 1]

// ---- r5: the pool past price + floor: the same package goes ---------------------
triAssertEq [(format ["%1", gmForcesForce ["pool", 38]]), "true"]
rfOp = []
rfT4 = time
triSimUntil { rfOp = []; {if ((_x select 5) == "Le Port") then {rfOp = _x}} forEach gmOccupierOps; ((count rfOp) > 0) or (time > rfT4 + 30) }
triAssertEq [(count rfOp), 13]
triAssertEq [(rfOp select 1), "RETAKE"]
triAssertEq [(rfOp select 2), "DIRECTOR"]
triAssertEq [(rfOp select 3), "APC"]
triAssertEq [(rfOp select 4), "Le Port Harbour"]
triAssertEq [(rfOp select 7), 6]
triAssertEq [(rfOp select 11), "BMP"]
triAssertEq [(rfOp select 12), "T72"]
triAssertGe [((gmOccupierForces) select 0), 10]
triAssertLe [((gmOccupierForces) select 0), 11]
triAssertEq [(count ((gmOccupierReserve) select 3)), 0]
gmSimDiagLog

triEndTest
