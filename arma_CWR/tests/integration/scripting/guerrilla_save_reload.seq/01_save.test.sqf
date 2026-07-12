// ============================================================================
//  Phase 1 of the guerrilla save/reload round-trip (NATIVE engine surface).
//    Demo-data twin of guerrilla_native_save_reload.seq (full-CWA/Abel); the
//    retired persistence.sqs is gone - zones/alert/garrison state and the
//    native event handlers serialize in the engine, script globals ride
//    GGameState. Boot the full mode, stamp DISTINCTIVE sentinels into BOTH
//    persistence layers - script globals AND native zone state (written
//    through the gmZoneSet command surface) - then triSaveGame to the shared
//    user dir. Phase 2 fresh-boots, reloads and diffs everything.
// ============================================================================

triSimUntil { GM_LIB_READY }
gsOut = gmZoneIndex "Outpost"
gsVil = gmZoneIndex "Village"
triAssertGe [gsOut, 0]
triAssertGe [gsVil, 0]
triAssertEq [((gmZone gsOut) select 2), gmOccupierSide]

// -- the companion (Petra) must have a live body before the snapshot: phase 2
//    asserts the post-load reconciliation rebuilds/keeps her alive ------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(format ["%1", alive (GM_COMP_OBJ select 0)]), "true"]

// -- stamp sentinels: script-global layer --------------------------------------
gmResources = 777
gmManpower = 9
gmWarLevel = 5
GM_COMP_XP set [0, 12345]
GM_GEAR_UNLOCKED = ["AK47"]

// -- stamp sentinels: native zone-registry layer -------------------------------
gmZoneSet [gsVil, "support", 55]
gmZoneSet [gsOut, "income", 99]
// owner FIRST: an owner write resets the capture meter (ownership
// discontinuity), so the capture sentinel must land after it
gmZoneSet [gsOut, "owner", gmResistanceSide]
gmZoneSet [gsOut, "capture", 50]

// -- write the binary save into the shared UserDir/Saved/Tmp/grr.fps ----------
triAssertEq [(triSaveGame "grr"), "OK"]

triEndTest
