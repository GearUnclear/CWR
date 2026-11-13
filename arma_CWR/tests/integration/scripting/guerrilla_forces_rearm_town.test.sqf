// ============================================================================
//  Occupier forces, the re-arm after a rest (OCCUPIER-FORCES.md 6.3 RE-ARM,
//  forcesRearmHeat; round 3.5 of OCCUPIER-FORCES-NEXT.md).  Video 4: Arudy
//  repelled two operations, its +15 decayed inside the 1800 s rest, the
//  informant tips kept it from being quiet for a push-out, and a STRONG
//  director with two tokens never went back.  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    r1  test levers: director tick 1 s, one distinct town wakes the
//        director, the first-town roll 0, the refill slowed to 48 h; the
//        pool forced to 0 (SPENT) so the wake launches nothing;
//    r2  Houdan (the Village) rises remotely and its three-man hold squad
//        stays (invulnerable, hold fire): the director wakes; Houdan rests
//        120 s (the lever, as a repel would), its heat goes to 0 and its
//        occupier standing to 100, so a local talks (the informant tip,
//        7.3) and the sighting table reports the three (D9); the pool is
//        restored (STRONG) with the born cap at 0, so nothing can launch;
//    r3  the rest runs out: Houdan is re-armed (gmOccupierForces [30]) and
//        its heat is held at forcesRearmHeat (40) against the held-zone
//        decay for 80 s, though nothing else raised it;
//    r4  the born cap comes back: a DIRECTOR operation on Houdan launches
//        within one STRONG token interval (600 s) and the re-arm is spent.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
raT0 = time
triSimUntil { time > raT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- r1: the levers; SPENT so the wake launches nothing ---------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
raStart = (gmOccupierForces) select 1
triAssertGe [raStart, 24]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
raRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]

// ---- r2: Houdan held and defended; it rests; a local talks -------------------
player setPos [7922, 8500, 0]
raV = gmZoneIndex "Village"
triAssertGe [raV, 0]
raVPos = (gmZone raV) select 8
triSimUntil { not (gmGarrisonSpawned raV) }
gmZoneSet [raV, "garrison", 0]
gmZoneSet [raV, "support", 55]
raSq = [raRes, gmFactionSquad [gmResistanceSide, 1, 3], raVPos] call GM_fnSpawnSquad
raSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units raSq)
doStop (units raSq)
triAssertEq [(count units raSq), 3]
triSimUntil { ((gmZone raV) select 2) == gmResistanceSide }
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(count gmOccupierOps), 0]
triAssertEq [(format ["%1", gmForcesForce ["cooldown", raV, 120]]), "true"]
gmZoneSet [raV, "heatDecay", 100]
gmZoneSet [raV, "standing", 100]
triAssertLe [((gmZone raV) select 6), 0]
raRep = 0
raT1 = time
triSimUntil { raRep = 0; {if ((_x select 0) == "Village") then {raRep = _x select 1}} forEach gmOccupierIntel; (raRep >= 1) or (time > raT1 + 60) }
triAssertGe [raRep, 1]
// STRONG again, but no born slot: the re-arm is watched before any launch
triAssertEq [(format ["%1", gmForcesTune ["opsMax", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["opsMaxStrong", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", raStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]

// ---- r3: the rest runs out: re-armed, the heat held at 40 --------------------
raResting = {raR = false; {if ((_x select 0) == "Village") then {raR = true}} forEach ((gmOccupierForces) select 12); raR}
triAssertEq [(format ["%1", call raResting]), "true"]
triSimUntil { not (call raResting) }
raTEnd = time
triSimUntil { ("Village" in ((gmOccupierForces) select 30)) or (time > raTEnd + 10) }
triAssertEq [(format ["%1", "Village" in ((gmOccupierForces) select 30)]), "true"]
triSimUntil { time > raTEnd + 80 }
// the held-zone decay alone (0.25 per 10 s) would have taken 2 points by now
triAssertGe [((gmZone raV) select 6), 39.5]
triAssertEq [(format ["%1", "Village" in ((gmOccupierForces) select 30)]), "true"]
triAssertEq [(count gmOccupierOps), 0]

// ---- r4: a born slot: the operation goes within one token interval -----------
triAssertEq [(format ["%1", gmForcesTune ["opsMax", 2]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["opsMaxStrong", 3]]), "true"]
raOp = []
triSimUntil { raOp = []; {if ((_x select 5) == "Village") then {raOp = _x}} forEach gmOccupierOps; ((count raOp) > 0) or (time > raTEnd + 640) }
triAssertGe [(count raOp), 6]
triAssertLe [time - raTEnd, 640]
triAssertEq [(raOp select 1), "RETAKE"]
triAssertEq [(raOp select 2), "DIRECTOR"]
triAssertEq [(format ["%1", "Village" in ((gmOccupierForces) select 30)]), "false"]
gmSimDiagLog

triEndTest
