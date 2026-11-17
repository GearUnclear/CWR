// ============================================================================
//  Occupier forces, the HULL FLAG (OCCUPIER-FORCES.md 5.5 "Stored spare
//  hulls"; owner decision 9 of 2026-10-16).  Video 5's "22 banked": an APC
//  came home to a full pool and became 22 bodies of garrison.  With
//  forcesBankHulls 0 (the default) only bodies bank; a hull over the
//  ceiling is a stored spare hull at its base (an airframe's airfield), and
//  it rejoins the pool as soon as the pool has room.  The companion of
//  guerrilla_forces_bank_garrison (the same strike home to a full pool, which
//  pins forcesBankHulls 1).  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    k1  a real camera; the pool STRONG; the director parked DORMANT; the
//        refill fast (0.01 h) on a 1 s director tick, so the pool sits at
//        poolStart; the default hull flag (gmOccupierLedger [11] false);
//    k2  the player walks out west; every occupier base is stored and
//        pinned at its garrisonMax, except the Southeast Compound, emptied
//        to 0 of 40 (its D2 clock parked): room for bodies;
//    k3  a test-lever strike flies on a held town far from the player; the
//        pool is forced back to poolStart while it is out;
//    k4  the airframe and its pilot come home over the ceiling: the pool
//        stays at poolStart, nothing is dropped, the Compound takes only the
//        crew (bodies), and the airframe's hull is ONE stored spare hull at an
//        occupier base (gmOccupierLedger [8]); price = crew banked + hull;
//    k5  the pool is forced down by the hull's units: on the next director
//        tick the spare rejoins the pool ahead of the refill, the stored row
//        is gone and the pool is back at poolStart.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
khT0 = time
triSimUntil { time > khT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

khOver = -999
khSample = {khF = gmOccupierForces; if (((khF select 0) - (khF select 1)) > khOver) then {khOver = (khF select 0) - (khF select 1)}}
khHeld = {((gmZone _this) select 3) + ((gmZoneSim _this) select 13)}

// ---- k1: a real camera; the pool STRONG; the levers -------------------------
khP = gmTrafficPercept (getPos player)
triAssertEq [(khP select 0), 1]
triAssertGe [(khP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", (gmOccupierLedger) select 11]), "false"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
khStart = (gmOccupierForces) select 1
triAssertGe [khStart, 20]
khDropped0 = (gmOccupierLedger) select 5

// ---- k2: every base pinned full but the Compound -----------------------------
player setPos [4500, 6000, 0]
khMil = []
khI = 0
while {khI < gmZoneCount} do {khZ = gmZone khI; if (((khZ select 1) != "CITY") and ((khZ select 1) != "CAMP") and ((khZ select 2) == gmOccupierSide)) then {khMil = khMil + [khI]}; khI = khI + 1}
triAssertGe [(count khMil), 3]
khN = gmZoneIndex "Southeast Compound"
triAssertGe [khN, 0]
triSimUntil { call khSample; khAny = false; {if (gmGarrisonSpawned _x) then {khAny = true}} forEach khMil; not khAny }
{gmZoneSet [_x, "garrison", ((gmZoneSim _x) select 7)]} forEach khMil
gmZoneSet [khN, "garrisonMax", 40]
gmZoneSet [khN, "garrison", 0]
gmZoneSet [khN, "recoverClock", -1000000]
khT1 = time
triSimUntil { call khSample; time > khT1 + 2 }
triAssertEq [(khN call khHeld), 0]

// ---- k3: a strike out; the pool forced back to poolStart -------------------
khPl = getPos player
khA = -1
khI = 0
while {khI < gmZoneCount} do {khZi = gmZone khI; if ((khA < 0) and ((khZi select 1) == "CITY") and (((khZi select 8) distance khPl) > 2000)) then {khA = khI}; khI = khI + 1}
triAssertGe [khA, 0]
khAName = (gmZone khA) select 0
gmZoneSet [khA, "garrison", 0]
gmZoneSet [khA, "owner", gmResistanceSide]
triSimUntil { call khSample; ((gmOccupierForces) select 0) == khStart }
khOthers = []
{khOthers = khOthers + [_x call khHeld]} forEach khMil
triAssertEq [(format ["%1", gmForcesForce ["strike", khA]]), "true"]
khSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == khAName)) then {khSOp = _x}} forEach gmOccupierOps
triAssertGe [(count khSOp), 4]
khSId = khSOp select 0
khPrice = khStart - ((gmOccupierForces) select 0)
triAssertGe [khPrice, 2]
triAssertEq [(format ["%1", gmForcesForce ["pool", khStart]]), "true"]
triAssertEq [((gmOccupierForces) select 0), khStart]

// ---- k4: home: the crew bank, the hull is a stored spare ---------------------
khT2 = time
khLive = true
triSimUntil { call khSample; khLive = false; {if ((_x select 0) == khSId) then {khLive = true}} forEach gmOccupierOps; (not khLive) or (time > khT2 + 400) }
triAssertEq [(format ["%1", khLive]), "false"]
khT3 = time
triSimUntil { call khSample; time > khT3 + 3 }
triAssertLe [khOver, 0]
triAssertEq [((gmOccupierForces) select 0), khStart]
triAssertEq [((gmOccupierLedger) select 5), khDropped0]
khHulls = (gmOccupierLedger) select 8
triAssertEq [(count khHulls), 1]
khHull = khHulls select 0
khUnits = khHull select 3
triAssertGe [khUnits, 2]
khHome = gmZoneIndex (khHull select 0)
triAssertGe [khHome, 0]
triAssertEq [((gmZone khHome) select 2), gmOccupierSide]
triAssertEq [(format ["%1", (khHull select 1) in ["JET", "ATTACK_HELI"]]), "true"]
// the Compound took only bodies (the crew), the hull none of it
khCrew = khN call khHeld
triAssertLe [khCrew, 2]
triAssertEq [(khCrew + khUnits), khPrice]
khK = 0
khMoved = 0
{if ((_x != khN) and ((_x call khHeld) != (khOthers select khK))) then {khMoved = khMoved + 1}; khK = khK + 1} forEach khMil
triAssertEq [khMoved, 0]

// ---- k5: room in the pool: the spare rejoins it first ------------------------
triAssertEq [(format ["%1", gmForcesForce ["pool", khStart - khUnits]]), "true"]
khT4 = time
triSimUntil { call khSample; ((count ((gmOccupierLedger) select 8)) == 0) or (time > khT4 + 30) }
triAssertEq [(count ((gmOccupierLedger) select 8)), 0]
triAssertEq [((gmOccupierForces) select 0), khStart]
triAssertEq [((gmOccupierLedger) select 5), khDropped0]
gmSimDiagLog

triEndTest
