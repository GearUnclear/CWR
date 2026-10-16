// ============================================================================
//  Occupier forces, a director strike keeps STRONG after both debits
//  (OCCUPIER-FORCES.md 7.3 "When a town strike is bought"; round 3.5 of
//  OCCUPIER-FORCES-NEXT.md), phase 1: the case video 4 bought.  There an
//  urgent retake took a pool of 62 to 40 (still STRONG) and the 23-unit jet
//  bought with it took it to 17 (THIN) for 38 s.  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    s1  a real camera; the levers of guerrilla_forces_opener_reported
//        (director tick 1 s, one town wakes it, the first-town roll 0,
//        fast abstract travel, the arrival rule off so the package is the
//        cheapest lethal one);
//    s2  the Outpost loses its garrison and is handed to the resistance,
//        hot (heat 100), with a 3-man hold squad in it (invulnerable, hold
//        fire); two occupier observers beside it are shown the three, so
//        the sighting table reports three fighters at the Outpost (D9);
//    s3  the pool is forced to 50 of 60: STRONG, and a ground operation of
//        up to 14 units leaves it STRONG (36 is the 60 % floor), but no
//        strike airframe does (an attack helicopter 20, a jet 23);
//    s4  Houdan rises remotely: the director wakes STRONG; its BASE retake
//        of the Outpost goes and leaves the band STRONG (the old rule's
//        "STRONG on the pool the ground operation leaves" would buy the
//        opener here), yet no STRIKE goes with it; the band, sampled every
//        frame, never leaves STRONG.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
sbT0 = time
triSimUntil { time > sbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// the band sampler: the worst band seen (0 STRONG, 1 STRETCHED, 2 THIN, 3 SPENT)
sbWorst = 0
sbSample = {sbB = (gmOccupierForces) select 2; sbR = 0; if (sbB == "STRETCHED") then {sbR = 1}; if (sbB == "THIN") then {sbR = 2}; if (sbB == "SPENT") then {sbR = 3}; if (sbR > sbWorst) then {sbWorst = sbR}}

// ---- s1: a real camera; the pool STRONG; the levers -------------------------
sbP = gmTrafficPercept (getPos player)
triAssertEq [(sbP select 0), 1]
triAssertGe [(sbP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
sbStart = (gmOccupierForces) select 1
triAssertEq [sbStart, 60]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["slowEta", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["airEta", 0]]), "true"]
sbRes = gmResistanceSide call GM_fnSideFromString
sbOcc = gmOccupierSide call GM_fnSideFromString

// ---- s2: the Outpost, held and hot, three defenders reported ----------------
sbO = gmZoneIndex "Outpost"
triAssertGe [sbO, 0]
sbOPos = (gmZone sbO) select 8
triSimUntil { gmGarrisonSpawned sbO }
sbUnits = []
"sbUnits = sbUnits + (units _x)" forEach (gmGarrisonGroups sbO)
{_x setDamage 1} forEach sbUnits
triSimUntil { (gmGarrisonLive sbO) == 0 }
gmZoneSet [sbO, "garrison", 0]
gmZoneSet [sbO, "owner", gmResistanceSide]
gmZoneSet [sbO, "heatRaise", 100]
sbHold = [sbRes, gmFactionSquad [gmResistanceSide, 1, 3], sbOPos] call GM_fnSpawnSquad
sbHold setCombatMode "BLUE"
{_x allowDammage false} forEach (units sbHold)
doStop (units sbHold)
triAssertEq [(count units sbHold), 3]
sbObsPos = [(sbOPos select 0) + 40, (sbOPos select 1) + 30, 0]
sbObs = [sbOcc, gmFactionSquad [gmOccupierSide, 1, 2], sbObsPos] call GM_fnSpawnSquad
sbObs setCombatMode "BLUE"
{_x allowDammage false} forEach (units sbObs)
doStop (units sbObs)
{sbObs reveal _x} forEach (units sbHold)
player setPos [7922, 8500, 0]
triSimUntil { call sbSample; not ((gmZoneSim sbO) select 0) }
triAssertEq [((gmZone sbO) select 2), gmResistanceSide]
sbRep = 0
sbT1 = time
triSimUntil { call sbSample; {sbObs reveal _x} forEach (units sbHold); sbRep = 0; {if ((_x select 0) == "Outpost") then {sbRep = _x select 1}} forEach gmOccupierIntel; (sbRep >= 3) or (time > sbT1 + 120) }
triAssertGe [sbRep, 3]

// ---- s3: the pool at 50 of 60 -------------------------------------------------
triAssertEq [(format ["%1", gmForcesForce ["pool", 50]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
call sbSample

// ---- s4: Houdan rises: the BASE retake goes, no opener ------------------------
sbV = gmZoneIndex "Village"
triAssertGe [sbV, 0]
sbVPos = (gmZone sbV) select 8
triSimUntil { call sbSample; not (gmGarrisonSpawned sbV) }
gmZoneSet [sbV, "garrison", 0]
gmZoneSet [sbV, "support", 55]
sbSq = [sbRes, gmFactionSquad [gmResistanceSide, 1, 1], sbVPos] call GM_fnSpawnSquad
sbSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units sbSq)
doStop (units sbSq)
triSimUntil { call sbSample; ((gmZone sbV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units sbSq)
sbStrike = []
sbStrikeSeen = {{if ((_x select 1) == "STRIKE") then {sbStrike = _x}} forEach gmOccupierOps}
triSimUntil { call sbSample; call sbStrikeSeen; ((gmOccupierForces) select 4) == "AWAKE" }
sbOp = []
triSimUntil { call sbSample; call sbStrikeSeen; sbOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {sbOp = _x}} forEach gmOccupierOps; (count sbOp) > 0 }
triAssertEq [(sbOp select 2), "DIRECTOR"]
// the ground operation left the pool STRONG: the old rule would buy the opener
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertGe [((gmOccupierForces) select 0) * 100, 60 * sbStart]
sbT2 = time
triSimUntil { call sbSample; call sbStrikeSeen; ((count sbStrike) > 0) or (time > sbT2 + 10) }
triAssertEq [(count sbStrike), 0]
triAssertEq [sbWorst, 0]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
gmSimDiagLog

triEndTest
