// ============================================================================
//  Occupier forces, a MOTORIZED operation (OCCUPIER-FORCES.md 3.2 and 14
//  item 5; native CounterAttack hull path).  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    m1  a real camera at the default view distance in daylight (the 750 m
//        day floor of the perception gate); Houdan (the "Village" CITY zone)
//        is held by the resistance and empty, the player stands on its
//        north-west edge; the Le Port Harbour (2.7 km) keeps its stored
//        reserve;
//    m2  an URGENT truck operation from the Harbour (the test lever
//        gmCounterAttackLaunchFunded, a director-style funded order): with
//        the town in the bubble it is born at once where the player cannot
//        see it, mounted: gmOccupierOps reports one live hull, a TRUCK
//        package, and the squad's leader rides in it;
//    m3  the truck drives the road, stops at the 250 m bound
//        (counterAttackBirthMinToTarget) and the infantry gets out, every man
//        alive, and moves in on foot while the truck holds back (its hull
//        still live).  gmCounterAttacks [19] (riders), [20] (leader) and
//        [21] (living infantry) are the engine's own counts.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded; the legacy flip answer is off (the town is set held, so
//  no flip event fires anyway).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
mzT0 = time
triSimUntil { time > mzT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- m1: a real camera, the day floor; Houdan held and empty -----------------
mzB = gmTrafficPercept (getPos player)
triAssertEq [(mzB select 0), 1]
triAssertGe [(mzB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
mzV = gmZoneIndex "Village"
mzH = gmZoneIndex "Le Port Harbour"
triAssertGe [mzV, 0]
triAssertGe [mzH, 0]
mzPos = (gmZone mzV) select 8
// the town garrison is live (the player starts at the Camp): kill it, then
// hand the town to the resistance
triSimUntil { gmGarrisonSpawned mzV }
triSimUntil { (gmGarrisonLive mzV) >= 4 }
mzUnits = []
"mzUnits = mzUnits + (units _x)" forEach (gmGarrisonGroups mzV)
{_x setDamage 1} forEach mzUnits
triSimUntil { (gmGarrisonLive mzV) == 0 }
gmZoneSet [mzV, "garrison", 0]
gmZoneSet [mzV, "owner", gmResistanceSide]
gmZoneSet [mzV, "support", 70]
triSimUntil { ((gmZone mzV) select 2) == gmResistanceSide }
// the Harbour is far away: its garrison is a stored reserve
triAssertEq [(format ["%1", gmGarrisonSpawned mzH]), "false"]
triAssertGe [((gmZone mzH) select 3), 6]
player setPos [(mzPos select 0) - 120, (mzPos select 1) + 120, 0]
mzT1 = time
triSimUntil { time > mzT1 + 2 }
triAssertEq [((gmTrafficPercept (getPos player)) select 0), 1]

// ---- m2: an urgent truck operation, born mounted out of sight ----------------
mzOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "truck", 6, true]
triAssertGe [mzOp, 1]
triAssertEq [(count gmOccupierOps), 1]
mzNone = [0, "", "", "", "", "", "NONE", 0, 0, 0, ""]
triSimUntil { (((gmOccupierOps + [mzNone]) select 0) select 6) == "LIVE" }
mzRow = gmOccupierOps select 0
triAssertEq [(mzRow select 0), mzOp]
triAssertEq [(mzRow select 3), "TRUCK"]
triAssertEq [(mzRow select 4), "Le Port Harbour"]
triAssertEq [(mzRow select 8), 1]
mzCa = gmCounterAttacks select 0
triAssertEq [(mzCa select 10), mzOp]
// born far from the player (beyond the audible floor, out of sight)
mzSpawn = mzCa select 8
triAssertGe [(mzSpawn distance player), 750]
// the squad rides: every living soldier sits in the hull
mzHull = mzCa select 18
triAssertEq [(format ["%1", isNull mzHull]), "false"]
mzN = mzCa select 21
triAssertGe [mzN, 1]
triAssertEq [(mzCa select 19), mzN]
gmSimDiagLog

// ---- m3: the truck drives in; the infantry dismounts at the 250 m bound ------
mzD0 = mzHull distance mzPos
mzT2 = time
triSimUntil { ((mzHull distance mzPos) < (mzD0 - 200)) or (time > mzT2 + 200) }
triAssertGe [(mzD0 - 200), (mzHull distance mzPos)]
mzNone2 = [[], [], "NONE"]
triSimUntil { ((((gmCounterAttacks + [mzNone2]) select 0) select 2) != "LIVE") or ((((gmCounterAttacks + [mzNone2]) select 0) select 19) == 0) }
mzCa = gmCounterAttacks select 0
triAssertEq [(mzCa select 2), "LIVE"]
triAssertEq [(mzCa select 19), 0]
// every rider got out alive (the hull stopped first), near the bound, and
// the hull held back where it stopped
triAssertEq [(mzCa select 21), mzN]
mzLead = mzCa select 20
triAssertGe [360, (mzLead distance mzPos)]
triAssertGe [(mzLead distance mzPos), 120]
triAssertEq [(((gmOccupierOps + [mzNone]) select 0) select 8), 1]
triAssertEq [(format ["%1", alive mzHull]), "true"]
// the squad moves in on foot
mzT3 = time
mzD1 = mzLead distance mzPos
triSimUntil { ((mzLead distance mzPos) < (mzD1 - 40)) or (time > mzT3 + 120) }
triAssertGe [(mzD1 - 40), (mzLead distance mzPos)]
triAssertGe [(mzHull distance mzPos), 150]

triEndTest
