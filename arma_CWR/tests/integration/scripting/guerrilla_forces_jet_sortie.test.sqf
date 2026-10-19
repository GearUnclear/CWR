// ============================================================================
//  Occupier air, the jet sortie spike (OCCUPIER-FORCES.md 3.1 "Air births",
//  3.2 "Jet or attack-helicopter strike", D3, D9, D17;
//  guerrilla-mode/AIR-SPIKE.md).  Boots the SHIPPED Guerrilla.Abel template
//  (occupier EAST) on the full Remaster.  One Su25 sortie through the
//  gmAirSortie lever:
//    j1  a real camera at the default view distance in daylight; the player
//        stands at the Camp facing north-west, damage-shielded and captive;
//        the director is off (gmForcesTune enabled 0);
//    j2  the birth point over the sea beyond the east map edge rates HIDDEN
//        at its birth altitude (gmAirExposure 2);
//    j3  the jet is born airborne (in flight, never on a runway: no taxi),
//        flies to Le Port and starts its search orbit (LOITER);
//    j4  the limited reveal (D9): a sighting older than 60 s or an exact fix
//        is refused, a fresh inexact one is passed to the crew;
//    j5  after the sortie time it flies home and is removed where the gate
//        says nobody sees it: the crew FIRST, then the hull; it never
//        touched the ground and no crew body is left in the world.
//  Tag slow_air: not in the default run.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
jsT0 = time
triSimUntil { time > jsT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
triAssertEq [(format ["%1", gmForcesTune ["enabled", 0]]), "true"]
player allowDammage false
player setCaptive true

// ---- j1: a real camera, the day floor; the player watches north-west ---------
jsCamp = (gmZone (gmZoneIndex "Camp")) select 8
player setPos [(jsCamp select 0), (jsCamp select 1), 0]
player setDir 315
jsT1 = time
triSimUntil { time > jsT1 + 2 }
jsP = gmTrafficPercept (getPos player)
triAssertEq [(jsP select 0), 1]
triAssertGe [(jsP select 3), 750]
triAssertEq [(format ["%1", gmAirTune ["trace", 5]]), "true"]

// ---- j2: the birth is hidden at altitude ------------------------------------
triAssertEq [(gmAirExposure [12700, 250, 3500]), 2]

// ---- j3: born airborne, flies to the target, searches -----------------------
jsId = gmAirSortie ["Su25", [12700, 3500, 250], [8193, 3129], 90, "EAST"]
triAssertGe [jsId, 1]
triAssertEq [(count gmOccupierAir), 1]
jsR = gmOccupierAir select 0
triAssertEq [(jsR select 8), jsId]
triAssertEq [(jsR select 9), "jet"]
triAssertEq [(jsR select 10), "INBOUND"]
triAssertGe [((jsR select 7) select 2), 100]
triAssertGe [(jsR select 15), 50]
jsCrew = jsR select 23
triAssertGe [(count units jsCrew), 1]
triSimUntil { ((gmOccupierAir select 0) select 0) >= 1 }
jsR = gmOccupierAir select 0
triAssertEq [(jsR select 10), "LOITER"]
triAssertEq [(jsR select 16), ""]

// ---- j4: the limited reveal -------------------------------------------------
jsTgt = "Jeep" createVehicle [8193, 3129, 0]
triAssertEq [(format ["%1", gmAirReveal [jsId, jsTgt, 70, 0.5]]), "false"]
triAssertEq [(format ["%1", gmAirReveal [jsId, jsTgt, 10, 1]]), "false"]
triAssertEq [(format ["%1", gmAirReveal [jsId, jsTgt, 10, 0.5]]), "true"]
triAssertEq [((gmOccupierAir select 0) select 14), 1]

// ---- j5: home, removed unseen, the crew first --------------------------------
triSimUntil { ((gmOccupierAir select 0) select 0) >= 2 }
jsR = gmOccupierAir select 0
triAssertEq [(jsR select 10), "HOME"]
triSimUntil { ((gmOccupierAir select 0) select 0) == 3 }
jsR = gmOccupierAir select 0
triAssertEq [(jsR select 10), "GONE"]
triAssertEq [(jsR select 16), ""]
triAssertEq [(jsR select 22), 1]
triAssertGe [(jsR select 12), 0]
triAssertGe [(jsR select 13), (jsR select 12)]
triAssertEq [(jsR select 21), 0]
triAssertGe [(jsR select 20), 30]
triAssertEq [(format ["%1", isNull (jsR select 18)]), "true"]
jsT2 = time
triSimUntil { time > jsT2 + 2 }
jsCrewLeft = 0
"jsCrewLeft = jsCrewLeft + 1" forEach (units jsCrew)
triAssertEq [jsCrewLeft, 0]

triEndTest
