// ============================================================================
//  Occupier forces, reinforcement wave 1 retakes a base (OCCUPIER-FORCES.md
//  phase 4, 7.1 / D7 waves, 7.2 the base recapture, 7.4 the win).  Boots the
//  SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    w1  a real camera at the default view distance in daylight; test levers:
//        director tick 1 s, no director tokens (only the wave sends anything),
//        one distinct town wakes the director, wave lead 5 s, a 300 m cluster
//        radius (every held zone its own cluster), the abstract leg at
//        100 m/s;
//    w2  the Outpost and the Southeast Compound fall to the resistance by
//        script (owner flips), the Outpost hot (heat 100) and empty; the
//        player waits in the emptied Compound, off the strike's road; Houdan
//        rises remotely (the director wakes; control drops further; more
//        towns are flipped until control is at or below 0.75);
//    w3  wave 1 fires after the two-tick dwell and lands: the densest
//        cluster is the hot Outpost; a WAVE strike of kind BASE goes there
//        from the occupier hub;
//    w4  the strike holds the empty base off-screen and its occCapture
//        retakes it (owner back to the occupier, lostCount 1);
//    w5  every other roster base is then handed to the resistance: no win
//        while the Outpost is enemy-held (gmCampaignWon stays false).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
wvT0 = time
triSimUntil { time > wvT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- w1: a real camera; the levers ------------------------------------------
wvP = gmTrafficPercept (getPos player)
triAssertEq [(wvP select 0), 1]
triAssertGe [(wvP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(format ["%1", (gmOccupierForces) select 15]), "false"]
triAssertEq [((gmOccupierForces) select 18), 0]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["tokensMax", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["waveLead", 5]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["waveCluster", 300]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
wvRes = gmResistanceSide call GM_fnSideFromString

// ---- w2: two bases fall by script; Houdan rises -----------------------------
wvO = gmZoneIndex "Outpost"
wvC = gmZoneIndex "Southeast Compound"
triAssertGe [wvO, 0]
triAssertGe [wvC, 0]
wvOPos = (gmZone wvO) select 8
triSimUntil { gmGarrisonSpawned wvO }
wvUnits = []
"wvUnits = wvUnits + (units _x)" forEach (gmGarrisonGroups wvO)
{_x setDamage 1} forEach wvUnits
triSimUntil { (gmGarrisonLive wvO) == 0 }
gmZoneSet [wvO, "garrison", 0]
gmZoneSet [wvO, "owner", gmResistanceSide]
gmZoneSet [wvO, "heatRaise", 100]
gmZoneSet [wvC, "garrison", 0]
gmZoneSet [wvC, "owner", gmResistanceSide]

// the player leaves for the emptied Southeast Compound (3.2 km from the
// Outpost, 3.5 km from Houdan), well off the Airfield's road to the Outpost:
// a strike whose road passes the player is born live on it and walks the
// rest at real speed (3.1), far longer than this test waits
player setPos [9715, 3940, 0]
triSimUntil { not ((gmZoneSim wvO) select 0) }

wvV = gmZoneIndex "Village"
triAssertGe [wvV, 0]
wvVPos = (gmZone wvV) select 8
triSimUntil { not (gmGarrisonSpawned wvV) }
gmZoneSet [wvV, "garrison", 0]
gmZoneSet [wvV, "support", 55]
wvSq = [wvRes, gmFactionSquad [gmResistanceSide, 1, 1], wvVPos] call GM_fnSpawnSquad
wvSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units wvSq)
doStop (units wvSq)
triSimUntil { ((gmZone wvV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units wvSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }

// three more cold, far towns by script: with the two bases down (M 0.625)
// control is 0.375 + 0.4 T, at or below 0.75 once T <= 0.9375
wvN = 0
wvI = 0
while {(wvI < gmZoneCount) and (wvN < 3)} do {wvZ = gmZone wvI; if (((wvZ select 1) == "CITY") and ((wvZ select 2) == "NEUTRAL") and (not ((gmZoneSim wvI) select 10)) and (((wvZ select 8) distance wvOPos) > 1000) and (((wvZ select 8) distance (getPos player)) > 900)) then {gmZoneSet [wvI, "garrison", 0]; gmZoneSet [wvI, "owner", gmResistanceSide]; wvN = wvN + 1}; wvI = wvI + 1}
triAssertEq [wvN, 3]
triSimUntil { ((gmOccupierForces) select 5) <= 0.75 }

// ---- w3: wave 1 fires, lands, strikes the hot Outpost ------------------------
triSimUntil { ((gmOccupierForces) select 18) >= 1 }
triAssertEq [((gmOccupierForces) select 18), 1]
triSimUntil { ((gmOccupierForces) select 22) >= 1 }
wvOp = []
triSimUntil { wvOp = []; {if (((_x select 2) == "WAVE") and ((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {wvOp = _x}} forEach gmOccupierOps; (count wvOp) > 0 }
triAssertEq [(format ["%1", (wvOp select 4) != ""]), "true"]
triAssertGe [(wvOp select 7), 4]
wvId = wvOp select 0
// the occupier war level rose with the landed wave
triAssertGe [((gmOccupierForces) select 23), 2]

// ---- w4: the strike holds the empty base and retakes it ----------------------
triSimUntil { ((gmZone wvO) select 2) == gmOccupierSide }
triAssertEq [((gmZoneSim wvO) select 15), 1]
triSimUntil { wvOp = []; {if ((_x select 0) == wvId) then {wvOp = _x}} forEach gmOccupierOps; (count wvOp) == 0 }

// ---- w5: every other roster base held: no win while the Outpost is theirs ---
{gmZoneSet [gmZoneIndex _x, "garrison", 0]; gmZoneSet [gmZoneIndex _x, "owner", gmResistanceSide]} forEach ["Southeast Compound", "Le Port Harbour", "Airfield"]
wvT2 = time
triSimUntil { time > wvT2 + 8 }
triAssertEq [(format ["%1", gmCampaignWon]), "false"]
triAssertEq [((gmZone wvO) select 2), gmOccupierSide]
gmSimDiagLog

triEndTest
