// ============================================================================
//  Occupier forces, the WAVE SURGE (OCCUPIER-FORCES.md 7.1 "The wave surge",
//  5.5; owner decision 5 of 2026-10-16).  Video 5: after wave 1 every base
//  sat at garrisonMax and what came home over the ceiling was dropped (9
//  dropped, 1 banked).  Now, while a landed wave is on, the excess is a surge
//  above garrisonMax at the bases nearest the wave's cluster, which decays
//  back to garrisonMax: 0 dropped.  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    s1  a real camera; the pool STRONG; the director parked DORMANT
//        (wakeTowns 99, so the wave sends no strikes); the refill fast on a
//        1 s director tick; a 5 s wave lead; forcesBankHulls 1 (bankHulls),
//        so a strike's whole price comes home as bodies; surgeShare 1, so
//        the bases' surge caps can take the whole price;
//    s2  the player walks out west; every occupier base is stored and
//        pinned at its garrisonMax;
//    s3  wave 1 is forced and lands: nothing to top up, no strikes, no
//        bled base, so it stays in the pool above poolStart and the surge
//        opens (gmOccupierLedger [9]);
//    s4  a test-lever strike flies on a held town far from the player; the
//        pool is forced back to its ceiling while it is out;
//    s5  the airframe and its pilot come home over the ceiling: the pool
//        never passes its ceiling (sampled every frame), nothing is dropped,
//        and the bases' reserve plus pending rose by exactly the price, at
//        least one base now above its garrisonMax (the surge);
//    s6  the decay (surgeDecayHours 0.01 h, a lever read live) takes every
//        base back to its garrisonMax; the pool sits at its ceiling, so the
//        whole surge stands down (gmOccupierLedger [6]); still 0 dropped.
//  tri reads no engine log: the "surge:" lines are pinned in the unit cases.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
suT0 = time
triSimUntil { time > suT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// the sampler: the largest pool - ceiling seen so far
suOver = -999
suSample = {suF = gmOccupierForces; suL = gmOccupierLedger; if (((suF select 0) - (suL select 10)) > suOver) then {suOver = (suF select 0) - (suL select 10)}}
// a base's stored strength: its reserve plus its S5 pending deposit
suHeld = {((gmZone _this) select 3) + ((gmZoneSim _this) select 13)}

// ---- s1: a real camera; the pool STRONG; the levers -------------------------
suP = gmTrafficPercept (getPos player)
triAssertEq [(suP select 0), 1]
triAssertGe [(suP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["waveLead", 5]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["bankHulls", 1]]), "true"]
// surgeShare 1 (a lever): with every Abel base pinned full the default +50 %
// caps hold about 22 bodies in all, one short of a jet's price
triAssertEq [(format ["%1", gmForcesTune ["surgeShare", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 0.01]]), "true"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
suStart = (gmOccupierForces) select 1
triAssertGe [suStart, 20]
triAssertEq [(format ["%1", (gmOccupierLedger) select 9]), "false"]
suDropped0 = (gmOccupierLedger) select 5

// ---- s2: every base pinned full ---------------------------------------------
player setPos [4500, 6000, 0]
suMil = []
suI = 0
while {suI < gmZoneCount} do {suZ = gmZone suI; if (((suZ select 1) != "CITY") and ((suZ select 1) != "CAMP") and ((suZ select 2) == gmOccupierSide)) then {suMil = suMil + [suI]}; suI = suI + 1}
triAssertGe [(count suMil), 3]
triSimUntil { call suSample; suAny = false; {if (gmGarrisonSpawned _x) then {suAny = true}} forEach suMil; not suAny }
{gmZoneSet [_x, "garrison", ((gmZoneSim _x) select 7)]} forEach suMil
suT1 = time
triSimUntil { call suSample; time > suT1 + 2 }
suFull = true
{if ((_x call suHeld) < ((gmZoneSim _x) select 7)) then {suFull = false}} forEach suMil
triAssertEq [(format ["%1", suFull]), "true"]

// ---- s3: wave 1 lands and stays in the pool; the surge opens ----------------
triSimUntil { call suSample; ((gmOccupierForces) select 0) == suStart }
triAssertEq [(format ["%1", gmForcesForce ["wave", 1]]), "true"]
triSimUntil { call suSample; ((gmOccupierForces) select 22) >= 1 }
suT2 = time
triSimUntil { call suSample; time > suT2 + 2 }
triAssertGe [((gmOccupierForces) select 0), suStart + 1]
triAssertEq [(format ["%1", (gmOccupierLedger) select 9]), "true"]
suCeil = (gmOccupierLedger) select 10
triAssertGe [suCeil, suStart + 1]

// ---- s4: a strike out; the pool forced back to its ceiling ------------------
suPl = getPos player
suA = -1
suI = 0
while {suI < gmZoneCount} do {suZi = gmZone suI; if ((suA < 0) and ((suZi select 1) == "CITY") and (((suZi select 8) distance suPl) > 2000)) then {suA = suI}; suI = suI + 1}
triAssertGe [suA, 0]
suAName = (gmZone suA) select 0
gmZoneSet [suA, "garrison", 0]
gmZoneSet [suA, "owner", gmResistanceSide]
suBefore = 0
{suBefore = suBefore + (_x call suHeld)} forEach suMil
suPool0 = (gmOccupierForces) select 0
triAssertEq [(format ["%1", gmForcesForce ["strike", suA]]), "true"]
suSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == suAName)) then {suSOp = _x}} forEach gmOccupierOps
triAssertGe [(count suSOp), 4]
suSId = suSOp select 0
suPrice = suPool0 - ((gmOccupierForces) select 0)
triAssertGe [suPrice, 2]
triAssertEq [(format ["%1", gmForcesForce ["pool", suCeil]]), "true"]
triAssertEq [((gmOccupierForces) select 0), suCeil]

