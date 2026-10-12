// ============================================================================
//  Off-screen contract (issue #71) scenario (d), phase 2 of 3: a fresh process
//  loads the FAR save.  The Outpost comes back as a stored reserve of 4 (not
//  the authored 6, not 0), the distant standoff at SEBase is still frozen at
//  capture 40 with its two fighters, and walking back in materializes exactly
//  the 4 survivors.
// ============================================================================

triSimUntil { GM_LIB_READY }
dsOut = gmZoneIndex "Outpost"
dsSE = gmZoneIndex "SEBase"
// fresh-boot baseline: the authored garrison, no standoff
triAssertEq [((gmZone dsSE) select 9), 0]

triAssertEq [(triLoadGame "offfar"), "OK"]
triSimFrames 5
player allowDammage false
triAssertEq [((gmZone dsOut) select 3), 4]
triAssertEq [(format ["%1", gmGarrisonSpawned dsOut]), "false"]
triAssertEq [((gmZone dsOut) select 2), gmOccupierSide]
triAssertEq [((gmZone dsSE) select 9), 40]
triAssertEq [((gmZone dsSE) select 3), 4]
triAssertEq [((gmZone dsSE) select 2), gmOccupierSide]
triAssertEq [(count units dsSq), 2]
triAssertGe [dsFarHeat, ((gmZone dsOut) select 6)]
// the standoff is still a standoff on the next zone tick
triSimUntil { (gmZoneSim dsSE) select 5 }
triAssertEq [((gmZone dsSE) select 9), 40]

// ---- walk back in: exactly the survivors -----------------------------------
player setPos dsHome
triSimUntil { gmGarrisonSpawned dsOut }
triSimUntil { (gmGarrisonLive dsOut) >= 4 }
triSimFrames 60
triAssertEq [(gmGarrisonLive dsOut), 4]

triEndTest
