// ============================================================================
//  Occupier forces, the air rule of the package choice (OCCUPIER-FORCES.md
//  6.5 "The arrival rule"; phase 1 C, 2026-10-16, after video 5 where all
//  five insertions flew to empty targets).  A helicopter insertion goes only
//  across water, past forcesAirEta by the fastest ground package, or on a
//  target out of walking reach with fighters reported.  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1, occupier EAST: Ural truck,
//  Mi17 transport helicopter) on the full Remaster:
//    a1  a real camera at the default view distance in daylight; the pool
//        full (STRONG); the levers of guerrilla_forces_arrival_rule: director
//        tick 1 s, one distinct town wakes the director, the first-town roll
//        0; the pool is forced to 0 while Houdan rises remotely (the wake
//        launches nothing), Houdan rests on a long cooldown and the pool is
//        restored: STRONG with two tokens;
//    a2  Goisse (4.6 km from the Outpost, about 66 minutes on foot by road)
//        is handed to the resistance, hot and empty: the operation is a
//        TRUCK (the Ural is there in about 450 s, under forcesAirEta, and
//        nobody is reported; the old rule flew it);
//    a3  the Southeast Compound (an island: every base stands on another
//        landmass) is handed over, hot and empty: its BASE retake is still a
//        TRANSPORT_HELI insertion (air: across water);
//    a4  Goisse reverts to the occupier with a whole garrison, so the truck
//        goes home and its record ends; then Goisse is handed over again,
//        a 3-man hold squad in it (invulnerable, hold fire) shown to two
//        occupier observers (invulnerable, hold fire), so the director's
//        sightings report three fighters there; hot, at 4x game time until
//        the next token: the new operation on Goisse is a TRANSPORT_HELI
//        insertion (air: fighters reported).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
afT0 = time
triSimUntil { time > afT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: a real camera; the levers; the director wakes ----------------------
afP = gmTrafficPercept (getPos player)
triAssertEq [(afP select 0), 1]
triAssertGe [(afP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
afStart = (gmOccupierForces) select 1
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
afRes = gmResistanceSide call GM_fnSideFromString
afOcc = gmOccupierSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmForcesForce ["pool", 0]]), "true"]
// the player stands north, away from every base (no base materialized)
player setPos [7922, 8500, 0]
afV = gmZoneIndex "Village"
triAssertGe [afV, 0]
afVPos = (gmZone afV) select 8
triSimUntil { not (gmGarrisonSpawned afV) }
gmZoneSet [afV, "garrison", 0]
gmZoneSet [afV, "support", 55]
afSq = [afRes, gmFactionSquad [gmResistanceSide, 1, 1], afVPos] call GM_fnSpawnSquad
afSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units afSq)
doStop (units afSq)
triSimUntil { ((gmZone afV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units afSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(format ["%1", gmForcesForce ["cooldown", afV, 99999]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["pool", afStart]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 3), 2]

// ---- a2: an empty Goisse gets a truck -----------------------------------------
afG = gmZoneIndex "Goisse"
triAssertGe [afG, 0]
afGPos = (gmZone afG) select 8
afC = gmZoneIndex "Southeast Compound"
triAssertGe [afC, 0]
triSimUntil { (not (gmGarrisonSpawned afG)) and (not (gmGarrisonSpawned afC)) }
gmZoneSet [afG, "garrison", 0]
gmZoneSet [afG, "owner", gmResistanceSide]
gmZoneSet [afG, "heatRaise", 100]
afOp = []
triSimUntil { afOp = []; {if ((_x select 5) == "Goisse") then {afOp = _x}} forEach gmOccupierOps; (count afOp) > 0 }
triAssertEq [(afOp select 1), "RETAKE"]
triAssertEq [(format ["%1", (afOp select 2) in ["DIRECTOR", "PUSHOUT"]]), "true"]
triAssertEq [(afOp select 3), "TRUCK"]
afOpId = afOp select 0

// ---- a3: the Southeast Compound still flies -----------------------------------
gmZoneSet [afC, "garrison", 0]
gmZoneSet [afC, "owner", gmResistanceSide]
gmZoneSet [afC, "heatRaise", 100]
afOpC = []
triSimUntil { afOpC = []; {if ((_x select 5) == "Southeast Compound") then {afOpC = _x}} forEach gmOccupierOps; (count afOpC) > 0 }
triAssertEq [(afOpC select 1), "BASE"]
triAssertEq [(afOpC select 3), "TRANSPORT_HELI"]

// ---- a4: Goisse with three fighters reported gets the insertion --------------
// the occupier takes Goisse back with a whole garrison: the truck goes home
afGMax = (gmZoneSim afG) select 7
gmZoneSet [afG, "owner", gmOccupierSide]
gmZoneSet [afG, "garrison", afGMax]
triSimUntil { afLeft = 0; {if ((_x select 0) == afOpId) then {afLeft = afLeft + 1}} forEach gmOccupierOps; afLeft == 0 }
// handed over again, empty; the hold squad and the observers take post
gmZoneSet [afG, "garrison", 0]
gmZoneSet [afG, "owner", gmResistanceSide]
afHold = [afRes, gmFactionSquad [gmResistanceSide, 1, 3], afGPos] call GM_fnSpawnSquad
afHold setCombatMode "BLUE"
{_x allowDammage false} forEach (units afHold)
doStop (units afHold)
triAssertEq [(count units afHold), 3]
afObsPos = [(afGPos select 0) + 40, (afGPos select 1) + 30, 0]
afObs = [afOcc, gmFactionSquad [gmOccupierSide, 1, 2], afObsPos] call GM_fnSpawnSquad
afObs setCombatMode "BLUE"
{_x allowDammage false} forEach (units afObs)
doStop (units afObs)
// the observers face the hold squad and watch it: reveal makes the three known
// but never seen, and the group reports a contact (posReported, which the
// sighting scan reads) only after one of its men sees it inside his eye cone;
// a stopped unit spawns facing north, away from a squad to its south-west
{_x setDir (((afGPos select 0) - ((getPos _x) select 0)) atan2 ((afGPos select 1) - ((getPos _x) select 1))); _x doWatch afGPos} forEach (units afObs)
// the sighting scan (5 s) reports the three at Goisse
afRep = 0
afT1 = time
triSimUntil { {afObs reveal _x} forEach (units afHold); afRep = 0; {if ((_x select 0) == "Goisse") then {afRep = _x select 1}} forEach gmOccupierIntel; (afRep >= 3) or (time > afT1 + 120) }
triAssertGe [afRep, 3]
// hot, off its rest; the next token comes within about 600 s of game time
triAssertEq [(format ["%1", gmForcesForce ["cooldown", afG, 1]]), "true"]
gmZoneSet [afG, "heatRaise", 100]
setAccTime 4
afOp2 = []
afT2 = time
triSimUntil { {afObs reveal _x} forEach (units afHold); afOp2 = []; {if (((_x select 5) == "Goisse") and ((_x select 0) != afOpId)) then {afOp2 = _x}} forEach gmOccupierOps; ((count afOp2) > 0) or (time > afT2 + 900) }
setAccTime 1
triAssertGe [(count afOp2), 4]
triAssertEq [(afOp2 select 1), "RETAKE"]
triAssertEq [(afOp2 select 3), "TRANSPORT_HELI"]
{deleteVehicle _x} forEach (units afHold)
{deleteVehicle _x} forEach (units afObs)
gmSimDiagLog

triEndTest
