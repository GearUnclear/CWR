// ============================================================================
//  Phase 2 of the guerrilla save/reload round-trip.
//    Fresh launch of the SAME mission against the SAME shared user dir. init.sqs
//    re-seeds the Phase-1 DEFAULTS first (proving we start from a clean slate);
//    then triLoadGame rehydrates GGameState from phase 1's save and we diff every
//    sentinel. Asserts run within a few frames of the load so no manager tick
//    (companion survival XP @5s, War-Level recompute @60s, economy @600s) can
//    perturb the restored values.
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- baseline: init.sqs seeds these BEFORE the load, so they must NOT already be
//    the sentinels (guards against a false pass from leftover state) -----------
triAssertEq [gmResources, 100]
triAssertEq [((GM_ZONES select 2) select GM_Z_OWNER), "EAST"]

// -- restore phase 1's binary save from the shared UserDir/Saved/Tmp/grr.fps ---
triAssertEq [(triLoadGame "grr"), "OK"]
triSimFrames 3

// -- DIFF: every persisted sentinel must have round-tripped --------------------
triAssertEq [gmResources, 777]
triAssertEq [gmManpower, 9]
triAssertEq [gmWarLevel, 5]
triAssertEq [((GM_ZONES select 2) select GM_Z_OWNER), "GUER"]
triAssertEq [(GM_COMP_XP select 0), 12345]
triAssert   [("AK47" in GM_GEAR_UNLOCKED)]

triEndTest
