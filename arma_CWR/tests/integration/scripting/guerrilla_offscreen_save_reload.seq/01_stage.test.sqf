// ============================================================================
//  Off-screen contract (issue #71) scenario (d), phase 1 of 3: cross the
//  simulation boundary, then save once NEAR (garrison materialized) and once
//  FAR (garrison stored as a reserve).  guerrilla_offscreen.abel, Remaster lane.
//  Invariant under test: crossing the bubble edge conserves bodies exactly -
//  a bled garrison is neither healed nor duplicated, in one process or across
//  a save/reload.  The expected values ride the save as script globals (ds*),
//  so phases 2 and 3 compare against what this phase saw.
//  The far trips are kept short (well under the mission's 15 s D2 interval)
//  so reserve recovery, the one allowed source of new bodies, cannot fire.
// ============================================================================

triSimUntil { GM_LIB_READY }
player allowDammage false
dsHome = getPos player
dsFar = [7922, 9590, 0]
dsOut = gmZoneIndex "Outpost"
dsSE = gmZoneIndex "SEBase"
triAssertGe [dsOut, 0]
triAssertGe [dsSE, 0]

// ---- bleed the materialized Outpost garrison 6 -> 4 -------------------------
triSimUntil { gmGarrisonSpawned dsOut }
triSimUntil { (gmGarrisonLive dsOut) >= 6 }
dsG0 = count (gmGarrisonGroups dsOut)
dsU = units ((gmGarrisonGroups dsOut) select 0)
(dsU select 1) setDammage 1
(dsU select 2) setDammage 1
triSimUntil { (gmGarrisonLive dsOut) == 4 }

// ---- out and back: written back as 4, rematerialized as 4 -------------------
player setPos dsFar
triSimUntil { not (gmGarrisonSpawned dsOut) }
triAssertEq [((gmZone dsOut) select 3), 4]
player setPos dsHome
triSimUntil { gmGarrisonSpawned dsOut }
triSimUntil { (gmGarrisonLive dsOut) >= 4 }
triSimFrames 60
triAssertEq [(gmGarrisonLive dsOut), 4]
triAssertEq [count (gmGarrisonGroups dsOut), dsG0]

// ---- a distant standoff that must survive both saves -------------------------
dsSEPos = (gmZone dsSE) select 8
dsSq = [gmResistanceSide call GM_fnSideFromString, gmFactionSquad [gmResistanceSide, 1, 2], dsSEPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units dsSq)
doStop (units dsSq)
gmZoneSet [dsSE, "capture", 40]
triSimUntil { (gmZoneSim dsSE) select 5 }

// ---- save NEAR: the garrison is live groups, not a reserve ------------------
dsNearLive = gmGarrisonLive dsOut
triAssertEq [dsNearLive, 4]
triAssertEq [(triSaveGame "offnear"), "OK"]

// ---- save FAR: the garrison is a reserve of 4 -------------------------------
player setPos dsFar
triSimUntil { not (gmGarrisonSpawned dsOut) }
triAssertEq [((gmZone dsOut) select 3), 4]
dsFarHeat = (gmZone dsOut) select 6
triAssertEq [(triSaveGame "offfar"), "OK"]

triEndTest
