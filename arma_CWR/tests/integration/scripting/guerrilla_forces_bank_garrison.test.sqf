// ============================================================================
//  Occupier forces, the ceiling BANKS its excess as garrison (OCCUPIER-FORCES.md
//  5.5 "The ceiling"; the owner's decision of 2026-10-16: "bank surplus
//  survivors into garrisons instead of dropping them").  3c9947b dropped the
//  units a refund brought home over poolStart; now they become a pending
//  deposit at an occupier base with room below its garrisonMax, and only what
//  no base can take is dropped.  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    b1  a real camera; the pool STRONG; the director parked DORMANT
//        (wakeTowns 99); the refill fast (0.01 h) on a 1 s director tick, so
//        the pool sits at poolStart and every unit the airframe brings home
//        is over the ceiling;
//    b2  the player walks out west; every occupier base is stored and
//        pinned at its garrisonMax, except the Southeast Compound, emptied
//        to 0 of 40 (its D2 clock parked), the one base with room;
//    b3  a test-lever strike (the pattern of guerrilla_forces_opener_empty_base
//        e4) flies on a held town far from the player; its price leaves the
//        pool, and the pool is forced back to poolStart while it is out;
//    b4  the airframe and its pilot come home over the ceiling: the pool
//        stays at poolStart (sampled every frame), the Compound's reserve
//        plus pending rises by exactly the price (all of it banked, nothing
//        dropped), and every other base is unchanged.
//  tri reads no engine log: the "banked at" line and the absence of the
//  "dropped" line are pinned in the unit cases ([occupierforces],
//  [counterattack]); here the ledger itself shows nothing was dropped.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
bgT0 = time
triSimUntil { time > bgT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// the pool sampler: the largest pool - poolStart seen so far
bgOver = -999
bgSample = {bgF = gmOccupierForces; if (((bgF select 0) - (bgF select 1)) > bgOver) then {bgOver = (bgF select 0) - (bgF select 1)}}
// a base's stored strength: its reserve plus its S5 pending deposit
bgHeld = {((gmZone _this) select 3) + ((gmZoneSim _this) select 13)}

// ---- b1: a real camera; the pool STRONG; the levers -------------------------
bgP = gmTrafficPercept (getPos player)
triAssertEq [(bgP select 0), 1]
triAssertGe [(bgP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
bgStart = (gmOccupierForces) select 1
triAssertGe [bgStart, 20]

// ---- b2: every base pinned full but the Compound -----------------------------
player setPos [4500, 6000, 0]
bgMil = []
bgI = 0
while {bgI < gmZoneCount} do {bgZ = gmZone bgI; if (((bgZ select 1) != "CITY") and ((bgZ select 1) != "CAMP") and ((bgZ select 2) == gmOccupierSide)) then {bgMil = bgMil + [bgI]}; bgI = bgI + 1}
triAssertGe [(count bgMil), 3]
bgN = gmZoneIndex "Southeast Compound"
triAssertGe [bgN, 0]
triAssertEq [((gmZone bgN) select 2), gmOccupierSide]
triSimUntil { call bgSample; bgAny = false; {if (gmGarrisonSpawned _x) then {bgAny = true}} forEach bgMil; not bgAny }
{gmZoneSet [_x, "garrison", ((gmZoneSim _x) select 7)]} forEach bgMil
gmZoneSet [bgN, "garrisonMax", 40]
gmZoneSet [bgN, "garrison", 0]
gmZoneSet [bgN, "recoverClock", -1000000]
bgT1 = time
triSimUntil { call bgSample; time > bgT1 + 2 }
triAssertEq [(bgN call bgHeld), 0]
// every other base is at (or above) its garrisonMax: no room anywhere else
bgFull = true
{if ((_x != bgN) and ((_x call bgHeld) < ((gmZoneSim _x) select 7))) then {bgFull = false}} forEach bgMil
triAssertEq [(format ["%1", bgFull]), "true"]

// ---- b3: a strike out; the pool forced back to poolStart -------------------
// a held CITY more than 2 km from the player, so the strike flies abstract
bgPl = getPos player
bgA = -1
bgI = 0
while {bgI < gmZoneCount} do {bgZi = gmZone bgI; if ((bgA < 0) and ((bgZi select 1) == "CITY") and (((bgZi select 8) distance bgPl) > 2000)) then {bgA = bgI}; bgI = bgI + 1}
triAssertGe [bgA, 0]
bgAName = (gmZone bgA) select 0
gmZoneSet [bgA, "garrison", 0]
gmZoneSet [bgA, "owner", gmResistanceSide]
triSimUntil { call bgSample; ((gmOccupierForces) select 0) == bgStart }
bgOthers = []
{bgOthers = bgOthers + [_x call bgHeld]} forEach bgMil
triAssertEq [(format ["%1", gmForcesForce ["strike", bgA]]), "true"]
bgSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == bgAName)) then {bgSOp = _x}} forEach gmOccupierOps
triAssertGe [(count bgSOp), 4]
bgSId = bgSOp select 0
bgPrice = bgStart - ((gmOccupierForces) select 0)
triAssertGe [bgPrice, 2]
triAssertEq [(format ["%1", gmForcesForce ["pool", bgStart]]), "true"]
triAssertEq [((gmOccupierForces) select 0), bgStart]

// ---- b4: the airframe home: banked at the Compound, nothing dropped ---------
bgT2 = time
bgLive = true
triSimUntil { call bgSample; bgLive = false; {if ((_x select 0) == bgSId) then {bgLive = true}} forEach gmOccupierOps; (not bgLive) or (time > bgT2 + 400) }
triAssertEq [(format ["%1", bgLive]), "false"]
bgT3 = time
triSimUntil { call bgSample; time > bgT3 + 5 }
triAssertLe [bgOver, 0]
triAssertEq [((gmOccupierForces) select 0), bgStart]
// the whole price banked at the one base with room
triAssertEq [(bgN call bgHeld), bgPrice]
// and nowhere else
bgK = 0
bgMoved = 0
{if ((_x != bgN) and ((_x call bgHeld) != (bgOthers select bgK))) then {bgMoved = bgMoved + 1}; bgK = bgK + 1} forEach bgMil
triAssertEq [bgMoved, 0]
gmSimDiagLog

triEndTest
