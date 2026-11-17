// ============================================================================
//  Occupier forces, a wave strike takes the arrival rule (OCCUPIER-FORCES.md
//  6.5 the arrival rule, 7.1 the wave's strikes; phase 1 A after video 5,
//  where the wave's HQ assault walked 41 minutes, "road 2450 s", from the
//  harbour that had just landed it).  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    w1  test levers: director tick 1 s, no director tokens (only the wave
//        sends anything), one distinct town wakes the director, the
//        first-town roll 0, wave lead 5 s, a 300 m cluster radius (every
//        held zone its own cluster);
//    w2  the default camp start is a real HQ at the Camp; Houdan (the
//        Village) rises remotely and wakes the director; the Camp is made
//        the hottest cluster (the HQ scores 4);
//    w3  wave 1 is forced and lands: its strike on the HQ is an HQ_ASSAULT
//        of origin WAVE from the hub, and the hub's road quote to the HQ on
//        foot (gmRoadQuote, else the straight line x forcesRoadFactor, over
//        1.5 m/s) exceeds forcesSlowEta (900 s), so the package is NOT foot.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
waT0 = time
triSimUntil { time > waT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- w1: the levers ----------------------------------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["waveLead", 5]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["waveCluster", 300]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
waRes = gmResistanceSide call GM_fnSideFromString

// ---- w2: the HQ at the Camp; Houdan wakes the director -------------------------
waCamp = gmZoneIndex "Camp"
triAssertGe [waCamp, 0]
triSimUntil { gmHqEstablished }
triAssertEq [((gmHqState) select 4), waCamp]
waHq = gmHqPos
waV = gmZoneIndex "Village"
triAssertGe [waV, 0]
waVPos = (gmZone waV) select 8
waFar = [4500, 5200, 0]
if ((gmLandmass waFar) < 0) then {waFar = [7922, 8500, 0]}
player setPos waFar
triSimUntil { not (gmGarrisonSpawned waV) }
gmZoneSet [waV, "garrison", 0]
gmZoneSet [waV, "support", 55]
waSq = [waRes, gmFactionSquad [gmResistanceSide, 1, 1], waVPos] call GM_fnSpawnSquad
waSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units waSq)
doStop (units waSq)
triSimUntil { ((gmZone waV) select 2) == gmResistanceSide }
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
triAssertEq [(count gmOccupierOps), 0]
gmZoneSet [waCamp, "heatRaise", 100]

// ---- w3: wave 1 lands; its HQ strike rides from the hub ------------------------
triAssertEq [(format ["%1", gmForcesForce ["wave", 1]]), "true"]
triSimUntil { ((gmOccupierForces) select 22) >= 1 }
waOp = []
waT1 = time
triSimUntil { waOp = []; {if (((_x select 2) == "WAVE") and ((_x select 1) == "HQ_ASSAULT")) then {waOp = _x}} forEach gmOccupierOps; ((count waOp) > 0) or (time > waT1 + 30) }
triAssertGe [(count waOp), 8]
waHub = gmZoneIndex (waOp select 4)
triAssertGe [waHub, 0]
waHubPos = (gmZone waHub) select 8
// the walk the arrival rule weighs: the road quote, else the line x 1.5
waLeg = (gmRoadQuote [waHubPos, waHq]) select 0
if (waLeg < 0) then {waLeg = 1.5 * (waHubPos distance waHq)}
triAssertGe [waLeg / 1.5, 900]
triAssertEq [(format ["%1", (waOp select 3) != "FOOT"]), "true"]
triAssertGe [(waOp select 7), 6]
gmSimDiagLog

triEndTest
