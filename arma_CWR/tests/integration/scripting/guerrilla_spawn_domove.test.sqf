// ============================================================================
//  Guerrilla Mode - garrison spawn-and-command on the NATIVE engine surface.
//    Demo-data twin of guerrilla_native_spawn.test.sqf (full-CWA/Abel); the
//    retired spawning.sqs distance-cache is GarrisonCache.cpp now, and doMove
//    is a registered engine command (the Gate-Zero group-move workaround this
//    test used to carry is history). Proves:
//      1. the native cache AUTO-SPAWNS the Outpost garrison because the
//         player boots at the Camp, ~290 m away - inside the 800 m
//         cacheRadius (no script pokes anything);
//      2. the spawned strength matches the config reserve (garrison = 8,
//         single group under groupSize = 12);
//      3. the spawned groups are real commandable AI: doMove displaces a unit.
//
//  Runs against the REAL mode: guerrilla_capture.Demo's two-line init.sqs
//  execs the ONE shared core at <GameDir>\gmcore (guerrilla-mode/core,
//  pinned by test_mission_script_core.cpp).
//
//  Assertions are framerate-agnostic: unit COUNTS prove the spawn; a
//  time-bounded displacement (not a fixed frame count) proves motion.
// ============================================================================

// -- mode fully booted (init.sqs published the helper-table sentinel) ---------
triSimUntil { GM_LIB_READY }

// -- zone table sanity: explicit zones resolve by name -------------------------
gnOut = gmZoneIndex "Outpost"
triAssertGe [gnOut, 0]
triAssertGe [gmZoneCount, 3]
triAssertEq [((gmZone gnOut) select 2), gmOccupierSide]

// -- 1) NATIVE AUTO-SPAWN: no script pokes the cache; being near is enough ---
triSimUntil { gmGarrisonSpawned gnOut }

// -- 2) full configured strength, one group ----------------------------------
triSimUntil { (gmGarrisonLive gnOut) >= 8 }
triAssertEq [(gmGarrisonLive gnOut), 8]
gnGrps = gmGarrisonGroups gnOut
triAssertEq [(count gnGrps), 1]

// -- 3) COMMANDABLE: doMove the group leader ~110 m and require displacement.
//    Time-bounded (polled), not frame-bounded, so it is framerate-agnostic. --
gnLdr = leader (gnGrps select 0)
// (bool asserts go through format: triAssert*'s stringifier cannot take a
//  raw GameBool - engine issue, see the parity-test report)
triAssertEq [(format ["%1", alive gnLdr]), "true"]
gnP0 = getPos gnLdr
gnDest = [(gnP0 select 0) + 80, (gnP0 select 1) + 80, 0]
gnLdr doMove gnDest
triSimUntil { ((getPos gnLdr) distance gnP0) > 15 }

// -- nobody died or despawned while moving (no crash, cache stayed put) ------
triAssertGe [(gmGarrisonLive gnOut), 8]

triEndTest
