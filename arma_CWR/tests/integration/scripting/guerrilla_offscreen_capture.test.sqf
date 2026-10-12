// ============================================================================
//  Off-screen contract (issue #71) scenario (a): leave friendly troops at a
//  distant zone.  guerrilla-mode/OFFSCREEN-CONTRACT.md, decision D1.
//  Full Remaster lane (packages/RemasterUD), mission guerrilla_offscreen.abel.
//    a1  troops left at an emptied Outpost finish the capture after the
//        player is 3.3 km away: the meter keeps climbing on the remote tick,
//        the zone flips with the in-bubble side effects (heat, income, hold
//        squad) and the intel stamp (seenDay/seenMinute) does NOT move.
//    a2  troops against a stored reserve (SEBase, garrison 4, never
//        materialized) are a frozen standoff: capture, owner and reserve
//        hold still.
//    a3  pull the troops: the half-capture decays (DEFENDED 10/tick with the
//        reserve, ABANDONED 2/tick without it).
//    a4  one organizer left in a distant town raises support +5/tick and the
//        town rises at 60.
//    a5  an occupier soldier in a distant town (no fighters) does not bleed
//        its support: no remote intimidation.
//  The player is damage-shielded throughout; every staged unit is shielded
//  and doStop'ed so presence is the only variable.
// ============================================================================

triSimUntil { GM_LIB_READY }
player allowDammage false
ocOut = gmZoneIndex "Outpost"
ocSE = gmZoneIndex "SEBase"
triAssertGe [ocOut, 0]
triAssertGe [ocSE, 0]
ocRes = gmResistanceSide call GM_fnSideFromString
ocOcc = gmOccupierSide call GM_fnSideFromString
ocFar = [7922, 9590, 0]

// ---- a1: empty the Outpost, leave one fighter in it, walk away -------------
triSimUntil { gmGarrisonSpawned ocOut }
triSimUntil { (gmGarrisonLive ocOut) >= 6 }
ocUnits = []
"ocUnits = ocUnits + (units _x)" forEach (gmGarrisonGroups ocOut)
{deleteVehicle _x} forEach ocUnits
gmGarrisonForceDespawn ocOut
gmZoneSet [ocOut, "garrison", 0]
triSimFrames 10
gmGarrisonForceDespawn ocOut
gmZoneSet [ocOut, "garrison", 0]

ocPos = (gmZone ocOut) select 8
ocSq = [ocRes, gmFactionSquad [gmResistanceSide, 1, 1], ocPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units ocSq)
doStop (units ocSq)
triSimUntil { ((gmZone ocOut) select 9) > 0 }
triAssertEq [((gmZone ocOut) select 2), gmOccupierSide]

player setPos ocFar
// first zone tick with the player gone: the Outpost is outside the bubble
triSimUntil { not ((gmZoneSim ocOut) select 0) }
ocSeenD = (gmZoneSim ocOut) select 3
ocSeenM = (gmZoneSim ocOut) select 4
// the remote tick advances it (lastRemote) ...
triSimUntil { ((gmZoneSim ocOut) select 6) or (((gmZone ocOut) select 2) == gmResistanceSide) }
// ... to the flip, with nobody watching
triSimUntil { ((gmZone ocOut) select 2) == gmResistanceSide }
triAssertEq [((gmZone ocOut) select 9), 0]
triAssertGe [((gmZone ocOut) select 6), 20]
triAssertGe [((gmZone ocOut) select 5), 25]
triAssertEq [((gmZone ocOut) select 3), 0]
triAssertEq [(format ["%1", gmGarrisonSpawned ocOut]), "false"]
// the intel stamp stayed where the player last saw the zone
triAssertEq [((gmZoneSim ocOut) select 3), ocSeenD]
triAssertEq [((gmZoneSim ocOut) select 4), ocSeenM]
// capture.sqs reacted exactly as in the bubble: a holding squad
triSimUntil { not (isNil "GM_cGrp") }
triAssertEq [(count units GM_cGrp), 3]

