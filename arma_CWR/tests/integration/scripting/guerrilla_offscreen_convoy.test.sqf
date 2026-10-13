// ============================================================================
//  Off-screen contract (issue #71) scenario (c): interrupt a convoy, leave,
//  return.  guerrilla-mode/OFFSCREEN-CONTRACT.md rows 6, 7, 9 and 11.
//  Full Remaster lane, mission guerrilla_offscreen.abel (Outpost <-> SEBase is
//  the occupier route; trafficAmbushWindow 30 s).
//  Convoys are LIVE ONLY: wiping one leaves an ambush on the alert ledger
//  (the Outpost goes RED, heat rises, the QRF may launch) and nothing else.
//  Leaving the region lets the alert run out: the zone returns GREEN, the
//  QRF stands down and is deleted, no new convoy is conjured, reserves are
//  untouched (the QRF is free, a convoy carries no cargo) and heat cools.
//  Returning finds the same quiet state: no duplicate convoy, no stale QRF.
// ============================================================================

triSimUntil { GM_LIB_READY }
player allowDammage false
cvOut = gmZoneIndex "Outpost"
cvSE = gmZoneIndex "SEBase"
cvVil = gmZoneIndex "Village"
triAssertGe [cvOut, 0]
triAssertGe [cvSE, 0]
triAssertGe [cvVil, 0]

cvVPos = (gmZone cvVil) select 8
player setPos [cvVPos select 0, cvVPos select 1, 0]
triSimFrames 5

// ---- interrupt: force a convoy at the Outpost and wreck both hulls ---------
cvTruck = gmTrafficForceSpawn ["convoy", cvOut]
triAssertEq [(format ["%1", isNull cvTruck]), "false"]
cvEsc = gmTrafficEscort cvTruck
triAssertEq [(format ["%1", isNull cvEsc]), "false"]
triAssertEq [gmTrafficCount "convoy", 1]
cvHeat0 = (gmZone cvOut) select 6
triSimFrames 5
// Issue #78: NativeMoveIn must establish InCargo before the wreck ejects its
// passengers. Keep live, natively seated cargo in BOTH hulls: deleting riders
// or reseating them with moveInCargo masks the broken native state transition.
cvCargo = []
{if ((_x != (driver cvTruck)) and (_x != (gunner cvTruck)) and (_x != (commander cvTruck))) then {cvCargo = cvCargo + [_x]}} forEach (crew cvTruck)
cvEscCargo = []
{if ((_x != (driver cvEsc)) and (_x != (gunner cvEsc)) and (_x != (commander cvEsc))) then {cvEscCargo = cvEscCargo + [_x]}} forEach (crew cvEsc)
triAssertGt [count cvCargo, 0]
triAssertGt [count cvEscCargo, 0]
triAssertEq [(format ["%1", alive (cvCargo select 0)]), "true"]
triAssertEq [(format ["%1", alive (cvEscCargo select 0)]), "true"]
cvTruck setDammage 1
cvEsc setDammage 1
triSimUntil { (gmTrafficCount "convoy") == 0 }
// the ambush floors the attributed zone (the Outpost, nearest occupier zone)
triSimUntil { (gmZoneAlert cvOut) >= 1 }
triSimUntil { ((gmZone cvOut) select 6) > cvHeat0 }

// ---- leave: the far point, 3.3 km away --------------------------------------
player setPos [7922, 9590, 0]
triSimUntil { (gmZoneAlert cvOut) == 0 }
triSimUntil { not GM_QRF_ACTIVE }
cvHeatLeft = (gmZone cvOut) select 6
triAssertEq [gmTrafficCount "convoy", 0]
triSimUntil { not (gmGarrisonSpawned cvOut) }
// reserves untouched: the garrison wrote back whole, SEBase never moved
triAssertEq [((gmZone cvOut) select 3), 6]
triAssertEq [((gmZone cvSE) select 3), 4]
triAssertEq [((gmZone cvOut) select 2), gmOccupierSide]
cvT = time
triSimUntil { time > cvT + 20 }
// heat keeps cooling off-screen (GREEN decay, 1 per 10 s)
triAssertGt [cvHeatLeft, ((gmZone cvOut) select 6)]
cvHeatAway = (gmZone cvOut) select 6

// ---- return: the same quiet region ------------------------------------------
// (checked on arrival, before the rematerialized garrison can spot the player)
player setPos [cvVPos select 0, cvVPos select 1, 0]
triAssertEq [(format ["%1", GM_QRF_ACTIVE]), "false"]
triAssertEq [(gmZoneAlert cvOut), 0]
triAssertEq [gmTrafficCount "convoy", 0]
triAssertGe [cvHeatAway, ((gmZone cvOut) select 6)]
triAssertEq [((gmZone cvOut) select 2), gmOccupierSide]
// the garrison rematerializes once, whole: no healing, no duplicates
triSimUntil { gmGarrisonSpawned cvOut }
triSimUntil { (gmGarrisonLive cvOut) >= 6 }
triSimFrames 30
triAssertEq [(gmGarrisonLive cvOut), 6]

triEndTest
