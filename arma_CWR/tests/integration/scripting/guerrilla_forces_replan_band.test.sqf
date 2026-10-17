// ============================================================================
//  Occupier forces, the AIR RE-PLAN IS ONE LEDGER MOVEMENT (OCCUPIER-FORCES.md
//  3.1 step 4, 5.5, 5.6; the director run of 2026-10-14, op 4, where a column
//  re-planned as a Mi17 insertion logged STRONG -> STRETCHED -> STRONG in one
//  tick).  Boots the SHIPPED Guerrilla.Abel template (occupierForces = 1) on
//  the full Remaster:
//    r1  a real camera at the default view distance in daylight; the director
//        stays DORMANT (wakeTowns 99) and the refill is slowed to 48 h, so
//        only the re-plan moves the pool; Le Port (600 m from the Le Port
//        Harbour) is held by the resistance and empty; the Outpost and the
//        Airfield keep a reserve of 2, too few for a 4-man force (written
//        while the player waits out west, so no live garrison overwrites
//        them);
//    r2  a funded TRUCK operation from the Harbour on Le Port (the test lever
//        gmCounterAttackLaunchFunded: truck 3 + its driver on the ledger row)
//        leaves abstractly; the player, captive, stands on its road 200 m
//        from the Harbour facing it, so the column cannot be born there;
//    r3  the pool is set where the airframe's debit alone (Mi17 from the
//        Airfield: 10 + 2 crew) would take the band below the 60 % STRONG
//        floor and the truck's refund alone could not bring it back (66 %),
//        while the net movement (-8) lands exactly on the floor;
//    r4  after counterAttackBirthGiveUp (20 s) no base can spare the force
//        and the operation is re-planned as a helicopter insertion: across
//        that tick the band never leaves STRONG (sampled every frame) and the
//        pool moves by exactly the price difference.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded and captive.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
rbT0 = time
triSimUntil { time > rbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true

// ---- r1: a real camera; the director parked; Le Port held and empty ----------
rbB = gmTrafficPercept (getPos player)
triAssertEq [(rbB select 0), 1]
triAssertGe [(rbB select 3), 750]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["refillHours", 48]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["birthGiveUp", 20]]), "true"]
rbF = gmOccupierForces
triAssertEq [(rbF select 2), "STRONG"]
triAssertEq [(rbF select 4), "DORMANT"]
rbStart = rbF select 1
triAssertGe [rbStart, 20]
rbL = gmZoneIndex "Le Port"
rbH = gmZoneIndex "Le Port Harbour"
rbO = gmZoneIndex "Outpost"
rbA = gmZoneIndex "Airfield"
triAssertGe [rbL, 0]
triAssertGe [rbH, 0]
triAssertGe [rbO, 0]
triAssertGe [rbA, 0]
triAssertEq [((gmZone rbA) select 2), gmOccupierSide]
rbLPos = (gmZone rbL) select 8
rbHPos = (gmZone rbH) select 8
triAssertEq [(format ["%1", gmGarrisonSpawned rbL]), "false"]
triAssertEq [(format ["%1", gmGarrisonSpawned rbH]), "false"]
triAssertEq [(format ["%1", gmGarrisonSpawned rbA]), "false"]
gmZoneSet [rbL, "garrison", 0]
gmZoneSet [rbL, "owner", gmResistanceSide]
gmZoneSet [rbL, "support", 70]
triSimUntil { ((gmZone rbL) select 2) == gmResistanceSide }
// the reserves are written while their garrisons are stored (a live
// garrison's census would overwrite them): the player waits out west, far
// from the Outpost, the Harbour and the Airfield
player setPos [4500, 6000, 0]
triSimUntil { (not (gmGarrisonSpawned rbO)) and (not (gmGarrisonSpawned rbH)) and (not (gmGarrisonSpawned rbA)) }
gmZoneSet [rbO, "garrison", 2]
gmZoneSet [rbA, "garrison", 2]
// the generated bases (811c5c3) would otherwise re-source the truck by road
{rbGI = gmZoneIndex (_x select 0); if (rbGI >= 0) then {gmZoneSet [rbGI, "garrison", 2]}} forEach gmGeneratedBases
rbT2 = time
triSimUntil { time > rbT2 + 2 }
triAssertEq [((gmZone rbO) select 3), 2]
triAssertEq [((gmZone rbA) select 3), 2]
triAssertGe [((gmZone rbH) select 3), 6]