// ---- a2: standoff against a stored reserve (SEBase, 5.9 km away) -----------
triAssertEq [((gmZone ocSE) select 3), 4]
triAssertEq [(format ["%1", gmGarrisonSpawned ocSE]), "false"]
ocSEPos = (gmZone ocSE) select 8
ocSq2 = [ocRes, gmFactionSquad [gmResistanceSide, 1, 2], ocSEPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units ocSq2)
doStop (units ocSq2)
gmZoneSet [ocSE, "capture", 40]
triSimUntil { ((gmZoneSim ocSE) select 1) >= 1 }
triSimUntil { (gmZoneSim ocSE) select 5 }
ocT = time
triSimUntil { time > ocT + 10 }
triAssertEq [((gmZone ocSE) select 9), 40]
triAssertEq [((gmZone ocSE) select 2), gmOccupierSide]
triAssertEq [((gmZone ocSE) select 3), 4]
triAssertEq [(format ["%1", gmGarrisonSpawned ocSE]), "false"]

// ---- a3: pull the troops; the half-capture decays ---------------------------
{deleteVehicle _x} forEach (units ocSq2)
triSimUntil { ((gmZone ocSE) select 9) == 0 }
triAssertEq [((gmZone ocSE) select 2), gmOccupierSide]
gmZoneSet [ocSE, "garrison", 0]
gmZoneSet [ocSE, "capture", 50]
triSimUntil { ((gmZone ocSE) select 9) < 50 }
triAssertGe [((gmZone ocSE) select 9), 46]
gmZoneSet [ocSE, "garrison", 4]
gmZoneSet [ocSE, "capture", 0]

// ---- pick two distant, empty, neutral towns (seeded from Abel's Names) -----
ocT1 = -1
ocT2 = -1
ocI = 0
while {ocI < gmZoneCount} do {ocZ = gmZone ocI; ocP = ocZ select 8; ocDx = (ocP select 0) - (ocFar select 0); ocDz = (ocP select 1) - (ocFar select 1); if (((ocZ select 1) == "CITY") and ((ocZ select 2) == "NEUTRAL") and (((ocDx * ocDx) + (ocDz * ocDz)) > 1440000) and (((gmZoneSim ocI) select 2) == 0) and (((gmZoneSim ocI) select 1) == 0)) then {if (ocT1 < 0) then {ocT1 = ocI} else {if (ocT2 < 0) then {ocT2 = ocI}}}; ocI = ocI + 1}
triAssertGe [ocT1, 0]
triAssertGe [ocT2, 0]

// ---- a4: one organizer left in a distant town; it rises -------------------
gmZoneSet [ocT1, "support", 20]
ocTPos = (gmZone ocT1) select 8
ocSq3 = [ocRes, gmFactionSquad [gmResistanceSide, 1, 1], ocTPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units ocSq3)
doStop (units ocSq3)
triSimUntil { ((gmZone ocT1) select 4) > 20 }
triAssertEq [(format ["%1", (gmZoneSim ocT1) select 6]), "true"]
triSimUntil { ((gmZone ocT1) select 2) == gmResistanceSide }
triAssertGe [((gmZone ocT1) select 4), 60]

// ---- a5: an occupier soldier alone in a distant town does not intimidate ---
gmZoneSet [ocT2, "support", 50]
ocT2Pos = (gmZone ocT2) select 8
ocOccGrp = [ocOcc, gmFactionTierClass [gmOccupierSide, 1], 1, ocT2Pos] call GM_fnSpawnGroup
ocOccGrp setCombatMode "BLUE"
doStop (units ocOccGrp)
triSimUntil { ((gmZoneSim ocT2) select 2) >= 1 }
ocT = time
triSimUntil { time > ocT + 30 }
triAssertEq [((gmZone ocT2) select 4), 50]
triAssertEq [((gmZone ocT2) select 2), "NEUTRAL"]

triEndTest
