// ============================================================================
//  Occupier forces, a BASE retake while STRONG (OCCUPIER-FORCES.md phase 4,
//  6.3 / D16 base candidates, 7.2 / D19 the base recapture, 6.5 a live fight
//  against present defenders).  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1, so baseRetake follows it) on the full Remaster:
//    r1  a real camera at the default view distance in daylight; the pool is
//        full (STRONG); test levers: director tick 1 s, the first-town roll
//        0, one distinct town wakes the director, the abstract leg at
//        100 m/s (gmCounterAttackTune "speed"), the arrival rule off
//        (forcesSlowEta and forcesAirEta 0: the ground package);
//    r2  the Outpost (515 m from the Camp) loses its garrison and is handed
//        to the resistance, hot (heat 100); a 3-man hold squad stands in it,
//        invulnerable and on hold fire; the player walks 2.2 km away;
//    r3  Houdan rises remotely: the director wakes with two tokens and its
//        first operation goes to the hottest candidate, the held base: a
//        BASE operation from an occupier base, gmOccupierOps kind "BASE";
//    r4  the force reaches the Outpost while the hold squad lives: it goes
//        LIVE (born at the 250 m bound, off-screen) and the occupier's base
//        meter (gmZoneSim slot 14) stays below 100 with both sides in the
//        base (frozen);
//    r5  the hold squad is removed (as if killed): the squad standing in the
//        base fills occCapture and retakes it (owner back to the occupier,
//        baseLost queued, lostCount 1), and its operation ends.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
brT0 = time
triSimUntil { time > brT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- r1: a real camera; the pool STRONG; the levers -------------------------
brP = gmTrafficPercept (getPos player)
triAssertEq [(brP select 0), 1]
triAssertGe [(brP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
// the arrival rule off (2026-10-15): from the Harbour the Outpost is a
// 30-minute walk, so the rule would send a Mi17; this test is about the
// recapture meter against a ground squad (guerrilla_forces_arrival_rule
// covers the rule)
triAssertEq [(format ["%1", gmForcesTune ["slowEta", 0]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["airEta", 0]]), "true"]
brRes = gmResistanceSide call GM_fnSideFromString

// ---- r2: the Outpost, held and hot, with a hold squad ------------------------
brO = gmZoneIndex "Outpost"
triAssertGe [brO, 0]
brOPos = (gmZone brO) select 8
triSimUntil { gmGarrisonSpawned brO }
brUnits = []
"brUnits = brUnits + (units _x)" forEach (gmGarrisonGroups brO)
{_x setDamage 1} forEach brUnits
triSimUntil { (gmGarrisonLive brO) == 0 }
gmZoneSet [brO, "garrison", 0]
gmZoneSet [brO, "owner", gmResistanceSide]
gmZoneSet [brO, "heatRaise", 100]
brHold = [brRes, gmFactionSquad [gmResistanceSide, 1, 3], brOPos] call GM_fnSpawnSquad
brHold setCombatMode "BLUE"
{_x allowDammage false} forEach (units brHold)
doStop (units brHold)
triAssertEq [((gmZoneSim brO) select 14), 0]
// the source is Le Port Harbour (the road quote, 2026-10-16): by road the
// Airfield is the nearer base (4986 m against 5089 m, the shortest leg wins,
// 6.5), but its road passes the player in the north, so its force would be
// born there and walk 3.8 km live.  The Airfield's reserve is held at its
// keep (3 of 12: no spare), so the Harbour is the one base that can staff
// the 6
brAf = gmZoneIndex "Airfield"
triAssertGe [brAf, 0]
gmZoneSet [brAf, "garrison", 3]

// the player leaves for the north (2.2 km from the Outpost)
brFar = [7922, 8500, 0]
player setPos brFar
triSimUntil { not ((gmZoneSim brO) select 0) }
triAssertEq [((gmZone brO) select 2), gmResistanceSide]

// ---- r3: Houdan rises remotely: AWAKE, the base goes first ------------------
brV = gmZoneIndex "Village"
triAssertGe [brV, 0]
brVPos = (gmZone brV) select 8
triSimUntil { not (gmGarrisonSpawned brV) }
gmZoneSet [brV, "garrison", 0]
gmZoneSet [brV, "support", 55]
brSq = [brRes, gmFactionSquad [gmResistanceSide, 1, 1], brVPos] call GM_fnSpawnSquad
brSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units brSq)
doStop (units brSq)
triSimUntil { ((gmZone brV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units brSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
brOp = []
triSimUntil { brOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {brOp = _x}} forEach gmOccupierOps; (count brOp) > 0 }
triAssertEq [(brOp select 2), "DIRECTOR"]
triAssertEq [(brOp select 4), "Le Port Harbour"]
triAssertGe [(brOp select 7), 4]
brId = brOp select 0

// ---- r4: defenders present: the force goes LIVE, the meter stays frozen ------
triSimUntil { brOp = []; {if ((_x select 0) == brId) then {brOp = _x}} forEach gmOccupierOps; ((count brOp) > 0) and ((brOp select 6) == "LIVE") }
triAssertEq [(format ["%1", (brOp select 10) != ""]), "true"]
brLeader = objNull
{if (((_x select 10) == brId)) then {brLeader = _x select 20}} forEach gmCounterAttacks
brT1 = time
triSimUntil { ((not (isNull brLeader)) and ((brLeader distance brOPos) < 140)) or (time > brT1 + 400) }
triAssertEq [(format ["%1", (not (isNull brLeader)) and ((brLeader distance brOPos) < 140)]), "true"]
brT2 = time
triSimUntil { time > brT2 + 12 }
triAssertEq [((gmZone brO) select 2), gmResistanceSide]
triAssertEq [((gmZoneSim brO) select 14), 0]

// ---- r5: the hold squad is gone: the squad in the base retakes it ------------
{deleteVehicle _x} forEach (units brHold)
triSimUntil { ((gmZone brO) select 2) == gmOccupierSide }
triAssertEq [((gmZoneSim brO) select 15), 1]
triAssertEq [((gmZoneSim brO) select 14), 0]
triAssertEq [((gmZone brO) select 5), 0]
triSimUntil { brOp = []; {if ((_x select 0) == brId) then {brOp = _x}} forEach gmOccupierOps; (count brOp) == 0 }
gmSimDiagLog

triEndTest
