// ============================================================================
//  Occupier forces, a SORTIE DOES NOT DEMOTE THE BAND (OCCUPIER-FORCES.md 5.6
//  "The band's input"; owner decision 6 of 2026-10-16).  Video 5: two Mi17s
//  took the pool from 52 to 31 and the band from STRONG to STRETCHED for
//  100 s while they were out.  Now an airframe out is committed, not spent:
//  the band is stepped on the pool plus the airframes the ledger rows still
//  hold.  Boots the SHIPPED Guerrilla.Abel template (occupierForces = 1) on
//  the full Remaster:
//    q1  a real camera; the pool STRONG; the director parked DORMANT
//        (wakeTowns 99); the refill slow (48 h) on a 1 s director tick;
//    q2  the pool forced to the lowest STRONG value (just over 66 %); the
//        player out west and every occupier base pinned at its garrisonMax,
//        so no D2 regrowth draws on the pool;
//    q3  a test-lever strike flies on a held town far from the player: its
//        price leaves the pool, which alone would read below 60 %
//        (STRETCHED or THIN by the old arithmetic), but gmOccupierLedger
//        reports it committed and the band stays STRONG, sampled every
//        frame, for the whole sortie;
//    q4  the airframe home: nothing committed any more, the band still
//        STRONG and never left it (the pool itself is not pinned: D2
//        regrowth at the unpinned bases draws on it meanwhile).
//  The shot-down case (a real loss that demotes) is pinned in the unit cases.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
sbT0 = time
triSimUntil { time > sbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// the sampler: every band seen, the largest committed, the lowest pool
sbBands = []
sbCommitted = 0
sbLow = 999999
sbSample = {sbF = gmOccupierForces; sbL = gmOccupierLedger; if (not ((sbF select 2) in sbBands)) then {sbBands = sbBands + [sbF select 2]}; if ((sbL select 1) > sbCommitted) then {sbCommitted = sbL select 1}; if ((sbF select 0) < sbLow) then {sbLow = sbF select 0}}

// ---- q1: a real camera; the pool STRONG; the levers -------------------------
sbP = gmTrafficPercept (getPos player)
triAssertEq [(sbP select 0), 1]
triAssertGe [(sbP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["refillHours", 48]]), "true"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
sbStart = (gmOccupierForces) select 1
triAssertGe [sbStart, 20]

// ---- q2: the pool at the lowest STRONG value -------------------------------
// STRONG holds while pool x 100 >= 60 x poolStart; forced from STRONG, the
// smallest pool over 66 % keeps it
sbPool = sbStart
while {((sbPool - 1) * 100) > (66 * sbStart)} do {sbPool = sbPool - 1}
triAssertEq [(format ["%1", gmForcesForce ["pool", sbPool]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
// the player out west; every occupier base stored and pinned at its
// garrisonMax, so no D2 regrowth draws on the pool during the sortie
player setPos [4500, 6000, 0]
sbMil = []
sbI = 0
while {sbI < gmZoneCount} do {sbZ = gmZone sbI; if (((sbZ select 1) != "CITY") and ((sbZ select 1) != "CAMP") and ((sbZ select 2) == gmOccupierSide)) then {sbMil = sbMil + [sbI]}; sbI = sbI + 1}
triSimUntil { sbAny = false; {if (gmGarrisonSpawned _x) then {sbAny = true}} forEach sbMil; not sbAny }
{gmZoneSet [_x, "garrison", ((gmZoneSim _x) select 7)]} forEach sbMil
triAssertEq [(format ["%1", gmForcesForce ["pool", sbPool]]), "true"]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]

// ---- q3: a strike out; the band stays STRONG -------------------------------
sbPl = getPos player
sbA = -1
sbI = 0
while {sbI < gmZoneCount} do {sbZi = gmZone sbI; if ((sbA < 0) and ((sbZi select 1) == "CITY") and (((sbZi select 8) distance sbPl) > 2000)) then {sbA = sbI}; sbI = sbI + 1}
triAssertGe [sbA, 0]
sbAName = (gmZone sbA) select 0
gmZoneSet [sbA, "garrison", 0]
gmZoneSet [sbA, "owner", gmResistanceSide]
triAssertEq [(format ["%1", gmForcesForce ["strike", sbA]]), "true"]
sbSOp = []
{if (((_x select 1) == "STRIKE") and ((_x select 5) == sbAName)) then {sbSOp = _x}} forEach gmOccupierOps
triAssertGe [(count sbSOp), 4]
sbSId = sbSOp select 0
sbPrice = sbPool - ((gmOccupierForces) select 0)
triAssertGe [sbPrice, 2]
// the pool alone reads under STRONG's exit: the old band would have demoted
triAssertLe [((((gmOccupierForces) select 0) * 100) - (60 * sbStart)), -1]
triAssertEq [((gmOccupierLedger) select 1), sbPrice]
triAssertGe [((gmOccupierLedger) select 0), sbPool]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
sbT1 = time
sbLive = true
triSimUntil { call sbSample; sbLive = false; {if ((_x select 0) == sbSId) then {sbLive = true}} forEach gmOccupierOps; (not sbLive) or (time > sbT1 + 400) }
triAssertEq [(format ["%1", sbLive]), "false"]

// ---- q4: home; nothing committed; STRONG all along -------------------------
sbT2 = time
triSimUntil { call sbSample; time > sbT2 + 3 }
triAssertEq [((gmOccupierLedger) select 1), 0]
triAssertGe [sbCommitted, sbPrice - 1]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [(count sbBands), 1]
triAssertEq [(sbBands select 0), "STRONG"]
gmSimDiagLog

triEndTest
