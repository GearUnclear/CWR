// ============================================================================
//  Phase 1 of the NATIVE save/reload round-trip (guerrilla_native.abel).
//    Boot the full mode, stamp DISTINCTIVE sentinels into BOTH persistence
//    layers - script globals (GGameState) and native zone state (ZoneRegistry,
//    written through the gmZoneSet command surface) - then triSaveGame to the
//    shared user dir. Phase 2 fresh-boots, reloads and diffs everything,
//    including the natively-serialized campaignLoaded event handler.
//
//  The zone-owner sentinel goes through gmZoneSet ["owner"], the exact write
//  the mission scripts use; the native serializer must carry it by NAME across
//  the config rebuild a load performs.
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
gmZoneSet [gsOut, "owner", gmResistanceSide]

// -- write the binary save into the shared UserDir/Saved/Tmp/gnat.fps ---------
triAssertEq [(triSaveGame "gnat"), "OK"]

triEndTest
