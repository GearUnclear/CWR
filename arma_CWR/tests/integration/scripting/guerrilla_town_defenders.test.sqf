// ============================================================================
//  Town defenders, the town retake and the D1 player-presence rule
//  (issue #79 B2 + D1; #72 first slice). Boots the SHIPPED Guerrilla.Abel
//  template on the full Remaster (packages/RemasterUD shadow) and proves, on
//  Houdan (the authored "Village" CITY zone, ~490 m from the Camp, inside the
//  800 m cacheRadius):
//    t1  the town is occupier-administered at boot: NEUTRAL owner, and the
//        production GarrisonCache materializes its cityGarrison (4) at boot;
//    t2  one resistance fighter standing in the defended town raises NO
//        support while the garrison lives (the support gate: contested);
//    t3  kill the garrison (casualties, not deletes): the town's live
//        garrison count reaches 0 with no reserve, support rises, the town
//        flips to the resistance and stamps flipDay;
//    t4  the town retake: the fighter leaves, an occupier soldier stands in
//        the held town, support drains and the town reverts to NEUTRAL with
//        its garrison refilled (cached back in through GarrisonCache);
//    t5  D1: with that garrison killed and nobody else in town, the armed
//        player ALONE counts as resistance presence (warrior outfit, cover
//        kept: gmUndercover stays true) and raises the town again.
//  Every staged unit is damage-shielded and holds fire (BLUE) where the
//  stage needs presence only; the companion roster and the QRF director are
//  parked so nothing else walks into the town.
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- staging isolation (the guerrilla_native_undercover recipe): retire the
//    companion roster, sweep the player's squad, park the QRF loop ----------
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
tdT0 = time
triSimUntil { time > tdT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

tdRes = gmResistanceSide call GM_fnSideFromString
tdOcc = gmOccupierSide call GM_fnSideFromString
tdV = gmZoneIndex "Village"
triAssertGe [tdV, 0]
tdPos = (gmZone tdV) select 8

// ---- t1: an administered town with a standing garrison ----------------------
triAssertEq [((gmZone tdV) select 1), "CITY"]
triAssertEq [((gmZone tdV) select 2), "NEUTRAL"]
triAssertEq [((gmZoneSim tdV) select 7), 4]
triSimUntil { gmGarrisonSpawned tdV }
triSimUntil { (gmGarrisonLive tdV) >= 4 }
triAssertEq [((gmZone tdV) select 3), 0]
{_x setCombatMode "BLUE"} forEach (gmGarrisonGroups tdV)
tdS0 = (gmZone tdV) select 4

// ---- t2: one fighter in the defended town raises nothing --------------------
tdSq = [tdRes, gmFactionSquad [gmResistanceSide, 1, 1], tdPos] call GM_fnSpawnSquad
tdSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units tdSq)
doStop (units tdSq)
triSimUntil { ((gmZoneSim tdV) select 1) >= 1 }
triSimUntil { (gmZoneSim tdV) select 5 }
tdT0 = time
triSimUntil { time > tdT0 + 12 }
triAssertEq [((gmZone tdV) select 4), tdS0]
triAssertEq [((gmZone tdV) select 2), "NEUTRAL"]
triAssertGe [((gmZoneSim tdV) select 9), 1]

// ---- t3: kill the garrison; support rises and the town flips ----------------
tdUnits = []
"tdUnits = tdUnits + (units _x)" forEach (gmGarrisonGroups tdV)
{_x setDamage 1} forEach tdUnits
triSimUntil { (gmGarrisonLive tdV) == 0 }
triSimUntil { ((gmZoneSim tdV) select 9) == 0 }
triSimUntil { ((gmZone tdV) select 4) > tdS0 }
triSimUntil { ((gmZone tdV) select 2) == gmResistanceSide }
triAssertGe [((gmZone tdV) select 4), 60]
triAssertGe [((gmZoneSim tdV) select 11), 1]
triAssertGe [((gmPlayerPresence) select 5), 1]

// ---- t4: the retake: fighter out, one occupier in, the town reverts ---------
{deleteVehicle _x} forEach (units tdSq)
// keep the core's queue handler and add a sentinel for the assert below
gmZoneOnEvent ["townLost", {gmEvtTownLost = gmEvtTownLost + [_this]; tdLost = _this}]
tdOccGrp = [tdOcc, gmFactionTierClass [gmOccupierSide, 1], 1, tdPos] call GM_fnSpawnGroup
tdOccGrp setCombatMode "BLUE"
{_x allowDammage false} forEach (units tdOccGrp)
doStop (units tdOccGrp)
triSimUntil { ((gmZoneSim tdV) select 2) >= 1 }
triSimUntil { ((gmZone tdV) select 2) == "NEUTRAL" }
triAssertGe [60, ((gmZone tdV) select 4)]
// the garrison came back through the production cache path
triSimUntil { gmGarrisonSpawned tdV }
triSimUntil { (gmGarrisonLive tdV) >= 4 }
// the native townLost event fired for Houdan, and the capture manager
// drained its queue (hint + diary line)
triSimUntil { not (isNil "tdLost") }
triAssertEq [(tdLost select 1), "Village"]
triSimUntil { (count gmEvtTownLost) == 0 }

// ---- t5: D1 - the armed player alone takes the cleared town -----------------
{deleteVehicle _x} forEach (units tdOccGrp)
tdUnits = []
"tdUnits = tdUnits + (units _x)" forEach (gmGarrisonGroups tdV)
{_x setDamage 1} forEach tdUnits
triSimUntil { (gmGarrisonLive tdV) == 0 }
player setPos [tdPos select 0, tdPos select 1, 0]
// cover kept (per-observer exposure), yet he counts: warrior outfit
triAssertEq [(format ["%1", gmUndercover]), "true"]
triAssertEq [(format ["%1", (gmPlayerPresence) select 0]), "true"]
triAssertEq [(format ["%1", (gmPlayerPresence) select 4]), "true"]
triSimUntil { ((gmZoneSim tdV) select 1) >= 1 }
triAssertEq [((gmZoneSim tdV) select 1), 1]
tdS1 = (gmZone tdV) select 4
triSimUntil { ((gmZone tdV) select 4) > tdS1 }
triSimUntil { ((gmZone tdV) select 2) == gmResistanceSide }

triEndTest
