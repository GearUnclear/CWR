// ============================================================================
//  Occupier forces, no strike opener on an EMPTY base, and the ledger ceiling
//  (OCCUPIER-FORCES.md 7.3 "When a town strike is bought", 5.5 "The
//  ceiling"; round 3 C of OCCUPIER-FORCES-NEXT.md).  The comparison video
//  (tmp/of-director-video-3, both branches) caught a base retake on an empty
//  base, 0 fighters reported, buying a 23-unit jet opener, and the opener's
//  refund landing the pool above poolStart (51 of 50).  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    e1  a real camera; the pool full (STRONG); the strike levers of
//        guerrilla_forces_town_strike (the legacy first-town roll 0) plus
//        the director levers of guerrilla_forces_base_retake (tick 1 s, one
//        distinct town wakes it, the abstract leg at 100 m/s, the arrival
//        rule off);
//    e2  the Outpost loses its garrison and is handed to the resistance,
//        hot (heat 100), with NOBODY in it; the player walks 2.2 km north;
//        the director's sighting table holds no report for it;
//    e3  Houdan rises remotely: the director wakes STRONG and its first
//        operation is the BASE retake of the Outpost; no STRIKE record on
//        the Outpost is created with it, or in the 20 s after;
//    e4  the ceiling: the refill is forced to 36 s for a full pool and a
//        test-lever strike flies on a held town far from the player; the
//        refill tops the pool back up to poolStart while the jet is out,
//        and the jet's refund never lifts the pool above poolStart.
//  The pool is sampled every frame from e1 to the end: it never exceeds
//  poolStart.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
ebT0 = time
triSimUntil { time > ebT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// the pool sampler: the largest pool - poolStart seen so far
ebOver = -999
ebSample = {ebF = gmOccupierForces; if (((ebF select 0) - (ebF select 1)) > ebOver) then {ebOver = (ebF select 0) - (ebF select 1)}}

// ---- e1: a real camera; the pool STRONG; the levers -------------------------
ebP = gmTrafficPercept (getPos player)
triAssertEq [(ebP select 0), 1]
triAssertGe [(ebP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["slowEta", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["airEta", 0]]), "true"]
ebRes = gmResistanceSide call GM_fnSideFromString
call ebSample

// ---- e2: the Outpost, held, hot and empty -----------------------------------
ebO = gmZoneIndex "Outpost"
triAssertGe [ebO, 0]
ebOPos = (gmZone ebO) select 8
triSimUntil { gmGarrisonSpawned ebO }
ebUnits = []
"ebUnits = ebUnits + (units _x)" forEach (gmGarrisonGroups ebO)
{_x setDamage 1} forEach ebUnits
triSimUntil { (gmGarrisonLive ebO) == 0 }
gmZoneSet [ebO, "garrison", 0]
gmZoneSet [ebO, "owner", gmResistanceSide]
gmZoneSet [ebO, "heatRaise", 100]
player setPos [7922, 8500, 0]
triSimUntil { call ebSample; not ((gmZoneSim ebO) select 0) }
triAssertEq [((gmZone ebO) select 2), gmResistanceSide]
// nobody reported at the Outpost (D9: the director's census of defenders)
ebRep = 0
{if ((_x select 0) == "Outpost") then {ebRep = _x select 1}} forEach gmOccupierIntel
triAssertEq [ebRep, 0]

// ---- e3: Houdan rises remotely: the BASE retake goes, no opener ----------------
ebV = gmZoneIndex "Village"
triAssertGe [ebV, 0]
ebVPos = (gmZone ebV) select 8
triSimUntil { call ebSample; not (gmGarrisonSpawned ebV) }
gmZoneSet [ebV, "garrison", 0]
gmZoneSet [ebV, "support", 55]
ebSq = [ebRes, gmFactionSquad [gmResistanceSide, 1, 1], ebVPos] call GM_fnSpawnSquad
ebSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units ebSq)
doStop (units ebSq)
triSimUntil { call ebSample; ((gmZone ebV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units ebSq)
ebStrike = false
ebStrikeSeen = {{if (((_x select 1) == "STRIKE") and ((_x select 5) == "Outpost")) then {ebStrike = true}} forEach gmOccupierOps}
triSimUntil { call ebSample; call ebStrikeSeen; ((gmOccupierForces) select 4) == "AWAKE" }
ebOp = []
triSimUntil { call ebSample; call ebStrikeSeen; ebOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {ebOp = _x}} forEach gmOccupierOps; (count ebOp) > 0 }
triAssertEq [(ebOp select 2), "DIRECTOR"]
// the opener would launch in the same director tick as the retake
call ebStrikeSeen
triAssertEq [(format ["%1", ebStrike]), "false"]
ebT1 = time
triSimUntil { call ebSample; call ebStrikeSeen; time > ebT1 + 20 }
triAssertEq [(format ["%1", ebStrike]), "false"]
triAssertLe [ebOver, 0]

// ---- e4: the ceiling: a jet out while the refill tops the pool up ------------
// a held CITY more than 2 km from the player and from the Outpost, so the
// strike flies abstract and its effects stay off the retake
ebPl = getPos player
ebA = -1
ebI = 0
while {ebI < gmZoneCount} do {ebZi = gmZone ebI; if ((ebA < 0) and ((ebZi select 1) == "CITY") and (((ebZi select 8) distance ebPl) > 2000) and (((ebZi select 8) distance ebOPos) > 2000)) then {ebA = ebI}; ebI = ebI + 1}
triAssertGe [ebA, 0]
ebAName = (gmZone ebA) select 0
gmZoneSet [ebA, "garrison", 0]
gmZoneSet [ebA, "owner", gmResistanceSide]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triSimUntil { call ebSample; ((gmOccupierForces) select 0) == ((gmOccupierForces) select 1) }
triAssertEq [(format ["%1", gmForcesForce ["strike", ebA]]), "true"]
ebSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == ebAName)) then {ebSOp = _x}} forEach gmOccupierOps
triAssertGe [(count ebSOp), 4]
ebSId = ebSOp select 0
triAssertLt [((gmOccupierForces) select 0), ((gmOccupierForces) select 1)]
// the refill tops the pool back up while the airframe is out ...
ebFull = false
ebT2 = time
triSimUntil { call ebSample; ebLive = false; {if ((_x select 0) == ebSId) then {ebLive = true}} forEach gmOccupierOps; if (ebLive and (((gmOccupierForces) select 0) == ((gmOccupierForces) select 1))) then {ebFull = true}; (not ebLive) or (time > ebT2 + 400) }
triAssertEq [(format ["%1", ebFull]), "true"]
// ... and its refund never lifted the pool above poolStart
ebT3 = time
triSimUntil { call ebSample; time > ebT3 + 5 }
triAssertLe [ebOver, 0]
triAssertLe [((gmOccupierForces) select 0), ((gmOccupierForces) select 1)]
gmSimDiagLog

triEndTest
