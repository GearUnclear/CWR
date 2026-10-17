// ============================================================================
//  Occupier forces, a strike opener on a base with defenders REPORTED
//  (OCCUPIER-FORCES.md 7.3 "When a town strike is bought"; round 3 C of
//  OCCUPIER-FORCES-NEXT.md).  The companion of
//  guerrilla_forces_opener_empty_base: the opener is a strike on defenders,
//  so with three reported it still flies.  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    o1  a real camera; the pool full (STRONG); the levers of
//        guerrilla_forces_opener_empty_base;
//    o2  the Outpost loses its garrison and is handed to the resistance,
//        hot (heat 100), with a 3-man hold squad in it (invulnerable, hold
//        fire); two occupier observers beside it (invulnerable, hold fire)
//        are shown the three (reveal), so the director's sighting table
//        reports three fighters at the Outpost (D9: what its men saw); the
//        player walks 2.2 km north;
//    o3  Houdan rises remotely: the director wakes STRONG; its first
//        operation is the BASE retake of the Outpost and a STRIKE on the
//        Outpost goes with it (a jet or attack helicopter), at most one
//        director tick later.
//  The pool is sampled every frame: it never exceeds poolStart.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
orT0 = time
triSimUntil { time > orT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// the pool sampler: the largest pool - ceiling seen so far (poolStart, or the
// staged 60 once o1 sets it)
orCeil = 0
orOver = -999
orSample = {orF = gmOccupierForces; orRef = orF select 1; if (orCeil > orRef) then {orRef = orCeil}; if (((orF select 0) - orRef) > orOver) then {orOver = (orF select 0) - orRef}}

// ---- o1: a real camera; the pool STRONG; the levers -------------------------
orP = gmTrafficPercept (getPos player)
triAssertEq [(orP select 0), 1]
triAssertGe [(orP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["slowEta", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["airEta", 0]]), "true"]
// since the generated bases (d5fa57b) Abel's poolStart is 48, and the keep-STRONG
// rule (3c9947b, 1ae2dc9) cannot afford a ground buy plus an opener on it; the
// opener this test pins is staged on a pool of 60 (SetPool raises the ceiling)
triAssertEq [(format ["%1", gmForcesForce ["pool", 60]]), "true"]
orCeil = 60
orRes = gmResistanceSide call GM_fnSideFromString
orOcc = gmOccupierSide call GM_fnSideFromString
call orSample

// ---- o2: the Outpost, held and hot, three defenders reported ----------------
orO = gmZoneIndex "Outpost"
triAssertGe [orO, 0]
orOPos = (gmZone orO) select 8
triSimUntil { gmGarrisonSpawned orO }
orUnits = []
"orUnits = orUnits + (units _x)" forEach (gmGarrisonGroups orO)
{_x setDamage 1} forEach orUnits
triSimUntil { (gmGarrisonLive orO) == 0 }
gmZoneSet [orO, "garrison", 0]
gmZoneSet [orO, "owner", gmResistanceSide]
gmZoneSet [orO, "heatRaise", 100]
orHold = [orRes, gmFactionSquad [gmResistanceSide, 1, 3], orOPos] call GM_fnSpawnSquad
orHold setCombatMode "BLUE"
{_x allowDammage false} forEach (units orHold)
doStop (units orHold)
triAssertEq [(count units orHold), 3]
orObsPos = [(orOPos select 0) + 40, (orOPos select 1) + 30, 0]
orObs = [orOcc, gmFactionSquad [gmOccupierSide, 1, 2], orObsPos] call GM_fnSpawnSquad
orObs setCombatMode "BLUE"
{_x allowDammage false} forEach (units orObs)
doStop (units orObs)
{orObs reveal _x} forEach (units orHold)
player setPos [7922, 8500, 0]
triSimUntil { call orSample; not ((gmZoneSim orO) select 0) }
triAssertEq [((gmZone orO) select 2), gmResistanceSide]
// the sighting scan (5 s) reports the three at the Outpost
orRep = 0
orT1 = time
triSimUntil { call orSample; {orObs reveal _x} forEach (units orHold); orRep = 0; {if ((_x select 0) == "Outpost") then {orRep = _x select 1}} forEach gmOccupierIntel; (orRep >= 3) or (time > orT1 + 120) }
triAssertGe [orRep, 3]

// ---- o3: Houdan rises remotely: the BASE retake and its opener ----------------
orV = gmZoneIndex "Village"
triAssertGe [orV, 0]
orVPos = (gmZone orV) select 8
triSimUntil { call orSample; not (gmGarrisonSpawned orV) }
gmZoneSet [orV, "garrison", 0]
gmZoneSet [orV, "support", 55]
orSq = [orRes, gmFactionSquad [gmResistanceSide, 1, 1], orVPos] call GM_fnSpawnSquad
orSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units orSq)
doStop (units orSq)
triSimUntil { call orSample; ((gmZone orV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units orSq)
orStrike = []
orStrikeSeen = {{if (((_x select 1) == "STRIKE") and ((_x select 5) == "Outpost")) then {orStrike = _x}} forEach gmOccupierOps}
triSimUntil { call orSample; call orStrikeSeen; ((gmOccupierForces) select 4) == "AWAKE" }
orOp = []
triSimUntil { call orSample; call orStrikeSeen; orOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {orOp = _x}} forEach gmOccupierOps; (count orOp) > 0 }
triAssertEq [(orOp select 2), "DIRECTOR"]
orT2 = time
triSimUntil { call orSample; call orStrikeSeen; ((count orStrike) > 0) or (time > orT2 + 5) }
triAssertGe [(count orStrike), 4]
triAssertEq [(orStrike select 2), "DIRECTOR"]
triAssertEq [(format ["%1", (orStrike select 3) in ["JET", "ATTACK_HELI"]]), "true"]
triAssertLe [orOver, 0]
gmSimDiagLog

triEndTest
