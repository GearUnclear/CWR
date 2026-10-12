// ============================================================================
//  Off-screen contract (issue #71) scenario (d), phase 3 of 3: a fresh process
//  loads the NEAR save.  The garrison comes back as the same live groups
//  (spawned, 4 alive, the same group count), not a second materialization,
//  and leaving writes back exactly 4.
// ============================================================================

triSimUntil { GM_LIB_READY }
dsOut = gmZoneIndex "Outpost"

triAssertEq [(triLoadGame "offnear"), "OK"]
triSimFrames 5
player allowDammage false
triAssertEq [(format ["%1", gmGarrisonSpawned dsOut]), "true"]
triAssertEq [(gmGarrisonLive dsOut), dsNearLive]
triAssertEq [count (gmGarrisonGroups dsOut), dsG0]
triSimFrames 60
triAssertEq [(gmGarrisonLive dsOut), 4]

player setPos dsFar
triSimUntil { not (gmGarrisonSpawned dsOut) }
triAssertEq [((gmZone dsOut) select 3), 4]

triEndTest
