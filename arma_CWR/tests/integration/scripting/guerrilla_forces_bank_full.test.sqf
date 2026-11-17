// ============================================================================
//  Occupier forces, the ceiling DROPS only when every base is full
//  (OCCUPIER-FORCES.md 5.5 "The ceiling"; the owner's decision of 2026-10-16:
//  "bank surplus survivors into garrisons instead of dropping them").  The
//  companion of guerrilla_forces_bank_garrison: the same strike home to a
//  full pool, but with EVERY occupier base pinned at its garrisonMax there is
//  no room to bank into, and the crew over poolStart are dropped.  Boots the
//  SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    f1  as b1 of the companion (a real camera, STRONG, DORMANT, wakeTowns
//        99 so the director never wakes, the fast refill on a 1 s director
//        tick);
//    f2  the player walks out west; every occupier base that is not a CITY
//        or CAMP (the generated bases included) is stored and pinned at its
//        garrisonMax, the Southeast Compound among them;
//    f3  a held CITY more than 2 km from the player is handed to the
//        resistance and a test-lever strike (gmForcesForce "strike") flies
//        at it abstract; the pool is forced back to poolStart while it is
//        out;
//    f4  the airframe and its pilot come home: the pool stays at poolStart
//        (sampled every frame) and no base's reserve plus pending moved.
//        The crew has no room at its base and none anywhere to bank, so it
//        is dropped; the hull's units are not banked as bodies either:
//        under forcesBankHulls 0 (the default since 7765904) they become a
//        stored spare hull at a base (gmOccupierLedger), which neither the
//        pool nor a base's reserve plus pending counts.
//  tri reads no engine log: the "dropped" and "stored as a spare hull" lines
//  are pinned in the unit cases ([occupierforces], [counterattack]); here the
//  pool and the bases show them.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
bfT0 = time
triSimUntil { time > bfT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// the pool sampler: the largest pool - poolStart seen so far
bfOver = -999
bfSample = {bfF = gmOccupierForces; if (((bfF select 0) - (bfF select 1)) > bfOver) then {bfOver = (bfF select 0) - (bfF select 1)}}
// a base's stored strength: its reserve plus its S5 pending deposit
bfHeld = {((gmZone _this) select 3) + ((gmZoneSim _this) select 13)}

// ---- f1: a real camera; the pool STRONG; the levers -------------------------
bfP = gmTrafficPercept (getPos player)
triAssertEq [(bfP select 0), 1]
triAssertGe [(bfP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
bfStart = (gmOccupierForces) select 1
triAssertGe [bfStart, 20]

// ---- f2: every base pinned full -------------------------------------------
player setPos [4500, 6000, 0]
bfMil = []
bfI = 0
while {bfI < gmZoneCount} do {bfZ = gmZone bfI; if (((bfZ select 1) != "CITY") and ((bfZ select 1) != "CAMP") and ((bfZ select 2) == gmOccupierSide)) then {bfMil = bfMil + [bfI]}; bfI = bfI + 1}
triAssertGe [(count bfMil), 3]
bfN = gmZoneIndex "Southeast Compound"
triAssertGe [bfN, 0]
triSimUntil { call bfSample; bfAny = false; {if (gmGarrisonSpawned _x) then {bfAny = true}} forEach bfMil; not bfAny }
{gmZoneSet [_x, "garrison", ((gmZoneSim _x) select 7)]} forEach bfMil
bfT1 = time
triSimUntil { call bfSample; time > bfT1 + 2 }
// every base is at (or above) its garrisonMax: no room anywhere
bfFull = true
{if ((_x call bfHeld) < ((gmZoneSim _x) select 7)) then {bfFull = false}} forEach bfMil
triAssertEq [(format ["%1", bfFull]), "true"]

// ---- f3: a strike out; the pool forced back to poolStart -------------------
// a held CITY more than 2 km from the player, so the strike flies abstract
bfPl = getPos player
bfA = -1
bfI = 0
while {bfI < gmZoneCount} do {bfZi = gmZone bfI; if ((bfA < 0) and ((bfZi select 1) == "CITY") and (((bfZi select 8) distance bfPl) > 2000)) then {bfA = bfI}; bfI = bfI + 1}
triAssertGe [bfA, 0]
bfAName = (gmZone bfA) select 0
gmZoneSet [bfA, "garrison", 0]
gmZoneSet [bfA, "owner", gmResistanceSide]
triSimUntil { call bfSample; ((gmOccupierForces) select 0) == bfStart }
bfOthers = []
{bfOthers = bfOthers + [_x call bfHeld]} forEach bfMil
triAssertEq [(format ["%1", gmForcesForce ["strike", bfA]]), "true"]
bfSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == bfAName)) then {bfSOp = _x}} forEach gmOccupierOps
triAssertGe [(count bfSOp), 4]
bfSId = bfSOp select 0
bfPrice = bfStart - ((gmOccupierForces) select 0)
triAssertGe [bfPrice, 2]
triAssertEq [(format ["%1", gmForcesForce ["pool", bfStart]]), "true"]
triAssertEq [((gmOccupierForces) select 0), bfStart]

// ---- f4: the airframe home: nowhere to bank, dropped ------------------------
bfT2 = time
bfLive = true
triSimUntil { call bfSample; bfLive = false; {if ((_x select 0) == bfSId) then {bfLive = true}} forEach gmOccupierOps; (not bfLive) or (time > bfT2 + 400) }
triAssertEq [(format ["%1", bfLive]), "false"]
bfT3 = time
triSimUntil { call bfSample; time > bfT3 + 5 }
triAssertLe [bfOver, 0]
triAssertEq [((gmOccupierForces) select 0), bfStart]
// no base took a body: the price over poolStart was dropped
bfK = 0
bfMoved = 0
{if ((_x call bfHeld) != (bfOthers select bfK)) then {bfMoved = bfMoved + 1}; bfK = bfK + 1} forEach bfMil
triAssertEq [bfMoved, 0]
gmSimDiagLog

triEndTest