// ---- r2: an abstract truck launch from the Harbour; the player watches it ----
rbOp = gmCounterAttackLaunchFunded ["Le Port", "Le Port Harbour", "truck", 4]
triAssertGe [rbOp, 1]
rbNone = [0, "", "", "", "", "", "NONE", 0, 0, 0, ""]
rbRow = gmOccupierOps select 0
triAssertEq [(rbRow select 0), rbOp]
triAssertEq [(rbRow select 3), "TRUCK"]
triAssertEq [(rbRow select 4), "Le Port Harbour"]
triAssertEq [(rbRow select 6), "TRAVEL"]
rbDx = (rbLPos select 0) - (rbHPos select 0)
rbDy = (rbLPos select 1) - (rbHPos select 1)
rbLen = sqrt (rbDx * rbDx + rbDy * rbDy)
rbVant = [(rbHPos select 0) + rbDx * 200 / rbLen, (rbHPos select 1) + rbDy * 200 / rbLen, 0]
player setPos rbVant
rbLook = ((rbHPos select 0) - (rbVant select 0)) atan2 ((rbHPos select 1) - (rbVant select 1))
player setDir rbLook

// ---- r3: the pool where a split debit and credit would flip the band ---------
// the truck's 4 come back, the Mi17's 12 (Airfield: hull 10 + 2 crew) go out
rbTruck = 4
rbHeli = 12
rbNet = rbHeli - rbTruck
// the lowest pool still STRONG (not below 60 % of poolStart)
rbFloor = 0
while {(rbFloor * 100) < (rbStart * 60)} do {rbFloor = rbFloor + 1}
rbPool = rbFloor + rbNet
triAssertEq [(format ["%1", gmForcesForce ["pool", rbPool]]), "true"]
triAssertEq [((gmOccupierForces) select 0), rbPool]
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
// split, the debit alone goes below 60 %, the credit alone stays at or below 66 %
triAssertGe [(rbStart * 60), ((rbPool - rbHeli) * 100) + 1]
triAssertGe [(rbStart * 66), ((rbPool - rbNet) * 100)]

// ---- r4: re-planned as an insertion; every frame sampled ---------------------
rbCur = [(gmOccupierForces select 0), (gmOccupierForces select 2)]
rbPrev = rbCur
rbEdges = 0
rbT3 = time
triSimUntil { player setDir rbLook; rbPrev = rbCur; rbF = gmOccupierForces; rbCur = [(rbF select 0), (rbF select 2)]; if ((rbCur select 1) != (rbPrev select 1)) then {rbEdges = rbEdges + 1}; rbRow = (gmOccupierOps + [rbNone]) select 0; ((rbRow select 0) != rbOp) or ((rbRow select 3) == "TRANSPORT_HELI") or (time > rbT3 + 600) }
triAssertEq [(rbRow select 0), rbOp]
triAssertEq [(rbRow select 3), "TRANSPORT_HELI"]
triAssertEq [(rbRow select 11), "Mi17"]
// the band never moved, not even for the re-plan's own frame
triAssertEq [rbEdges, 0]
triAssertEq [(rbPrev select 1), "STRONG"]
triAssertEq [(rbCur select 1), "STRONG"]
// one net ledger movement: the price difference, nothing else
triAssertEq [(rbCur select 0) - (rbPrev select 0), 0 - rbNet]
triAssertEq [(rbCur select 0), rbFloor]
// a moment later the band still reads STRONG (no deferred edge)
rbT1 = time
triSimUntil { player setDir rbLook; time > rbT1 + 2 }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
gmSimDiagLog

triEndTest
