// ============================================================================
//  Phase 1 of the guerrilla save/reload round-trip.
//    Boot the full mode, stamp DISTINCTIVE sentinel values into the persisted
//    globals (faction scalars, a zone-ownership flip, companion XP, a gear
//    unlock), then triSaveGame to the shared user dir. Phase 2 reloads and diffs.
//
//  These are the exact "plain array / scalar" globals plan 13 says round-trip for
//  free via GGameState serialization - no object handles are saved here.
//  Sentinels are set and saved back-to-back (no sim advance between) so no
//  manager tick can overwrite them before the snapshot is taken.
// ============================================================================

triSimUntil { GM_LIB_READY }
triAssertEq [(count GM_ZONES), 3]

// -- stamp sentinels into the persisted schema --------------------------------
gmResources = 777
gmManpower  = 9
gmWarLevel  = 5
(GM_ZONES select 2) set [GM_Z_OWNER, "GUER"]
GM_COMP_XP set [0, 12345]
GM_GEAR_UNLOCKED = ["AK47"]

// -- write the binary save into the shared UserDir/Saved/Tmp/grr.fps ----------
triAssertEq [(triSaveGame "grr"), "OK"]

triEndTest