// ---- s5: the airframe home: a surge above garrisonMax, nothing dropped -----
suT3 = time
suLive = true
triSimUntil { call suSample; suLive = false; {if ((_x select 0) == suSId) then {suLive = true}} forEach gmOccupierOps; (not suLive) or (time > suT3 + 400) }
triAssertEq [(format ["%1", suLive]), "false"]
suT4 = time
triSimUntil { call suSample; time > suT4 + 3 }
triAssertLe [suOver, 0]
triAssertEq [((gmOccupierLedger) select 5), suDropped0]
suAfter = 0
{suAfter = suAfter + (_x call suHeld)} forEach suMil
triAssertEq [(suAfter - suBefore), suPrice]
suAbove = 0
{if ((_x call suHeld) > ((gmZoneSim _x) select 7)) then {suAbove = suAbove + 1}} forEach suMil
triAssertGe [suAbove, 1]
suSurge = 0
{suSurge = suSurge + (_x select 1)} forEach ((gmOccupierLedger) select 7)
triAssertEq [suSurge, suPrice]
suStood0 = (gmOccupierLedger) select 6

// ---- s6: the decay: back to garrisonMax, the surge stood down ---------------
triAssertEq [(format ["%1", gmForcesTune ["surgeDecayHours", 0.01]]), "true"]
suT5 = time
triSimUntil { call suSample; ((count ((gmOccupierLedger) select 7)) == 0) or (time > suT5 + 300) }
triAssertEq [(count ((gmOccupierLedger) select 7)), 0]
suT6 = time
triSimUntil { call suSample; time > suT6 + 2 }
triAssertLe [suOver, 0]
suBack = true
{if ((_x call suHeld) > ((gmZoneSim _x) select 7)) then {suBack = false}} forEach suMil
triAssertEq [(format ["%1", suBack]), "true"]
triAssertEq [((gmOccupierLedger) select 5), suDropped0]
// the pool sat at its ceiling: the whole surge stood down
triAssertEq [(((gmOccupierLedger) select 6) - suStood0), suPrice]
gmSimDiagLog

triEndTest
