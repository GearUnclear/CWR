// ============================================================================
//  Occupier forces, an ARMOURED COLUMN (OCCUPIER-FORCES.md 6.5 "Armour
//  packages (phase 5)": a carrier plus at most one escort hull).  Boots the
//  SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    m1  a real camera at the default view distance in daylight (the 750 m
//        day floor of the perception gate); Houdan (the "Village" CITY zone)
//        is held by the resistance and empty, the player stands on its
//        north-west edge; the Le Port Harbour (2.7 km) keeps its stored
//        reserve;
//    m2  an URGENT column from the Harbour through the test lever
//        gmCounterAttackLaunchFunded: a Ural truck carries the infantry, a
//        T72 escorts it (a tank is only ever bought as an escort).  Both
//        hulls are born at once out of the player's sight, the escort on the
//        road behind the carrier (gmOccupierOps: two live hulls, carrier and
//        escort classes in [11] and [12]);
//    m3  the player walks far away: the whole column returns to abstract
//        travel, the hulls and their crews deleted (crew first, unseen);
//        when he is back in the town the column is born again with both
//        hulls;
//    m4  both hulls drive the road; at the 250 m bound
//        (counterAttackBirthMinToTarget) the infantry gets out alive, the
//        escort closes up to the town edge and supports while the truck holds
//        back where it stopped.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded; the legacy flip answer is off.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
acT0 = time
triSimUntil { time > acT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- m1: a real camera, the day floor; Houdan held and empty -----------------
acB = gmTrafficPercept (getPos player)
triAssertEq [(acB select 0), 1]
triAssertGe [(acB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
acV = gmZoneIndex "Village"
acH = gmZoneIndex "Le Port Harbour"
triAssertGe [acV, 0]
triAssertGe [acH, 0]
acPos = (gmZone acV) select 8
acHPos = (gmZone acH) select 8
triSimUntil { gmGarrisonSpawned acV }
triSimUntil { (gmGarrisonLive acV) >= 4 }
acUnits = []
"acUnits = acUnits + (units _x)" forEach (gmGarrisonGroups acV)
{_x setDamage 1} forEach acUnits
triSimUntil { (gmGarrisonLive acV) == 0 }
gmZoneSet [acV, "garrison", 0]
gmZoneSet [acV, "owner", gmResistanceSide]
gmZoneSet [acV, "support", 70]
triSimUntil { ((gmZone acV) select 2) == gmResistanceSide }
triAssertEq [(format ["%1", gmGarrisonSpawned acH]), "false"]
triAssertGe [((gmZone acH) select 3), 6]
acHere = [(acPos select 0) - 120, (acPos select 1) + 120, 0]
player setPos acHere
acT1 = time
triSimUntil { time > acT1 + 2 }
triAssertEq [((gmTrafficPercept (getPos player)) select 0), 1]

// ---- m2: a truck carrier with a tank escort, born out of sight ---------------
acOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "truck", 6, true, "Ural", "RETAKE", "T72"]
triAssertGe [acOp, 1]
triAssertEq [(count gmOccupierOps), 1]
acNone = [0, "", "", "", "", "", "NONE", 0, 0, 0, "", "", ""]
triSimUntil { (((gmOccupierOps + [acNone]) select 0) select 6) == "LIVE" }
acRow = gmOccupierOps select 0
triAssertEq [(acRow select 0), acOp]
triAssertEq [(acRow select 3), "TRUCK"]
triAssertEq [(acRow select 8), 2]
triAssertEq [(acRow select 11), "Ural"]
triAssertEq [(acRow select 12), "T72"]
acCa = gmCounterAttacks select 0
triAssertEq [(acCa select 22), "T72"]
acHull = acCa select 18
acEsc = acCa select 23
triAssertEq [(format ["%1", isNull acHull]), "false"]
triAssertEq [(format ["%1", isNull acEsc]), "false"]
// born far from the player, the escort just behind the carrier (farther
// from the town on the same road)
triAssertGe [((acCa select 8) distance player), 750]
triAssertGe [(acEsc distance player), 750]
triAssertGe [60, (acEsc distance acHull)]
triAssertGe [(acEsc distance acPos), (acHull distance acPos)]
gmSimDiagLog

// ---- m3: out of the player's world and back -----------------------------------
// the player walks 1.6 km off to the side of the town: the column returns to
// abstract travel, both hulls and both crews deleted where nobody sees them
acDx = (acPos select 0) - (acHPos select 0)
acDy = (acPos select 1) - (acHPos select 1)
acDl = sqrt ((acDx * acDx) + (acDy * acDy))
acAway = [(acPos select 0) - ((acDy / acDl) * 1600), (acPos select 1) + ((acDx / acDl) * 1600), 0]
acCrew = (crew acHull) + (crew acEsc)
triAssertGe [(count acCrew), 2]
player setPos acAway
acT2 = time
triSimUntil { ((((gmOccupierOps + [acNone]) select 0) select 6) == "TRAVEL") or (time > acT2 + 120) }
triAssertEq [(((gmOccupierOps + [acNone]) select 0) select 6), "TRAVEL"]
triAssertEq [(((gmOccupierOps + [acNone]) select 0) select 8), 0]
triSimUntil { isNull acHull }
triSimUntil { isNull acEsc }
// (a deleted crewman leaves the world on the following frames)
acT5 = time
acLeft = 99
triSimUntil { acLeft = 0; "if (!(isNull _x)) then {acLeft = acLeft + 1}" forEach acCrew; (acLeft == 0) or (time > acT5 + 30) }
triAssertEq [acLeft, 0]
// back in the town: born again with both hulls
player setPos acHere
triSimUntil { (((gmOccupierOps + [acNone]) select 0) select 6) == "LIVE" }
triAssertEq [(((gmOccupierOps + [acNone]) select 0) select 8), 2]
acCa = gmCounterAttacks select 0
acHull = acCa select 18
acEsc = acCa select 23
triAssertEq [(format ["%1", isNull acHull]), "false"]
triAssertEq [(format ["%1", isNull acEsc]), "false"]
triAssertGe [(acEsc distance player), 750]

// ---- m4: both drive; the infantry dismounts at the bound; the escort supports -
// the player stands inside the town (a uniformed player holds it, D23), on
// the side away from the Harbour, so the town is still his when the column
// arrives and the fight stays live
player setPos [(acPos select 0) + ((acDx / acDl) * 60), (acPos select 1) + ((acDy / acDl) * 60), 0]
acD0 = acHull distance acPos
acE0 = acEsc distance acPos
acT3 = time
triSimUntil { ((acHull distance acPos) < (acD0 - 200)) or (time > acT3 + 240) }
triAssertGe [(acD0 - 200), (acHull distance acPos)]
triAssertGe [(acE0 - 200), (acEsc distance acPos)]
acNone2 = [[], [], "NONE"]
triSimUntil { ((((gmCounterAttacks + [acNone2]) select 0) select 2) != "LIVE") or ((((gmCounterAttacks + [acNone2]) select 0) select 19) == 0) }
acCa = gmCounterAttacks select 0
triAssertEq [(acCa select 2), "LIVE"]
triAssertEq [(acCa select 19), 0]
acLead = acCa select 20
triAssertGe [360, (acLead distance acPos)]
triAssertGe [(acLead distance acPos), 120]
// the escort closes up to the town edge (zoneArea 150) and stays crewed;
// the truck holds back where it stopped
acT4 = time
triSimUntil { ((acEsc distance acPos) < 230) or (time > acT4 + 240) }
triAssertGe [230, (acEsc distance acPos)]
triAssertEq [(format ["%1", alive acEsc]), "true"]
triAssertGe [(count crew acEsc), 1]
triAssertGe [(acHull distance acPos), 180]
triAssertEq [(format ["%1", alive acHull]), "true"]
triAssertEq [(((gmOccupierOps + [acNone]) select 0) select 8), 2]

triEndTest
