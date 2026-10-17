// ============================================================================
//  Occupier forces, the repel ladder (OCCUPIER-FORCES.md 6.3 THE REPEL
//  LADDER, 7.3 the escalation strike; phase 1 A after video 5, where Le Port
//  got the same Ural and T72 seven times and Arudy the same BMP five times).
//  Boots the SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full
//  Remaster:
//    e1  a real camera; test levers: director tick 1 s, one distinct town
//        wakes the director with three tokens, the first-town roll 0, the
//        refill slowed to 48 h, a 30 s rest, urgency only from alert edges
//        (so the plain strike rule stays out), the abstract leg at 40 m/s
//        and a 240 s live timeout; the pool forced to 0 (SPENT) so the wake
//        launches nothing;
//    e2  Houdan (the Village) rises remotely: three resistance fighters,
//        invulnerable and free to fire, take it and stay; the player waits
//        in the western hills, far from Houdan and every base road; a
//        local talks (standing 100), so the director has them reported;
//    e3  the pool comes back (90, above poolStart): op 1, a DIRECTOR retake,
//        is repelled: gmOccupierRepels remembers one;
//    e4  op 2 (escalated: twice the fighters as infantry) is repelled:
//        two;
//    e5  op 3 is a different approach: not op 2's package from op 2's
//        source, and a STRIKE goes with it on Houdan (the escalation strike,
//        the pool staying STRETCHED or better).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
eaT0 = time
triSimUntil { time > eaT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- e1: a real camera; the levers; SPENT so the wake launches nothing ------
eaB = gmTrafficPercept (getPos player)
triAssertEq [(eaB select 0), 1]
triAssertGe [(eaB select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
eaStart = (gmOccupierForces) select 1
triAssertGe [eaStart, 24]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 3]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["townCooldown", 30]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["urgentHeat", 100]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 40]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["liveTimeout", 240]]), "true"]
eaRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "SPENT"]

// ---- e2: Houdan held by three fighters who fight back; the player far ------
eaV = gmZoneIndex "Village"
triAssertGe [eaV, 0]
eaVPos = (gmZone eaV) select 8
eaFar = [4500, 5200, 0]
if ((gmLandmass eaFar) < 0) then {eaFar = [7922, 8500, 0]}
triAssertGe [(eaFar distance eaVPos), 1500]
player setPos eaFar
triSimUntil { not (gmGarrisonSpawned eaV) }
gmZoneSet [eaV, "garrison", 0]
gmZoneSet [eaV, "support", 55]
eaSq = [eaRes, gmFactionSquad [gmResistanceSide, 1, 3], eaVPos] call GM_fnSpawnSquad
eaSq setCombatMode "RED"
{_x allowDammage false} forEach (units eaSq)
doStop (units eaSq)
triAssertEq [(count units eaSq), 3]
triSimUntil { ((gmZone eaV) select 2) == gmResistanceSide }
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(count gmOccupierOps), 0]
gmZoneSet [eaV, "standing", 100]
eaRep = 0
eaT1 = time
triSimUntil { eaRep = 0; {if ((_x select 0) == "Village") then {eaRep = _x select 1}} forEach gmOccupierIntel; (eaRep >= 3) or (time > eaT1 + 60) }
triAssertGe [eaRep, 3]
setAccTime 4

// ---- e3: op 1, repelled: one repel remembered --------------------------------
triAssertEq [(format ["%1", gmForcesForce ["pool", 90]]), "true"]
eaRepels = { eaN = 0; {if ((_x select 0) == "Village") then {eaN = _x select 1}} forEach gmOccupierRepels; eaN }
eaNextOp = { eaOp = []; {if (((_x select 5) == "Village") and ((_x select 1) == "RETAKE") and ((_x select 0) > eaLast)) then {eaOp = _x}} forEach gmOccupierOps; eaOp }
eaLast = 0
eaT2 = time
triSimUntil { ((count (call eaNextOp)) > 0) or (time > eaT2 + 300) }
triAssertGe [(count eaOp), 6]
triAssertEq [(eaOp select 2), "DIRECTOR"]
eaLast = eaOp select 0
triSimUntil { ((call eaRepels) >= 1) or (time > eaT2 + 1500) }
triAssertEq [(call eaRepels), 1]
triAssertEq [((gmZone eaV) select 2), gmResistanceSide]
gmSimDiagLog

// ---- e4: op 2 (escalated), repelled: two -------------------------------------
eaT3 = time
triSimUntil { ((count (call eaNextOp)) > 0) or (time > eaT3 + 600) }
triAssertGe [(count eaOp), 6]
triAssertEq [(eaOp select 2), "DIRECTOR"]
triAssertGe [(eaOp select 7), 6]
eaLast = eaOp select 0
eaPkg2 = eaOp select 3
eaSrc2 = eaOp select 4
triSimUntil { ((call eaRepels) >= 2) or (time > eaT3 + 1500) }
triAssertEq [(call eaRepels), 2]
triAssertEq [((gmZone eaV) select 2), gmResistanceSide]
gmSimDiagLog

// ---- e5: op 3 is a different approach, and a strike goes with it --------------
triAssertEq [(format ["%1", gmForcesForce ["pool", 90]]), "true"]
eaT4 = time
triSimUntil { ((count (call eaNextOp)) > 0) or (time > eaT4 + 600) }
triAssertGe [(count eaOp), 6]
triAssertEq [(eaOp select 2), "DIRECTOR"]
eaSame = ((eaOp select 3) == eaPkg2) and ((eaOp select 4) == eaSrc2)
triAssertEq [(format ["%1", eaSame]), "false"]
eaStruck0 = (gmOccupierForces) select 24
eaStrike = false
eaT5 = time
triSimUntil { eaStrike = false; {if (((_x select 1) == "STRIKE") and ((_x select 5) == "Village")) then {eaStrike = true}} forEach gmOccupierOps; eaStrike or (((gmOccupierForces) select 24) > eaStruck0) or (time > eaT5 + 30) }
triAssertEq [(format ["%1", eaStrike or (((gmOccupierForces) select 24) > eaStruck0)]), "true"]
triAssertEq [(format ["%1", ((gmOccupierForces) select 2) != "THIN"]), "true"]
setAccTime 1
gmSimDiagLog

triEndTest
