// ============================================================================
//  Off-screen contract (issue #71) scenario (b): wait in an untouched region.
//  guerrilla-mode/OFFSCREEN-CONTRACT.md, decision D2 + the abstract ticks.
//  Full Remaster lane, mission guerrilla_offscreen.abel (D2 at 15 s).
//  The player parks on Malden's airfield, 3.3 km from the Outpost and
//  5.9 km from SEBase, and waits ~90 s of game time.
//  Must persist: every zone's owner, support and capture; the war level; no
//    garrison materializes anywhere; a reserve already at strength (Outpost,
//    6 of 6) never grows.
//  Must change: SEBase, bled to 1 of 4, regains one body per 15 s and stops
//    at 4 (D2); the Outpost's raised heat cools (1 per 10 s on GREEN); the
//    economy pays (GM_ECON_TICK shortened to 20 s).
// ============================================================================

triSimUntil { GM_LIB_READY }
player allowDammage false
ibOut = gmZoneIndex "Outpost"
ibSE = gmZoneIndex "SEBase"
triAssertGe [ibOut, 0]
triAssertGe [ibSE, 0]

player setPos [7922, 9590, 0]
// the Outpost garrison (spawned at boot, 600 m from the Camp) is written
// back as the player leaves; nothing anywhere stays materialized
triSimUntil { not (gmGarrisonSpawned ibOut) }
triAssertEq [((gmZone ibOut) select 3), 6]

gmZoneSet [ibSE, "garrison", 1]
gmZoneSet [ibOut, "heatRaise", 30]
GM_ECON_TICK = 20
ibHeat0 = (gmZone ibOut) select 6
ibRes0 = gmResources
ibWar0 = gmWarLevel
// snapshot owner / support / capture of every zone
ibSnap = ""
ibI = 0
while {ibI < gmZoneCount} do {ibZ = gmZone ibI; ibSnap = ibSnap + format ["%1:%2:%3:%4;", ibZ select 0, ibZ select 2, ibZ select 4, ibZ select 9]; ibI = ibI + 1}

// D2: one body per 15 s, 1 -> 2 -> 3 -> 4
triSimUntil { ((gmZone ibSE) select 3) >= 2 }
triAssertEq [((gmZone ibSE) select 3), 2]
triSimUntil { ((gmZone ibSE) select 3) >= 4 }
ibT = time
triSimUntil { time > ibT + 35 }
// capped at the authored strength; the full Outpost never grew
triAssertEq [((gmZone ibSE) select 3), 4]
triAssertEq [((gmZoneSim ibSE) select 7), 4]
triAssertEq [((gmZone ibOut) select 3), 6]

// heat cooled on the GREEN Outpost
triAssertGe [ibHeat0 - ((gmZone ibOut) select 6), 5]
// the economy ticked (seeded towns pay support x 0.5 regardless of owner)
triAssertGt [gmResources, ibRes0]

// everything else held still
ibNow = ""
ibI = 0
while {ibI < gmZoneCount} do {ibZ = gmZone ibI; ibNow = ibNow + format ["%1:%2:%3:%4;", ibZ select 0, ibZ select 2, ibZ select 4, ibZ select 9]; ibI = ibI + 1}
triAssertEq [ibNow, ibSnap]
triAssertEq [gmWarLevel, ibWar0]
ibSpawned = 0
ibI = 0
while {ibI < gmZoneCount} do {if (gmGarrisonSpawned ibI) then {ibSpawned = ibSpawned + 1}; ibI = ibI + 1}
triAssertEq [ibSpawned, 0]
triAssertEq [((gmSimDiag) select 4), 0]

triEndTest
