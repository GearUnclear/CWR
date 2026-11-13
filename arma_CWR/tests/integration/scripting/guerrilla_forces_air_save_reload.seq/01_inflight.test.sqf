// ============================================================================
//  Occupier forces across a save (OCCUPIER-FORCES.md 11, 12.2; phase 6A),
//  phase 1 of 2: a RETAKE of Houdan bought as a helicopter insertion from the
//  Airfield (the guerrilla_forces_air_op staging) is saved MID-FLIGHT, the
//  Mi17 airborne with its cargo seated.  The expected values ride the save as
//  script globals (sv*), so phase 2 compares against what this phase saw.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
svT0 = time
triSimUntil { time > svT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
triAssertEq [(format ["%1", gmForcesTune ["enabled", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
player allowDammage false
player setCaptive true

svV = gmZoneIndex "Village"
svA = gmZoneIndex "Airfield"
triAssertGe [svV, 0]
triAssertGe [svA, 0]
svPos = (gmZone svV) select 8
triSimUntil { gmGarrisonSpawned svV }
triSimUntil { (gmGarrisonLive svV) >= 4 }
svUnits = []
"svUnits = svUnits + (units _x)" forEach (gmGarrisonGroups svV)
{_x setDamage 1} forEach svUnits
triSimUntil { (gmGarrisonLive svV) == 0 }
gmZoneSet [svV, "garrison", 0]
gmZoneSet [svV, "owner", gmResistanceSide]
gmZoneSet [svV, "support", 70]
triSimUntil { ((gmZone svV) select 2) == gmResistanceSide }
player setPos [(svPos select 0) - 120, (svPos select 1) + 120, 0]
svT1 = time
triSimUntil { time > svT1 + 2 }
triAssertEq [((gmTrafficPercept (getPos player)) select 0), 1]

// ---- the insertion is bought, born airborne and still inbound ---------------
svOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "heli", 6, true, "Mi17", "RETAKE", "", "Airfield"]
triAssertGe [svOp, 1]
svNone = [[], [], "NONE"]
triSimUntil { (((gmCounterAttacks + [svNone]) select 0) select 2) == "LIVE" }
svCa = gmCounterAttacks select 0
svId = svCa select 24
triAssertGe [svId, 1]
svLz = svCa select 25
triAssertEq [(count svLz), 2]
svR = gmOccupierAir select 0
triAssertEq [(svR select 8), svId]
triAssertEq [(svR select 10), "INBOUND"]
svBorn = svR select 20
triAssertGe [svBorn, 1]
triAssertEq [(svR select 5), svBorn]
svT2 = time
triSimUntil { time > svT2 + 5 }
svR = gmOccupierAir select 0
triAssertEq [(svR select 10), "INBOUND"]
triAssertGe [((svR select 7) select 2), 30]

triAssertEq [(triSaveGame "airflight"), "OK"]
triEndTest
