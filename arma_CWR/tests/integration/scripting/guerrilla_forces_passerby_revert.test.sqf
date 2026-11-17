// ============================================================================
//  Occupier forces, a PASSER-BY revert (OCCUPIER-FORCES.md 5.4 and 5.5, D5:
//  any enemy standing in an empty town of the player's drains it; native
//  ZoneRegistry::RevertTown through the pool).  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    p1  a real camera at the default view distance in daylight; Houdan (the
//        "Village" CITY zone) has lost its garrison and is held by the
//        resistance, empty, its support just above the revert line; the
//        player stands 200 m out (inside the town's deposit bubble, outside
//        its 150 m presence circle);
//    p2  an occupier patrol (a spawned squad on hold fire) stops in the
//        street: the drain reverts the town; its refill is a PENDING deposit
//        (gmZoneSim slot 13; nothing appears beside the player), paid from
//        the occupier pool (gmOccupierForces slot 0 drops by it); the patrol
//        is not the town's garrison;
//    p3  the patrol walks on and leaves; the town stays administered.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
pbT0 = time
triSimUntil { time > pbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- p1: a real camera; Houdan held, empty, support 35 ------------------------
pbB = gmTrafficPercept (getPos player)
triAssertEq [(pbB select 0), 1]
triAssertGe [(pbB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
pbOcc = gmOccupierSide call GM_fnSideFromString
pbV = gmZoneIndex "Village"
triAssertGe [pbV, 0]
pbPos = (gmZone pbV) select 8
triSimUntil { gmGarrisonSpawned pbV }
triSimUntil { (gmGarrisonLive pbV) >= 4 }
pbUnits = []
"pbUnits = pbUnits + (units _x)" forEach (gmGarrisonGroups pbV)
{_x setDamage 1} forEach pbUnits
triSimUntil { (gmGarrisonLive pbV) == 0 }
gmZoneSet [pbV, "garrison", 0]
gmZoneSet [pbV, "owner", gmResistanceSide]
gmZoneSet [pbV, "support", 35]
triSimUntil { not (gmGarrisonSpawned pbV) }
player setPos [(pbPos select 0) + 200, pbPos select 1, 0]
pbT1 = time
triSimUntil { time > pbT1 + 4 }
triAssertEq [((gmZone pbV) select 2), gmResistanceSide]
triAssertEq [((gmZoneSim pbV) select 13), 0]
pbPool = (gmOccupierForces) select 0
triAssertGe [pbPool, 5]

// ---- p2: a patrol stops in the street; the drain reverts the town --------------
pbSq = [pbOcc, gmFactionSquad [gmOccupierSide, 1, 2], pbPos] call GM_fnSpawnSquad
pbSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units pbSq)
doStop (units pbSq)
triSimUntil { ((gmZone pbV) select 2) == "NEUTRAL" }
// the refill is pending: it neither defends nor materializes beside him
triAssertEq [((gmZoneSim pbV) select 13), 4]
triAssertEq [((gmZone pbV) select 3), 0]
triAssertEq [(format ["%1", gmGarrisonSpawned pbV]), "false"]
// ... and the pool paid it (one refill unit of slack: the director tick may
// have accrued one meanwhile)
triAssertGe [pbPool - 3, (gmOccupierForces) select 0]
// the patrol keeps its job: it is not the town's garrison
triAssertEq [(format ["%1", pbSq in (gmGarrisonGroups pbV)]), "false"]

// ---- p3: the patrol walks on ----------------------------------------------------
(units pbSq) doMove [(pbPos select 0) - 500, (pbPos select 1) + 300, 0]
pbT2 = time
triSimUntil { (((leader pbSq) distance pbPos) > 250) or (time > pbT2 + 100) }
triAssertGe [((leader pbSq) distance pbPos), 250]
triAssertEq [((gmZone pbV) select 2), "NEUTRAL"]
triAssertEq [(format ["%1", gmGarrisonSpawned pbV]), "false"]
triAssertEq [((gmZoneSim pbV) select 13), 4]

triEndTest
