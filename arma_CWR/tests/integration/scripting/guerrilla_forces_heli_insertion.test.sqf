// ============================================================================
//  Occupier air, the helicopter insertion spike (OCCUPIER-FORCES.md 3.1 "Air
//  births", 3.2 "Helicopter insertion", D3, D20; guerrilla-mode/AIR-SPIKE.md).
//  Boots the SHIPPED Guerrilla.Abel template (occupier EAST) on the full
//  Remaster.  Two Mi17 insertions fly at once through the gmAirInsert lever:
//    h1  a real camera at the default view distance in daylight; the player
//        stands at the Camp facing north-east, damage-shielded and captive;
//        the director is off (gmForcesTune enabled 0) so nothing else flies;
//    h2  both birth points, over the sea beyond the south and the west map
//        edge, rate HIDDEN at their birth altitude (gmAirExposure 2): the
//        altitude-aware gate, not the ground gate;
//    h3  A flies from the south edge to Le Port, its LZ picked natively on
//        the 300..500 m ring; B flies from the west edge to Houdan (490 m
//        from the player, so its landing is in plain view, D20), its LZ on
//        the side away from the Camp.  Each is born airborne, 30+ m up;
//    h4  each lands within 100 m of its LZ on the invisible pad, every cargo
//        body gets out alive (two groups: the cargo is not the crew's), and
//        the airframe lifts off;
//    h5  each is removed where the gate says nobody sees it: the crew FIRST,
//        then the hull (gmOccupierAir [22] = 1, [12] <= [13]), no crew body
//        left in the world, the dismounted squad still standing.
//  Tag slow_air: not in the default run.  Landing offsets per trial are in
//  the engine log ("OccupierAir: insertion #N LANDING -> UNLOADING ... LZ
//  offset X m"), which tri keeps as game_stdout.log in the output dir.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
hiT0 = time
triSimUntil { time > hiT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
triAssertEq [(format ["%1", gmForcesTune ["enabled", 0]]), "true"]
player allowDammage false
player setCaptive true

// ---- h1: a real camera, the day floor; the player watches north-east ---------
hiCamp = (gmZone (gmZoneIndex "Camp")) select 8
hiHoudan = (gmZone (gmZoneIndex "Village")) select 8
player setPos [(hiCamp select 0), (hiCamp select 1), 0]
player setDir 45
hiT1 = time
triSimUntil { time > hiT1 + 2 }
hiP = gmTrafficPercept (getPos player)
triAssertEq [(hiP select 0), 1]
triAssertGe [(hiP select 3), 750]
triAssertEq [(format ["%1", gmAirTune ["trace", 5]]), "true"]

// ---- h2: both births are hidden at altitude ---------------------------------
hiBirthA = [8200, 150, 100]
hiBirthB = [150, 6100, 100]
triAssertEq [(gmAirExposure [8200, 100, 150]), 2]
triAssertEq [(gmAirExposure [150, 100, 6100]), 2]

// ---- h3: two insertions, born airborne --------------------------------------
hiA = gmAirInsert ["Mi17", 8, hiBirthA, [8193, 3129], "EAST"]
hiB = gmAirInsert ["Mi17", 8, hiBirthB, [(hiHoudan select 0), (hiHoudan select 1)], "EAST", [(hiCamp select 0), (hiCamp select 1)], "away"]
triAssertGe [hiA, 1]
triAssertGe [hiB, 1]
triAssertEq [(count gmOccupierAir), 2]
hiRa = gmOccupierAir select 0
hiRb = gmOccupierAir select 1
triAssertEq [(hiRa select 8), hiA]
triAssertEq [(hiRb select 8), hiB]
triAssertEq [(hiRa select 10), "INBOUND"]
triAssertGe [((hiRa select 7) select 2), 30]
triAssertGe [((hiRb select 7) select 2), 30]
triAssertEq [(hiRa select 20), 8]
triAssertEq [(hiRb select 20), 8]
triAssertEq [(hiRa select 5), 8]
triAssertEq [(hiRb select 5), 8]
// the LZs sit on the ring (300..500 m from the target)
triAssertGe [(([(hiRa select 11) select 0, 0, (hiRa select 11) select 1]) distance [8193, 0, 3129]), 299]
triAssertLe [(([(hiRa select 11) select 0, 0, (hiRa select 11) select 1]) distance [8193, 0, 3129]), 501]
hiCargoA = hiRa select 19
hiCargoB = hiRb select 19
hiCrewA = hiRa select 23
hiCrewB = hiRb select 23
triAssertEq [(count units hiCargoA), 8]
triAssertGe [(count units hiCrewA), 1]

// ---- h4: landed on the LZ, unloaded, lifted off -----------------------------
triSimUntil { (((gmOccupierAir select 0) select 0) >= 3) and (((gmOccupierAir select 1) select 0) >= 3) }
hiRa = gmOccupierAir select 0
hiRb = gmOccupierAir select 1
triAssertEq [(hiRa select 16), ""]
triAssertEq [(hiRb select 16), ""]
triAssertGe [(hiRa select 6), 0]
triAssertLe [(hiRa select 6), 100]
triAssertGe [(hiRb select 6), 0]
triAssertLe [(hiRb select 6), 100]
triAssertEq [(hiRa select 5), 0]
triAssertEq [(hiRb select 5), 0]
triAssertEq [(hiRa select 4), 8]
triAssertEq [(hiRb select 4), 8]
hiOutA = 0
hiOutB = 0
"if ((alive _x) and ((vehicle _x) == _x)) then {hiOutA = hiOutA + 1}" forEach (units hiCargoA)
"if ((alive _x) and ((vehicle _x) == _x)) then {hiOutB = hiOutB + 1}" forEach (units hiCargoB)
triAssertEq [hiOutA, 8]
triAssertEq [hiOutB, 8]
// lifted off, or already removed unseen on the way up (a removal can come
// before the 20 m lift-off mark when nobody looks: trial 8, AIR-SPIKE.md)
triSimUntil { ((((gmOccupierAir select 0) select 21) >= 0) or (((gmOccupierAir select 0) select 0) == 4)) and ((((gmOccupierAir select 1) select 21) >= 0) or (((gmOccupierAir select 1) select 0) == 4)) }

// ---- h5: removed unseen, the crew first, then the hull ----------------------
triSimUntil { (((gmOccupierAir select 0) select 0) == 4) and (((gmOccupierAir select 1) select 0) == 4) }
hiRa = gmOccupierAir select 0
hiRb = gmOccupierAir select 1
triAssertEq [(hiRa select 16), ""]
triAssertEq [(hiRb select 16), ""]
triAssertEq [(hiRa select 22), 1]
triAssertEq [(hiRb select 22), 1]
triAssertGe [(hiRa select 12), 0]
triAssertGe [(hiRa select 13), (hiRa select 12)]
triAssertGe [(hiRb select 12), 0]
triAssertGe [(hiRb select 13), (hiRb select 12)]
triAssertEq [(format ["%1", isNull (hiRa select 18)]), "true"]
triAssertEq [(format ["%1", isNull (hiRb select 18)]), "true"]
hiT2 = time
triSimUntil { time > hiT2 + 2 }
hiCrewLeft = 0
"hiCrewLeft = hiCrewLeft + 1" forEach (units hiCrewA)
"hiCrewLeft = hiCrewLeft + 1" forEach (units hiCrewB)
triAssertEq [hiCrewLeft, 0]
// the dismounted squads stay
hiOutA = 0
hiOutB = 0
"if (alive _x) then {hiOutA = hiOutA + 1}" forEach (units hiCargoA)
"if (alive _x) then {hiOutB = hiOutB + 1}" forEach (units hiCargoB)
triAssertEq [hiOutA, 8]
triAssertEq [hiOutB, 8]

triEndTest
