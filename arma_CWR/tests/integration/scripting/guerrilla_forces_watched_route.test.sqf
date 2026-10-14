// ============================================================================
//  Occupier forces, the WHOLE ROUTE WATCHED (OCCUPIER-FORCES.md 3.1 and D6:
//  "If the whole route is in view, start beyond the base ... the next base
//  back"; native CounterAttack birth chain).  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    w1  a real camera at the default view distance in daylight (the 750 m
//        day floor); Le Port (a seeded town 600 m from the Le Port Harbour)
//        is held by the resistance and empty; with the player at the Camp
//        both the Harbour and the Southeast Compound keep stored reserves;
//    w2  a funded foot operation from the Harbour on Le Port (the test lever
//        gmCounterAttackLaunchFunded) leaves abstractly; the player then
//        stands on its road 200 m from the Harbour, facing it: every point
//        of the Harbour road behind the column is in his view or inside the
//        audible floor, so the force cannot be born there;
//    w3  after counterAttackBirthGiveUp (tuned to 20 s) the operation is
//        re-sourced to the next base back, the Southeast Compound
//        (gmOccupierOps shows its source change), the Harbour is refunded;
//    w4  it is born at the Compound, 1.7 km away and out of sight, and the
//        player watches it set off.  The Compound's roads are not joined to
//        the island's net in Abel's data, so the road route leaves them by
//        the west gate track and crosses country to the town's road
//        (RoadRoute.hpp stage 2).  The arrival is not asserted: the squad
//        stalls just inside the west gate (parked, see OCCUPIER-FORCES.md
//        phase 3 known gaps).
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
wrT0 = time
triSimUntil { time > wrT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- w1: a real camera, the day floor; Le Port held and empty ----------------
wrB = gmTrafficPercept (getPos player)
triAssertEq [(wrB select 0), 1]
triAssertGe [(wrB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["birthGiveUp", 20]]), "true"]
wrL = gmZoneIndex "Le Port"
wrH = gmZoneIndex "Le Port Harbour"
wrC = gmZoneIndex "Southeast Compound"
triAssertGe [wrL, 0]
triAssertGe [wrH, 0]
triAssertGe [wrC, 0]
wrLPos = (gmZone wrL) select 8
wrHPos = (gmZone wrH) select 8
wrCPos = (gmZone wrC) select 8
// the Compound's roads are a pocket of their own: the road route leaves
// them and joins the town's net (found, joined)
wrRr = gmRoadRoute [wrCPos, wrLPos]
triAssertEq [(format ["%1 %2", wrRr select 0, wrRr select 9]), "true true"]
triAssertEq [(format ["%1", (gmRoadRoute [wrCPos, wrLPos, 0]) select 0]), "false"]
// far from the player: no live garrison anywhere down there
triAssertEq [(format ["%1", gmGarrisonSpawned wrL]), "false"]
triAssertEq [(format ["%1", gmGarrisonSpawned wrH]), "false"]
triAssertEq [(format ["%1", gmGarrisonSpawned wrC]), "false"]
gmZoneSet [wrL, "garrison", 0]
gmZoneSet [wrL, "owner", gmResistanceSide]
gmZoneSet [wrL, "support", 70]
triSimUntil { ((gmZone wrL) select 2) == gmResistanceSide }
wrH0 = (gmZone wrH) select 3
wrC0 = (gmZone wrC) select 3
triAssertGe [wrH0, 6]
triAssertGe [wrC0, 5]

// ---- w2: an abstract launch from the Harbour; the player watches its road ----
wrOp = gmCounterAttackLaunchFunded ["Le Port", "Le Port Harbour", "foot", 4]
triAssertGe [wrOp, 1]
wrNone = [0, "", "", "", "", "", "NONE", 0, 0, 0, ""]
wrRow = gmOccupierOps select 0
triAssertEq [(wrRow select 0), wrOp]
triAssertEq [(wrRow select 4), "Le Port Harbour"]
triAssertEq [(wrRow select 6), "TRAVEL"]
triAssertEq [((gmZone wrH) select 3), wrH0 - 4]
// 200 m from the Harbour toward Le Port, facing the Harbour
wrDx = (wrLPos select 0) - (wrHPos select 0)
wrDy = (wrLPos select 1) - (wrHPos select 1)
wrLen = sqrt (wrDx * wrDx + wrDy * wrDy)
wrVant = [(wrHPos select 0) + wrDx * 200 / wrLen, (wrHPos select 1) + wrDy * 200 / wrLen, 0]
player setPos wrVant
wrLook = ((wrHPos select 0) - (wrVant select 0)) atan2 ((wrHPos select 1) - (wrVant select 1))
player setDir wrLook
triAssertGe [800, (player distance wrLPos)]

// ---- w3: nothing behind the column is unseen: the next base back -------------
triSimUntil { player setDir wrLook; (((gmOccupierOps + [wrNone]) select 0) select 4) == "Southeast Compound" }
wrRow = gmOccupierOps select 0
triAssertEq [(wrRow select 0), wrOp]
triAssertEq [(wrRow select 7), 4]
// the Harbour got its four back (a pending deposit: the player stands at it)
triSimUntil { (((gmZone wrH) select 3) + ((gmZoneSim wrH) select 13) + (gmGarrisonLive wrH)) >= wrH0 }
// the Compound paid them
triAssertEq [((gmZone wrC) select 3), wrC0 - 4]

// ---- w4: born at the Compound out of sight; it comes on ------------------------
triSimUntil { player setDir wrLook; (((gmOccupierOps + [wrNone]) select 0) select 6) == "LIVE" }
wrRow = gmOccupierOps select 0
triAssertEq [(wrRow select 10), "base"]
wrCa = gmCounterAttacks select 0
wrSpawn = wrCa select 8
triAssertGe [300, (wrSpawn distance wrCPos)]
triAssertGe [(wrSpawn distance player), 750]
triAssertEq [(wrCa select 21), 4]
wrLead = wrCa select 20
wrT1 = time
// it sets off: the lead leaves its birth point (its road runs north-west to
// the west gate first, so the distance to Le Port barely changes yet)
triSimUntil { ((wrLead distance wrSpawn) > 20) or (time > wrT1 + 180) }
triAssertGe [(wrLead distance wrSpawn), 20]
triAssertEq [((gmZone wrL) select 2), gmResistanceSide]
gmSimDiagLog

triEndTest
