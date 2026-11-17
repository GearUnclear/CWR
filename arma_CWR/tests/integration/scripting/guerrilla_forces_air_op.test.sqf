// ============================================================================
//  Occupier forces, an AIR operation (OCCUPIER-FORCES.md 3.1 "Air births",
//  3.2 "Helicopter insertion", 6.5, D3, D20; phase 6A).  Boots the SHIPPED
//  Guerrilla.Abel template (occupier EAST, occupierForces = 1) on the full
//  Remaster.  A RETAKE of Houdan (the "Village" CITY zone) is bought as a
//  helicopter insertion through the test lever gmCounterAttackLaunchFunded
//  (a director-style funded order, package "heli", the Mi17, flown from the
//  occupier-held Airfield):
//    a1  a real camera at the default view distance in daylight (the 750 m
//        day floor); Houdan is held by the resistance and empty, the player
//        stands on its north-west edge, damage-shielded and captive; the
//        director is off so nothing else flies;
//    a2  the order goes LIVE at once (the town is in the bubble): the record
//        carries an OccupierAir id, its air origin is the Airfield, and the
//        Mi17 is born AIRBORNE (30+ m up) far beyond the player's
//        perception (750 m and more), with the infantry seated as cargo;
//    a3  its LZ lies 300..500 m from the town (never within 250 m), the
//        helicopter lands within 100 m of it (D20: in plain view if it must)
//        and every cargo body gets out;
//    a4  the infantry is the record's live squad (gmCounterAttacks [27]
//        dismounted, [21] living) and assaults: its leader closes on the
//        town;
//    a5  the airframe flies home and is removed where the altitude-aware
//        gate says nobody sees it: the crew FIRST, then the hull
//        (gmOccupierAir [22] = 1, [12] <= [13]), no crew body left.
//  Tag slow_air: not in the default run.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
aoT0 = time
triSimUntil { time > aoT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
triAssertEq [(format ["%1", gmForcesTune ["enabled", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
player allowDammage false
player setCaptive true

// ---- a1: a real camera, the day floor; Houdan held and empty ----------------
aoV = gmZoneIndex "Village"
aoH = gmZoneIndex "Le Port Harbour"
aoA = gmZoneIndex "Airfield"
triAssertGe [aoV, 0]
triAssertGe [aoH, 0]
triAssertGe [aoA, 0]
aoPos = (gmZone aoV) select 8
triSimUntil { gmGarrisonSpawned aoV }
triSimUntil { (gmGarrisonLive aoV) >= 4 }
aoUnits = []
"aoUnits = aoUnits + (units _x)" forEach (gmGarrisonGroups aoV)
{_x setDamage 1} forEach aoUnits
triSimUntil { (gmGarrisonLive aoV) == 0 }
gmZoneSet [aoV, "garrison", 0]
gmZoneSet [aoV, "owner", gmResistanceSide]
gmZoneSet [aoV, "support", 70]
triSimUntil { ((gmZone aoV) select 2) == gmResistanceSide }
triAssertGe [((gmZone aoH) select 3), 6]
player setPos [(aoPos select 0) - 120, (aoPos select 1) + 120, 0]
aoT1 = time
triSimUntil { time > aoT1 + 2 }
aoP = gmTrafficPercept (getPos player)
triAssertEq [(aoP select 0), 1]
triAssertGe [(aoP select 3), 750]

// ---- a2: the insertion is bought and born airborne, unseen -----------------
aoOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "heli", 6, true, "Mi17", "RETAKE", "", "Airfield"]
triAssertGe [aoOp, 1]
aoNone = [[], [], "NONE"]
triSimUntil { (((gmCounterAttacks + [aoNone]) select 0) select 2) == "LIVE" }
aoCa = gmCounterAttacks select 0
triAssertEq [(aoCa select 10), aoOp]
triAssertEq [(aoCa select 16), "Mi17"]
triAssertEq [(aoCa select 26), aoA]
aoId = aoCa select 24
triAssertGe [aoId, 1]
// born far beyond the player's perception (the gate at altitude passed there)
aoSpawn = aoCa select 8
triAssertGe [(aoSpawn distance player), 750]
triAssertGe [(count gmOccupierAir), 1]
aoR = gmOccupierAir select 0
triAssertEq [(aoR select 8), aoId]
triAssertEq [(aoR select 9), "heli"]
triAssertGe [((aoR select 7) select 2), 30]
triAssertGe [(aoR select 20), 1]
aoBorn = aoR select 20
aoCargo = aoR select 19
aoCrew = aoR select 23
triAssertEq [(aoR select 5), aoBorn]

// ---- a3: the LZ band, the landing, the unload -------------------------------
aoLz = aoCa select 25
triAssertEq [(count aoLz), 2]
aoLzD = [(aoLz select 0), 0, (aoLz select 1)] distance [(aoPos select 0), 0, (aoPos select 1)]
triAssertGe [aoLzD, 299]
triAssertLe [aoLzD, 501]
triSimUntil { ((gmOccupierAir select 0) select 0) >= 3 }
aoR = gmOccupierAir select 0
triAssertEq [(aoR select 16), ""]
triAssertGe [(aoR select 6), 0]
triAssertLe [(aoR select 6), 100]
triAssertEq [(aoR select 5), 0]
aoOut = 0
"if ((alive _x) and ((vehicle _x) == _x)) then {aoOut = aoOut + 1}" forEach (units aoCargo)
triAssertEq [aoOut, aoBorn]

// ---- a4: the infantry is the live squad and assaults ------------------------
aoNoneAir = [[], [], "NONE", 0, 0, 0, 0, objNull, [], "", 0, 0, 0, 0, 0, 0, "", 0, objNull, 0, objNull, 0, "", objNull, 0, [], 0, false, false]
triSimUntil { ((gmCounterAttacks + [aoNoneAir]) select 0) select 27 }
aoCa = gmCounterAttacks select 0
triAssertEq [(aoCa select 2), "LIVE"]
triAssertEq [(aoCa select 24), 0]
triAssertEq [(aoCa select 21), aoBorn]
aoLead = aoCa select 20
aoD1 = aoLead distance [(aoPos select 0), (aoPos select 1), 0]
aoT2 = time
triSimUntil { ((aoLead distance [(aoPos select 0), (aoPos select 1), 0]) < (aoD1 - 40)) or (time > aoT2 + 150) }
triAssertGe [(aoD1 - 40), (aoLead distance [(aoPos select 0), (aoPos select 1), 0])]

// ---- a5: the airframe is removed unseen, the crew first ---------------------
triSimUntil { ((gmOccupierAir select 0) select 0) == 4 }
aoR = gmOccupierAir select 0
triAssertEq [(aoR select 16), ""]
triAssertEq [(aoR select 22), 1]
triAssertGe [(aoR select 12), 0]
triAssertGe [(aoR select 13), (aoR select 12)]
triAssertEq [(format ["%1", isNull (aoR select 18)]), "true"]
aoT3 = time
triSimUntil { time > aoT3 + 2 }
aoCrewLeft = 0
"aoCrewLeft = aoCrewLeft + 1" forEach (units aoCrew)
triAssertEq [aoCrewLeft, 0]

triEndTest
