// ============================================================================
//  Town defenders, the town retake and the player-presence predicates
//  (issue #79 B2; #72 first slice; OCCUPIER-FORCES.md 3.3 and 8.1, issue
//  #86). Boots the SHIPPED Guerrilla.Abel
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
//        its garrison refilled.  The player at the Camp is inside Houdan's
//        deposit bubble, so the refill is a PENDING deposit (S5: no pop-in);
//        he walks out of the bubble, it joins the reserve, and back at the
//        Camp the garrison is cached in through GarrisonCache;
//    t5  CAPTURE (#86): with that garrison killed and nobody else in town,
//        the player ALONE with his weapon IN HANDS counts as resistance
//        presence (cover kept: gmUndercover stays true) and raises the town
//        again;
//    t6  HOLD (D23): the same uniformed player with his rifle slung stands
//        in the town he holds while one occupier soldier stands in it: the
//        drain is frozen for 15 s; he walks 400 m out and it drains again.
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
// S5: the refill never pops in beside the player at the Camp (inside
// Houdan's deposit bubble): it waits as a pending deposit (gmZoneSim slot
// 13), or has already joined the reserve where nobody could see it; no body
// is lost either way
triSimUntil { (((gmZoneSim tdV) select 13) + ((gmZone tdV) select 3) + (gmGarrisonLive tdV)) == 4 }
// out of the bubble the deposit joins the reserve ...
tdHome = getPos player
player setPos [7922, 8500, 0]
triSimUntil { not ((gmZoneSim tdV) select 0) }
triSimUntil { ((gmZoneSim tdV) select 13) == 0 }
triSimUntil { (((gmZone tdV) select 3) + (gmGarrisonLive tdV)) >= 4 }
// ... and back at the Camp the garrison comes back through the production
// cache path
player setPos tdHome
triSimUntil { gmGarrisonSpawned tdV }
triSimUntil { (gmGarrisonLive tdV) >= 4 }
// the native townLost event fired for Houdan, and the capture manager
// drained its queue (hint + diary line)
triSimUntil { not (isNil "tdLost") }
triAssertEq [(tdLost select 1), "Village"]
triSimUntil { (count gmEvtTownLost) == 0 }

// ---- t5: CAPTURE - the player alone, weapon in hands, takes the cleared town -
{deleteVehicle _x} forEach (units tdOccGrp)
tdUnits = []
"tdUnits = tdUnits + (units _x)" forEach (gmGarrisonGroups tdV)
{_x setDamage 1} forEach tdUnits
triSimUntil { (gmGarrisonLive tdV) == 0 }
player setPos [tdPos select 0, tdPos select 1, 0]
if ((primaryWeapon player) == "") then {player addMagazine "AK47"; player addMagazine "AK47"; player addWeapon "AK47"}
player selectWeapon (primaryWeapon player)
player action ["WEAPONINHAND", player]
triSimUntil { (gmPlayerPresence) select 3 }
// cover kept (per-observer exposure), yet he counts: a weapon in hands
// (the outfit alone no longer captures, issue #86)
triAssertEq [(format ["%1", gmUndercover]), "true"]
triAssertEq [(format ["%1", (gmPlayerPresence) select 0]), "true"]
triAssertEq [(format ["%1", (gmPlayerPresence) select 3]), "true"]
triSimUntil { ((gmZoneSim tdV) select 1) >= 1 }
triAssertEq [((gmZoneSim tdV) select 1), 1]
tdS1 = (gmZone tdV) select 4
triSimUntil { ((gmZone tdV) select 4) > tdS1 }
triSimUntil { ((gmZone tdV) select 2) == gmResistanceSide }

// ---- t6: HOLD - uniformed, rifle slung, he holds the town against a drain ----
player action ["WEAPONONBACK", player]
triSimUntil { not ((gmPlayerPresence) select 3) }
triAssertEq [(format ["%1", (gmPlayerPresence) select 4]), "true"]
triAssertEq [(format ["%1", (gmPlayerPresence) select 6]), "true"]
tdOcc2 = [tdOcc, gmFactionTierClass [gmOccupierSide, 1], 1, tdPos] call GM_fnSpawnGroup
tdOcc2 setCombatMode "BLUE"
{_x allowDammage false} forEach (units tdOcc2)
doStop (units tdOcc2)
triSimUntil { ((gmZoneSim tdV) select 2) >= 1 }
tdS2 = (gmZone tdV) select 4
tdT0 = time
triSimUntil { time > tdT0 + 15 }
triAssertGe [((gmZone tdV) select 4), tdS2]
triAssertEq [((gmZone tdV) select 2), gmResistanceSide]
// he walks 400 m out of town toward the Camp: nobody holds it, it drains
tdC = (gmZone (gmZoneIndex "Camp")) select 8
tdDx = (tdC select 0) - (tdPos select 0)
tdDy = (tdC select 1) - (tdPos select 1)
tdD = sqrt ((tdDx * tdDx) + (tdDy * tdDy))
player setPos [(tdPos select 0) + (400 * tdDx / tdD), (tdPos select 1) + (400 * tdDy / tdD), 0]
tdT0 = time
triSimUntil { time > tdT0 + 15 }
triAssertGe [tdS2 - 5, ((gmZone tdV) select 4)]
{deleteVehicle _x} forEach (units tdOcc2)

triEndTest
