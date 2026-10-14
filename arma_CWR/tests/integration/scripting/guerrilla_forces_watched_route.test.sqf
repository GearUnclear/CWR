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
//        re-sourced to the next base back.  The nearest, the Southeast
//        Compound (1.7 km), stands on an island of its own, 800 m of sea
//        east of Le Port (gmLandmass; its roads are a pocket for that
//        reason and no road route leaves it), so a foot force from it could
//        never arrive: it is passed over (its reserve untouched) for the next
//        base on Le Port's landmass (the Airfield on Abel's roster), the
//        Harbour is refunded;
//    w4  it is born at that base, out of sight, and marches: within 10
//        simulated minutes the lead is 300 m from its birth point and 300 m
//        nearer Le Port (the whole 8 km road is not waited out).
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
// the Compound stands on another landmass than Le Port and the Harbour: no
// road route, not even the cross-country join (it would swim the strait)
wrLandC = gmLandmass wrCPos
wrLandL = gmLandmass wrLPos
triAssertGe [wrLandC, 0]
triAssertGe [wrLandL, 0]
triAssertEq [(format ["%1", wrLandC == wrLandL]), "false"]
triAssertEq [(gmLandmass wrHPos), wrLandL]
triAssertEq [(format ["%1", (gmRoadRoute [wrCPos, wrLPos]) select 0]), "false"]
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
triSimUntil { player setDir wrLook; (((gmOccupierOps + [wrNone]) select 0) select 4) != "Le Port Harbour" }
wrRow = gmOccupierOps select 0
triAssertEq [(wrRow select 0), wrOp]
triAssertEq [(wrRow select 7), 4]
// passed over: the new source shares Le Port's landmass
wrS = gmZoneIndex (wrRow select 4)
triAssertGe [wrS, 0]
triAssertEq [(gmLandmass ((gmZone wrS) select 8)), wrLandL]
// the Harbour got its four back (a pending deposit: the player stands at it)
triSimUntil { (((gmZone wrH) select 3) + ((gmZoneSim wrH) select 13) + (gmGarrisonLive wrH)) >= wrH0 }
// the Compound did not pay
triAssertEq [((gmZone wrC) select 3), wrC0]

// ---- w4: born at the new source out of sight; it comes on --------------------
triSimUntil { player setDir wrLook; (((gmOccupierOps + [wrNone]) select 0) select 6) == "LIVE" }
wrRow = gmOccupierOps select 0
triAssertEq [(wrRow select 10), "base"]
wrCa = gmCounterAttacks select 0
wrSpawn = wrCa select 8
triAssertGe [300, (wrSpawn distance ((gmZone wrS) select 8))]
triAssertGe [(wrSpawn distance player), 750]
triAssertEq [(wrCa select 21), 4]
wrLead = wrCa select 20
wrD0 = wrSpawn distance wrLPos
wrT1 = time
// it marches, never stalls: 300 m from its birth point and 300 m nearer
// Le Port within 10 simulated minutes (about 4 at a walk)
setAccTime 2
triSimUntil { player setDir wrLook; (((wrLead distance wrSpawn) > 300) and ((wrLead distance wrLPos) < wrD0 - 300)) or (time > wrT1 + 600) or (not alive wrLead) }
setAccTime 1
triAssertGe [(wrLead distance wrSpawn), 300]
triAssertGe [wrD0 - 300, (wrLead distance wrLPos)]
triAssertEq [((gmZone wrL) select 2), gmResistanceSide]
gmSimDiagLog

triEndTest
